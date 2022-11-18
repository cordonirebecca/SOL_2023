# Makefile
CC	=  gcc 
CFLAGS	= -lm -Wall -pedantic -g 

.PHONY: clean all

objects = masterWorker.o

masterWorker: masterWorker.o
	$(CC) -pthread -o $@ $^

masterWorker.o: masterWorker.c
	$(CC) $(CFLAGS) -c $< 
	
list.o: list.c list.h
	$(CC) $(CFLAGS) -c $<
	
all:
	make masterWorker

clean:
	-rm *.o
	-rm masterWorker
	 
test:	
	make all
	./test.sh
	
