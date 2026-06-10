#!/bin/bash
set -e

# ============================================================
# rodar.sh — Build e empacotamento do MeuJogo como AppImage
# ============================================================

APP_NAME="Main"
APP_DIR="${APP_NAME}.AppDir"
APPIMAGE_OUT="${APP_NAME}.AppImage"
DESKTOP_ID="meu-jogo"
LUAJIT_BIN="luajit"

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log()  { echo -e "${GREEN}[OK]${NC} $1"; }
warn() { echo -e "${YELLOW}[AVISO]${NC} $1"; }
erro() { echo -e "${RED}[ERRO]${NC} $1"; exit 1; }

# ============================================================
# 1. Verificações de dependências
# ============================================================
echo ""
echo "==> Verificando dependências..."

command -v appimagetool &>/dev/null || erro "appimagetool não encontrado. Baixe em: https://github.com/AppImage/AppImageKit/releases"
command -v "$LUAJIT_BIN" &>/dev/null || erro "luajit não encontrado no PATH."

[ -f "libbird.so" ]      || erro "libbird.so não encontrada na pasta atual."
[ -f "main.lua" ]        || erro "main.lua não encontrado."
[ -d "lua" ]             || erro "Pasta lua/ não encontrada."
[ -d "assets" ]          || erro "Pasta assets/ não encontrada."
[ -f "assets/Stern.png" ] || warn "assets/Stern.png não encontrado — ícone padrão ausente."

log "Dependências OK."

# ============================================================
# 2. Limpeza de build anterior
# ============================================================
echo ""
echo "==> Limpando build anterior..."

rm -rf "$APP_DIR"
rm -f "$APPIMAGE_OUT"

log "Limpeza concluída."

# ============================================================
# 3. Estrutura do AppDir
# ============================================================
echo ""
echo "==> Criando estrutura AppDir..."

mkdir -p "$APP_DIR/usr/bin"
mkdir -p "$APP_DIR/usr/lib"
mkdir -p "$APP_DIR/usr/share/$DESKTOP_ID"

# Copia arquivos do jogo
cp libbird.so        "$APP_DIR/usr/lib/"
cp main.lua          "$APP_DIR/usr/share/$DESKTOP_ID/"
cp -r lua            "$APP_DIR/usr/share/$DESKTOP_ID/"
cp -r assets         "$APP_DIR/usr/share/$DESKTOP_ID/"

log "Arquivos copiados."

# ============================================================
# 4. AppRun
# ============================================================
cat > "$APP_DIR/AppRun" <<'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="$HERE/usr/lib:$LD_LIBRARY_PATH"
cd "$HERE/usr/share/meu-jogo"
exec luajit main.lua "$@"
EOF

chmod +x "$APP_DIR/AppRun"
log "AppRun criado."

# ============================================================
# 5. Arquivo .desktop (dentro do AppDir — para o appimagetool)
# ============================================================
cat > "$APP_DIR/${DESKTOP_ID}.desktop" <<EOF
[Desktop Entry]
Name=${APP_NAME}
Exec=AppRun %F
Icon=${DESKTOP_ID}
Type=Application
Categories=Game;
Comment=Jogo feito com BirdEngine + LuaJIT
EOF

log ".desktop interno criado."

# ============================================================
# 6. Ícone
# ============================================================
if [ -f "assets/Stern.png" ]; then
    cp assets/Stern.png "$APP_DIR/${DESKTOP_ID}.png"
    log "Ícone copiado."
else
    warn "Sem ícone — o appimagetool pode reclamar."
fi

# ============================================================
# 7. Geração do AppImage
# ============================================================
echo ""
echo "==> Gerando AppImage..."

ARCH=$(uname -m)
ARCH="${ARCH}" appimagetool "$APP_DIR" "$APPIMAGE_OUT"

chmod +x "$APPIMAGE_OUT"
log "AppImage gerado: $APPIMAGE_OUT"

# Remove o AppDir temporário
rm -rf "$APP_DIR"
log "Pasta temporária ($APP_DIR) removida."

# ============================================================
# 8. Instala atalho no menu/desktop do sistema
# ============================================================
echo ""
read -p "Instalar atalho no menu de aplicativos do sistema? [s/N] " RESPOSTA

if [[ "$RESPOSTA" =~ ^[Ss]$ ]]; then
    # Caminho absoluto real do AppImage (sem symlinks)
    APPIMAGE_ABS="$(readlink -f "$APPIMAGE_OUT")"

    # Caminho absoluto do ícone
    if [ -f "assets/Stern.png" ]; then
        ICON_ABS="$(readlink -f assets/Stern.png)"
    else
        ICON_ABS="application-x-executable"  # ícone genérico do sistema
    fi

    # Garante que a pasta existe
    mkdir -p ~/.local/share/applications

    # .desktop do sistema — usa caminho absoluto para o AppImage
    # NUNCA use "Exec=AppRun" aqui; isso só funciona dentro do AppImage montado
    cat > ~/.local/share/applications/${DESKTOP_ID}.desktop <<EOF
[Desktop Entry]
Name=${APP_NAME}
Exec=${APPIMAGE_ABS}
Icon=${ICON_ABS}
Type=Application
Categories=Game;
Comment=Jogo feito com BirdEngine + LuaJIT
StartupNotify=true
Terminal=false
EOF

    chmod 644 ~/.local/share/applications/${DESKTOP_ID}.desktop

    # Atualiza banco de dados de aplicativos
    update-desktop-database ~/.local/share/applications/ 2>/dev/null || true

    log "Atalho instalado: ~/.local/share/applications/${DESKTOP_ID}.desktop"
    log "AppImage aponta para: ${APPIMAGE_ABS}"
fi

# ============================================================
# 9. Resumo final
# ============================================================
echo ""
echo "============================================"
echo -e " ${GREEN}Build concluído com sucesso!${NC}"
echo "============================================"
echo "  AppImage : ./$APPIMAGE_OUT"
echo "  Executar : ./$APPIMAGE_OUT"
echo "============================================"
echo ""