CC = clang
CFLAGS = -Wall -Werror -Wextra -pedantic

all: queue.o rwlock.o

format: queue.c queue.h rwlock.c rwlock.h
	$ clang-format -i -style=file queue.c queue.h rwlock.c rwlock.h

test: test.o queue.o test_rwlock.o
	$(CC) $(LFLAGS) $^ -o $@ 

queue.o: queue.c 
	$(CC) $(CFLAGS) -c queue.c 

rwlock.o: rwlock.c
	$(CC) $(CFLAGS) -c rwlock.c

test_rwlock.o: test_rwlock.c
	$(CC) $(CFLAGS) -c test_rwlock.c

clean:
	rm -f queue.o rwlock.o



