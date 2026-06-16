/* renderizador.c — BirdEngine */

#include "renderizador.h"
#include "../Inputs/input.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>


#ifdef __SSE2__
#  include <immintrin.h>
#endif

/* =========================================================================
 * Estado global
 * ========================================================================= */

static int      _janela_rodando = 1;
static float    _delta_tempo    = 0.0f;
static uint64_t _tempo_anterior = 0;
static uint32_t _cor_fundo      = 0xFF000000;

static TileSpr _tilesspr[256];

/* =========================================================================
 * Helpers internos
 * ========================================================================= */


static inline void _iswap(int *a, int *b) { int t = *a; *a = *b; *b = t; }
static inline void _blender(uint32_t *dst, uint32_t cor);

static inline void _pixel(Janelas *j, int x, int y, uint32_t cor) {
    uint8_t a = (uint8_t)(cor >> 24);
    if (a == 0)   return;
    if (a == 255) { j->pixels[y * j->largura + x] = cor; return; }
    _blender(&j->pixels[y * j->largura + x], cor);
}

static inline void _preencher_linha(uint32_t *dst, int n, uint32_t cor) {
    uint8_t a = (uint8_t)(cor >> 24);
    if (a == 0) return;

    if (a == 255) {
#ifdef __SSE2__
        __m128i c = _mm_set1_epi32((int)cor);
        int i = 0;
        for (; i + 4 <= n; i += 4) _mm_storeu_si128((__m128i *)(dst + i), c);
        for (; i < n; i++) dst[i] = cor;
#else
        for (int i = 0; i < n; i++) dst[i] = cor;
#endif
    } else {
        for (int i = 0; i < n; i++) _blender(&dst[i], cor);
    }
}

static inline void _cam_pixel(const Janelas *j, float x, float y, int *ox, int *oy) {
    *ox = (int)lich_floorf(x - j->cam_x + 0.5f);
    *oy = (int)lich_floorf(y - j->cam_y + 0.5f);
}

static inline void _cam_aplicar(const Janelas *j, int *x, int *y) {
    *x -= (int)lich_floorf(j->cam_x + 0.5f);
    *y -= (int)lich_floorf(j->cam_y + 0.5f);
}

/* =========================================================================
 * Câmera 2D
 * ========================================================================= */

void set_camera(Janelas *janelas, float x, float y) {
    janelas->cam_x = x;
    janelas->cam_y = y;
}

void reset_camera(Janelas *janelas) {
    janelas->cam_x = 0.0f;
    janelas->cam_y = 0.0f;
}

Camera2D obter_camera(const Janelas *janelas) {
    Camera2D c = { (int)janelas->cam_x, (int)janelas->cam_y };
    return c;
}

/* =========================================================================
 * Janela
 * ========================================================================= */

uint32_t criar_cor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

int  rodando      (void)  { return _janela_rodando; }
void parar_janela (void)  { _janela_rodando = 0; }
float obter_delta (void)  { return _delta_tempo; }

void prosesar_eventos(Janelas *janelas) {
    (void)janelas;
    input_iniciar_frame();
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) _janela_rodando = 0;
        input_processar_evento(&ev);
    }
}

void cor_de_fundo(Janelas *janelas, uint32_t cor) {
    (void)janelas;
    _cor_fundo = cor;
}

void limpar_janela(Janelas *janelas, uint32_t cor) {
    uint32_t c = (cor != 0xFF000000) ? cor : _cor_fundo;
    _preencher_linha(janelas->pixels, janelas->largura * janelas->altura, c);
}

void apresentar_janela(Janelas *janelas) {
    void *ptr   = NULL;
    int   pitch = 0;
    if (SDL_LockTexture(janelas->texture, NULL, &ptr, &pitch) == 0) {
        if (pitch == janelas->largura * (int)sizeof(uint32_t)) {
            memcpy(ptr, janelas->pixels,
                   (size_t)janelas->largura * janelas->altura * sizeof(uint32_t));
        } else {
            uint8_t       *dst      = (uint8_t *)ptr;
            const uint8_t *src      = (const uint8_t *)janelas->pixels;
            int            row_bytes = janelas->largura * (int)sizeof(uint32_t);
            for (int y = 0; y < janelas->altura; y++, dst += pitch, src += row_bytes)
                memcpy(dst, src, row_bytes);
        }
        SDL_UnlockTexture(janelas->texture);
    }
    SDL_RenderClear(janelas->renderer);
    SDL_RenderCopy(janelas->renderer, janelas->texture, NULL, NULL);
    SDL_RenderPresent(janelas->renderer);
}

