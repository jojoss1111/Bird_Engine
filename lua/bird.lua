local ok, native = pcall(require, "engineffi")
if not ok then
    native = require("lua.engineffi")
end
local ffi = native.ffi
local C   = native.C

local bird = {
    ModoTela = {
        JANELA    = 0,
        CHEIO     = 1,
        LETTERBOX = 2,
    },
    TipoFonte = {
        ASCII = 0,
        UTF8  = 1,
    },
    Filtros = {
    color = {
        {  0,   0,   0}, { 29,  43,  83}, {126,  37,  83}, {  0, 135,  81},
        {171,  82,  54}, { 95,  87,  79}, {194, 195, 199}, {255, 241, 232},
        {255,   0,  77}, {255, 163,   0}, {255, 236,  39}, {  0, 228,  54},
        { 41, 173, 255}, {131, 118, 156}, {255, 119, 168}, {255, 204, 170},
    },
    gameboy = {
        { 15,  56,  15}, { 48,  98,  48}, {139, 172,  15}, {155, 188,  15},
    },
    gameboy_pocket = {
        { 40,  40,  40}, { 87,  87,  87}, {168, 168, 168}, {199, 199, 167},
    },
    cga = {
    {  0,   0,   0}, {  0,   0, 170}, {  0, 170,   0}, {  0, 170, 170},
    {170,   0,   0}, {170,   0, 170}, {170,  85,   0}, {170, 170, 170},
    { 85,  85,  85}, { 85,  85, 255}, { 85, 255,  85}, { 85, 255, 255},
    {255,  85,  85}, {255,  85, 255}, {255, 255,  85}, {255, 255, 255},
    },
    cga_brilhante = {
    {  0,   0,   0}, { 20,   0,  60}, {  0,  60,  20}, {  0, 200, 200},
    {180,   0,   0}, {200,   0, 200}, {220, 180,   0}, {200, 200, 200},
    {100, 100, 100}, { 80,  80, 255}, { 80, 255,  80}, {  0, 255, 255},
    {255,  80,  80}, {255,   0, 255}, {255, 255,   0}, {255, 255, 255},
    },
    spectrum = {
        {  0,   0,   0}, {  0,   0, 215}, {215,   0,   0}, {215,   0, 215},
        {  0, 215,   0}, {  0, 215, 215}, {215, 215,   0}, {215, 215, 215},
    },
    c64 = {
        {  0,   0,   0}, {255, 255, 255}, {136,   0,   0}, {170, 255, 238},
        {204,  68, 204}, {  0, 204,  85}, {  0,   0, 170}, {238, 238, 119},
        {221, 136,  85}, {102,  68,   0}, {255, 119, 119}, { 51,  51,  51},
        {119, 119, 119}, {170, 255, 102}, {  0, 136, 255}, {187, 187, 187},
    },
    sweetie16 = {
        { 26,  28,  44}, { 93,  39,  93}, {177,  62,  83}, {239, 125,  87},
        {255, 205, 117}, {167, 240, 112}, { 56, 183, 100}, { 37, 113, 121},
        { 41,  54, 111}, { 59,  93, 201}, { 65, 166, 246}, {115, 239, 247},
        {244, 244, 244}, {148, 176, 194}, { 86, 108, 134}, { 51,  60,  87},
    },
    mono_verde   = {{  2,  14,   2}, { 60, 255,  60}},
    mono_ambar   = {{ 24,   8,   0}, {255, 176,  20}},
    mono_branco  = {{ 10,  10,  15}, {220, 236, 255}},
    mono_azul    = {{  5,   5,  20}, { 60, 100, 255}},
    mono_roxo    = {{ 15,   5,  20}, {180,  60, 255}},
    mono_vermelho= {{ 20,   5,   5}, {255,  60,  60}},
    mono_ciano   = {{  5,  20,  20}, {  0, 255, 238}},
    mono_rosa    = {{ 20,   5,  20}, {255, 120, 240}},
    synthwave = {
        { 11,   2,  28}, { 41,  10,  89}, {101,  14, 133}, {196,   0, 130},
        {255,  20, 147}, {255, 105, 180}, {  0, 255, 255}, {  0, 200, 180},
        { 20, 100, 220}, {100,  50, 200}, {255, 237,  78}, {255, 175,   0},
        {255, 100,  20}, {200,  25,  50}, { 50, 255, 150}, {255, 255, 255},
    },
    por_do_sol = {
        { 20,   5,  20}, { 60,  15,  60}, {120,  20,  80}, {200,  40,  60},
        {255,  80,  30}, {255, 140,  20}, {255, 200,  60}, {255, 240, 180},
    },
    floresta = {
        { 10,  20,   5}, { 30,  61,  10}, { 45, 100,  20}, { 60, 140,  30},
        {100, 180,  50}, {150, 210,  80}, {200, 240, 120}, {230, 255, 180},
    },
    oceano = {
    {  0,   8,  18}, {  0,  28,  58}, {  0,  60, 100}, {  0, 100, 155},
    {  0, 145, 195}, { 30, 185, 220}, {100, 220, 240}, {210, 245, 255},
    },
}
}

