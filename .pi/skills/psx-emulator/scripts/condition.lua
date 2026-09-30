-- Small data-only terminal conditions; physical RAM reads have no MMIO side effects.
local ffi = require 'ffi'
local M = {}
local function integer(word, maximum)
    local value = tonumber(word)
    assert(type(word) == 'string' and (word:match('^%d+$') or word:match('^0[xX]%x+$'))
        and value and value % 1 == 0 and value >= 0 and value <= maximum, 'invalid condition integer')
    return value
end
function M.parse(words)
    local kind = words[3]
    local condition = {kind = kind}
    if kind == 'pc' then
        assert(#words == 4, 'pc condition requires a value')
        condition.value = integer(words[4], 4294967295)
        assert(condition.value % 4 == 0, 'condition PC must be word aligned')
    elseif kind == 'register' then
        assert(#words == 5, 'register condition requires index and value')
        condition.index, condition.value = integer(words[4], 31), integer(words[5], 4294967295)
    elseif kind == 'memory' then
        assert(#words == 5, 'memory condition requires physical RAM offset and hex bytes')
        condition.offset = integer(words[4], 2097151)
        local hex = words[5]
        assert(#hex >= 2 and #hex <= 128 and #hex % 2 == 0 and hex:match('^%x+$'),
            'condition memory requires 1..64 hex byte pairs')
        condition.hex = hex:lower()
        assert(condition.offset + #hex / 2 <= 2097152, 'condition memory exceeds RAM')
    else error('unknown stop condition: ' .. tostring(kind)) end
    return condition
end
function M.observe(condition)
    local value
    if condition.kind == 'memory' then
        local bytes = ffi.string(ffi.cast('uint8_t*', PCSX.getMemPtr()) + condition.offset, #condition.hex / 2)
        value = bytes:gsub('.', function(c) return string.format('%02x', c:byte()) end)
    else
        local registers = PCSX.getRegisters()
        value = tonumber(condition.kind == 'pc' and registers.pc or registers.GPR.r[condition.index])
    end
    return {matched = value == (condition.value or condition.hex), value = value, condition = condition}
end
return M
