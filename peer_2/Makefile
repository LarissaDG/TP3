all:
	gcc -Wall -c lista.c -lm
	gcc -Wall -c funcoes.c -lm
	gcc -Wall server.c  funcoes.o lista.o -lm -lpthread -o peer

clean:
	rm *.o
