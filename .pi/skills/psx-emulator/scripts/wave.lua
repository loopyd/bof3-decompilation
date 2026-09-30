-- Preserve native f32le stereo samples in an IEEE-float RIFF/WAVE container.
local M = {}
local function integer(value, bytes)
    local parts = {}
    for n = 1, bytes do
        parts[n] = string.char(value % 256)
        value = math.floor(value / 256)
    end
    assert(value == 0, 'RIFF integer overflow')
    return table.concat(parts)
end

function M.encode(pcm)
    assert(type(pcm) == 'string' and #pcm % 8 == 0 and #pcm <= 2646000 * 8,
        'PCM must contain at most 60 seconds of complete 44100 Hz f32le stereo frames')
    local format = integer(3, 2) .. integer(2, 2) .. integer(44100, 4) ..
        integer(352800, 4) .. integer(8, 2) .. integer(32, 2) .. integer(0, 2)
    local chunks = 'fmt ' .. integer(#format, 4) .. format ..
        'fact' .. integer(4, 4) .. integer(#pcm / 8, 4) .. 'data' .. integer(#pcm, 4)
    return 'RIFF' .. integer(4 + #chunks + #pcm, 4) .. 'WAVE' .. chunks .. pcm
end

return M
