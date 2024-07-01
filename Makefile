CC=clang
CFLAGS=-Wall

all: editor

editor: main.o term.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $^

clean:
	rm editor *.o
