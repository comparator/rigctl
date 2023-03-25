CC = gcc
OBJ = main.o config.o network.o ermak.o rigctl.o commcat.o
BINNAME = pollsrv

all: $(OBJ)
	$(CC) -o $(BINNAME) $(OBJ)
	rm -f *.o

%.o: %.c
	$(CC) -c $<

clean:
	rm -f *.o $(BINNAME)
