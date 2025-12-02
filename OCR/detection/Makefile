CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lm

TARGET = test_decoupe
SRCS = main_test.c decoupe.c decoupe_lettre.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run debug help

all: $(TARGET)
	@echo "✓ Compilation terminée: ./$(TARGET)"

$(TARGET): $(OBJS)
	@echo "Édition des liens..."
	@$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c decoupe.h
	@echo "Compilation de $<..."
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f $(TARGET) $(OBJS)
	@echo "✓ Nettoyage terminé"

run: all
	@echo ""
	@./$(TARGET) $(ARGS)

debug: CFLAGS += -g -O0 -DDEBUG
debug: clean all
	@echo "✓ Version debug compilée"

help:
	@echo "╔════════════════════════════════════════╗"
	@echo "║       Projet OCR - Makefile            ║"
	@echo "╠════════════════════════════════════════╣"
	@echo "║  make          Compile le projet       ║"
	@echo "║  make clean    Supprime les binaires   ║"
	@echo "║  make debug    Compile avec debug      ║"
	@echo "║  make run ARGS='<img> <nom>'  Exécute  ║"
	@echo "╚════════════════════════════════════════╝"
	@echo ""
	@echo "Exemple:"
	@echo "  make run ARGS='../input/image.bmp test_01'"
