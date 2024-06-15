FLAGS = -Wall -Wextra -std=gnu11 -pedantic

all: tree

debug: CFLAGS += -g
debug: clean all

tree: tree.o
	gcc -o tree tree.o

%.o: %.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o
