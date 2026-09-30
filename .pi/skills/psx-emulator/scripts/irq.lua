-- Inspect interrupt latches, masks and CP0 eligibility without acknowledging them.
local m, s, devices = require 'support', require 'snapshot', require 'devices'
m.begin('target', function()
    s.observe(function(state, source)
        m.report('irq.json', {schema = 'psx.runtime-irq/v1', source = source, state = devices.irq(state)})
        m.finish()
    end)
end)
