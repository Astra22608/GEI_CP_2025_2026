CC = gcc
CFLAGS = -Wall -Wextra -pthread -g

# Ejecutables
TARGETS = p1_4 p1_5

all: $(TARGETS)

# Compilación de p1_4
p1_4: p4.c
	$(CC) $(CFLAGS) -o $@ $<

# Compilación de p1_5
p1_5: p5.c
	$(CC) $(CFLAGS) -o $@ $<

# Reglas de Valgrind Helgrind
helgrind_p1_4: p1_4
	valgrind --tool=helgrind ./p1_4

helgrind_p1_5: p1_5
	valgrind --tool=helgrind ./p1_5

# Limpiar compilados y ejecutables
clean:
	rm -f $(TARGETS) *.o
