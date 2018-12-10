CC=mpicc
CFLAGS= -Wall -g
EXEC=dsfm

all: run mrproper

run: $(EXEC)
	mpiexec --hostfile ./hostfile ./$(EXEC) #--xterm -1\!

$(EXEC):
	$(CC) -o $(EXEC) $(EXEC).c $(CFLAGS)

mrproper:
	rm -r $(EXEC)
