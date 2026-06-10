/* bird.c - BirdEngine */
#include "../Render/renderizador.h"
#include "../Objetos/objetos.h"
#include "../Inputs/input.h"
#include "../Audio/audio.h"
#include "../Render/filtro.h"
#include "../Mapas/mapa.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Cache de Atlas                                                               */
/* -------------------------------------------------------------------------- */

#define CACHE_ATLAS_MAX 64

typedef struct {
    char   caminho[512];
    Atlas *atlas;
} CacheAtlasEntry;

static CacheAtlasEntry _cache_atlas[CACHE_ATLAS_MAX];
static int             _cache_atlas_n = 0;

static Atlas *_obter_atlas_cache(const char *caminho) {
    if (!caminho) return NULL;
    for (int i = 0; i < _cache_atlas_n; i++) {
        if (strcmp(_cache_atlas[i].caminho, caminho) == 0)
            return _cache_atlas[i].atlas;
    }
    if (_cache_atlas_n >= CACHE_ATLAS_MAX) return NULL;
    Atlas *a = (Atlas *)calloc(1, sizeof(Atlas));
    if (!a) return NULL;
    if (!carregar_atlas(a, caminho)) { free(a); return NULL; }
    strncpy(_cache_atlas[_cache_atlas_n].caminho, caminho, 511);
    _cache_atlas[_cache_atlas_n].caminho[511] = '\0';
    _cache_atlas[_cache_atlas_n].atlas = a;
    _cache_atlas_n++;
    return a;
}

/* janela ativa global para funções sem parâmetro de janela */
static Janelas *_janela_ativa = NULL;

/* -------------------------------------------------------------------------- */
/* Janela                                                                       */
/* -------------------------------------------------------------------------- */

Janelas *bird_janela_criar(int largura, int altura, int fps, const char *titulo,
                           int ampliar, int modo_tela, const char *icone) {
    Janelas *janela = (Janelas *)calloc(1, sizeof(Janelas));
    if (!janela) return NULL;
    Icone icone_janela = {icone};
    if (iniciar_janela(janela, largura, altura, fps, titulo, ampliar,
                       (ModoTela)modo_tela, icone_janela) != 0) {
        free(janela);
        return NULL;
    }
    _janela_ativa = janela;
    return janela;
}

void bird_janela_destruir(Janelas *janela) {
    if (!janela) return;
    fechar_janela(janela);
    if (_janela_ativa == janela) _janela_ativa = NULL;
    free(janela);
}

int   bird_rodando(void)           { return rodando(); }
void  bird_parar(void)             { parar_janela(); }
void  bird_processar_eventos(void) { prosesar_eventos(_janela_ativa); }
float bird_delta(void)             { return obter_delta(); }

void bird_cor_de_fundo(Janelas *janela, uint32_t cor) { if (janela) cor_de_fundo(janela, cor); }
void bird_limpar(Janelas *janela, uint32_t cor)       { if (janela) limpar_janela(janela, cor); }
void bird_apresentar(Janelas *janela)                 { if (janela) apresentar_janela(janela); }
void bird_atualizar(Janelas *janela)                  { if (janela) atualizar_janela(janela); }
void bird_modo_tela(Janelas *janela, int modo_tela)   { if (janela) set_modo_tela(janela, (ModoTela)modo_tela); }

uint32_t bird_cor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return criar_cor(r, g, b, a); }

/* -------------------------------------------------------------------------- */
/* Filtros de Tela                                                              */
/* -------------------------------------------------------------------------- */

Filtro *bird_filtro_criar(uint8_t *rs, uint8_t *gs, uint8_t *bs,
                           int num_cores, int num_tons) {
    if (!rs || !gs || !bs || num_cores < 1) return NULL;
    if (num_cores > FILTRO_MAX_CORES) num_cores = FILTRO_MAX_CORES;
    CorFiltro cores[FILTRO_MAX_CORES];
    for (int i = 0; i < num_cores; i++) {
        cores[i].r = rs[i];
        cores[i].g = gs[i];
        cores[i].b = bs[i];
    }
    Filtro *f = (Filtro *)calloc(1, sizeof(Filtro));
    if (!f) return NULL;
    filtro_construir(f, cores, num_cores, num_tons);
    return f;
}

