#Integrantes:
#Bruno Lopes Altoé
#Vitor Facco Calmon
#Tales Viana Ferracioli

# Compilador e flags do compilador
CC = gcc
CFLAGS = 

# Alvo padrão
all: programa

# Regra para compilar o programa
programa: main.o terminal.o
	$(CC) $(CFLAGS) -o programa main.o terminal.o

# Regra para compilar o arquivo main.c
main.o: main.c terminal.h
	$(CC) $(CFLAGS) -c main.c

# Regra para compilar o arquivo terminal.c
terminal.o: terminal.c terminal.h
	$(CC) $(CFLAGS) -c terminal.c

# Regra para limpar os arquivos objeto e o executável
clean:
	@rm -f programa *.o

# Regra para limpar os arquivos objeto
cleanob:
	@rm -f *.o

# Regra para mostrar os processos
ps:
	ps  xao pid,ppid,pgid,sid,comm,stat

# Digitar @comando não mostra o comando no terminal.