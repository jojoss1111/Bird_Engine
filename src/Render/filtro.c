#include "filtro.h"
#include <string.h>
#include <stdlib.h>

/* _lum removido: usar lich_rgba_lum() do Lich diretamente */

static inline int _dist_rgb(int pr, int pg, int pb,
                             int cr, int cg, int cb) {
    int dr = pr - cr, dg = pg - cg, db = pb - cb;
    return (dr*dr*3) + (dg*dg*4) + (db*db);
}

static void _construir_lut_fosfor(Filtro *filtro) {
    const int tons = filtro->num_tons;
    uint8_t r0 = 0, g0 = 0, b0 = 0;
    uint8_t r1 = filtro->cores[0].r, g1 = filtro->cores[0].g, b1 = filtro->cores[0].b;

    if (filtro->num_cores == 2) {
        r0 = filtro->cores[0].r; g0 = filtro->cores[0].g; b0 = filtro->cores[0].b;
        r1 = filtro->cores[1].r; g1 = filtro->cores[1].g; b1 = filtro->cores[1].b;
    }

    uint32_t degraus[FILTRO_MAX_TONS];
    for (int t = 0; t < tons; t++) {
        float tf = (float)t / (float)(tons - 1);
        uint8_t r = (uint8_t)lich_lerpf((float)r0, (float)r1, tf);
        uint8_t g = (uint8_t)lich_lerpf((float)g0, (float)g1, tf);
        uint8_t b = (uint8_t)lich_lerpf((float)b0, (float)b1, tf);
        degraus[t] = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    for (int lum = 0; lum < 256; lum++) {
        int t = (lum * (tons - 1) + 127) / 255;
        if (t >= tons) t = tons - 1;
        filtro->lut[lum] = degraus[t];
    }
}

static void _construir_lut3d(Filtro *filtro) {
    if (!filtro->lut3d) return;

    const int DIM  = FILTRO_LUT3D_DIM;
    const int BITS = FILTRO_LUT3D_BITS;

    for (int ri = 0; ri < DIM; ri++) {
        int pr = (ri << (8 - BITS)) | (1 << (7 - BITS));
        for (int gi = 0; gi < DIM; gi++) {
            int pg = (gi << (8 - BITS)) | (1 << (7 - BITS));
            for (int bi = 0; bi < DIM; bi++) {
                int pb = (bi << (8 - BITS)) | (1 << (7 - BITS));

                int best = 0x7FFFFFFF, ci_best = 0;
                for (int ci = 0; ci < filtro->num_cores; ci++) {
                    int d = _dist_rgb(pr, pg, pb,
                                      filtro->cores[ci].r,
                                      filtro->cores[ci].g,
                                      filtro->cores[ci].b);
                    if (d < best) { best = d; ci_best = ci; }
                }

                CorFiltro c = filtro->cores[ci_best];
                filtro->lut3d[ri * DIM * DIM + gi * DIM + bi] =
                    0xFF000000u | ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | (uint32_t)c.b;
            }
        }
    }
}

static void _finalizar_construcao(Filtro *filtro) {
    if (filtro->num_cores == 0) return;

    if (filtro->num_tons < 2)                filtro->num_tons = 2;
    if (filtro->num_tons > FILTRO_MAX_TONS)  filtro->num_tons = FILTRO_MAX_TONS;

    if (filtro->num_cores <= 2) {
        _construir_lut_fosfor(filtro);
    } else {
        filtro->lut3d = (uint32_t *)malloc(FILTRO_LUT3D_N * sizeof(uint32_t));
        if (filtro->lut3d)
            _construir_lut3d(filtro);
    }
}

void filtro_construir(Filtro *filtro, const CorFiltro *cores, int num_cores, int num_tons) {
    filtro_limpar(filtro);
    if (!cores || num_cores < 1) return;
    if (num_cores > FILTRO_MAX_CORES) num_cores = FILTRO_MAX_CORES;
    for (int i = 0; i < num_cores; i++)
        filtro->cores[i] = cores[i];
    filtro->num_cores = num_cores;
    filtro->num_tons  = num_tons;
    _finalizar_construcao(filtro);
}

static inline void _aplicar_bloco(uint32_t *pixels, int n, const Filtro *filtro) {
    if (filtro->num_cores == 0) return;

    if (filtro->num_cores <= 2) {
        for (int i = 0; i < n; i++) {
            uint32_t px = pixels[i];
            uint8_t  pa = (uint8_t)(px >> 24);
            if (pa == 0) continue;
            uint32_t cor = filtro->lut[lich_rgba_lum(px)];
            pixels[i]   = (cor & 0x00FFFFFFu) | ((uint32_t)pa << 24);
        }
    } else if (filtro->lut3d) {
        const int DIM   = FILTRO_LUT3D_DIM;
        const int SHIFT = 8 - FILTRO_LUT3D_BITS;

        for (int i = 0; i < n; i++) {
            uint32_t px = pixels[i];
            uint8_t  pa = (uint8_t)(px >> 24);
            if (pa == 0) continue;

            int ri = (int)((px >> 16) & 0xFF) >> SHIFT;
            int gi = (int)((px >>  8) & 0xFF) >> SHIFT;
            int bi = (int)( px        & 0xFF) >> SHIFT;

            uint32_t cor = filtro->lut3d[ri * DIM * DIM + gi * DIM + bi];
            pixels[i]   = (cor & 0x00FFFFFFu) | ((uint32_t)pa << 24);
        }
    }
}

void filtro_aplicar(Janelas *janelas, const Filtro *filtro) {
    _aplicar_bloco(janelas->pixels, janelas->largura * janelas->altura, filtro);
}

void filtro_aplicar_regiao(Janelas *janelas, const Filtro *filtro,
                           int x, int y, int largura, int altura) {
    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = x + largura; if (x1 > janelas->largura) x1 = janelas->largura;
    int y1 = y + altura;  if (y1 > janelas->altura)  y1 = janelas->altura;
    if (x0 >= x1 || y0 >= y1) return;

    int w = x1 - x0;
    for (int linha = y0; linha < y1; linha++)
        _aplicar_bloco(janelas->pixels + linha * janelas->largura + x0, w, filtro);
}

void filtro_limpar(Filtro *filtro) {
    if (filtro->lut3d) {
        free(filtro->lut3d);
        filtro->lut3d = NULL;
    }
    memset(filtro, 0, sizeof(Filtro));
}

/* _escurecer removido: usar lich_rgba_mul_alpha() diretamente */

void pixel_separar_regiao(Janelas *janelas, int tamanho_pixel, float forca,
                          int x, int y, int largura, int altura) {
    if (tamanho_pixel <= 1 || forca <= 0.0f) return;
    if (forca > 1.0f) forca = 1.0f;

    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = x + largura; if (x1 > janelas->largura) x1 = janelas->largura;
    int y1 = y + altura;  if (y1 > janelas->altura)  y1 = janelas->altura;
    if (x0 >= x1 || y0 >= y1) return;

    uint32_t forca_256  = (uint32_t)(forca * 256.0f + 0.5f);
    if (forca_256 > 256) forca_256 = 256;
    uint32_t fator_borda = 256u - forca_256;
    const int tp    = tamanho_pixel;
    const int borda = tp - 1;

    for (int py = y0; py < y1; py++) {
        int eh_borda_v = (py % tp == borda);
        uint32_t *row  = janelas->pixels + py * janelas->largura;
        for (int px = x0; px < x1; px++) {
            if (eh_borda_v || (px % tp == borda))
                row[px] = (fator_borda < 256u)
                          ? lich_rgba_mul_alpha(row[px], (uint8_t)fator_borda)
                          : row[px];
        }
    }
}

void pixel_separar(Janelas *janelas, int tamanho_pixel, float forca) {
    pixel_separar_regiao(janelas, tamanho_pixel, forca,
                         0, 0, janelas->largura, janelas->altura);
}

void monitor_chiado_regiao(Janelas *janelas, float forca, uint32_t seed,
                           int x, int y, int largura, int altura) {
    if (forca <= 0.0f) return;
    if (forca >  1.0f) forca = 1.0f;

    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = x + largura; if (x1 > janelas->largura) x1 = janelas->largura;
    int y1 = y + altura;  if (y1 > janelas->altura)  y1 = janelas->altura;
    if (x0 >= x1 || y0 >= y1) return;

    int      amp   = (int)(forca * 127.0f + 0.5f);
    uint32_t bmask = 1;
    while (bmask < (uint32_t)(amp * 2 + 1)) bmask <<= 1;
    bmask -= 1;

    for (int py = y0; py < y1; py++) {
        uint32_t s = seed
                   ^ ((uint32_t)py * 2654435761u)
                   ^ ((uint32_t)x0 * 805459861u);
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;

        uint32_t *row = janelas->pixels + py * janelas->largura;
        for (int px = x0; px < x1; px++) {
            s ^= s << 13;
            s ^= s >> 17;
            s ^= s << 5;
            int ruido = (int)(s & bmask) - amp;

            uint32_t cor = row[px];
            uint8_t  pa  = (uint8_t)(cor >> 24);
            if (pa == 0) continue;

            int r = (int)((cor >> 16) & 0xFF) + ruido;
            int g = (int)((cor >>  8) & 0xFF) + ruido;
            int b = (int)( cor        & 0xFF) + ruido;

            row[px] = ((uint32_t)pa                      << 24)
                    | ((uint32_t)lich_iclamp(r, 0, 255)  << 16)
                    | ((uint32_t)lich_iclamp(g, 0, 255)  <<  8)
                    |  (uint32_t)lich_iclamp(b, 0, 255);
        }
    }
}

void monitor_chiado(Janelas *janelas, float forca, uint32_t seed) {
    monitor_chiado_regiao(janelas, forca, seed,
                          0, 0, janelas->largura, janelas->altura);
}

void monitor_limpeza(Janelas *janelas, float progresso,
                     int largura_faixa, float brilho, float escurecer) {
    if (progresso <= 0.0f && escurecer <= 0.0f) return;
    if (progresso >  1.0f) progresso = 1.0f;
    if (brilho    >  1.0f) brilho    = 1.0f;
    if (escurecer >  1.0f) escurecer = 1.0f;

    const int W = janelas->largura;
    const int H = janelas->altura;

    int d_max    = W + H - 2;
    int d_frente = (int)(progresso * (float)d_max);
    int d_faixa  = (largura_faixa < 1) ? 1 : largura_faixa;

    int brilho256    = (int)(lich_clampf(brilho,    0.0f, 1.0f) * 256.0f + 0.5f);
    int escurecer256 = (int)(lich_clampf(escurecer, 0.0f, 1.0f) * 256.0f + 0.5f);

    for (int py = 0; py < H; py++) {
        uint32_t *row = janelas->pixels + py * W;
        for (int px = 0; px < W; px++) {
            uint32_t cor = row[px];
            uint8_t  pa  = (uint8_t)(cor >> 24);
            if (pa == 0) continue;

            int d = px + py;
            int r = (int)((cor >> 16) & 0xFF);
            int g = (int)((cor >>  8) & 0xFF);
            int b = (int)( cor        & 0xFF);

            if (d >= d_frente - d_faixa && d <= d_frente) {
                int dist_borda = d - (d_frente - d_faixa);
                int meio = d_faixa / 2;
                int t    = (dist_borda <= meio)
                           ? (dist_borda * 255) / (meio + 1)
                           : ((d_faixa - dist_borda) * 255) / (meio + 1);
                t = lich_imin(t, 255);
                int add = (t * brilho256) >> 8;
                r = lich_iclamp(r + add, 0, 255);
                g = lich_iclamp(g + add, 0, 255);
                b = lich_iclamp(b + add, 0, 255);
            } else if (d < d_frente - d_faixa && escurecer256 > 0) {
                int fator = 256 - escurecer256;
                r = (r * fator) >> 8;
                g = (g * fator) >> 8;
                b = (b * fator) >> 8;
            }

            row[px] = ((uint32_t)pa << 24)
                    | ((uint32_t)r  << 16)
                    | ((uint32_t)g  <<  8)
                    |  (uint32_t)b;
        }
    }
}