local function check(ptr, mensagem)
    if ptr == nil or ptr == ffi.NULL then error(mensagem, 2) end
    return ptr
end

function bird.cor(r, g, b, a)
    return C.bird_cor(r or 0, g or 0, b or 0, a or 255)
end

function bird.janela(largura, altura, fps, titulo, ampliar, modo, icone)
    return check(C.bird_janela_criar(
        largura or 160,
        altura  or 144,
        fps     or 60,
        titulo  or "Birdengine",
        ampliar or 1,
        modo    or bird.ModoTela.JANELA,
        icone
    ), "falha ao criar janela")
end

function bird.modo_tela(janela, modo) C.bird_modo_tela(janela, modo) end
function bird.fechar(janela)          C.bird_janela_destruir(janela) end
function bird.rodando()               return C.bird_rodando() end
function bird.eventos()               C.bird_processar_eventos() end
function bird.delta()                 return C.bird_delta() end
function bird.fundo(janela, cor)      C.bird_cor_de_fundo(janela, cor) end
function bird.limpar(janela, cor)     C.bird_limpar(janela, cor or 0xff000000) end
function bird.apresentar(janela)      C.bird_apresentar(janela) end
function bird.atualizar(janela)       C.bird_atualizar(janela) end
function bird.parar()                 C.bird_parar() end

function bird.key(nome)                return C.bird_keycode(nome) end
function bird.tecla_pressionada(tecla) return C.bird_tecla_pressionada(tecla) end
function bird.tecla_segurada(tecla)    return C.bird_tecla_segurada(tecla) end
function bird.tecla_soltou(tecla)      return C.bird_tecla_soltou(tecla) end
function bird.mouse_pressionado(botao) return C.bird_mouse_pressionado(botao) end
function bird.mouse_segurado(botao)    return C.bird_mouse_segurado(botao) end

function bird.mouse_posicao()
    local x = ffi.new("int[1]")
    local y = ffi.new("int[1]")
    C.bird_mouse_posicao(x, y)
    return x[0], y[0]
end

function bird.input(janela, fonte, tipo, prompt, x, y, max_chars,
                    cor_texto, cor_cursor, cor_fundo)
    prompt     = prompt     or "> "
    x          = x          or 0
    y          = y          or 0
    max_chars  = max_chars  or 0
    cor_texto  = cor_texto  or 0xFFFFFFFF
    cor_cursor = cor_cursor or 0xFFFFFFFF
    cor_fundo  = cor_fundo  or 0xFF000000
    if tipo == "int" then
        return C.bird_input_int(janela, fonte, prompt, x, y, max_chars,
                                cor_texto, cor_cursor, cor_fundo)
    elseif tipo == "float" then
        return C.bird_input_float(janela, fonte, prompt, x, y, max_chars,
                                  cor_texto, cor_cursor, cor_fundo)
    elseif tipo == "char" then
        return string.char(C.bird_input_char(janela, fonte, prompt, x, y,
                                             cor_texto, cor_cursor, cor_fundo))
    else
        return ffi.string(C.bird_input_string(janela, fonte, prompt, x, y,
                          max_chars, cor_texto, cor_cursor, cor_fundo))
    end
