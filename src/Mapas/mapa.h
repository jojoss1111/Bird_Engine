#ifndef MAPA_H
#define MAPA_H

#include "../tipos.h"
#include <stdint.h>

#define MAPA_MAX_LARGURA 512
#define MAPA_MAX_ALTURA  512

typedef struct {
    char    celulas[MAPA_MAX_ALTURA][MAPA_MAX_LARGURA];
    uint8_t flags[256];
    int     largura;
    int     altura;
    int     tile_w;
    int     tile_h;
    Janelas *janela;
} Mapa;

void    map       (Mapa *mapa, Janelas *janela, int largura, int altura, int tile_w, int tile_h);
void    tile      (Mapa *mapa, int cel_x, int cel_y, char c);
char    mget      (const Mapa *mapa, int cel_x, int cel_y);
void    mset      (Mapa *mapa, int cel_x, int cel_y, char c);
int     fget      (const Mapa *mapa, char c, int flag);
uint8_t fget_byte (const Mapa *mapa, char c);
void    fset      (Mapa *mapa, char c, int flag, int valor);
void    fset_byte (Mapa *mapa, char c, uint8_t valor);

void desenhar_mapa2d (Mapa *mapa,
                      int cel_x0,  int cel_y0,
                      int n_cel_x, int n_cel_y,
                      int scr_x,   int scr_y);

#endif /* MAPA_H */