void bird_filtro_destruir(Filtro *f)                 { free(f); }
void bird_filtro_limpar(Filtro *f)                   { if (f) filtro_limpar(f); }
void bird_filtro_aplicar(Janelas *janela, Filtro *f) { if (janela && f) filtro_aplicar(janela, f); }

void bird_filtro_aplicar_regiao(Janelas *janela, Filtro *f,
                                 int x, int y, int largura, int altura) {
    if (janela && f) filtro_aplicar_regiao(janela, f, x, y, largura, altura);
}

void bird_pixel_separar(Janelas *janela, int tamanho_pixel, float forca) {
    if (janela) pixel_separar(janela, tamanho_pixel, forca);
}

void bird_pixel_separar_regiao(Janelas *janela, int tamanho_pixel, float forca,
                                int x, int y, int largura, int altura) {
    if (janela) pixel_separar_regiao(janela, tamanho_pixel, forca, x, y, largura, altura);
}

void bird_monitor_chiado(Janelas *janela, float forca, uint32_t seed) {
    if (janela) monitor_chiado(janela, forca, seed);
}

void bird_monitor_chiado_regiao(Janelas *janela, float forca, uint32_t seed,
                                 int x, int y, int largura, int altura) {
    if (janela) monitor_chiado_regiao(janela, forca, seed, x, y, largura, altura);
}

void bird_monitor_limpeza(Janelas *janela, float progresso,
                           int largura_faixa, float brilho, float escurecer) {
    if (janela) monitor_limpeza(janela, progresso, largura_faixa, brilho, escurecer);
}

/* -------------------------------------------------------------------------- */
/* Input - Teclado e Mouse                                                      */
/* -------------------------------------------------------------------------- */

int  bird_keycode(const char *nome)    { return nome ? SDL_GetKeyFromName(nome) : SDLK_UNKNOWN; }
int  bird_tecla_pressionada(int tecla) { return tecla_pressionada((SDL_Keycode)tecla); }
int  bird_tecla_segurada(int tecla)    { return tecla_segurada((SDL_Keycode)tecla); }
int  bird_tecla_soltou(int tecla)      { return tecla_soltou((SDL_Keycode)tecla); }
int  bird_mouse_pressionado(int botao) { return botao_mouse_pressionado(botao); }
int  bird_mouse_segurado(int botao)    { return botao_mouse_segurado(botao); }

void bird_mouse_posicao(int *x, int *y) {
    MousePosicao pos = obter_posicao_mouse();
    if (x) *x = pos.x;
    if (y) *y = pos.y;
}

/* -------------------------------------------------------------------------- */
/* Input - Campos de Texto                                                      */
/* -------------------------------------------------------------------------- */

const char *bird_input_string(Janelas *janela, Fonte *fonte, const char *prompt,
                               int x, int y, int max_chars,
                               uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo) {
    static char resultado[1024];
    resultado[0] = '\0';
    if (!janela || !fonte) return resultado;
    input_string(janela, fonte, prompt ? prompt : "",
                 x, y, resultado, (int)sizeof(resultado), max_chars,
                 cor_texto, cor_cursor, cor_fundo);
    return resultado;
}

int bird_input_int(Janelas *janela, Fonte *fonte, const char *prompt,
                   int x, int y, int max_chars,
                   uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo) {
    if (!janela || !fonte) return 0;
    return input_int(janela, fonte, prompt ? prompt : "",
                     x, y, max_chars, cor_texto, cor_cursor, cor_fundo);
}

float bird_input_float(Janelas *janela, Fonte *fonte, const char *prompt,
                        int x, int y, int max_chars,
                        uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo) {
    if (!janela || !fonte) return 0.0f;
    return input_float(janela, fonte, prompt ? prompt : "",
                       x, y, max_chars, cor_texto, cor_cursor, cor_fundo);
}

