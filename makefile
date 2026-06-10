CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11 -I. `sdl2-config --cflags`
SOFLAGS = -fPIC -shared
LIBS    = `sdl2-config --libs` -lSDL2_mixer

SRC_COMMON = src/Render/renderizador.c \
             src/Inputs/input.c \
             src/Objetos/objetos.c \
             src/Audio/audio.c \
             src/Render/filtro.c \
             src/Mapas/mapa.c

all: bird

bird: src/FFI/bird.c $(SRC_COMMON)
	$(CC) $(CFLAGS) $(SOFLAGS) -o libbird.so src/FFI/bird.c $(SRC_COMMON) $(LIBS)

clean:
	rm -f libbird.so