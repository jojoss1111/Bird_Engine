#ifndef OBJETOS_H
#define OBJETOS_H

#include "../tipos.h"

typedef struct {
    float    posX, posY;
    int      tamX, tamY;
    int      coluna, linha;
    int      angulo;
    int      largura_destino;
    int      altura_destino;
    int      espelhado;
    const Atlas *atlas;
    Recorte      recorte;
    Janelas     *janela;
} Objeto2d;

void destruir_objeto      (Objeto2d *obj);

void criar_objeto_sprite  (Objeto2d *obj,
                            int tamX, int tamY,
                            int coluna, int linha,
                            float posX, float posY,
                            int angulo,
                            int largura_destino,
                            int altura_destino,
                            const Atlas *atlas,
                            Janelas *janela);

void mover_objeto         (Objeto2d *obj, float velX, float velY);
void posicionar_objeto    (Objeto2d *obj, float posX, float posY);
void girar_objeto         (Objeto2d *obj, int angulo);
void escalar_objeto       (Objeto2d *obj, int largura_destino, int altura_destino);
void escalar_objeto_uni   (Objeto2d *obj, int tamanho_destino);
void espelhar_objeto      (Objeto2d *obj, int espelhado);
void seguir_objeto        (Objeto2d *obj, float posX2, float posY2, float vel);
void set_frame            (Objeto2d *obj, int coluna, int linha);

/* ---- AABB ---- */

typedef struct {
    int x, y, w, h;
} AABB;

static inline AABB aabb_objeto(const Objeto2d *obj) {
    AABB a = {
        (int)lich_roundf(obj->posX),
        (int)lich_roundf(obj->posY),
        obj->largura_destino,
        obj->altura_destino
    };
    return a;
}

static inline AABB aabb_objeto_offset(const Objeto2d *obj,
                                       int ox, int oy, int w, int h) {
    AABB a = {
        (int)lich_roundf(obj->posX) + ox,
        (int)lich_roundf(obj->posY) + oy,
        (w > 0) ? w : obj->largura_destino - ox,
        (h > 0) ? h : obj->altura_destino  - oy
    };
    return a;
}

static inline int aabb_colidindo(const AABB *a, const AABB *b) {
    return a->x       < b->x + b->w &&
           a->x + a->w > b->x       &&
           a->y       < b->y + b->h &&
           a->y + a->h > b->y;
}

static inline int aabb_resolver(const AABB *a, const AABB *b, int *dx, int *dy) {
    if (!aabb_colidindo(a, b)) { *dx = 0; *dy = 0; return 0; }

    int over_x = (a->x < b->x) ? (a->x + a->w - b->x) : (b->x + b->w - a->x);
    int over_y = (a->y < b->y) ? (a->y + a->h - b->y) : (b->y + b->h - a->y);

    if (over_x <= over_y) { *dx = (a->x < b->x) ? -over_x :  over_x; *dy = 0; }
    else                  { *dx = 0; *dy = (a->y < b->y) ? -over_y :  over_y; }
    return 1;
}

static inline int aabb_resolver_f(const AABB *a, const AABB *b,
                                   float *dx, float *dy) {
    int ix, iy;
    int col = aabb_resolver(a, b, &ix, &iy);
    *dx = (float)ix;
    *dy = (float)iy;
    return col;
}

#endif /* OBJETOS_H */