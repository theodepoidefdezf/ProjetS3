CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native
TARGET = main
SRC = main.c

# Cible par défaut
all: $(TARGET)

# Compilation
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Mode debug (avec symboles de débogage, sans optimisation)
debug: CFLAGS = -Wall -Wextra -g -O0
debug: $(TARGET)

# Nettoyage standard (ne supprime QUE l'exécutable, garde le modèle)
clean:
	rm -f $(TARGET)

# Nettoyage complet (supprime l'exécutable ET le modèle entraîné)
clean-all: clean
	rm -f model.txt

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
	@echo "  make clean  - Supprimer l'exécutable (garde le modèle)"
	@echo "  make clean-all - Tout supprimer (exécutable + modèle)"

.PHONY: all clean clean-all train test debug help
