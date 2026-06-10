/* mapa.c — BirdEngine */

#include "mapa.h"
#include "../Render/renderizador.h"
#include <string.h>

void map(Mapa *mapa, Janelas *janela, int largura, int altura, int tile_w, int tile_h) {
    memset(mapa, 0, sizeof(Mapa));
    mapa->largura = lich_imin(largura, MAPA_MAX_LARGURA);
    mapa->altura  = lich_imin(altura,  MAPA_MAX_ALTURA);
    mapa->tile_w  = lich_imax(tile_w, 1);
    mapa->tile_h  = lich_imax(tile_h, 1);
    mapa->janela  = janela;
}

void tile(Mapa *mapa, int cel_x, int cel_y, char c) {
    mset(mapa, cel_x, cel_y, c);
}

char mget(const Mapa *mapa, int cel_x, int cel_y) {
    if (cel_x < 0 || cel_y < 0 || cel_x >= mapa->largura || cel_y >= mapa->altura)
        return 0;
    return mapa->celulas[cel_y][cel_x];
}

void mset(Mapa *mapa, int cel_x, int cel_y, char c) {
    if (cel_x < 0 || cel_y < 0 || cel_x >= mapa->largura || cel_y >= mapa->altura)
        return;
    mapa->celulas[cel_y][cel_x] = c;
}

int fget(const Mapa *mapa, char c, int flag) {
    if (flag < 0 || flag > 7) return 0;
    return (mapa->flags[(uint8_t)c] >> flag) & 1;
}

uint8_t fget_byte(const Mapa *mapa, char c) {
    return mapa->flags[(uint8_t)c];
}

void fset(Mapa *mapa, char c, int flag, int valor) {
    if (flag < 0 || flag > 7) return;
    if (valor)
        mapa->flags[(uint8_t)c] |=  (uint8_t)(1 << flag);
    else
        mapa->flags[(uint8_t)c] &= ~(uint8_t)(1 << flag);
}

void fset_byte(Mapa *mapa, char c, uint8_t valor) {
    mapa->flags[(uint8_t)c] = valor;
}

void desenhar_mapa2d(Mapa *mapa,
                     int cel_x0,  int cel_y0,
                     int n_cel_x, int n_cel_y,
                     int scr_x,   int scr_y) {
    if (!mapa->janela) return;

    int cx_fim = (n_cel_x <= 0) ? mapa->largura : cel_x0 + n_cel_x;
    int cy_fim = (n_cel_y <= 0) ? mapa->altura  : cel_y0 + n_cel_y;
    cx_fim = lich_imin(cx_fim, mapa->largura);
    cy_fim = lich_imin(cy_fim, mapa->altura);

    for (int cy = cel_y0; cy < cy_fim; cy++) {
        for (int cx = cel_x0; cx < cx_fim; cx++) {
            char c = mapa->celulas[cy][cx];
            if (c == 0 || c == ' ') continue;

            Recorte rec = obter_tilespr(c);
            if (!rec.pixels) continue;

            int px = scr_x + (cx - cel_x0) * mapa->tile_w;
            int py = scr_y + (cy - cel_y0) * mapa->tile_h;

            if (fget(mapa, c, 7)) {
                uint32_t cor_sombra = lich_rgba_escurecer(0xFFFFFFFF, 0.5f);
                desenhar_pixel(mapa->janela, px, py, cor_sombra);
            }

            desenhar_sprite(mapa->janela, &rec, (float)px, (float)py, 0, 0, 0, 0);
        }
    }
}