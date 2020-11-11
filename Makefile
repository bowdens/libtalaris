CC=gcc
LDFLAGS=-lreadline
CFLAGS :=
SYSH :=

ifeq ($(OS),Windows_NT)
	CFLAGS	:= $(CFLAGS) -I./windeps -DWINNT=1
endif

OUTPUT=example
CFILE=example.c

default: $(OUTPUT)

wordsplit.o: wordsplit.c

libtalaris.o: libtalaris.c

libtalaris.a: libtalaris.o wordsplit.o
	ar cr $@ $^

$(OUTPUT): $(CFILE) libtalaris.a
	gcc $(CFLAGS) $^  -o $(OUTPUT) $(LDFLAGS)

clean:
	rm *.o *.a *.exe
