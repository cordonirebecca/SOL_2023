# Makefile
CC	=  gcc 
CFLAGS	= -lm -Wall -pedantic -g 

.PHONY: clean all

objects = masterWorker.o
objectsCollector= auxiliaryMW.o workers.o collector.o list.o

masterWorker: masterWorker.o auxiliaryMW.o workers.o list.o
	$(CC) -pthread -o $@ $^
	
collector: $(objectsCollector)
	$(CC) -o $@ $^

collector.o: collector.c collector.h
	$(CC) $(CFLAGS) -c $<

masterWorker.o: masterWorker.c auxiliaryMW.h workers.h list.h
	$(CC) $(CFLAGS) -c $< 
	
list.o: list.c list.h
	$(CC) $(CFLAGS) -c $<
	
auxiliaryMW.o: auxiliaryMW.c auxiliaryMW.h list.h
	$(CC) $(CFLAGS) -c $< 
	
workers.o: workers.c workers.h
	$(CC) $(CFLAGS) -c $< 
	
all:
	make masterWorker
	make collector
	
clean:
	-rm *.o
	-rm masterWorker
	-rm collector
	 
test:	
	make all
	./test.sh
	
