CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = cooo
SRCS = main.c cooo.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET) output_with_coords.pbm