void set_modo_tela(Janelas *janelas, ModoTela modo) {
    janelas->modo_tela = modo;
    switch (modo) {
        case MODO_TELA_CHEIO:
        case MODO_TELA_LETTERBOX:
            SDL_SetWindowFullscreen(janelas->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            SDL_RenderSetIntegerScale(janelas->renderer, SDL_FALSE);
            break;
        case MODO_TELA_JANELA:
        default:
            SDL_SetWindowFullscreen(janelas->window, 0);
            SDL_SetWindowSize(janelas->window,
                              janelas->largura  * janelas->ampliar,
                              janelas->altura * janelas->ampliar);
            SDL_SetWindowPosition(janelas->window,
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            SDL_RenderSetIntegerScale(janelas->renderer, SDL_FALSE);
            break;
    }
    SDL_RenderSetLogicalSize(janelas->renderer, janelas->largura, janelas->altura);
}

int iniciar_janela(Janelas *janelas, int largura, int altura, int fps,
                   const char *titulo, int ampliar,
                   ModoTela modo_tela, Icone icone) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erro SDL: %s", SDL_GetError()); return -1;
    }
    janelas->largura    = largura;
    janelas->altura     = altura;
    janelas->fps        = fps;
    janelas->titulo     = titulo;
    janelas->ampliar    = ampliar;
    janelas->modo_tela  = modo_tela;
    janelas->cam_x      = 0.0f;
    janelas->cam_y      = 0.0f;

    janelas->window = SDL_CreateWindow(titulo,
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       largura * ampliar, altura * ampliar,
                                       SDL_WINDOW_SHOWN);
    if (!janelas->window) {
        SDL_Log("Erro janela: %s", SDL_GetError()); SDL_Quit(); return -1;
    }

    if (icone.caminho) {
        int iw, ih, ic;
        unsigned char *dados = stbi_load(icone.caminho, &iw, &ih, &ic, 4);
        if (dados) {
            SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(
                dados, iw, ih, 32, iw * 4,
                0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            if (surf) { SDL_SetWindowIcon(janelas->window, surf); SDL_FreeSurface(surf); }
            stbi_image_free(dados);
        }
    }

    janelas->renderer = SDL_CreateRenderer(janelas->window, -1, SDL_RENDERER_ACCELERATED);
    if (!janelas->renderer) {
        SDL_Log("Erro renderer: %s", SDL_GetError());
        SDL_DestroyWindow(janelas->window); SDL_Quit(); return -1;
    }

    janelas->texture = SDL_CreateTexture(janelas->renderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         largura, altura);
    if (!janelas->texture) {
        SDL_Log("Erro textura: %s", SDL_GetError());
        SDL_DestroyRenderer(janelas->renderer);
        SDL_DestroyWindow(janelas->window); SDL_Quit(); return -1;
    }

    janelas->pixels = (uint32_t *)calloc((size_t)largura * (size_t)altura, sizeof(uint32_t));
    if (!janelas->pixels) {
        SDL_Log("Erro pixels");
        SDL_DestroyTexture(janelas->texture);
        SDL_DestroyRenderer(janelas->renderer);
        SDL_DestroyWindow(janelas->window); SDL_Quit(); return -1;
    }

    set_modo_tela(janelas, modo_tela);
    lich_lut_init();
    return 0;
}

void fechar_janela(Janelas *janelas) {
    free(janelas->pixels); janelas->pixels = NULL;
    SDL_DestroyTexture(janelas->texture);
    SDL_DestroyRenderer(janelas->renderer);
    SDL_DestroyWindow(janelas->window);
    SDL_Quit();
}

void atualizar_janela(Janelas *janelas) {
    uint64_t frequencia = SDL_GetPerformanceFrequency();
    if (janelas->fps > 0) {
        float    tempo_alvo  = 1.0f / (float)janelas->fps;
        uint64_t agora       = SDL_GetPerformanceCounter();
        float    tempo_gasto = (float)(agora - _tempo_anterior) / (float)frequencia;
        float    sobra       = tempo_alvo - tempo_gasto - 0.0005f;
        if (sobra > 0.0f)
            SDL_Delay((uint32_t)(sobra * 1000.0f));
        do {
            agora       = SDL_GetPerformanceCounter();
            tempo_gasto = (float)(agora - _tempo_anterior) / (float)frequencia;
        } while (tempo_gasto < tempo_alvo);
    }
    uint64_t fim    = SDL_GetPerformanceCounter();
    _delta_tempo    = (float)(fim - _tempo_anterior) / (float)frequencia;
    _tempo_anterior = fim;
}

/* =========================================================================
 * Desenho 2D
 * ========================================================================= */

void desenhar_pixel(Janelas *janelas, int x, int y, uint32_t cor) {
    _cam_aplicar(janelas, &x, &y);
    if ((unsigned)x < (unsigned)janelas->largura &&
        (unsigned)y < (unsigned)janelas->altura)
        _pixel(janelas, x, y, cor);
}

void desenhar_linha_horizontal(Janelas *janelas, int x0, int x1, int y, uint32_t cor) {
    _cam_aplicar(janelas, &x0, &y);
    _cam_aplicar(janelas, &x1, &y);  /* y já deslocado, aplicar só x em x1 */
    if ((unsigned)y >= (unsigned)janelas->altura) return;
    if (x0 > x1) _iswap(&x0, &x1);
    x0 = lich_imax(x0, 0);
    x1 = lich_imin(x1, janelas->largura - 1);
    if (x0 > x1) return;
    _preencher_linha(janelas->pixels + y * janelas->largura + x0, x1 - x0 + 1, cor);
}

void desenhar_linha(Janelas *janelas, int x1, int y1, int x2, int y2,
                    int espesura, uint32_t cor) {
    _cam_aplicar(janelas, &x1, &y1);
    _cam_aplicar(janelas, &x2, &y2);

    int dx = lich_iabs(x2 - x1), sx = x1 < x2 ?  1 : -1;
    int dy = -lich_iabs(y2 - y1), sy = y1 < y2 ?  1 : -1;
    int err = dx + dy, e2;

    if (espesura <= 1) {
        for (;;) {
            if ((unsigned)x1 < (unsigned)janelas->largura &&
                (unsigned)y1 < (unsigned)janelas->altura)
                _pixel(janelas, x1, y1, cor);
            if (x1 == x2 && y1 == y2) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
        }
    } else {
        int offset = espesura / 2;
        for (;;) {
            for (int i = -offset; i <= offset; i++)
                for (int j = -offset; j <= offset; j++) {
                    int px = x1 + i, py = y1 + j;
                    if ((unsigned)px < (unsigned)janelas->largura &&
                        (unsigned)py < (unsigned)janelas->altura)
                        _pixel(janelas, px, py, cor);
                }
            if (x1 == x2 && y1 == y2) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
        }
    }
}

void rotacionar_ponto(int cx, int cy, float rad, int px, int py, int *rx, int *ry) {
    float dx = (float)(px - cx), dy = (float)(py - cy);
    float s, c;
    lich_sincosf(rad, &s, &c);
    *rx = cx + (int)(dx * c - dy * s);
    *ry = cy + (int)(dx * s + dy * c);
}

void preencher_poligono(Janelas *janelas, int *vx, int *vy, int n, uint32_t cor) {
    if (n < 3) return;
    /* aplica câmera nos vértices copiando para buffers locais */
    int cvx[64], cvy[64];
    if (n > 64) n = 64;
    int camx = (int)lich_floorf(janelas->cam_x + 0.5f);
    int camy = (int)lich_floorf(janelas->cam_y + 0.5f);
    for (int i = 0; i < n; i++) { cvx[i] = vx[i] - camx; cvy[i] = vy[i] - camy; }

    int y_min = cvy[0], y_max = cvy[0];
    for (int i = 1; i < n; i++) {
        if (cvy[i] < y_min) y_min = cvy[i];
        if (cvy[i] > y_max) y_max = cvy[i];
    }
    y_min = lich_imax(y_min, 0);
    y_max = lich_imin(y_max, janelas->altura - 1);

    int intersecoes[64];
    for (int y = y_min; y <= y_max; y++) {
        int ni = 0;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            int yi = cvy[i], yj = cvy[j];
            if ((yi <= y && yj > y) || (yj <= y && yi > y)) {
                intersecoes[ni++] = cvx[i] + (y - yi) * (cvx[j] - cvx[i]) / (yj - yi);
                if (ni >= 64) break;
            }
        }
        /* insertion sort (n pequeno) */
        for (int a = 1; a < ni; a++) {
            int v = intersecoes[a], b = a - 1;
            while (b >= 0 && intersecoes[b] > v) { intersecoes[b + 1] = intersecoes[b]; b--; }
            intersecoes[b + 1] = v;
        }
        for (int k = 0; k + 1 < ni; k += 2) {
            int x0 = lich_imax(intersecoes[k],     0);
            int x1 = lich_imin(intersecoes[k + 1], janelas->largura - 1);
            if (x0 <= x1 && (unsigned)y < (unsigned)janelas->altura)
                _preencher_linha(janelas->pixels + y * janelas->largura + x0,
                                 x1 - x0 + 1, cor);
        }
    }
}

