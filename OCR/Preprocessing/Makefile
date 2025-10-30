CC = gcc

TARGET = preprocessing_test

UTILS_DIR = ../UTILS

SRCS = \
	main.c \
	color_modif.c \
	binarisation.c \
	rotation.c \
	cleaner.c \
	preprocessing.c \
	$(UTILS_DIR)/image.c \

OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -Wextra -O2 $(shell pkg-config --cflags sdl2 SDL2_image) -D_THREAD_SAFE
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image) -lm


all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Édition de liens..."
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Compilation terminée !"

%.o: %.c
	@echo "Compilation de $< ..."
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	@echo "Exécution du programme..."
	@./$(TARGET)

clean:
	@echo " Suppression des fichiers objets..."
	rm -f $(OBJS)

fclean: clean
	@echo "Suppression de l'exécutable..."
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re run
