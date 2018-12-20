EXEC = testListeC

all : mrproper run mrproper

run : $(EXEC)
	./$(EXEC)
	
$(EXEC) :
	gcc -o $(EXEC) linkedList.c $(EXEC).c -std=c99

mrproper :
	rm -f $(EXEC)