void desenhar_circulo(Janelas *janelas, int cx, int cy, int raio,
                      uint32_t cor, int prenchido) {
    _cam_aplicar(janelas, &cx, &cy);
    int x = 0, y = raio, d = 3 - 2 * raio;
    while (y >= x) {
        if (prenchido) {
            int xa, xb;
            xa = lich_imax(cx - x, 0); xb = lich_imin(cx + x, janelas->largura - 1);
            int ya = cy + y, yb = cy - y;
            if ((unsigned)ya < (unsigned)janelas->altura)
                _preencher_linha(janelas->pixels + ya * janelas->largura + xa, xb - xa + 1, cor);
            if ((unsigned)yb < (unsigned)janelas->altura && yb != ya)
                _preencher_linha(janelas->pixels + yb * janelas->largura + xa, xb - xa + 1, cor);
            xa = lich_imax(cx - y, 0); xb = lich_imin(cx + y, janelas->largura - 1);
            int yc = cy + x, yd = cy - x;
            if ((unsigned)yc < (unsigned)janelas->altura)
                _preencher_linha(janelas->pixels + yc * janelas->largura + xa, xb - xa + 1, cor);
            if ((unsigned)yd < (unsigned)janelas->altura && yd != yc)
                _preencher_linha(janelas->pixels + yd * janelas->largura + xa, xb - xa + 1, cor);
        } else {
            /* 8 pontos de Bresenham */
            int pts[8][2] = {
                {cx+x, cy+y},{cx-x, cy+y},{cx+x, cy-y},{cx-x, cy-y},
                {cx+y, cy+x},{cx-y, cy+x},{cx+y, cy-x},{cx-y, cy-x}
            };
            for (int p = 0; p < 8; p++) {
                int px = pts[p][0], py = pts[p][1];
                if ((unsigned)px < (unsigned)janelas->largura &&
                    (unsigned)py < (unsigned)janelas->altura)
                    _pixel(janelas, px, py, cor);
            }
        }
        x++;
        if (d > 0) { y--; d += 4 * (x - y) + 10; }
        else       {      d += 4 * x + 6;         }
    }
}

