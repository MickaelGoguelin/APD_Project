CC=mpicc
CFLAGS= -Wall -g
EXEC=dsfm
HOST=hostfile #hostf_univ

all: run mrproper

run: $(EXEC)
	mpiexec --hostfile ./$(HOST) ./$(EXEC) #--xterm -1\!

$(EXEC):
	$(CC) -o $(EXEC) $(EXEC).c $(EXEC)_fct.c $(CFLAGS)

mrproper:
	rm -r $(EXEC)
