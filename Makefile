CC = gcc
OBJ = main.o rigctl.o ermak.o
BINNAME = rigsrv

all: $(OBJ)
	$(CC) -o $(BINNAME) $(OBJ)
	rm -f *.o

%.o: %.c
	$(CC) -c $<

clean:
	rm -f *.o $(BINNAME)
