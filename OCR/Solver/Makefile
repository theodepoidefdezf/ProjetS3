CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
OBJ = solver.o main.o
TARGET = solver

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

solver.o: solver.c solver.h
	$(CC) $(CFLAGS) -c solver.c

main.o: main.c solver.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f $(OBJ) $(TARGET) coordonnees

rebuild: clean all
