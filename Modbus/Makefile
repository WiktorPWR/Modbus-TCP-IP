# Kompilator i flagi
CC = gcc
CFLAGS = -Wall -Iinclude -Iadu/include

# Pliki źródłowe
SRC = main.c \
      adu/src/adu.c adu/src/adu_utils.c 

# Pliki obiektowe
OBJ = $(SRC:.c=.o)

# Nazwa programu
TARGET = program

# Domyślna reguła
all: $(TARGET)

# Łączenie
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Czyszczenie
clean:
	rm -f $(OBJ) $(TARGET)
