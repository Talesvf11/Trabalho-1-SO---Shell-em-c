CC = gcc
CFLAGS = 

# Alvo padrão
all: programa

# Regra para compilar o programa
programa: main.o terminal.o
	$(CC) $(CFLAGS) -o programa main.o terminal.o
	rm -f *.o

# Regra para compilar o arquivo main.c
main.o: main.c terminal.h
	$(CC) $(CFLAGS) -c main.c

# Regra para compilar o arquivo terminal.c
terminal.o: terminal.c terminal.h
	$(CC) $(CFLAGS) -c terminal.c

# Regra para limpar os arquivos objeto
clean:
	rm -f *.o

# Regra para limpar os arquivos objeto e o executável
clean_all:
	rm -f programa *.o