#ifndef FILTRO_H
#define FILTRO_H

#include "../tipos.h"
#include <stdint.h>

#define FILTRO_MAX_CORES 16
#define FILTRO_MAX_TONS  16

/* FILTRO_LUT3D_BITS controla a resolução da LUT 3D para paletas multi-cor.
 * BITS=4 → 16³ entradas × 4B = 16 KB por filtro (suficiente para paletas CGA/retro).
 * BITS=5 → 32³ entradas × 4B = 128 KB por filtro (precisa mais se quiser fidelidade foto). */
#define FILTRO_LUT3D_BITS 4
#define FILTRO_LUT3D_DIM  (1 << FILTRO_LUT3D_BITS)
#define FILTRO_LUT3D_N    (FILTRO_LUT3D_DIM * FILTRO_LUT3D_DIM * FILTRO_LUT3D_DIM)

typedef struct {
    uint8_t r, g, b;
} CorFiltro;

typedef struct {
    CorFiltro cores[FILTRO_MAX_CORES];
    int       num_cores;
    int       num_tons;
    uint32_t  lut[256];
    uint32_t *lut3d;
} Filtro;

void filtro_construir(Filtro *filtro, const CorFiltro *cores, int num_cores, int num_tons);
void filtro_aplicar(Janelas *janelas, const Filtro *filtro);
void filtro_aplicar_regiao(Janelas *janelas, const Filtro *filtro, int x, int y, int largura, int altura);
void filtro_limpar(Filtro *filtro);

void pixel_separar(Janelas *janelas, int tamanho_pixel, float forca);
void pixel_separar_regiao(Janelas *janelas, int tamanho_pixel, float forca, int x, int y, int largura, int altura);

void monitor_chiado(Janelas *janelas, float forca, uint32_t seed);
void monitor_chiado_regiao(Janelas *janelas, float forca, uint32_t seed, int x, int y, int largura, int altura);

void monitor_limpeza(Janelas *janelas, float progresso, int largura_faixa, float brilho, float escurecer);

#endif