void desenhar_quadrado(Janelas *janelas, int x, int y, int largura, int altura,
                       int angulo, uint32_t cor, int prenchido) {
    _cam_aplicar(janelas, &x, &y);
    float rad = (float)(angulo % 360) * (LICH_PI / 180.0f);
    int cx = x + largura / 2, cy = y + altura / 2;
    int v[4][2] = {
        {x,         y        }, {x + largura, y        },
        {x + largura, y + altura}, {x,         y + altura}
    };
    int rv[4][2];
    for (int i = 0; i < 4; i++)
        rotacionar_ponto(cx, cy, rad, v[i][0], v[i][1], &rv[i][0], &rv[i][1]);
    if (prenchido) {
        int tvx[4] = {rv[0][0], rv[1][0], rv[2][0], rv[3][0]};
        int tvy[4] = {rv[0][1], rv[1][1], rv[2][1], rv[3][1]};
        preencher_poligono(janelas, tvx, tvy, 4, cor);
    }
    desenhar_linha(janelas, rv[0][0], rv[0][1], rv[1][0], rv[1][1], 1, cor);
    desenhar_linha(janelas, rv[1][0], rv[1][1], rv[2][0], rv[2][1], 1, cor);
    desenhar_linha(janelas, rv[2][0], rv[2][1], rv[3][0], rv[3][1], 1, cor);
    desenhar_linha(janelas, rv[3][0], rv[3][1], rv[0][0], rv[0][1], 1, cor);
}

