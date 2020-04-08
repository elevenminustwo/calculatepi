
CC=mpicc
LB=lamboot
LW=lamwipe 

ifeq (, $(shell which $(CC)))
$(error "Can not able to link 'mpicc', consider doing 'sudo apt-get install lam4-dev'")
endif

out:
	${CC} $< pi.c -o $@
start:
	${LB}
	./out
stop:
	${LW}
clean:
	rm out
