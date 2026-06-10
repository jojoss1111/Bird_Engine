/* objetos.c — BirdEngine */

#include "objetos.h"
#include "../Render/renderizador.h"

static void _atualizar_recorte(Objeto2d *obj) {
    obj->recorte = cortar_atlas(obj->atlas, obj->tamX, obj->tamY,
                                obj->coluna, obj->linha);
}

void criar_objeto_sprite(Objeto2d *obj,
                          int tamX, int tamY,
                          int coluna, int linha,
                          float posX, float posY,
                          int angulo,
                          int largura_destino,
                          int altura_destino,
                          const Atlas *atlas,
                          Janelas *janela)
{
    obj->tamX            = tamX;
    obj->tamY            = tamY;
    obj->coluna          = coluna;
    obj->linha           = linha;
    obj->posX            = posX;
    obj->posY            = posY;
    obj->angulo          = angulo % 360;
    obj->largura_destino = (largura_destino <= 0) ? tamX : largura_destino;
    obj->altura_destino  = (altura_destino  <= 0) ? tamY : altura_destino;
    obj->espelhado       = 0;
    obj->atlas           = atlas;
    obj->janela          = janela;
    _atualizar_recorte(obj);
}

void destruir_objeto(Objeto2d *obj) {
    Recorte vazio = { NULL, 0, 0, 0 };
    obj->recorte = vazio;
    obj->atlas   = NULL;
    obj->janela  = NULL;
}

void mover_objeto(Objeto2d *obj, float velX, float velY) {
    obj->posX += velX;
    obj->posY += velY;
}

void posicionar_objeto(Objeto2d *obj, float posX, float posY) {
    obj->posX = posX;
    obj->posY = posY;
}

void girar_objeto(Objeto2d *obj, int angulo) {
    obj->angulo = angulo % 360;
    if (obj->angulo < 0) obj->angulo += 360;
}

void escalar_objeto(Objeto2d *obj, int largura_destino, int altura_destino) {
    obj->largura_destino = (largura_destino <= 0) ? obj->tamX : largura_destino;
    obj->altura_destino  = (altura_destino  <= 0) ? obj->tamY : altura_destino;
}

void escalar_objeto_uni(Objeto2d *obj, int tamanho_destino) {
    escalar_objeto(obj, tamanho_destino, tamanho_destino);
}

void espelhar_objeto(Objeto2d *obj, int espelhado) {
    obj->espelhado = espelhado;
}

void seguir_objeto(Objeto2d *obj, float posX2, float posY2, float vel) {
    float dx   = posX2 - obj->posX;
    float dy   = posY2 - obj->posY;
    float len2 = dx * dx + dy * dy;
    if (len2 < 1.0f) return;

    float inv   = lich_rsqrtf(len2);
    float dist  = len2 * inv;
    float passo = lich_minf(vel, dist);

    mover_objeto(obj, dx * inv * passo, dy * inv * passo);
}

void set_frame(Objeto2d *obj, int coluna, int linha) {
    obj->coluna = coluna;
    obj->linha  = linha;
    _atualizar_recorte(obj);
}