void desenhar_triangulo(Janelas *janelas, int x, int y, int largura, int altura,
                        int angulo, uint32_t cor, int prenchido) {
    _cam_aplicar(janelas, &x, &y);
    float rad = (float)(angulo % 360) * (LICH_PI / 180.0f);
    int cx = x + largura / 2, cy = y + altura / 2;
    int v[3][2] = {
        {cx,         y        },
        {x + largura, y + altura},
        {x,          y + altura}
    };
    int rv[3][2];
    for (int i = 0; i < 3; i++)
        rotacionar_ponto(cx, cy, rad, v[i][0], v[i][1], &rv[i][0], &rv[i][1]);
    if (prenchido) {
        int tvx[3] = {rv[0][0], rv[1][0], rv[2][0]};
        int tvy[3] = {rv[0][1], rv[1][1], rv[2][1]};
        preencher_poligono(janelas, tvx, tvy, 3, cor);
    }
    desenhar_linha(janelas, rv[0][0], rv[0][1], rv[1][0], rv[1][1], 1, cor);
    desenhar_linha(janelas, rv[1][0], rv[1][1], rv[2][0], rv[2][1], 1, cor);
    desenhar_linha(janelas, rv[2][0], rv[2][1], rv[0][0], rv[0][1], 1, cor);
}

static inline void _blender(uint32_t *dst, uint32_t cor) {
    uint8_t a = (uint8_t)(cor >> 24);
    if (a == 0)   return;
    if (a == 255) { *dst = cor; return; }
    uint32_t fundo = *dst;
    uint32_t ia = 255u - (uint32_t)a;
    uint8_t nr = (uint8_t)((((uint32_t)(cor   >> 16) & 0xFF) * a
                           + ((uint32_t)(fundo >> 16) & 0xFF) * ia) >> 8);
    uint8_t ng = (uint8_t)((((uint32_t)(cor   >>  8) & 0xFF) * a
                           + ((uint32_t)(fundo >>  8) & 0xFF) * ia) >> 8);
    uint8_t nb = (uint8_t)((((uint32_t)(cor         & 0xFF)) * a + ((uint32_t)(fundo       & 0xFF)) * ia) >> 8);
    uint8_t na = (uint8_t)(a + (uint8_t)(((uint32_t)(fundo >> 24) * ia) >> 8));
    *dst = ((uint32_t)na << 24) | ((uint32_t)nr << 16) | ((uint32_t)ng << 8) | nb;
}

