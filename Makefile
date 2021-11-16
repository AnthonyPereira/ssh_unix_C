CC=gcc
CFLAGS=-O3 -Wall -Wextra -g#-Werror

all: mysh myssh mysshd myssh-server myls myps decoupeCommande.o mypipe.o variableLocal.o

mysh: mysh.c utils.h decoupeCommande.o mypipe.o variableLocal.o
	$(CC) $(CFLAGS) mysh.c decoupeCommande.o mypipe.o variableLocal.o -o mysh

myssh: myssh.c utils.h
	$(CC) $(CFLAGS) myssh.c -o myssh

mysshd: mysshd.c utils.h
	$(CC) $(CFLAGS) mysshd.c -o mysshd -lcrypt

myssh-server: myssh-server.c utils.h
	$(CC) $(CFLAGS) myssh-server.c -o myssh-server

myls: myls.c utils.h
	$(CC) $(CFLAGS) myls.c -o myls

myps: myps.c utils.h
	$(CC) $(CFLAGS) myps.c -o myps

decoupeCommande.o: decoupeCommande.c decoupeCommande.h utils.h
	$(CC) $(CFLAGS) -c decoupeCommande.c

mypipe.o: mypipe.c mypipe.h utils.h decoupeCommande.o
	$(CC) $(CFLAGS) -c mypipe.c

variableLocal.o: variableLocal.c variableLocal.h utils.h decoupeCommande.o
	$(CC) $(CFLAGS) -c variableLocal.c

clean:
	@rm mysh myssh mysshd myssh-server myls myps decoupeCommande.o mypipe.o variableLocal.o 2> /dev/null || true
	@echo "Cleaning done"
