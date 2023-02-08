CC = gcc
ARGS = -Wall -O2 -I .

all: vodserver 

clean:
	rm -f *.o vodserver *~