char bird_input_char(Janelas *janela, Fonte *fonte, const char *prompt,
                      int x, int y,
                      uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo) {
    if (!janela || !fonte) return '\0';
    return input_char(janela, fonte, prompt ? prompt : "",
                      x, y, cor_texto, cor_cursor, cor_fundo);
}

/* -------------------------------------------------------------------------- */
/* Atlas e Fonte                                                                */
/* -------------------------------------------------------------------------- */

Atlas *bird_atlas_carregar(const char *caminho) { return _obter_atlas_cache(caminho); }

void bird_atlas_destruir(Atlas *atlas) {
    if (!atlas) return;
    for (int i = 0; i < _cache_atlas_n; i++) {
        if (_cache_atlas[i].atlas == atlas) {
            liberar_atlas(atlas);
            free(atlas);
            _cache_atlas[i] = _cache_atlas[--_cache_atlas_n];
            return;
        }
    }
}

Fonte *bird_fonte_criar(Atlas *atlas, int largura_char, int altura_char,
                        int tipo, uint32_t cp_inicio) {
    if (!atlas) return NULL;
    Fonte *fonte = (Fonte *)calloc(1, sizeof(Fonte));
    if (!fonte) return NULL;
    carregar_fonte(fonte, atlas, largura_char, altura_char, (TipoFonte)tipo, cp_inicio);
    return fonte;
}

void bird_fonte_destruir(Fonte *fonte) { free(fonte); }

void bird_escrever(Janelas *janela, Fonte *fonte, const char *texto,
                   int x, int y, float escala, uint32_t cor) {
    if (janela && fonte) escrever(janela, fonte, texto, x, y, escala, cor);
}

/* -------------------------------------------------------------------------- */
/* Objetos 2D                                                                   */
/* -------------------------------------------------------------------------- */

Objeto2d *bird_objeto_sprite(Janelas *janela, int tamX, int tamY,
                              int coluna, int linha,
                              float posX, float posY, int angulo,
                              int largura_destino, int altura_destino,
                              const char *sprite) {
    if (!janela) return NULL;
    Atlas *atlas = _obter_atlas_cache(sprite);
    if (!atlas) return NULL;
    Objeto2d *obj = (Objeto2d *)calloc(1, sizeof(Objeto2d));
    if (!obj) return NULL;
    criar_objeto_sprite(obj, tamX, tamY, coluna, linha, posX, posY, angulo,
                        largura_destino, altura_destino, atlas, janela);
    return obj;
}

void bird_objeto_destruir(Objeto2d *obj) {
    if (!obj) return;
    destruir_objeto(obj);
    free(obj);
}

void bird_objeto_desenhar(Objeto2d *obj) {
    if (obj) desenhar_sprite(obj->janela, &obj->recorte,
                             obj->posX, obj->posY,
                             obj->largura_destino, obj->altura_destino,
                             obj->angulo, obj->espelhado);
}
void bird_objeto_mover(Objeto2d *obj, float vx, float vy)    { if (obj) mover_objeto(obj, vx, vy); }
void bird_objeto_posicionar(Objeto2d *obj, float x, float y) { if (obj) posicionar_objeto(obj, x, y); }
void bird_objeto_girar(Objeto2d *obj, int angulo)             { if (obj) girar_objeto(obj, angulo); }
void bird_objeto_escalar(Objeto2d *obj, int lw, int lh)       { if (obj) escalar_objeto(obj, lw, lh); }
void bird_objeto_espelhar(Objeto2d *obj, int e)               { if (obj) espelhar_objeto(obj, e); }
void bird_objeto_frame(Objeto2d *obj, int col, int lin)       { if (obj) set_frame(obj, col, lin); }

void bird_objeto_posicao(Objeto2d *obj, float *x, float *y) {
    if (!obj) return;
    if (x) *x = obj->posX;
    if (y) *y = obj->posY;
}

void bird_objeto_seguir(Objeto2d *obj, float posX2, float posY2, float vel) {
    if (obj) seguir_objeto(obj, posX2, posY2, vel);
}

/* -------------------------------------------------------------------------- */
/* Desenho Primitivo                                                            */
/* -------------------------------------------------------------------------- */

