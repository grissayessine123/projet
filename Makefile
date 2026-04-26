# Makefile – Jeu de Plateformes SDL2
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 -Iinclude \
           $(shell sdl2-config --cflags) -I/usr/include/SDL2

LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_image -lm

TARGET  = game
SRCS    = src/main.c \
          src/serie.c \
          src/background.c \
          src/platforms.c \
          src/display.c \
          src/guide.c \
          src/events.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) scores.dat
