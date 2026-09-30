-- Inspect complete CPU register banks or apply an explicit GPR/PC edit while paused.
local m = require 'support'
m.begin('target,register,value,pc,savestate', function()
    local index, value, pc
    if m.argument('register') or m.argument('value') then
        index = m.integer('register', nil, 1, 31)
        value = m.integer('value', nil, 0, 4294967295)
    end
    if m.argument('pc') then pc = m.address('pc') end
    local save = m.integer('savestate', 0, 0, 1) == 1
    local function inspect()
        local regs = PCSX.getRegisters()
        local state = m.registers()
        state.hi, state.lo = tonumber(regs.GPR.n.hi), tonumber(regs.GPR.n.lo)
        for _, name in ipairs({'CP0', 'CP2D', 'CP2C'}) do
            state[name] = {}
            for i = 0, 31 do state[name][i + 1] = tonumber(regs[name].r[i]) end
        end
        return state
    end
    m.atTarget(function()
        PCSX.pauseEmulator()
        local before, regs = inspect(), PCSX.getRegisters()
        if index or pc then m.requireIdleCapture() end
        if index then regs.GPR.r[index] = value end
        if pc then regs.pc = pc end
        m.report('cpu.json', {schema = 'psx.runtime-cpu/v1', before = before, after = inspect(),
            edited = index ~= nil or pc ~= nil})
        if save then m.write('state.pbuf', tostring(PCSX.createSaveState())) end
        m.finish()
    end)
end)
