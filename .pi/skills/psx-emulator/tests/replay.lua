-- Inject observable timing drift while leaving the selected PC unchanged.
local m, pb = require 'support', require 'pb'
local file = assert(io.open(m.input('fault', true), 'rb'))
local boundary = assert(file:read('*a')):gsub('%s+$', ''); file:close()
assert(boundary == 'start' or boundary == 'stop' or boundary == 'checkpoint')
local ffi = require 'ffi'
local oldCycles, oldState, calls, captures = PCSX.getCPUCycles, PCSX.createSaveState, 0, 0
PCSX.getCPUCycles = function()
    calls = calls + 1
    local value = oldCycles()
    if (boundary == 'start' and calls == 3) or (boundary == 'stop' and calls == 4) then
        return value + ffi.new('uint64_t', 1)
    end
    return value
end
PCSX.createSaveState = function()
    local bytes = oldState()
    captures = captures + 1
    if boundary == 'checkpoint' and captures == 3 then
        local value = assert(pb.decode('SaveState', tostring(bytes)))
        value.registers.cycle = value.registers.cycle + 1
        return assert(pb.encode('SaveState', value))
    end
    return bytes
end
local report = m.report
m.report = function(name, value)
    if name == 'replay.json' then
        assert(value.equal == false and value.first_divergence.boundary == boundary)
        assert(value.first_divergence.cycles_equal == false)
        if boundary == 'checkpoint' then assert(value.first_divergence.state_equal) end
    end
    return report(name, value)
end
require 'replay'
