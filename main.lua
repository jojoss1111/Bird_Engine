local bird = require("lua.bird")
local janela = bird.janela(320, 240, 30, "Meu Jogo", 3, bird.ModoTela.JANELA)

local function _iniciar_()

end

local function _rodar_()

end

local function desenhar()

end

bird.rodar(janela, _iniciar_, _rodar_, desenhar, bird.cor(20, 20, 30))
