NAME = zeroredisq

INCLUDES = -I../deps/jansson/src -I../deps/zeromq2-1/include -I../deps/jemalloc-2.2.5/include/
INCLUDES += -I /usr/local/include
CFLAGS = -Wall -g -O2
LDFLAGS = -L /usr/local/lib
LIBS = -lpthread ../deps/jansson/src/libjansson.la ../deps/zeromq2-1/src/libzmq.la
STATIC_LIBS = -lpthread ../deps/jansson/src/.libs/libjansson.a ../deps/zeromq2-1/src/.libs/libzmq.a -ldb
LIBTOOL = ../deps/zeromq2-1/libtool

CC = gcc
CPP = g++
RM = rm -f

OBJECTS = \
	zrq_zmq.o \
	zrq_main.o

.SUFFIXES: .c
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

all:	$(OBJECTS)
	$(CPP) -o $(NAME) $(OBJECTS) $(LDFLAGS) $(STATIC_LIBS)
#	$(LIBTOOL) $(CC) -o $(NAME) $(OBJECTS) $(LDFLAGS) $(LIBS)

clean:
	$(RM) $(NAME) $(OBJECTS)
