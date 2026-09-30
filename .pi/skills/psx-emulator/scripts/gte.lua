-- Inspect/edit COP2 banks or run the current GTE command through the native interpreter.
local ffi, bit = require 'ffi', require 'bit'
local m, s = require 'support', require 'snapshot'
m.begin('action,target,bank,register,value,savestate', function()
    local action = m.argument('action', 'inspect')
    assert(action == 'inspect' or action == 'write' or action == 'step', 'unknown GTE action')
    local save = m.integer('savestate', 0, 0, 1) == 1
    local function describe(state)
        local data, control, flags = state.registers.cp2d, state.registers.cp2c, m.array()
        assert(#data == 32 and #control == 32, 'invalid COP2 banks')
        for i = 12, 31 do if bit.band(control[32], bit.lshift(1, i)) ~= 0 then flags[#flags + 1] = i end end
        local mac = {}
        for i = 24, 27 do local v = data[i + 1]; mac[#mac + 1] = v >= 2147483648 and v - 4294967296 or v end
        return {data = data, control = control, flag_bits = flags, mac_signed = mac, pc = state.registers.pc}
    end
    local function finish(before, after, source, opcode)
        m.report('gte.json', {schema = 'psx.runtime-gte/v1', action = action, source = source,
            before = before, after = after, opcode = opcode,
            command = opcode and bit.band(opcode, 63), host_register_edit = action == 'write'})
        if save then m.write('state.pbuf', tostring(PCSX.createSaveState())) end
        m.finish()
    end
    if action == 'inspect' then
        assert(not save, 'inspect savestate requires a live mutation/step action')
        s.observe(function(state, source) finish(describe(state), describe(state), source) end)
        return
    end
    m.atTarget(function()
        m.requireIdleCapture()
        local state = s.live()
        local before = describe(state)
        if action == 'write' then
            local bank = m.argument('bank', 'data')
            assert(bank == 'data' or bank == 'control', 'bank must be data or control')
            local index = m.integer('register', nil, 0, 31)
            PCSX.getRegisters()[bank == 'data' and 'CP2D' or 'CP2C'].r[index] = m.integer('value', nil, 0, 4294967295)
            finish(before, describe(s.live()), 'live'); return
        end
        assert(not state.registers.next_is_delay_slot, 'cannot step GTE in a delay slot')
        assert(bit.band(state.registers.cp0[13], 0x40000000) ~= 0, 'COP2 is disabled in Status.CU2')
        local pc = before.pc
        assert(({[0]=true,[4]=true,[5]=true})[math.floor(pc / 0x20000000)], 'unsupported instruction address alias')
        local physical = bit.band(pc, 0x1fffffff)
        assert(physical < 2097148 and pc % 4 == 0, 'GTE step requires a RAM instruction')
        local opcode = tonumber(ffi.cast('uint32_t*', PCSX.getMemPtr())[physical / 4])
        assert(bit.band(opcode, 0xfe000000) == 0x4a000000, 'current instruction is not a GTE command')
        local supported = {[1]=true,[6]=true,[12]=true,[16]=true,[17]=true,[18]=true,[19]=true,
            [20]=true,[22]=true,[27]=true,[28]=true,[30]=true,[32]=true,[40]=true,[41]=true,
            [42]=true,[45]=true,[46]=true,[48]=true,[61]=true,[62]=true,[63]=true}
        assert(supported[bit.band(opcode, 63)], 'reserved GTE command')
        m.breakpoint(pc + 4, 'Exec', 4, function()
            PCSX.pauseEmulator()
            finish(before, describe(s.live()), 'live', opcode)
            return false
        end)
        PCSX.resumeEmulator()
    end)
end)
