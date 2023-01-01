
CC=gcc

LDFLAGS=-g -L../utils/libs -L./ -L../utils/libs -L/usr/local/lib -lini -lmiscutils -lstrutils -llogutils -lz -lpthread -lm
CFLAGS=-std=gnu99

CFLAGS += -g -Wall -O2 -I./ -I../utils/incs -I/usr/include/postgresql

all: dtsengine dtsclient

dtsengine: dtsengine.o dtsparse.o easyudp.o trietree.o
	$(CC) dtsengine.o dtsparse.o easyudp.o trietree.o -o dtsengine $(LDFLAGS)

dtsclient: dtsclient.o easyudp.o
	$(CC) dtsclient.o easyudp.o -o dtsclient $(LDFLAGS)

dtsengine.o: dtsengine.c dts.h
	$(CC) $(CFLAGS) -c $< -o $@

dtsclient.o: dtsclient.c dts.h
	$(CC) $(CFLAGS) -c $< -o $@

easyudp.o: easyudp.c easyudp.h
	$(CC) $(CFLAGS) -c $< -o $@

dtsparse.o: dtsparse.c dts.h
	$(CC) $(CFLAGS) -c $< -o $@

trietree.o: trietree.c trietree.h dts.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f dtsengine dtsclient dtsengine.o dtsclient.o dtsparse.o easyudp.o trietree.o
