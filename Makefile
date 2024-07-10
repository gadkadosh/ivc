CC=clang
CFLAGS=-Wall -g

all: editor

editor: main.o term.o keymaps.o modes.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $^

clean:
	rm editor *.o *.h.gch
