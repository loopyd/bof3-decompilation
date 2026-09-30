-- Inspect decoded DMA channel state from a live boundary or saved state.
local m, s, devices = require 'support', require 'snapshot', require 'devices'
m.begin('target,channel', function()
    local channel = m.argument('channel') and m.integer('channel', nil, 0, 6)
    s.observe(function(state, source)
        m.report('dma.json', {schema = 'psx.runtime-dma/v1', source = source, state = devices.dma(state, channel)})
        m.finish()
    end)
end)
