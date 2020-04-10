
CC=mpicc

ifeq (, $(shell which $(CC)))
$(error "Can not able to link 'mpicc', consider doing 'sudo apt-get install openmpi-bin libopenmpi-dev'")
endif

out:
	${CC} $< pi.c -o $@
run:
	./out
clean:
	rm out
