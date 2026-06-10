--engineffi.lua - Birdengine
local ffi = require("ffi")
ffi.cdef[[
typedef struct Janelas    Janelas;
typedef struct Atlas      Atlas;
typedef struct Fonte      Fonte;
typedef struct Objeto2d   Objeto2d;
typedef struct Musica     Musica;
typedef struct Som        Som;
typedef struct Mapa       Mapa;


Janelas *bird_janela_criar(int largura, int altura, int fps, const char *titulo,
                           int ampliar, int modo_tela, const char *icone);
void bird_janela_destruir(Janelas *janela);
bool bird_rodando(void);
void bird_parar(void);
void bird_processar_eventos(void);
float bird_delta(void);
uint32_t bird_cor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void bird_cor_de_fundo(Janelas *janela, uint32_t cor);
void bird_limpar(Janelas *janela, uint32_t cor);
void bird_apresentar(Janelas *janela);
void bird_atualizar(Janelas *janela);
void bird_modo_tela(Janelas *janela, int modo_tela);

typedef struct Filtro Filtro;
Filtro *bird_filtro_criar(uint8_t *rs, uint8_t *gs, uint8_t *bs,
                           int num_cores, int num_tons);
void bird_filtro_destruir(Filtro *f);
void bird_filtro_aplicar(Janelas *janela, Filtro *f);
void bird_filtro_aplicar_regiao(Janelas *janela, Filtro *f,
                                 int x, int y, int largura, int altura);
void bird_filtro_limpar(Filtro *f);
void bird_pixel_separar(Janelas *janela, int tamanho_pixel, float forca);
void bird_pixel_separar_regiao(Janelas *janela, int tamanho_pixel, float forca,
                                int x, int y, int largura, int altura);
void bird_monitor_chiado(Janelas *janela, float forca, uint32_t seed);
void bird_monitor_chiado_regiao(Janelas *janela, float forca, uint32_t seed,
                                 int x, int y, int largura, int altura);
void bird_monitor_limpeza(Janelas *janela, float progresso,
                           int largura_faixa, float brilho, float escurecer);

int  bird_keycode(const char *nome);
bool bird_tecla_pressionada(int tecla);
bool bird_tecla_segurada(int tecla);
bool bird_tecla_soltou(int tecla);
bool bird_mouse_pressionado(int botao);
bool bird_mouse_segurado(int botao);
void bird_mouse_posicao(int *x, int *y);
const char *bird_input_string(Janelas *janela, Fonte *fonte, const char *prompt,
                               int x, int y, int max_chars,
                               uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo);
int   bird_input_int  (Janelas *janela, Fonte *fonte, const char *prompt,
                       int x, int y, int max_chars,
                       uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo);
float bird_input_float(Janelas *janela, Fonte *fonte, const char *prompt,
                       int x, int y, int max_chars,
                       uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo);
char  bird_input_char (Janelas *janela, Fonte *fonte, const char *prompt,
                       int x, int y,
                       uint32_t cor_texto, uint32_t cor_cursor, uint32_t cor_fundo);

Atlas *bird_atlas_carregar(const char *caminho);
void   bird_atlas_destruir(Atlas *atlas);

Fonte *bird_fonte_criar(Atlas *atlas, int largura_char, int altura_char,
                        int tipo, uint32_t cp_inicio);
void bird_fonte_destruir(Fonte *fonte);
void bird_escrever(Janelas *janela, Fonte *fonte, const char *texto,
                   int x, int y, float escala, uint32_t cor);

Objeto2d *bird_objeto_sprite(Janelas *janela, int tamX, int tamY,
                              int coluna, int linha,
                              float posX, float posY, int angulo,
                              int largura_destino, int altura_destino,
                              const char *sprite);
void bird_objeto_destruir(Objeto2d *obj);
void bird_objeto_desenhar(Objeto2d *obj);
void bird_objeto_mover(Objeto2d *obj, float velX, float velY);
void bird_objeto_posicionar(Objeto2d *obj, float posX, float posY);
void bird_objeto_girar(Objeto2d *obj, int angulo);
void bird_objeto_escalar(Objeto2d *obj, int largura_destino, int altura_destino);
void bird_objeto_espelhar(Objeto2d *obj, bool espelhado);
void bird_objeto_frame(Objeto2d *obj, int coluna, int linha);
void bird_objeto_posicao(Objeto2d *obj, float *x, float *y);
void bird_objeto_seguir(Objeto2d *obj, float posX2, float posY2, float vel);

void bird_pixel(Janelas *janela, int x, int y, uint32_t cor);
void bird_linha(Janelas *janela, int x1, int y1, int x2, int y2,
                int espessura, uint32_t cor);
void bird_quadrado(Janelas *janela, int x, int y, int largura, int altura,
                   int angulo, uint32_t cor, bool preenchido);
void bird_triangulo(Janelas *janela, int x, int y, int largura, int altura,
                    int angulo, uint32_t cor, bool preenchido);
void bird_circulo(Janelas *janela, int x, int y, int raio, uint32_t cor,
                  bool preenchido);

void bird_set_camera(Janelas *janela, float x, float y);
void bird_reset_camera(Janelas *janela);
void bird_obter_camera(Janelas *janela, float *x, float *y);

void bird_tilespr(char c, Atlas *atlas, int largura_corte, int altura_corte,
                  int coluna, int linha);

Mapa *bird_mapa2d_criar(Janelas *janela, int largura, int altura,
                         int tile_w, int tile_h);
void    bird_mapa2d_destruir(Mapa *m);
void    bird_mapa2d_tile(Mapa *m, int cx, int cy, char c);
char    bird_mapa2d_mget(Mapa *m, int cx, int cy);
void    bird_mapa2d_mset(Mapa *m, int cx, int cy, char c);
bool    bird_mapa2d_fget(Mapa *m, char c, int flag);
uint8_t bird_mapa2d_fget_byte(Mapa *m, char c);
void    bird_mapa2d_fset(Mapa *m, char c, int flag, bool valor);
void    bird_mapa2d_fset_byte(Mapa *m, char c, uint8_t valor);
void    bird_mapa2d_desenhar(Mapa *m, int cel_x0, int cel_y0,
                              int n_cel_x, int n_cel_y,
                              int scr_x, int scr_y);
                              
bool bird_audio_iniciar(int frequencia, int canais_mix);
void bird_audio_encerrar(void);

Musica *bird_musica_criar(const char *arquivo);
void    bird_musica_destruir(Musica *musica);
void    bird_musica_tocar(Musica *musica, int loops);
void    bird_musica_pausar(void);
void    bird_musica_retomar(void);
void    bird_musica_parar(void);
void    bird_musica_volume(int volume);
void    bird_musica_fade_in(Musica *musica, int loops, int ms);
void    bird_musica_fade_out(int ms);
bool    bird_musica_tocando(void);
bool    bird_musica_pausada(void);

Som *bird_som_criar(const char *arquivo);
void bird_som_destruir(Som *som);
int  bird_som_tocar(Som *som, int loops);
void bird_som_parar(Som *som);
void bird_som_volume(Som *som, int volume);
void bird_som_parar_todos(void);
bool bird_canal_tocando(int canal);

float    lich_absf  (float v);
float    lich_minf  (float a, float b);
float    lich_maxf  (float a, float b);
float    lich_clampf(float v, float mn, float mx);

float    lich_floorf(float v);
float    lich_ceilf (float v);
float    lich_roundf(float v);
float    lich_truncf(float v);
float    lich_lerpf (float a, float b, float t);
float    lich_fmodf (float x, float y);

float    lich_rsqrtf(float v);
float    lich_sqrtf (float v);

void     lich_lut_init(void);

float    lich_powf(float a, float b);

float    lich_sinf  (float ang);
float    lich_cosf  (float ang);
void     lich_sincosf(float ang, float *s, float *c);
float    lich_tanf  (float ang);
float    lich_atanf (float x);
float    lich_atan2f(float y, float x);

int32_t  lich_fp_de_float  (float v);
int32_t  lich_fp_de_frac   (float v);
float    lich_fp_para_float(int32_t fp);
int32_t  lich_fp_soma      (int32_t a, int32_t b);
int32_t  lich_fp_sub       (int32_t a, int32_t b);
int32_t  lich_fp_mul       (int32_t a, int32_t b);
int32_t  lich_fp_div       (int32_t a, int32_t b);
int32_t  lich_fp_lerp      (int32_t a, int32_t b, int32_t t);
int32_t  lich_fp_floor     (int32_t fp);
int32_t  lich_fp_ceil      (int32_t fp);
int32_t  lich_fp_round     (int32_t fp);
int32_t  lich_fp_frac      (int32_t fp);
int32_t  lich_fp_abs       (int32_t fp);
int32_t  lich_fp_min       (int32_t a, int32_t b);
int32_t  lich_fp_max       (int32_t a, int32_t b);
int32_t  lich_fp_clamp     (int32_t v, int32_t mn, int32_t mx);
int32_t  lich_fp_wrap_pot2 (int32_t fp, int32_t mascara);
int32_t  lich_fp_mod       (int32_t fp, int32_t periodo);

int32_t  lich_iabs      (int32_t v);
int32_t  lich_imin      (int32_t a, int32_t b);
int32_t  lich_imax      (int32_t a, int32_t b);
int32_t  lich_iclamp    (int32_t v, int32_t mn, int32_t mx);
uint32_t lich_isqrt     (uint32_t n);
int      lich_ilog2     (uint32_t n);
int      lich_eh_pot2   (uint32_t n);
uint32_t lich_prox_pot2 (uint32_t n);
uint32_t lich_udiv_round(uint32_t a, uint32_t b);
int32_t  lich_lerp_i    (int32_t a, int32_t b, int32_t t);
void bird_objeto_aabb(Objeto2d *obj, int *x, int *y, int *w, int *h);
void bird_objeto_aabb_offset(Objeto2d *obj, int ox, int oy, int w, int h,
                              int *ox_out, int *oy_out, int *w_out, int *h_out);
bool bird_aabb_colidindo(int ax, int ay, int aw, int ah,
                          int bx, int by, int bw, int bh);
bool bird_aabb_resolver(int ax, int ay, int aw, int ah,
                         int bx, int by, int bw, int bh,
                         float *dx, float *dy);
void bird_objeto_hitbox(Objeto2d *obj, int ox, int oy, int w, int h,
                         int *x_out, int *y_out, int *w_out, int *h_out);
]]

local function this_dir()
    local info = debug.getinfo(1, "S").source
    local path = info:sub(1, 1) == "@" and info:sub(2) or info
    return path:match("^(.*[/\\])") or ""
end

local dir = this_dir()
local candidates = {
    "bird",
    "./libbird.so",
    dir .. "libbird.so",
    dir .. "../libbird.so",
}

local lib
local last_error
for _, path in ipairs(candidates) do
    local ok, result = pcall(ffi.load, path)
    if ok then
        lib = result
        break
    end
    last_error = result
end

if not lib then
    error("nao foi possivel carregar bird.o: " .. tostring(last_error))
end

return {
    ffi = ffi,
    C   = lib,   
}