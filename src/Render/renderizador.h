#ifndef RENDERIZADOR_H
#define RENDERIZADOR_H
/*
 * renderizador.h — BirdEngine
 * Módulo central de renderização: janela SDL2, framebuffer de software,
 * primitivas 2D, sprites/atlas, fontes bitmap e câmera 2D.
 */

#include "../tipos.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- janela ---- */
int      iniciar_janela    (Janelas *janelas, int largura, int altura, int fps,
                            const char *titulo, int ampliar,
                            ModoTela modo_tela, Icone icone);
void     fechar_janela     (Janelas *janelas);
void     atualizar_janela  (Janelas *janelas);
void     limpar_janela     (Janelas *janelas, uint32_t cor);
void     apresentar_janela (Janelas *janelas);
float    obter_delta       (void);
uint32_t criar_cor         (uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int      rodando           (void);
void     prosesar_eventos  (Janelas *janelas);
void     cor_de_fundo      (Janelas *janelas, uint32_t cor);
void     set_modo_tela     (Janelas *janelas, ModoTela modo);
void     parar_janela      (void);

/* ---- primitivas 2D ---- */
void desenhar_pixel            (Janelas *janelas, int x, int y, uint32_t cor);
void desenhar_linha            (Janelas *janelas, int x1, int y1, int x2, int y2, int espesura, uint32_t cor);
void desenhar_linha_horizontal (Janelas *janelas, int x0, int x1, int y, uint32_t cor);
void desenhar_quadrado         (Janelas *janelas, int x, int y, int largura, int altura, int angulo, uint32_t cor, int prenchido);
void desenhar_triangulo        (Janelas *janelas, int x, int y, int largura, int altura, int angulo, uint32_t cor, int prenchido);
void desenhar_circulo          (Janelas *janelas, int cx, int cy, int raio, uint32_t cor, int prenchido);
void preencher_poligono        (Janelas *janelas, int *vx, int *vy, int n, uint32_t cor);
void rotacionar_ponto          (int cx, int cy, float rad, int px, int py, int *rx, int *ry);

/* ---- atlas / sprites ---- */
int     carregar_atlas  (Atlas *atlas, const char *caminho);
void    liberar_atlas   (Atlas *atlas);
Recorte cortar_atlas    (const Atlas *atlas, int largura_corte, int altura_corte, int coluna, int linha);

void desenhar_sprite (Janelas *janelas, const Recorte *recorte,
                      float x, float y,
                      int largura_destino, int altura_destino,
                      int angulo, int espelhado);

/* overload int por compatibilidade — inline no header */
static inline void desenhar_sprite_i(Janelas *janelas, const Recorte *recorte,
                                     int x, int y,
                                     int largura_destino, int altura_destino,
                                     int angulo, int espelhado) {
    desenhar_sprite(janelas, recorte, (float)x, (float)y,
                    largura_destino, altura_destino, angulo, espelhado);
}

/* ---- fontes bitmap ---- */
void carregar_fonte (Fonte *fonte, const Atlas *atlas,
                     int largura_char, int altura_char,
                     TipoFonte tipo, uint32_t cp_inicio);

void escrever (Janelas *janelas, const Fonte *fonte, const char *texto,
               int x, int y, float escala, uint32_t cor);

/* ---- câmera 2D ---- */
void     set_camera   (Janelas *janelas, float x, float y);
void     reset_camera (Janelas *janelas);
Camera2D obter_camera (const Janelas *janelas);

/* ---- tilespr ---- */
void    definir_tilespr (char c, const Atlas *atlas,
                         int largura_corte, int altura_corte,
                         int coluna, int linha);
Recorte obter_tilespr   (char c);

#ifdef __cplusplus
}
#endif

#endif /* RENDERIZADOR_H */