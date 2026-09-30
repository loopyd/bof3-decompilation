-- Inspect serialized timer state separately from native port-read behavior.
local m, s, devices = require 'support', require 'snapshot', require 'devices'
m.begin('target,timer', function()
    local timer = m.argument('timer') and m.integer('timer', nil, 0, 2)
    s.observe(function(state, source)
        m.report('timers.json', {schema = 'psx.runtime-timers/v1', source = source, state = devices.timers(state, timer)})
        m.finish()
    end)
end)