void bird_pixel(Janelas *janela, int x, int y, uint32_t cor) {
    if (janela) desenhar_pixel(janela, x, y, cor);
}

void bird_linha(Janelas *janela, int x1, int y1, int x2, int y2,
                int espessura, uint32_t cor) {
    if (janela) desenhar_linha(janela, x1, y1, x2, y2, espessura, cor);
}

void bird_quadrado(Janelas *janela, int x, int y, int largura, int altura,
                   int angulo, uint32_t cor, int preenchido) {
    if (janela) desenhar_quadrado(janela, x, y, largura, altura, angulo, cor, preenchido);
}

void bird_triangulo(Janelas *janela, int x, int y, int largura, int altura,
                    int angulo, uint32_t cor, int preenchido) {
    if (janela) desenhar_triangulo(janela, x, y, largura, altura, angulo, cor, preenchido);
}

void bird_circulo(Janelas *janela, int x, int y, int raio, uint32_t cor, int preenchido) {
    if (janela) desenhar_circulo(janela, x, y, raio, cor, preenchido);
}

/* -------------------------------------------------------------------------- */
/* Camera 2D                                                                    */
/* -------------------------------------------------------------------------- */

void bird_set_camera(Janelas *janela, float x, float y) { if (janela) set_camera(janela, x, y); }
void bird_reset_camera(Janelas *janela)                  { if (janela) reset_camera(janela); }

void bird_obter_camera(Janelas *janela, float *x, float *y) {
    if (!janela) return;
    Camera2D cam = obter_camera(janela);
    if (x) *x = cam.x;
    if (y) *y = cam.y;
}

/* -------------------------------------------------------------------------- */
/* TileSpr                                                                      */
/* -------------------------------------------------------------------------- */

void bird_tilespr(char c, Atlas *atlas, int lw, int lh, int col, int lin) {
    if (atlas) definir_tilespr(c, atlas, lw, lh, col, lin);
}

/* -------------------------------------------------------------------------- */
/* Mapa 2D                                                                      */
/* -------------------------------------------------------------------------- */

Mapa *bird_mapa2d_criar(Janelas *janela, int largura, int altura,
                         int tile_w, int tile_h) {
    if (!janela) return NULL;
    Mapa *m = (Mapa *)calloc(1, sizeof(Mapa));
    if (!m) return NULL;
    map(m, janela, largura, altura, tile_w, tile_h);
    return m;
}

void bird_mapa2d_destruir(Mapa *m) { free(m); }

void bird_mapa2d_tile(Mapa *m, int cx, int cy, char c) { if (m) tile(m, cx, cy, c); }

char bird_mapa2d_mget(Mapa *m, int cx, int cy) { return m ? mget(m, cx, cy) : 0; }

void bird_mapa2d_mset(Mapa *m, int cx, int cy, char c) { if (m) mset(m, cx, cy, c); }

int     bird_mapa2d_fget(Mapa *m, char c, int flag)        { return m ? fget(m, c, flag) : 0; }
uint8_t bird_mapa2d_fget_byte(Mapa *m, char c)             { return m ? fget_byte(m, c) : 0; }
void    bird_mapa2d_fset(Mapa *m, char c, int flag, int v) { if (m) fset(m, c, flag, v); }
void    bird_mapa2d_fset_byte(Mapa *m, char c, uint8_t v)  { if (m) fset_byte(m, c, v); }

void bird_mapa2d_desenhar(Mapa *m, int cel_x0, int cel_y0,
                           int n_cel_x, int n_cel_y,
                           int scr_x, int scr_y) {
    if (m) desenhar_mapa2d(m, cel_x0, cel_y0, n_cel_x, n_cel_y, scr_x, scr_y);
}

/* -------------------------------------------------------------------------- */
/* Audio - Musica                                                               */
/* -------------------------------------------------------------------------- */

int  bird_audio_iniciar(int frequencia, int canais_mix) { return iniciar_audio(frequencia, canais_mix); }
void bird_audio_encerrar(void)                           { encerrar_audio(); }

