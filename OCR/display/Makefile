CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0` -lm

SRC = main.c display.c pipeline_run.c
OBJ = $(SRC:.c=.o)
TARGET = display
COOO = cooo

.PHONY: all clean run

all: $(TARGET) $(COOO)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(COOO): cooo.c
	$(CC) $(CFLAGS) cooo.c -o $(COOO) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(COOO)

run: $(TARGET)
	./$(TARGET)
