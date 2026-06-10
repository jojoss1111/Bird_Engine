#include "input.h"
#include "../Render/renderizador.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_MAX_TECLAS 64
#define INPUT_MAX_BOTOES  8
#define CURSOR_INTERVALO 500

typedef struct {
    SDL_Keycode codigo;
    int         estado;
} EntradaTecla;

static EntradaTecla _teclas_atual   [INPUT_MAX_TECLAS];
static EntradaTecla _teclas_anterior[INPUT_MAX_TECLAS];
static int          _num_teclas_atual    = 0;
static int          _num_teclas_anterior = 0;

static int       _botoes_atual   [INPUT_MAX_BOTOES];
static int       _botoes_anterior[INPUT_MAX_BOTOES];
static MousePosicao _posicao_mouse = {0, 0};

/* ---- helpers internos ---- */

static int _tecla_buscar(const EntradaTecla *tab, int n, SDL_Keycode codigo) {
    for (int i = 0; i < n; i++)
        if (tab[i].codigo == codigo) return i;
    return -1;
}

static void _tecla_setar(EntradaTecla *tab, int *n, SDL_Keycode codigo, int estado) {
    int i = _tecla_buscar(tab, *n, codigo);
    if (i >= 0) { tab[i].estado = estado; return; }
    if (*n >= INPUT_MAX_TECLAS) return;
    tab[*n].codigo = codigo;
    tab[*n].estado = estado;
    (*n)++;
}

/* ---- API pública ---- */

void input_iniciar_frame(void) {
    memcpy(_teclas_anterior, _teclas_atual, _num_teclas_atual * sizeof(EntradaTecla));
    _num_teclas_anterior = _num_teclas_atual;
    memcpy(_botoes_anterior, _botoes_atual, sizeof(_botoes_atual));
}

void input_processar_evento(const SDL_Event *ev) {
    switch (ev->type) {
        case SDL_KEYDOWN:
            if (ev->key.repeat == 0)
                _tecla_setar(_teclas_atual, &_num_teclas_atual, ev->key.keysym.sym, 1);
            break;
        case SDL_KEYUP:
            _tecla_setar(_teclas_atual, &_num_teclas_atual, ev->key.keysym.sym, 0);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (ev->button.button < INPUT_MAX_BOTOES)
                _botoes_atual[ev->button.button] = 1;
            break;
        case SDL_MOUSEBUTTONUP:
            if (ev->button.button < INPUT_MAX_BOTOES)
                _botoes_atual[ev->button.button] = 0;
            break;
        case SDL_MOUSEMOTION:
            _posicao_mouse.x = ev->motion.x;
            _posicao_mouse.y = ev->motion.y;
            break;
        default: break;
    }
}

int tecla_pressionada(SDL_Keycode tecla) {
    int ia = _tecla_buscar(_teclas_atual,    _num_teclas_atual,    tecla);
    int ib = _tecla_buscar(_teclas_anterior, _num_teclas_anterior, tecla);
    return ((ia >= 0) && _teclas_atual[ia].estado) &&
          !((ib >= 0) && _teclas_anterior[ib].estado);
}

int tecla_segurada(SDL_Keycode tecla) {
    int i = _tecla_buscar(_teclas_atual, _num_teclas_atual, tecla);
    return (i >= 0) && _teclas_atual[i].estado;
}

int tecla_soltou(SDL_Keycode tecla) {
    int ia = _tecla_buscar(_teclas_atual,    _num_teclas_atual,    tecla);
    int ib = _tecla_buscar(_teclas_anterior, _num_teclas_anterior, tecla);
    return !((ia >= 0) && _teclas_atual[ia].estado) &&
            ((ib >= 0) && _teclas_anterior[ib].estado);
}

int botao_mouse_pressionado(int botao) {
    if (botao < 0 || botao >= INPUT_MAX_BOTOES) return 0;
    return _botoes_atual[botao] && !_botoes_anterior[botao];
}

int botao_mouse_segurado(int botao) {
    if (botao < 0 || botao >= INPUT_MAX_BOTOES) return 0;
    return _botoes_atual[botao];
}

MousePosicao obter_posicao_mouse(void) {
    SDL_GetMouseState(&_posicao_mouse.x, &_posicao_mouse.y);
    return _posicao_mouse;
}

/* ---- input_string ---- */

void input_string(Janelas *janelas, const Fonte *fonte,
                  const char *prompt,
                  int x, int y,
                  char *saida, int saida_max,
                  int max_chars,
                  uint32_t cor_texto,
                  uint32_t cor_cursor,
                  uint32_t cor_fundo)
{
    (void)cor_cursor;

    int    len        = 0;
    int    cursor_vis = 1;
    Uint32 ultimo_tick = SDL_GetTicks();
    char   exibir[512];

    if (saida_max > 0) saida[0] = '\0';
    if (max_chars <= 0 || max_chars >= saida_max) max_chars = saida_max - 1;

    SDL_StartTextInput();

    for (;;) {
        Uint32 agora = SDL_GetTicks();
        if (agora - ultimo_tick >= CURSOR_INTERVALO) {
            cursor_vis  = !cursor_vis;
            ultimo_tick = agora;
        }

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { SDL_StopTextInput(); return; }

            if (ev.type == SDL_TEXTINPUT) {
                const char *c    = ev.text.text;
                int         clen = (int)strlen(c);
                if (max_chars == 1) {
                    saida[0] = c[0]; saida[1] = '\0'; len = 1;
                } else if (len + clen < max_chars) {
                    memcpy(saida + len, c, clen);
                    len       += clen;
                    saida[len] = '\0';
                }
            }

            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        SDL_StopTextInput();
                        return;
                    case SDLK_BACKSPACE:
                        if (len > 0) {
                            len--;
                            while (len > 0 && (saida[len] & 0xC0) == 0x80) len--;
                            saida[len] = '\0';
                        }
                        break;
                    case SDLK_ESCAPE:
                        saida[0] = '\0'; len = 0;
                        break;
                    default: break;
                }
            }
        }

        limpar_janela(janelas, cor_fundo);

        int plen  = (int)strlen(prompt);
        int total = plen + len + (cursor_vis ? 1 : 0);
        if (total >= (int)sizeof(exibir)) total = (int)sizeof(exibir) - 1;
        memcpy(exibir, prompt, plen);
        memcpy(exibir + plen, saida, len);
        if (cursor_vis) exibir[plen + len] = '_';
        exibir[total] = '\0';

        escrever(janelas, fonte, exibir, x, y, 1.0f, cor_texto);
        apresentar_janela(janelas);
        SDL_Delay(8);
    }
}

int input_int(Janelas *janelas, const Fonte *fonte,
              const char *prompt, int x, int y, int max_chars,
              uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo) {
    char buf[64];
    input_string(janelas, fonte, prompt, x, y, buf, sizeof(buf),
                 max_chars, cor_texto, cor_cursor, cor_fundo);
    return buf[0] ? atoi(buf) : 0;
}

float input_float(Janelas *janelas, const Fonte *fonte,
                  const char *prompt, int x, int y, int max_chars,
                  uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo) {
    char buf[64];
    input_string(janelas, fonte, prompt, x, y, buf, sizeof(buf),
                 max_chars, cor_texto, cor_cursor, cor_fundo);
    return buf[0] ? (float)atof(buf) : 0.0f;
}

char input_char(Janelas *janelas, const Fonte *fonte,
                const char *prompt, int x, int y,
                uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo) {
    char buf[4];
    input_string(janelas, fonte, prompt, x, y, buf, sizeof(buf),
                 1, cor_texto, cor_cursor, cor_fundo);
    return buf[0];
}