Musica *bird_musica_criar(const char *arquivo) {
    Musica *musica = (Musica *)calloc(1, sizeof(Musica));
    if (!musica) return NULL;
    if (!carregar_musica(musica, arquivo)) { free(musica); return NULL; }
    return musica;
}

void bird_musica_destruir(Musica *musica) {
    if (!musica) return;
    liberar_musica(musica);
    free(musica);
}

void bird_musica_tocar(Musica *musica, int loops) { if (musica) tocar_musica(musica, loops); }
void bird_musica_pausar(void)                      { pausar_musica(); }
void bird_musica_retomar(void)                     { retomar_musica(); }
void bird_musica_parar(void)                       { parar_musica(); }
void bird_musica_volume(int volume)                { volume_musica(volume); }
void bird_musica_fade_out(int ms)                  { fade_out_musica(ms); }
int  bird_musica_tocando(void)                     { return musica_tocando(); }
int  bird_musica_pausada(void)                     { return musica_pausada(); }

void bird_musica_fade_in(Musica *musica, int loops, int ms) {
    if (musica) fade_in_musica(musica, loops, ms);
}

/* -------------------------------------------------------------------------- */
/* Audio - Sons                                                                 */
/* -------------------------------------------------------------------------- */

Som *bird_som_criar(const char *arquivo) {
    Som *som = (Som *)calloc(1, sizeof(Som));
    if (!som) return NULL;
    if (!carregar_som(som, arquivo)) { free(som); return NULL; }
    return som;
}

void bird_som_destruir(Som *som) {
    if (!som) return;
    liberar_som(som);
    free(som);
}

int  bird_som_tocar(Som *som, int loops) { return som ? tocar_som(som, loops) : -1; }
void bird_som_parar(Som *som)            { if (som) parar_som(som); }
void bird_som_volume(Som *som, int v)    { if (som) volume_som(som, v); }
void bird_som_parar_todos(void)          { parar_todos_sons(); }
int  bird_canal_tocando(int canal)       { return canal_tocando(canal); }

/* -------------------------------------------------------------------------- */
/* Colisao AABB                                                                 */
/* -------------------------------------------------------------------------- */

void bird_objeto_aabb(Objeto2d *obj, int *x, int *y, int *w, int *h) {
    if (!obj) return;
    AABB a = aabb_objeto(obj);
    if (x) *x = a.x;
    if (y) *y = a.y;
    if (w) *w = a.w;
    if (h) *h = a.h;
}

void bird_objeto_aabb_offset(Objeto2d *obj, int ox, int oy, int w, int h,
                              int *ox_out, int *oy_out, int *w_out, int *h_out) {
    if (!obj) return;
    AABB a = aabb_objeto_offset(obj, ox, oy, w, h);
    if (ox_out) *ox_out = a.x;
    if (oy_out) *oy_out = a.y;
    if (w_out)  *w_out  = a.w;
    if (h_out)  *h_out  = a.h;
}

int bird_aabb_colidindo(int ax, int ay, int aw, int ah,
                         int bx, int by, int bw, int bh) {
    AABB a = {ax, ay, aw, ah};
    AABB b = {bx, by, bw, bh};
    return aabb_colidindo(&a, &b);
}

int bird_aabb_resolver(int ax, int ay, int aw, int ah,
                        int bx, int by, int bw, int bh,
                        float *dx, float *dy) {
    AABB a = {ax, ay, aw, ah};
    AABB b = {bx, by, bw, bh};
    float fdx = 0.0f, fdy = 0.0f;
    int col = aabb_resolver_f(&a, &b, &fdx, &fdy);
    if (dx) *dx = fdx;
    if (dy) *dy = fdy;
    return col;
}

void bird_objeto_hitbox(Objeto2d *obj, int ox, int oy, int w, int h,
                        int *x_out, int *y_out, int *w_out, int *h_out) {
    if (!obj) return;
    int x = (int)lich_roundf(obj->posX) + ox;
    int y = (int)lich_roundf(obj->posY) + oy;
    if (x_out) *x_out = x;
    if (y_out) *y_out = y;
    if (w_out) *w_out = w;
    if (h_out) *h_out = h;
}