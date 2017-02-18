CC     = gcc
CLEAN  = rm -f
CFLAGS = -Wall -g `pkg-config glib-2.0 json-glib-1.0 --cflags`
LDADD  = `pkg-config glib-2.0 json-glib-1.0 --libs` -lm

all: lew_main

lew_main: lew_main.o
	$(CC) -o lew_main lew_main.o $(LDADD)

lew_main.o: lew_main.c
	$(CC) $(CFLAGS) -c -o lew_main.o lew_main.c

clean:
	$(CLEAN) lew_main
	$(CLEAN) lew_main.o