end

function bird.atlas(caminho)
    return check(C.bird_atlas_carregar(caminho),
                 "falha ao carregar atlas: " .. tostring(caminho))
end

function bird.destruir_atlas(atlas) C.bird_atlas_destruir(atlas) end

function bird.fonte(atlas, largura_char, altura_char, tipo, cp_inicio)
    return check(C.bird_fonte_criar(
        atlas,
        largura_char or 8,
        altura_char  or 8,
        tipo         or bird.TipoFonte.UTF8,
        cp_inicio    or 32
    ), "falha ao criar fonte")
end

function bird.destruir_fonte(fonte) C.bird_fonte_destruir(fonte) end

function bird.escrever(janela, fonte, texto, x, y, escala, cor)
    C.bird_escrever(janela, fonte, texto,
                    x or 0, y or 0, escala or 1.0, cor or 0xffffffff)
end

function bird.filtro(cores, num_tons)
    assert(type(cores) == "table" and #cores >= 1,
           "filtro: cores deve ser tabela com ao menos 1 entrada")
    local n  = math.min(#cores, 16)
    local rs = ffi.new("uint8_t[16]")
    local gs = ffi.new("uint8_t[16]")
    local bs = ffi.new("uint8_t[16]")
    for i = 1, n do
        local c = cores[i]
        rs[i-1] = c[1] or 0
        gs[i-1] = c[2] or 0
        bs[i-1] = c[3] or 0
    end
    return check(C.bird_filtro_criar(rs, gs, bs, n, num_tons), "falha ao criar filtro")
end

function bird.destruir_filtro(f)
    C.bird_filtro_destruir(f)
end

function bird.filtro_aplicar(janela, f)
    C.bird_filtro_aplicar(janela, f)
end

function bird.filtro_aplicar_regiao(janela, f, x, y, lw, lh)
    C.bird_filtro_aplicar_regiao(janela, f, x, y, lw, lh)
end

function bird.filtro_limpar(f)
    C.bird_filtro_limpar(f)
end

function bird.pixel_separar(janela, tamanho_pixel, forca)
    C.bird_pixel_separar(janela, tamanho_pixel or 2, forca or 0.5)
end

function bird.pixel_separar_regiao(janela, tamanho_pixel, forca, x, y, lw, lh)
    C.bird_pixel_separar_regiao(janela, tamanho_pixel or 2, forca or 0.5, x, y, lw, lh)
end

function bird.monitor_chiado(janela, forca, seed)
    C.bird_monitor_chiado(janela, forca or 0.08, seed or 0)
end

function bird.monitor_chiado_regiao(janela, forca, seed, x, y, lw, lh)
    C.bird_monitor_chiado_regiao(janela, forca or 0.08, seed or 0, x, y, lw, lh)
end

function bird.monitor_limpeza(janela, progresso, largura_faixa, brilho, escurecer)
    C.bird_monitor_limpeza(janela, progresso or 0.0,
                           largura_faixa or 40,
                           brilho        or 0.6,
                           escurecer     or 0.9)
end

function bird.sprite(janela, caminho, frame_w, frame_h,
                     coluna, linha, x, y, dw, dh, angulo)
    return check(C.bird_objeto_sprite(
        janela,
        frame_w, frame_h,
        coluna  or 0, linha  or 0,
        x       or 0, y      or 0,
        angulo  or 0,
        dw or frame_w, dh or frame_h,
        caminho
    ), "falha ao criar sprite: " .. tostring(caminho))
end

function bird.mover(obj, vx, vy)
    C.bird_objeto_mover(obj, vx or 0, vy or 0)
end

function bird.seguir(obj, x, y, vel)
    C.bird_objeto_seguir(obj, x, y, vel or 1.0)
end

function bird.destruir_objeto(obj)
    C.bird_objeto_destruir(obj)
end
function bird.desenhar(obj)                C.bird_objeto_desenhar(obj) end
function bird.posicionar(obj, x, y)        C.bird_objeto_posicionar(obj, x or 0, y or 0) end
function bird.girar(obj, angulo)           C.bird_objeto_girar(obj, angulo or 0) end
function bird.escalar(obj, lw, lh)         C.bird_objeto_escalar(obj, lw, lh or lw) end
function bird.frame(obj, coluna, linha)    C.bird_objeto_frame(obj, coluna or 0, linha or 0) end
function bird.espelhar(obj, espelhado)     C.bird_objeto_espelhar(obj, not not espelhado) end

function bird.posicao_objeto(obj)
    local x = ffi.new("float[1]")
    local y = ffi.new("float[1]")
    C.bird_objeto_posicao(obj, x, y)
    return x[0], y[0]
end

function bird.hitbox(obj, ox, oy, lw, lh)
    local x = ffi.new("int[1]")
    local y = ffi.new("int[1]")
    local w = ffi.new("int[1]")
    local h = ffi.new("int[1]")
    C.bird_objeto_hitbox(obj, ox or 0, oy or 0, lw, lh, x, y, w, h)
    return x[0], y[0], w[0], h[0]
end

function bird.pixel(janela, x, y, cor)
    C.bird_pixel(janela, x, y, cor)
end

function bird.linha(janela, x1, y1, x2, y2, espessura, cor)
    C.bird_linha(janela, x1, y1, x2, y2, espessura or 1, cor)
end

function bird.quadrado(janela, x, y, largura, altura, angulo, cor, preenchido)
    C.bird_quadrado(janela, x, y, largura, altura, angulo or 0, cor, not not preenchido)
end

function bird.triangulo(janela, x, y, largura, altura, angulo, cor, preenchido)
    C.bird_triangulo(janela, x, y, largura, altura, angulo or 0, cor, not not preenchido)
end

function bird.circulo(janela, x, y, raio, cor, preenchido)
    C.bird_circulo(janela, x, y, raio, cor, not not preenchido)
end

function bird.set_camera(janela, x, y) C.bird_set_camera(janela, x or 0, y or 0) end
function bird.reset_camera(janela)     C.bird_reset_camera(janela) end

function bird.obter_camera(janela)
    local x = ffi.new("float[1]")
    local y = ffi.new("float[1]")
    C.bird_obter_camera(janela, x, y)
    return x[0], y[0]
end
function bird.tilespr(c, atlas, lw, lh, coluna, linha)
    -- aceita char ('G') ou número legado (1) sem quebrar
    local b = type(c) == "string" and string.byte(c) or c
    C.bird_tilespr(b, atlas, lw, lh, coluna or 0, linha or 0)
end

-- nova API: sem atlas, tiles registrados via tilespr
function bird.mapa2d_criar(janela, largura, altura, tile_w, tile_h)
    return check(C.bird_mapa2d_criar(janela, largura, altura, tile_w, tile_h),
                 "falha ao criar mapa2d")
end

function bird.destruir_mapa2d(m)  C.bird_mapa2d_destruir(m) end

function bird.mapa_mget(m, cx, cy)
    local b = C.bird_mapa2d_mget(m, cx, cy)
    return b ~= 0 and string.char(b) or nil
end

function bird.mapa_mset(m, cx, cy, c)
    C.bird_mapa2d_mset(m, cx, cy, string.byte(c))
end

function bird.mapa_fget(m, c, flag)
    return C.bird_mapa2d_fget(m, string.byte(c), flag)
end

function bird.mapa_fget_byte(m, c)
    return C.bird_mapa2d_fget_byte(m, string.byte(c))
end

function bird.mapa_fset(m, c, flag, valor)
    C.bird_mapa2d_fset(m, string.byte(c), flag, not not valor)
end

function bird.mapa_fset_byte(m, c, valor)
    C.bird_mapa2d_fset_byte(m, string.byte(c), valor)
end

function bird.desenhar_mapa2d(m, cel_x0, cel_y0, n_cel_x, n_cel_y, scr_x, scr_y)
    C.bird_mapa2d_desenhar(m,
        cel_x0  or 0, cel_y0  or 0,
        n_cel_x or 0, n_cel_y or 0,
        scr_x   or 0, scr_y   or 0)
end

function bird.mapa_layout_tamanho(mapa)
    if type(mapa) == "string" then
        local linhas = {}
        for l in mapa:gmatch("[^\n]+") do linhas[#linhas+1] = l end
        mapa = linhas
    end
    local h = #mapa
    local w = 0
    for _, l in ipairs(mapa) do
        if #l > w then w = #l end
    end
    return w, h
end

function bird.layout_mapa(m, mapa)
    for cy, linha in ipairs(mapa) do
        for cx = 1, #linha do
            bird.mapa_mset(m, cx - 1, cy - 1, linha:sub(cx, cx))
        end
    end
end

function bird.audio_iniciar(frequencia, canais)
    return C.bird_audio_iniciar(frequencia or 44100, canais or 16)
end

function bird.audio_encerrar() C.bird_audio_encerrar() end

function bird.musica(arquivo)
    return check(C.bird_musica_criar(arquivo),
                 "falha ao carregar música: " .. tostring(arquivo))
end

function bird.destruir_musica(musica)           C.bird_musica_destruir(musica) end
function bird.musica_tocar(musica, loops)       C.bird_musica_tocar(musica, loops or -1) end
function bird.musica_pausar()                   C.bird_musica_pausar() end
function bird.musica_retomar()                  C.bird_musica_retomar() end
function bird.musica_parar()                    C.bird_musica_parar() end
function bird.musica_volume(v)                  C.bird_musica_volume(v or 128) end
function bird.musica_fade_in(musica, loops, ms) C.bird_musica_fade_in(musica, loops or -1, ms or 1000) end
function bird.musica_fade_out(ms)               C.bird_musica_fade_out(ms or 1000) end
function bird.musica_tocando()                  return C.bird_musica_tocando() end
function bird.musica_pausada()                  return C.bird_musica_pausada() end

function bird.som(arquivo)
    return check(C.bird_som_criar(arquivo),
                 "falha ao carregar som: " .. tostring(arquivo))
end

function bird.destruir_som(som)     C.bird_som_destruir(som) end
function bird.som_tocar(som, loops) return C.bird_som_tocar(som, loops or 0) end
function bird.som_parar(som)        C.bird_som_parar(som) end
function bird.som_volume(som, v)    C.bird_som_volume(som, v or 128) end
function bird.som_parar_todos()     C.bird_som_parar_todos() end
function bird.canal_tocando(canal)  return C.bird_canal_tocando(canal) end

function bird.rodar(janela, iniciar, rodar, desenhar, cor_fundo)
    cor_fundo = cor_fundo or bird.cor(30, 30, 30)
    if type(iniciar) == "function" then iniciar() end
    while bird.rodando() do
        bird.eventos()
        if type(rodar) == "function" then rodar() end
        if not bird.rodando() then break end
        bird.limpar(janela, cor_fundo)
        if type(desenhar) == "function" then desenhar(cor_fundo) end
        bird.apresentar(janela)
        bird.atualizar(janela)  -- mede delta DEPOIS de tudo, incluindo eventos
    end
    bird.fechar(janela)
end

function bird.definir_anim(obj, frames, fps, angulo, escala_x, escala_y, loop, flip)
    assert(obj ~= nil,
           "definir_anim: objeto inválido")
    assert(type(frames) == "table" and #frames >= 1,
           "definir_anim: frames deve ser tabela com ao menos um {coluna, linha}")

    local anim = {
        obj      = obj,
        frames   = frames,
        fps      = fps      or 8,
        angulo   = angulo   or 0,
        escala_x = escala_x or 1,
        escala_y = escala_y or 1,
        loop     = loop ~= false,
        flip     = flip == true,

        _frame   = 1,
        _tempo   = 0.0,
        _tocando = false,
    }

    if anim.angulo   ~= 0 then bird.girar(obj, anim.angulo) end
    if anim.escala_x ~= 1
    or anim.escala_y ~= 1 then bird.escalar(obj, anim.escala_x, anim.escala_y) end

    return anim
end

function bird.tocar_anim(anim)
    local dt = C.bird_delta()
    anim._tocando = true
    anim._tempo   = anim._tempo + dt

    local dur = 1.0 / anim.fps
    if anim._tempo >= dur then
        anim._tempo = anim._tempo - dur

        local prox = anim._frame + 1
        if prox > #anim.frames then
            prox = anim.loop and 1 or #anim.frames
        end
        anim._frame = prox
    end

    local f = anim.frames[anim._frame]
    bird.frame(anim.obj, f[1], f[2])
    bird.espelhar(anim.obj, anim.flip)
end

function bird.parar_anim(anim, coluna, linha)
    anim._tocando = false
    anim._tempo   = 0.0
    anim._frame   = 1

    local col = coluna ~= nil and coluna or anim.frames[1][1]
    local lin = linha  ~= nil and linha  or anim.frames[1][2]
    bird.frame(anim.obj, col, lin)
end

function bird.anim_tocando(anim)
    return anim._tocando
end

function bird.anim_terminou(anim)
    return not anim.loop and anim._frame == #anim.frames
end

function bird.reiniciar_anim(anim)
    anim._frame   = 1
    anim._tempo   = 0.0
    anim._tocando = true
    local f = anim.frames[1]
    bird.frame(anim.obj, f[1], f[2])
end

function bird.objeto_aabb(obj)
    local x = ffi.new("int[1]")
    local y = ffi.new("int[1]")
    local w = ffi.new("int[1]")
    local h = ffi.new("int[1]")
    C.bird_objeto_aabb(obj, x, y, w, h)
    return x[0], y[0], w[0], h[0]
end

function bird.objeto_aabb_offset(obj, ox, oy, w, h)
    local ox_out = ffi.new("int[1]")
    local oy_out = ffi.new("int[1]")
    local w_out  = ffi.new("int[1]")
    local h_out  = ffi.new("int[1]")
    C.bird_objeto_aabb_offset(obj, ox, oy, w, h, ox_out, oy_out, w_out, h_out)
    return ox_out[0], oy_out[0], w_out[0], h_out[0]
end

function bird.aabb_colidindo(ax, ay, aw, ah, bx, by, bw, bh)
    return C.bird_aabb_colidindo(ax, ay, aw, ah, bx, by, bw, bh)
end

function bird.aabb_resolver(ax, ay, aw, ah, bx, by, bw, bh)
    local dx = ffi.new("float[1]")
    local dy = ffi.new("float[1]")
    local col = C.bird_aabb_resolver(ax, ay, aw, ah, bx, by, bw, bh, dx, dy)
    return col, dx[0], dy[0]
end

function bird.tiles_colisores(hx, hy, hw, hh, m, tile_w, tile_h, map_w, map_h, flag)
    flag = flag or 0
    local mtv_x_total, mtv_y_total = 0, 0
    for cy = 0, map_h - 1 do
        for cx = 0, map_w - 1 do
            local c = bird.mapa_mget(m, cx, cy)
            if c and bird.mapa_fget(m, c, flag) then
                local bx = cx * tile_w
                local by = cy * tile_h
                local col, mx, my = bird.aabb_resolver(
                    hx + mtv_x_total, hy + mtv_y_total, hw, hh,
                    bx, by, tile_w, tile_h
                )
                if col then
                    mtv_x_total = mtv_x_total + mx
                    mtv_y_total = mtv_y_total + my
                end
            end
        end
    end
    return mtv_x_total ~= 0 or mtv_y_total ~= 0, mtv_x_total, mtv_y_total
end

return bird