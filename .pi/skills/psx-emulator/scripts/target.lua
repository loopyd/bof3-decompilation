-- Capture before a chosen instruction or immediately after restoring a state.
local m = require 'support'
m.begin('target,savestate', function()
    local save = m.integer('savestate', 0, 0, 1) == 1
    m.atTarget(function() m.capture(save); m.finish() end)
end)
