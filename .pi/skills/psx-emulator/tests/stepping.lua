-- Synthetic native stepping acceptance: delayed loads, branches, a call and syscall.
local m, ffi = require 'support', require 'ffi'
m.begin('', function()
    PCSX.pauseEmulator()
    assert(PCSX.Debugger and PCSX.Debugger.stepInto, 'approved native binding is unavailable')
    PCSX.resumeEmulator()
    assert(not pcall(PCSX.Debugger.stepInto), 'running emulator accepted a step request')
    PCSX.pauseEmulator()
    local registers = PCSX.getRegisters()
    local words = {0x24080001, 0x24090100, 0x8d2a0000, 0x01405821, 0x01406021,
        0x11080002, 0x24100009, 0x0000000d, 0x0c00400c, 0x24110005,
        0x0000000c, 0, 0x24120006, 0x03e00008, 0}
    local code = ffi.new('uint32_t[?]', #words)
    for i, word in ipairs(words) do code[i - 1] = word end
    local ram = ffi.cast('uint8_t*', PCSX.getMemPtr())
    ffi.copy(ram + 0x10000, code, #words * 4)
    ffi.cast('uint32_t*', ram + 0x100)[0] = 42
    registers.pc, registers.CP0.r[12], registers.GPR.r[10] = 0x80010000, 0, 7
    PCSX.invalidateCache()
    local expected = {0x80010004, 0x80010008, 0x8001000c, 0x80010010, 0x80010014,
        0x80010018, 0x80010020, 0x80010024, 0x80010030, 0x80010034,
        0x80010038, 0x80010028, 0x80000080}
    local observations = m.array()
    m.event('ExecutionFlow::Pause', function(event)
        -- Native triggerBP resets its step state after dispatch; defer the next request.
        PCSX.nextTick(m.guard(function()
            local index = #observations + 1
            local pc = tonumber(registers.pc)
            assert(pc == expected[index], string.format('step %d PC %08x', index, pc))
            observations[index] = {pc = pc, cycles = tostring(PCSX.getCPUCycles()), exception_pause = event.exception}
            if index == 4 then assert(registers.GPR.r[11] == 7 and registers.GPR.r[10] == 42) end
            if index < #expected then PCSX.Debugger.stepInto(); return end
            assert(registers.GPR.r[12] == 42 and registers.GPR.r[16] == 9)
            assert(registers.GPR.r[17] == 5 and registers.GPR.r[18] == 6)
            assert(registers.CP0.r[13] == 32 and registers.CP0.r[14] == 0x80010028)
            m.report('checks.json', {schema = 'psx.skill-checks/v1', passed = 6,
                checks = {'running request rejects', 'native step boundaries', 'load delay',
                    'taken branch delay', 'call and return delays', 'syscall Cause/EPC'}, observations = observations})
            m.write('state.pbuf', tostring(PCSX.createSaveState()))
            m.finish()
        end))
    end)
    PCSX.Debugger.stepInto()
end)
