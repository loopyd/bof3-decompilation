-- Observe bounded debugger accesses; these are pre-instruction observations.
local m = require 'support'
m.begin('target,address,kind,width,hits,savestate', function()
    local address = m.integer('address', nil, 0, 4294967295)
    local width = m.integer('width', 1, 1, 65536)
    assert(address + width <= 4294967296, 'watch range overflows address space')
    local kind = m.argument('kind', 'Write')
    assert(kind == 'Exec' or kind == 'Read' or kind == 'Write', 'kind must be Exec, Read, or Write')
    local hits = m.integer('hits', 1, 1, 4096)
    local save = m.integer('savestate', 0, 0, 1) == 1
    local events = {}
    m.atTarget(function()
        m.breakpoint(address, kind, width, function(actual, size, cause)
            events[#events + 1] = {address = actual, width = size, cause = cause,
                registers = m.registers()}
            if #events == hits then
                PCSX.pauseEmulator()
                m.report('watch.json', {schema = 'psx.runtime-watch/v1', kind = kind,
                    address = address, width = width, boundary = 'before instruction', events = events})
                m.capture(save)
                m.finish()
                return false
            end
            return true
        end)
        PCSX.resumeEmulator()
    end)
end)
