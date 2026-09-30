-- Prepare synthetic stepping contexts and check boundary-analysis helpers in Redux.
local m, ffi, execution = require 'support', require 'ffi', require 'execution'
m.begin('scenario', function()
    PCSX.pauseEmulator()
    local programs = {loop = {0x08004000, 0}, delays = {
        0x24080001, 0x24090100, 0x8d2a0000, 0x01405821, 0x01406021,
        0x11080002, 0x24100009, 0x0000000d, 0x0c00400c, 0x24110005,
        0x0000000c, 0, 0x24120006, 0x03e00008, 0},
        nested = {0x24080000, 0x0c004008, 0x25080001, 0xc, 0, 0, 0, 0,
            0x03e08021, 0x3c198001, 0x37390040, 0x0320f809, 0x25080002,
            0x0200f821, 0x03e00008, 0x25080004, 0x08004014, 0x25080008,
            0xd, 0, 0x03e00008, 0x25080010},
        fault = {0x08004004, 0xc, 0, 0, 0xd},
        break_even = {0x08004004, 0xd, 0, 0, 0xc},
        fault_odd = {0, 0x08004004, 0xc, 0, 0xd},
        break_odd = {0, 0x08004004, 0xd, 0, 0xc},
        recursion = {0x3c08fffe, 0x35080130, 0x3c090001, 0x3529e988, 0xad090000,
            0x241d4000, 0x24040004, 0x0c00400b, 0, 0xc, 0,
            0x27bdfff8, 0xafbf0000, 0x10800004, 0, 0x2484ffff, 0x0c00400b,
            0, 0x8fbf0000, 0x27bd0008, 0x03e00008, 0}}
    local scenario = m.argument('scenario', 'delays')
    local words = assert(programs[scenario], 'unknown synthetic scenario')
    local ram = ffi.cast('uint8_t*', PCSX.getMemPtr())
    local buffer = ffi.new('uint32_t[?]', #words)
    for i, word in ipairs(words) do buffer[i - 1] = word end
    ffi.copy(ram + 0x10000, buffer, #words * 4)
    ffi.cast('uint32_t*', ram + 0x100)[0] = 42
    local r = PCSX.getRegisters()
    r.pc, r.CP0.r[12], r.GPR.r[10] = 0x80010000, 0, 7
    PCSX.invalidateCache()
    assert(execution.word(0x80010000) == words[1])
    assert(execution.word(0xa0010000) == words[1])
    assert(execution.word(0x1f801810) == nil and execution.word(0x80010001) == nil)
    assert(execution.classify(0x0c00400c) == 'call')
    assert(execution.classify(0x03e00008) == 'jump')
    assert(execution.classify(0x11080002) == 'branch')
    assert(execution.classify(0x04100001) == 'conditional-link')
    assert(execution.classify(0x04110001) == 'conditional-link')
    assert(execution.classify(0x04020001) == 'unsupported-regimm')
    assert(execution.classify(0xc) == 'trap')
    local before, after = execution.capture(), execution.capture()
    assert(#execution.deltas(before, after) == 0)
    after.gpr[11] = 99
    local changes = execution.deltas(before, after)
    assert(#changes == 1 and changes[1].index == 10 and changes[1].before == 7)
    local definitions = {delays = '0x80010030 fixture/callee\n',
        nested = '0x80010020 fixture/function\n0x80010040 fixture/indirect\n0x80010050 fixture/tail\n',
        recursion = '0x8001002c fixture/recursive\n'}
    m.write('symbols.txt', '0x80010000 fixture/main\n' .. (definitions[scenario] or ''))
    m.write('case.txt', scenario)
    local symbols = execution.symbols('symbols.txt')
    assert(symbols[0x80010000] == 'fixture/main')
    local coverage, observe = execution.coverage(symbols)
    observe(before, before); observe(before, before)
    assert(coverage.addresses[1].hits == 2 and coverage.functions[1].hits == 2)
    m.write('invalid-symbols.txt', '0x80010000 first\n0x80010000 duplicate\n')
    assert(not pcall(execution.symbols, 'invalid-symbols.txt'))
    m.report('checks.json', {schema = 'psx.skill-checks/v1', passed = 5,
        checks = {'safe memory words', 'transfer classification', 'register deltas',
            'qualified coverage counts', 'duplicate symbol rejection'}, context = before})
    m.write('state.pbuf', tostring(PCSX.createSaveState()))
    m.finish()
end)