int carregar_atlas(Atlas *atlas, const char *caminho) {
    int largura, altura, canais;
    unsigned char *dados = stbi_load(caminho, &largura, &altura, &canais, 4);
    if (!dados) { atlas->pixels = NULL; atlas->largura = 0; atlas->altura = 0; return 0; }
    atlas->largura = largura;
    atlas->altura  = altura;
    int total = largura * altura;
    atlas->pixels = (uint32_t *)malloc((size_t)total * sizeof(uint32_t));
    if (!atlas->pixels) { stbi_image_free(dados); return 0; }

    const uint32_t *src = (const uint32_t *)dados;
    uint32_t       *dst = atlas->pixels;
    for (int i = 0; i < total; i++) {
        uint32_t p = src[i];
        uint8_t r = (uint8_t) p;
        uint8_t g = (uint8_t)(p >>  8);
        uint8_t b = (uint8_t)(p >> 16);
        uint8_t a = (uint8_t)(p >> 24);
        dst[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
    stbi_image_free(dados);
    return 1;
}

void liberar_atlas(Atlas *atlas) {
    free(atlas->pixels); atlas->pixels = NULL; atlas->largura = 0; atlas->altura = 0;
}

Recorte cortar_atlas(const Atlas *atlas, int largura_corte, int altura_corte,
                     int coluna, int linha) {
    Recorte rec = { NULL, 0, 0, 0 };
    if (!atlas->pixels || largura_corte <= 0 || altura_corte <= 0) return rec;
    int x = coluna * largura_corte, y = linha * altura_corte;
    if (x < 0 || y < 0 ||
        x + largura_corte > atlas->largura ||
        y + altura_corte  > atlas->altura) return rec;
    rec.pixels  = atlas->pixels + y * atlas->largura + x;
    rec.largura = largura_corte;
    rec.altura  = altura_corte;
    rec.passo   = atlas->largura;
    return rec;
}

void desenhar_sprite(Janelas *janelas, const Recorte *recorte,
                     float x, float y, int largura_destino, int altura_destino,
                     int angulo, int espelhado) {
    if (!recorte || !recorte->pixels) return;

    int dst_w = (largura_destino <= 0) ? recorte->largura : largura_destino;
    int dst_h = (altura_destino  <= 0) ? recorte->altura  : altura_destino;
    if (dst_w < 1 || dst_h < 1) return;

    int ix, iy;
    _cam_pixel(janelas, x, y, &ix, &iy);

    if ((angulo % 360) == 0) {
        int dx_ini = lich_imax(ix, 0);
        int dx_fim = lich_imin(ix + dst_w, janelas->largura);
        int dy_ini = lich_imax(iy, 0);
        int dy_fim = lich_imin(iy + dst_h, janelas->altura);
        if (dx_ini >= dx_fim || dy_ini >= dy_fim) return;
        int32_t passo_u = (int32_t)(((uint32_t)recorte->largura << 16) / (uint32_t)dst_w);
        int32_t passo_v = (int32_t)(((uint32_t)recorte->altura  << 16) / (uint32_t)dst_h);
        int32_t v0 = passo_v * (dy_ini - iy);

        for (int dy = dy_ini; dy < dy_fim; dy++, v0 += passo_v) {
            int sy = lich_imin((int)(v0 >> 16), recorte->altura - 1);
            const uint32_t *linha_src = recorte->pixels + sy * recorte->passo;
            uint32_t       *linha_dst = janelas->pixels + dy * janelas->largura;

            int32_t u = passo_u * (dx_ini - ix);
            for (int dx = dx_ini; dx < dx_fim; dx++, u += passo_u) {
                int sx  = lich_imin((int)(u >> 16), recorte->largura - 1);
                int sxl = espelhado ? (recorte->largura - 1 - sx) : sx;
                uint32_t cor = linha_src[sxl];
                if ((uint8_t)(cor >> 24) == 0) continue;
                _blender(&linha_dst[dx], cor);
            }
        }
    } else {
        float rad = (float)(angulo % 360) * (LICH_PI / 180.0f);
        float sinA, cosA;
        lich_sincosf(rad, &sinA, &cosA);

        float sw = (float)dst_w, sh = (float)dst_h;
        float ocx = (float)ix + sw * 0.5f, ocy = (float)iy + sh * 0.5f;
        float hw = sw * 0.5f, hh = sh * 0.5f;
        float diag = lich_sqrtf(hw*hw + hh*hh);

        int dx_min = lich_imax((int)(ocx - diag), 0);
        int dx_max = lich_imin((int)(ocx + diag), janelas->largura - 1);
        int dy_min = lich_imax((int)(ocy - diag), 0);
        int dy_max = lich_imin((int)(ocy + diag), janelas->altura  - 1);

        float inv_sw = 1.0f / sw, inv_sh = 1.0f / sh;

        for (int dy = dy_min; dy <= dy_max; dy++) {
            float ly = (float)dy - ocy + 0.5f;
            float ux_base = ly * sinA;
            float uy_base = ly * cosA;
            for (int dx = dx_min; dx <= dx_max; dx++) {
                float lx = (float)dx - ocx + 0.5f;
                float ux = lx * cosA + ux_base;
                float uy = -lx * sinA + uy_base;
                float px = ux + hw, py = uy + hh;
                if (px < 0.0f || py < 0.0f || px >= sw || py >= sh) continue;
                int sx  = lich_imin((int)(px * (float)recorte->largura * inv_sw), recorte->largura - 1);
                int sy  = lich_imin((int)(py * (float)recorte->altura  * inv_sh), recorte->altura  - 1);
                int sxl = espelhado ? (recorte->largura - 1 - sx) : sx;
                uint32_t cor = recorte->pixels[sy * recorte->passo + sxl];
                _blender(&janelas->pixels[dy * janelas->largura + dx], cor);
            }
        }
    }
}

/* =========================================================================
 * Fontes bitmap
 * ========================================================================= */

void carregar_fonte(Fonte *fonte, const Atlas *atlas,
                    int largura_char, int altura_char,
                    TipoFonte tipo, uint32_t cp_inicio) {
    fonte->atlas        = atlas;
    fonte->largura_char = largura_char;
    fonte->altura_char  = altura_char;
    fonte->tipo         = tipo;
    fonte->colunas      = atlas->largura / largura_char;
    fonte->cp_inicio    = cp_inicio;
}

static uint32_t _utf8_proximo(const char **p) {
    uint8_t c = (uint8_t)*(*p)++;
    if (c < 0x80) return c;
    uint32_t cp; int bytes;
    if      ((c & 0xE0) == 0xC0) { cp = c & 0x1F; bytes = 1; }
    else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; bytes = 2; }
    else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; bytes = 3; }
    else return '?';
    while (bytes--) {
        if ((**p & 0xC0) != 0x80) return '?';
        cp = (cp << 6) | ((uint8_t)*(*p)++ & 0x3F);
    }
    return cp;
}

