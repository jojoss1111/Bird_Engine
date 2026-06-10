/* audio.c — BirdEngine */

#include "audio.h"
#include <SDL2/SDL.h>
#include <stdio.h>

int iniciar_audio(int frequencia, int canais_mix) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "[Audio] Erro ao iniciar SDL_AUDIO: %s\n", SDL_GetError());
        return 0;
    }
    if (Mix_OpenAudio(frequencia, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "[Audio] Erro ao abrir mixer: %s\n", Mix_GetError());
        return 0;
    }
    Mix_AllocateChannels(canais_mix);
    return 1;
}

void encerrar_audio(void) {
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int carregar_musica(Musica *musica, const char *arquivo) {
    musica->arquivo = arquivo;
    musica->tocando = 0;
    musica->dados   = Mix_LoadMUS(arquivo);
    if (!musica->dados) {
        fprintf(stderr, "[Audio] Falha ao carregar música '%s': %s\n", arquivo, Mix_GetError());
        return 0;
    }
    return 1;
}

void liberar_musica(Musica *musica) {
    if (musica->dados) {
        Mix_FreeMusic(musica->dados);
        musica->dados   = NULL;
        musica->tocando = 0;
    }
}

void tocar_musica(Musica *musica, int loops) {
    if (!musica->dados) return;
    if (Mix_PlayMusic(musica->dados, loops) < 0)
        fprintf(stderr, "[Audio] Erro ao tocar música: %s\n", Mix_GetError());
    else
        musica->tocando = 1;
}

void pausar_musica(void)  { Mix_PauseMusic();  }
void retomar_musica(void) { Mix_ResumeMusic(); }
void parar_musica(void)   { Mix_HaltMusic();   }

void volume_musica(int volume) {
    if (volume < 0)              volume = 0;
    if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
    Mix_VolumeMusic(volume);
}

void fade_in_musica(Musica *musica, int loops, int ms) {
    if (!musica->dados) return;
    if (Mix_FadeInMusic(musica->dados, loops, ms) < 0)
        fprintf(stderr, "[Audio] Erro fade-in: %s\n", Mix_GetError());
    else
        musica->tocando = 1;
}

void fade_out_musica(int ms) { Mix_FadeOutMusic(ms); }

int musica_tocando(void) { return Mix_PlayingMusic() != 0; }
int musica_pausada(void) { return Mix_PausedMusic()  != 0; }

int carregar_som(Som *som, const char *arquivo) {
    som->arquivo = arquivo;
    som->canal   = -1;
    som->dados   = Mix_LoadWAV(arquivo);
    if (!som->dados) {
        fprintf(stderr, "[Audio] Falha ao carregar som '%s': %s\n", arquivo, Mix_GetError());
        return 0;
    }
    return 1;
}

void liberar_som(Som *som) {
    if (som->dados) {
        Mix_FreeChunk(som->dados);
        som->dados = NULL;
        som->canal = -1;
    }
}

int tocar_som(Som *som, int loops) {
    if (!som->dados) return -1;
    int canal = Mix_PlayChannel(-1, som->dados, loops);
    if (canal < 0)
        fprintf(stderr, "[Audio] Erro ao tocar som '%s': %s\n", som->arquivo, Mix_GetError());
    else
        som->canal = canal;
    return canal;
}

void parar_som(Som *som) {
    if (som->canal >= 0) {
        Mix_HaltChannel(som->canal);
        som->canal = -1;
    }
}

void volume_som(Som *som, int volume) {
    if (!som->dados) return;
    if (volume < 0)              volume = 0;
    if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
    Mix_VolumeChunk(som->dados, volume);
}

void parar_todos_sons(void) { Mix_HaltChannel(-1); }

int canal_tocando(int canal) { return Mix_Playing(canal) != 0; }