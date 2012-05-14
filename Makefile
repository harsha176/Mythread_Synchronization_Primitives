CC=gcc
CFLAGS = -c -Wall -Werror
CUDA_CC = /usr/local/cuda/bin/nvcc
TEST_FILE1 = mytest.c
TEST_FILE2 = mytest2.c

all:mymutex.o mycond.o $(TEST_FILE1) $(TEST_FILE2)
	$(CC) -g $(TEST_FILE1) mymutex.o mythread.a mycond.o -o mytest
	$(CC) -g $(TEST_FILE2) mymutex.o mythread.a mycond.o -o mytest2

mymutex.o:mymutex.c mymutex.h
	$(CC) $(CFLAGS) mymutex.c 

mycond.o:mycond.c mycond.h
	$(CC) $(CFLAGS) mycond.c 
clean:
	rm -rf *.o mytest mytest2 *~
