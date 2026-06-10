#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL_mixer.h>
#include <stdint.h>

typedef struct {
    Mix_Music  *dados;
    const char *arquivo;
    int         tocando;
} Musica;

typedef struct {
    Mix_Chunk  *dados;
    const char *arquivo;
    int         canal;
} Som;

int  iniciar_audio    (int frequencia, int canais_mix);
void encerrar_audio   (void);

int  carregar_musica  (Musica *musica, const char *arquivo);
void liberar_musica   (Musica *musica);
void tocar_musica     (Musica *musica, int loops);
void pausar_musica    (void);
void retomar_musica   (void);
void parar_musica     (void);
void volume_musica    (int volume);
void fade_in_musica   (Musica *musica, int loops, int ms);
void fade_out_musica  (int ms);
int  musica_tocando   (void);
int  musica_pausada   (void);

int  carregar_som     (Som *som, const char *arquivo);
void liberar_som      (Som *som);
int  tocar_som        (Som *som, int loops);
void parar_som        (Som *som);
void volume_som       (Som *som, int volume);
void parar_todos_sons (void);
int  canal_tocando    (int canal);

#endif /* AUDIO_H */