static uint32_t _unicode_para_cp437(uint32_t cp) {
    switch (cp) {
        case 0x00C7: return 128; case 0x00FC: return 129; case 0x00E9: return 130;
        case 0x00E2: return 131; case 0x00E4: return 132; case 0x00E0: return 133;
        case 0x00E5: return 134; case 0x00E7: return 135; case 0x00EA: return 136;
        case 0x00EB: return 137; case 0x00E8: return 138; case 0x00EF: return 139;
        case 0x00EE: return 140; case 0x00EC: return 141; case 0x00C4: return 142;
        case 0x00C5: return 143; case 0x00C9: return 144; case 0x00E6: return 145;
        case 0x00C6: return 146; case 0x00F4: return 147; case 0x00F6: return 148;
        case 0x00F2: return 149; case 0x00FB: return 150; case 0x00F9: return 151;
        case 0x00FF: return 152; case 0x00D6: return 153; case 0x00DC: return 154;
        case 0x00E1: return 160; case 0x00ED: return 161; case 0x00F3: return 162;
        case 0x00FA: return 163; case 0x00F1: return 164; case 0x00D1: return 165;
        case 0x00AA: return 166; case 0x00BA: return 167; case 0x00BF: return 168;
        case 0x00AC: return 170; case 0x00BD: return 171; case 0x00BC: return 172;
        case 0x00A1: return 173; case 0x00AB: return 174; case 0x00BB: return 175;
        case 0x00DF: return 225; case 0x00B5: return 230; case 0x00B1: return 241;
        case 0x00F7: return 246; case 0x00B0: return 248; case 0x00B7: return 250;
        case 0x00B2: return 253;
        default: return (cp < 0x80) ? cp : '?';
    }
}

