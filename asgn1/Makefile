CC       = clang
FORMAT   = clang-format
CFLAGS   = -Wall -Wpedantic -Werror -Wextra

all : memory

memory : memory.c
	$(CC) $(CFLAGS) -o memory memory.c

clean :
	rm -f memory memory.o