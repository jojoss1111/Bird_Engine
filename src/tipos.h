#ifndef TIPOS_H
#define TIPOS_H

#include "Lich.h"
#include <SDL2/SDL.h>
#include <stdint.h>

typedef enum {
    MODO_TELA_JANELA    = 0,
    MODO_TELA_CHEIO     = 1,
    MODO_TELA_LETTERBOX = 2
} ModoTela;

typedef struct {
    int           largura, altura;
    int           fps;
    const char   *titulo;
    int           ampliar;
    ModoTela      modo_tela;
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;
    uint32_t     *pixels;
    float         cam_x, cam_y;   /* câmera 2D embutida (evita hashmap) */
} Janelas;

typedef struct {
    uint32_t *pixels;
    int       largura;
    int       altura;
} Atlas;

typedef struct {
    const uint32_t *pixels;
    int             largura;
    int             altura;
    int             passo;
} Recorte;

typedef struct {
    int x, y;
} Camera2D;

typedef enum {
    TIPO_FONTE_ASCII = 0,
    TIPO_FONTE_UTF8  = 1
} TipoFonte;

typedef struct {
    const Atlas *atlas;
    int          largura_char;
    int          altura_char;
    TipoFonte    tipo;
    int          colunas;
    uint32_t     cp_inicio;
} Fonte;

typedef struct {
    int    ativo;
    Recorte recorte;
} TileSpr;

typedef struct {
    const char *caminho;
} Icone;

#endif /* TIPOS_H */