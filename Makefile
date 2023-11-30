CC = gcc
CFLAGS = -lraylib -lm -lpthread -lopengl32 -lgdi32 -lwinmm -Wall -Wextra
SOURCES = ./src/*.c

all: contour run clean

contour:
	$(CC) $(SOURCES) $(CFLAGS) -o contour

run:
	./contour

clean:
	rm contour
