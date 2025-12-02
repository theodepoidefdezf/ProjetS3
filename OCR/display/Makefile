CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0`
LIBS = `pkg-config --libs gtk+-3.0 gdk-pixbuf-2.0` -lm

all: display

display: main.o display.o
	$(CC) $(CFLAGS) -o display main.o display.o $(LIBS)

main.o: main.c display.h
	$(CC) $(CFLAGS) -c main.c

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c

clean:
	rm -f *.o display
