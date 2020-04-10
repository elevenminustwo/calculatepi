#include "mpi.h" 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define M_PI 3.14159265358979323846

typedef struct 
 {
   double     sum; 
   int     range;
   int     remaining; 
   int   numthrds;
 } DATA;

/* Define globally accessible variables and a mutex */

DATA dotstr; 
pthread_mutex_t mutexsum;


void *picall(void *arg)
{

   /* Define and use local variables for convenience */

   int i, start, end, len, numthrds, myid,sign;
   long mythrd;
   double mysum, *x, *y;

   mythrd = (long)arg;
   MPI_Comm_rank (MPI_COMM_WORLD, &myid);

   numthrds = dotstr.numthrds;
   len = dotstr.range;
   if(mythrd!=0){
   start = myid*numthrds*len + (mythrd-1)*len+dotstr.remaining+1;
   end   = start + len;   
   }else{
   if(dotstr.remaining!=0){
   start = 1;
   end   = start+dotstr.remaining;  
   }
   //printf("start = %d\n",start);
   //printf("end = %d\n",end);
   }
   mysum = 0;
   printf("start = %d\n",start);
   printf("end = %d\n",end);
   for (i=start; i<end ; i++) 
    {
      //printf("i value= %d\n",i);
      if(i%2==0){
	sign = -1;
      }else{
	sign = 1;   
      }
      mysum += (sign)*(1.0 / (2 * i - 1)); 
      }
   /*
   Lock a mutex prior to updating the value in the structure, and unlock it 
   upon updating.
   */
   pthread_mutex_lock (&mutexsum);
   printf("Task %d thread %ld adding partial sum of %f to node sum of %f\n",
           myid, mythrd, mysum, dotstr.sum);
   dotstr.sum += mysum;
   pthread_mutex_unlock (&mutexsum);

   pthread_exit((void*)0);
}

/* 
As before,the main program does very little computation. It creates
threads on each node and the main thread does all the MPI calls. 
*/

int main(int argc, char* argv[])
{
int len, myid, numprocs,MAXTHRDS,numops; 
long i;
int nump1, numthrds;
double begin=0.0, end=0.0;
double nodesum, allsum;
void *status;
pthread_attr_t attr;


printf ("Total number of operations? ");
scanf("%d", &numops); 

printf ("Number of threads? ");
scanf("%d", &MAXTHRDS); 
/* MPI Initialization */

if(MAXTHRDS!=1){
len = numops/(MAXTHRDS-1);
}else{
len = numops/MAXTHRDS;
}

pthread_t callThd[MAXTHRDS];

MPI_Init (&argc, &argv);
MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
MPI_Comm_rank (MPI_COMM_WORLD, &myid);

//Synchronize all processes and get the begin time
MPI_Barrier(MPI_COMM_WORLD);
begin = MPI_Wtime();

/*initialize values */
numthrds=MAXTHRDS;
dotstr.range = len; 
dotstr.sum=0;
dotstr.numthrds=MAXTHRDS;
if(MAXTHRDS!=1){
dotstr.remaining = numops-(len*(MAXTHRDS-1));
}else{
dotstr.remaining = numops;
}
  
/* 
Create thread attribute to specify that the main thread needs
to join with the threads it creates.
*/
pthread_attr_init(&attr );
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

/* Create a mutex */
pthread_mutex_init (&mutexsum, NULL);

/* Create threads */
for(i=0;i<numthrds;i++) {
  pthread_create( &callThd[i], &attr, picall, (void *)i); 
  }

/* Release the thread attribute handle as it is no longer needed */
pthread_attr_destroy(&attr );

/* Wait on the other threads within this node */
for(i=0;i<numthrds;i++) {
  pthread_join( callThd[i], &status);
  }

nodesum = dotstr.sum;
printf("Task %d node sum is %f\n",myid, nodesum);

/* After the pi calculation, master node(main thread) performs a summation of results on each node */
MPI_Reduce (&nodesum, &allsum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

//Synchronize all processes and get the end time
MPI_Barrier(MPI_COMM_WORLD);
end = MPI_Wtime();

/* Finishes own part and adds to summation */
printf("Remaining part for master node = %d\n",dotstr.remaining);

if (myid == 0)  
printf ("Done. PI=  %f\n", allsum*4);
printf ("Approximation error: %f \n", allsum*4-M_PI);
printf("Time=%fs\n", end-begin);
MPI_Finalize();
pthread_mutex_destroy(&mutexsum);
exit (0);
}   

