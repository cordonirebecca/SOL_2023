# makefile
CC	=  gcc 
CFLAGS	= -lm -Wall -pedantic -g 

masterWorker: masterWorker.o 
	$(CC) -pthread -o $@ $^

all:
	make masterWorker

#test1:	
#	make all
#	./test1.sh
	