static int _fonte_tem_glyph(const Fonte *fonte, uint32_t cp, int *idx) {
    if (!fonte->atlas || !fonte->atlas->pixels || fonte->colunas <= 0) return 0;
    if (cp < fonte->cp_inicio) return 0;
    *idx = (int)(cp - fonte->cp_inicio);
    int col = *idx % fonte->colunas;
    int lin = *idx / fonte->colunas;
    int ox  = col * fonte->largura_char;
    int oy  = lin * fonte->altura_char;
    return ox + fonte->largura_char <= fonte->atlas->largura &&
           oy + fonte->altura_char  <= fonte->atlas->altura;
}

static void _desenhar_char(Janelas *janelas, const Fonte *fonte,
                            uint32_t cp, int x, int y, float escala, uint32_t cor) {
    int idx = 0;
    if (!_fonte_tem_glyph(fonte, cp, &idx))
        if (!_fonte_tem_glyph(fonte, '?', &idx)) return;

    int col = idx % fonte->colunas;
    int lin = idx / fonte->colunas;
    int ox  = col * fonte->largura_char;
    int oy  = lin * fonte->altura_char;

    int dst_w = (int)(fonte->largura_char * escala + 0.5f);
    int dst_h = (int)(fonte->altura_char  * escala + 0.5f);
    if (dst_w < 1 || dst_h < 1) return;

    uint8_t cr = (uint8_t)(cor >> 16);
    uint8_t cg = (uint8_t)(cor >>  8);
    uint8_t cb = (uint8_t) cor;

    int dx_ini = lich_imax(x, 0);
    int dx_fim = lich_imin(x + dst_w, janelas->largura);
    int dy_ini = lich_imax(y, 0);
    int dy_fim = lich_imin(y + dst_h, janelas->altura);

    for (int dy = dy_ini; dy < dy_fim; dy++) {
        int sy = lich_imin((int)(((float)(dy - y) + 0.5f) * (float)fonte->altura_char  / (float)dst_h), fonte->altura_char  - 1);
        for (int dx = dx_ini; dx < dx_fim; dx++) {
            int sx = lich_imin((int)(((float)(dx - x) + 0.5f) * (float)fonte->largura_char / (float)dst_w), fonte->largura_char - 1);
            uint32_t src = fonte->atlas->pixels[(oy + sy) * fonte->atlas->largura + (ox + sx)];
            uint8_t  a   = (uint8_t)(src >> 24);
            if (a == 0) continue;
            uint32_t pixel = ((uint32_t)a << 24) | ((uint32_t)cr << 16) | ((uint32_t)cg << 8) | cb;
            _blender(&janelas->pixels[dy * janelas->largura + dx], pixel);
        }
    }
}

void escrever(Janelas *janelas, const Fonte *fonte, const char *texto,
              int x, int y, float escala, uint32_t cor) {
    _cam_aplicar(janelas, &x, &y);
    if (!texto) return;
    int cx       = x;
    int avanço_x = (int)(fonte->largura_char * escala + 0.5f);
    int avanço_y = (int)(fonte->altura_char  * escala + 0.5f);
    if (avanço_x < 1) avanço_x = 1;
    if (avanço_y < 1) avanço_y = 1;
    const char *p = texto;
    while (*p) {
        if (*p == '\n') { cx = x; y += avanço_y; p++; continue; }
        uint32_t cp = (fonte->tipo == TIPO_FONTE_UTF8)
                    ? _unicode_para_cp437(_utf8_proximo(&p))
                    : (uint8_t)*p++;
        _desenhar_char(janelas, fonte, cp, cx, y, escala, cor);
        cx += avanço_x;
    }
}

/* =========================================================================
 * TileSpr
 * ========================================================================= */

void definir_tilespr(char c, const Atlas *atlas,
                     int largura_corte, int altura_corte,
                     int coluna, int linha) {
    unsigned char idx = (unsigned char)c;
    _tilesspr[idx].ativo   = 1;
    _tilesspr[idx].recorte = cortar_atlas(atlas, largura_corte, altura_corte, coluna, linha);
}

Recorte obter_tilespr(char c) {
    unsigned char idx = (unsigned char)c;
    if (_tilesspr[idx].ativo) return _tilesspr[idx].recorte;
    Recorte vazio = { NULL, 0, 0, 0 };
    return vazio;
}