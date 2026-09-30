-- Issue MMIO stores through the native CPU, not the debugger's side-effect-free memory file.
local ffi, bit = require 'ffi', require 'bit'
local m, s = require 'support', require 'snapshot'
local M = {}
function M.write(width, operations, callback)
    m.requireIdleCapture()
    assert(#operations > 0 and #operations <= 262000, 'invalid MMIO operation count')
    local start = m.address('scratch')
    assert(({[0]=true,[4]=true,[5]=true})[math.floor(start / 0x20000000)],
        'scratch must use a RAM address in KUSEG, KSEG0 or KSEG1')
    local physical = bit.band(start, 0x1fffffff)
    local bytes = 52 + #operations * 8
    assert(physical + bytes <= 2097152, 'scratch region does not fit RAM')
    local state, regs = s.live(), PCSX.getRegisters()
    assert(not state.registers.next_is_delay_slot, 'MMIO probe cannot start in a delay slot')
    for _, name in ipairs({'delay_slot_info_1', 'delay_slot_info_2'}) do
        local delay = state.registers[name]
        assert(not delay or (not delay.pc_active and not delay.active), 'MMIO probe requires no pending delayed load/branch')
    end
    local saved, pc = {}, tonumber(regs.pc)
    for i = 0, 33 do saved[i] = tonumber(regs.GPR.r[i]) end
    local pointer = ffi.cast('uint8_t*', PCSX.getMemPtr()) + physical
    local original = ffi.string(pointer, bytes)
    local data = start + 52
    local store = assert(({[8] = 0xa14b0000, [16] = 0xa54b0000, [32] = 0xad4b0000})[width])
    local words = {0x3c080000 + math.floor(data / 65536), 0x35080000 + data % 65536,
        0x3c090000 + math.floor(#operations / 65536), 0x35290000 + #operations % 65536,
        0x8d0a0000, 0x8d0b0004, 0, store, 0x25080008, 0x2529ffff, 0x1520fff9, 0, 0}
    for _, op in ipairs(operations) do words[#words + 1] = op[1]; words[#words + 1] = op[2] end
    local buffer = ffi.new('uint32_t[?]', #words)
    for i, word in ipairs(words) do buffer[i - 1] = word end
    ffi.copy(pointer, buffer, bytes)
    PCSX.invalidateCache()
    m.breakpoint(start + 48, 'Exec', 4, function()
        PCSX.pauseEmulator()
        m.requireIdleCapture()
        ffi.copy(pointer, original, bytes)
        PCSX.invalidateCache()
        for i = 0, 33 do regs.GPR.r[i] = saved[i] end
        regs.pc = pc
        callback({scratch = start, scratch_bytes = bytes, operations = #operations,
            store_bits = width, before_cycles = state.registers.cycle,
            after_cycles = tonumber(PCSX.getCPUCycles())})
        return false
    end)
    regs.pc = start
    PCSX.resumeEmulator()
end
return M
