#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "../tipos.h"

typedef struct {
    int x, y;
} MousePosicao;

/* ---- teclado ---- */
int          tecla_pressionada       (SDL_Keycode tecla);
int          tecla_segurada          (SDL_Keycode tecla);
int          tecla_soltou            (SDL_Keycode tecla);

/* ---- mouse ---- */
int          botao_mouse_pressionado (int botao);
int          botao_mouse_segurado    (int botao);
MousePosicao obter_posicao_mouse     (void);

/* ---- ciclo de frame ---- */
void input_iniciar_frame    (void);
void input_processar_evento (const SDL_Event *evento);

/* ---- leitura de string/tipos na tela ---- */
void  input_string (Janelas *janelas, const Fonte *fonte,
                    const char *prompt,
                    int x, int y,
                    char *saida, int saida_max,
                    int max_chars,
                    uint32_t cor_texto,
                    uint32_t cor_cursor,
                    uint32_t cor_fundo);

int   input_int    (Janelas *janelas, const Fonte *fonte,
                    const char *prompt, int x, int y, int max_chars,
                    uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo);

float input_float  (Janelas *janelas, const Fonte *fonte,
                    const char *prompt, int x, int y, int max_chars,
                    uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo);

char  input_char   (Janelas *janelas, const Fonte *fonte,
                    const char *prompt, int x, int y,
                    uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo);

#endif /* INPUT_H */