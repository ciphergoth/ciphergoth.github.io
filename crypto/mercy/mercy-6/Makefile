# -*- mode: makefile -*-

# Mercy-6 - a 4096-bit block cipher for disk block encryption
# 
# Paul Crowley, 1996 - 2000
# This software is placed in the public domain.
#
# See http://www.cluefactory.org.uk/paul/mercy/
# or mail mercy@paul.cluefactory.org.uk

CC=gcc
DEFINES=-DCLOCK_SPEED=451042961
CFLAGS = $(DEFINES) -Wall -O6 -march=pentium \
	-fexpensive-optimizations -fomit-frame-pointer \
	-funroll-loops -fstrict-aliasing
#CFLAGS=$(DEFINES) -Wall -g
OBJS = unix-time.o fast-mercy.o


#all:	unix-time
all:	run-unix-time

run-unix-time:	unix-time
	./unix-time 10

unix-time: $(OBJS)
	$(CC) $(LDFLAGS) -o unix-time $(OBJS) $(LOADLIBES)

fast-mercy.o:	mercy.h
unix-time.o:	mercy.h

clean:
	rm unix-time $(OBJS)
