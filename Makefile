CC=gcc
LDFLAGS=-lreadline -g

OUTPUT=test
CFILE=test.c

default: $(OUTPUT)

wordsplit.o: wordsplit.c

libtalaris.o: libtalaris.c

libtalaris.a: libtalaris.o wordsplit.o
	ar cr $@ $^

$(OUTPUT): $(CFILE) libtalaris.a
	gcc $(CFLAGS) $^  -o $(OUTPUT) $(LDFLAGS)

clean:
	trash *.o *.a
