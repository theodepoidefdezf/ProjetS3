# Makefile pour le CNN de reconnaissance de lettres

CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native
TARGET = cnn
SRC = main.c

# Cible par défaut
all: $(TARGET)

# Compilation
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Mode debug (avec symboles de débogage, sans optimisation)
debug: CFLAGS = -Wall -Wextra -g -O0
debug: $(TARGET)

# Nettoyage
clean:
	rm -f $(TARGET) model.txt

# Entraînement
train: $(TARGET)
	./$(TARGET) 1

# Test interactif
test: $(TARGET)
	./$(TARGET) 2

# Aide
help:
	@echo "Cibles disponibles:"
	@echo "  make        - Compiler le programme"
	@echo "  make debug  - Compiler en mode debug"
	@echo "  make train  - Entraîner le modèle"
	@echo "  make test   - Tester une image"
	@echo "  make clean  - Nettoyer les fichiers générés"

.PHONY: all clean train test debug help
