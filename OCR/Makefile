CC = gcc
CFLAGS = -Wall -Wextra \
	-I./Preprocessing \
	-I./detection \
	-I./ocr \
	-I./Solver \
	-I./display \
	-I./UTILS

SRC = $(wildcard Preprocessing/*.c detection/*.c ocr/*.c Solver/*.c display/*.c utils/*.c) main.c
OBJ = $(SRC:.c=.o)
EXEC = ocr_solver

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(EXEC)

re: fclean all
