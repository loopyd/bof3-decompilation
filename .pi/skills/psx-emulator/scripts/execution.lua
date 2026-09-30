-- Native CPU boundary observations and bounded symbol-qualified coverage.
local m, s = require 'support', require 'snapshot'
local ffi, bit = require 'ffi', require 'bit'
local M = {}

function M.word(address)
    if address < 0 or address > 4294967295 or address % 4 ~= 0 then return nil end
    local segment = math.floor(address / 0x20000000)
    if segment ~= 0 and segment ~= 4 and segment ~= 5 then return nil end
    local physical, pointer = bit.band(address, 0x1fffffff)
    if physical < 2097152 then pointer = PCSX.getMemPtr() + physical
    elseif physical >= 0x1fc00000 and physical < 0x1fc80000 then
        pointer = PCSX.getRomPtr() + physical - 0x1fc00000
    elseif physical >= 0x1f800000 and physical < 0x1f800400 then
        pointer = PCSX.getScratchPtr() + physical - 0x1f800000
    end
    if pointer then return tonumber(ffi.cast('uint32_t*', pointer)[0]) end
end

function M.classify(word)
    local op, fn = bit.rshift(word, 26), bit.band(word, 63)
    if op == 3 or (op == 0 and fn == 9) then return 'call' end
    if op == 2 or (op == 0 and fn == 8) then return 'jump' end
    if op == 1 then
        local rt = bit.band(bit.rshift(word, 16), 31)
        if rt == 16 or rt == 17 then return 'conditional-link' end
        return (rt == 0 or rt == 1) and 'branch' or 'unsupported-regimm'
    end
    if (op >= 4 and op <= 7) or (op == 18 and bit.band(bit.rshift(word, 21), 31) == 8) then
        return 'branch'
    end
    if op == 0 and (fn == 12 or fn == 13) then return 'trap' end
    return 'other'
end

function M.capture()
    local r = s.live().registers
    local near = m.array()
    for offset = -8, 8, 4 do
        local address = r.pc + offset
        local word = M.word(address)
        if word then near[#near + 1] = {address = address, word = word} end
    end
    return {pc = r.pc, cycles = tostring(r.cycle):gsub('^#', ''),
        gpr = r.gpr, cp0 = r.cp0, last_executed_word = r.code,
        next_memory_word = M.word(r.pc), nearby_memory = near,
        in_isr = r.in_isr, next_is_delay_slot = r.next_is_delay_slot,
        current_delayed_load = r.current_delayed_load,
        delay_slot_info_1 = r.delay_slot_info_1, delay_slot_info_2 = r.delay_slot_info_2,
        exception = {cause = r.cp0[14], epc = r.cp0[15], status = r.cp0[13],
            badvaddr = r.cp0[9], code = bit.band(bit.rshift(r.cp0[14], 2), 31),
            branch_delay = bit.band(r.cp0[14], 0x80000000) ~= 0}}
end

function M.deltas(before, after)
    local result = m.array()
    for _, bank in ipairs({'gpr', 'cp0'}) do
        for index, value in ipairs(after[bank]) do
            if value ~= before[bank][index] then
                result[#result + 1] = {bank = bank, index = index - 1,
                    before = before[bank][index], after = value}
            end
        end
    end
    return result
end

function M.symbols(path)
    local symbols = {}
    if not path then return symbols end
    local file = assert(io.open(path, 'rb'))
    local bytes = file:read(32769) or ''; file:close()
    assert(#bytes <= 32768 and not bytes:find('%z'), 'symbol input exceeds text/size bounds')
    local count, names = 0, {}
    for line in bytes:gmatch('[^\r\n]+') do
        local raw, name = line:match('^(%S+)%s+([%w_.:/@-]+)%s*$')
        local address = raw and tonumber(raw)
        assert(address and address >= 0 and address <= 4294967295 and address % 4 == 0,
            'symbols require aligned U32 address and qualified name per line')
        assert(name and #name <= 160 and not names[name] and not symbols[address], 'duplicate/invalid symbol')
        count = count + 1
        assert(count <= 256, 'symbol count exceeds 256')
        symbols[address], names[name] = name, true
    end
    return symbols
end

function M.coverage(symbols)
    local addresses, functions, transfers = {}, {}, {}
    local result = {addresses = m.array(), functions = m.array(), transfers = m.array()}
    local function increment(map, list, key, item)
        if not map[key] then map[key] = item; list[#list + 1] = item; item.hits = 0 end
        map[key].hits = map[key].hits + 1
    end
    return result, function(before, after)
        increment(addresses, result.addresses, before.pc, {address = before.pc})
        if symbols[before.pc] then
            increment(functions, result.functions, before.pc, {address = before.pc, name = symbols[before.pc]})
        end
        local kind = M.classify(after.last_executed_word)
        if kind ~= 'other' then
            local key = before.pc .. ':' .. after.pc .. ':' .. after.last_executed_word
            increment(transfers, result.transfers, key, {from_boundary = before.pc,
                to_boundary = after.pc, last_executed_word = after.last_executed_word, kind = kind})
        end
    end
end
return M
