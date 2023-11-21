SOURCES  = $(wildcard *.c)
OBJECTS  = trie.o word.o io.o

CC       = clang
CFLAGS   = -Wall -Wpedantic -Werror -Wextra -gdwarf-4

.PHONY: all clean format 

all: encode decode

encode: $(OBJECTS) encode.o
	$(CC) -o $@ $^ $(LIBFLAGS)

decode: $(OBJECTS) decode.o
	$(CC) -o $@ $^ $(LIBFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) encode decode $(SOURCES:%.c=%.o)

format:
	clang-format -i -style=file *.[ch]
