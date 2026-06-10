@echo off
set SDL2_DIR=SDL2
set SDL2_MIXER_DIR=SDL2_mixer
set OUT=bird.dll

set SRC=src\FFI\bird.c ^
        src\Render\renderizador.c ^
        src\Render\filtro.c ^
        src\Inputs\input.c ^
        src\Objetos\objetos.c ^
        src\Audio\audio.c ^
        src\Mapas\mapa.c

set INCLUDES=/Isrc /Isrc\include /Isrc\Render\include ^
             /I%SDL2_DIR%\include ^
             /I%SDL2_MIXER_DIR%\include

set LIBS=%SDL2_DIR%\lib\x64\SDL2.lib ^
         %SDL2_DIR%\lib\x64\SDL2main.lib ^
         %SDL2_MIXER_DIR%\lib\x64\SDL2_mixer.lib

set FLAGS=/W3 /Wall /O2 /std:c11 /utf-8 /arch:SSE2 /DSDL_MAIN_HANDLED /LD

cl %FLAGS% %INCLUDES% %SRC% /Fe%OUT% /link %LIBS%

if errorlevel 1 (
    echo.
    echo [ERRO] Build falhou.
    exit /b 1
)

for %%D in (%SDL2_DIR%\SDL2.dll %SDL2_MIXER_DIR%\SDL2_mixer.dll) do (
    if exist %%D (
        copy /Y %%D . >nul
        echo Copiado: %%D
    )
)

echo.
echo [OK] %OUT% gerado com sucesso.