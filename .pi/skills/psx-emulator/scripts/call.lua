-- Host-directed function invocation from an explicitly chosen machine context.
local m = require 'support'
m.begin('target,function,return,a0,a1,a2,a3,trace,trace_limit,savestate', function()
    local func = m.address('function')
    local stop = m.address('return', 0x80010000)
    assert(func ~= stop, 'return address overlaps function')
    local save = m.integer('savestate', 0, 0, 1) == 1
    local limit = m.integer('trace_limit', 4096, 1, 65536)
    local arguments, trace, started = {}, m.array(), false
    for i = 0, 3 do arguments[i] = m.integer('a' .. i, 0, 0, 4294967295) end
    if m.argument('trace') then
        m.breakpoint(m.address('trace'), 'Exec', 4, function()
            if started then
                assert(#trace < limit, 'trace_limit exceeded')
                trace[#trace + 1] = m.registers()
            end
            return true
        end)
    end
    m.breakpoint(stop, 'Exec', 4, function()
        if not started then return true end
        PCSX.pauseEmulator()
        m.capture(save)
        if m.argument('trace') then
            m.report('trace.json', {schema = 'psx.runtime-trace/v1', events = trace})
        end
        m.finish()
        return false
    end)
    m.atTarget(function()
        local regs = PCSX.getRegisters()
        m.requireIdleCapture()
        assert(tonumber(regs.pc) ~= stop, 'return address overlaps context entry')
        started = true
        for i = 0, 3 do regs.GPR.r[4 + i] = arguments[i] end
        regs.GPR.n.ra, regs.pc = stop, func
        PCSX.resumeEmulator()
    end)
end)
