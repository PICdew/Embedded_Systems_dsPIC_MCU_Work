
EXEC = lab4
OBJS = $(EXEC).o linuxanalog.o

CC = gcc
CFLAGS = -I.. -g -Wall -lrt

all : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean : 
	rm -f *.o $(EXEC)
