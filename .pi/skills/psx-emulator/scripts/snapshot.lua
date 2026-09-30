-- Decode through the emulator's schema and bundled lua-protobuf, never wire guesses.
local pb = require 'pb'
local M = {}
M.schema = PCSX.getSaveStateProtoSchema()
assert(require('protoc').new():load(M.schema))
pb.option('int64_as_string') -- Large integers retain the library's exact #decimal form.

function M.live()
    return assert(pb.decode('SaveState', tostring(PCSX.createSaveState())))
end

-- Inspection of a supplied state is offline unless an execution boundary was requested.
function M.observe(callback)
    local m = require 'support'
    local path = m.input('state', false)
    if path and not m.argument('target') and not os.getenv('PSX_RUNTIME_ENTRY') then
        PCSX.pauseEmulator()
        callback(M.read(path), 'saved-state')
    else
        m.atTarget(function() callback(M.live(), 'live') end)
    end
end

-- Check tags with the bundled reader before decode can discard unknown fields.
function M.compatible(bytes)
    local slice = require('pb.slice').new(bytes)
    local formats = {[0] = 'v', [1] = 'q', [5] = 'd'}
    local fixed = {double = 1, fixed64 = 1, sfixed64 = 1,
        float = 5, fixed32 = 5, sfixed32 = 5, string = 2, bytes = 2}
    local values = 0
    local function count()
        values = values + 1
        assert(values <= 100000, 'state encoded value limit exceeded')
    end
    local function varint(narrow)
        local first = slice:level(slice:level())
        local value = slice:unpack('v')
        local last = slice:level(slice:level()) - 1
        assert(value ~= nil, 'truncated varint')
        assert(last - first < 9 or bytes:byte(last) <= 1, 'varint overflow')
        if narrow then
            assert(type(value) == 'number' and value <= 4294967295, '32-bit varint overflow')
        end
        return value
    end
    local function scalar(wire, kind)
        count()
        if wire ~= 0 then
            assert(slice:unpack(formats[wire]) ~= nil, 'truncated scalar field')
            return
        end
        local value = varint(kind == 'uint32' or kind == 'sint32')
        if kind == 'bool' then assert(value == 0 or value == 1, 'invalid encoded boolean') end
        if kind == 'int32' then
            local positive = type(value) == 'number' and value <= 2147483647
            local negative = type(value) == 'string' and #value == 21 and
                value >= '#18446744071562067968' and value <= '#18446744073709551615'
            assert(positive or negative, 'invalid encoded int32 sign extension')
        end
    end
    local function inspect(kind, depth)
        assert(depth <= 64, 'state schema nesting exceeds 64')
        local seen = {}
        while #slice > 0 do
            count()
            local tag = varint(true)
            assert(type(tag) == 'number' and tag >= 8 and tag <= 4294967295, 'invalid state field tag')
            local number, wire = math.floor(tag / 8), tag % 8
            local name, _, fieldType, _, label = pb.field(kind, number)
            assert(name, 'unknown encoded field: ' .. kind .. '#' .. number)
            local _, _, category = pb.type(fieldType)
            local expected = category == 'message' and 2 or fixed[fieldType] or 0
            local repeated = label == 'repeated' or label == 'packed'
            assert(repeated or not seen[number], 'duplicate singular field: ' .. kind .. '.' .. name)
            seen[number] = true
            local packed = repeated and wire == 2 and expected ~= 2
            assert(wire == expected or packed, 'incompatible wire type: ' .. kind .. '.' .. name)
            if wire == 2 then
                local position = slice:unpack('@')
                local length = varint(true)
                assert(length <= #slice, 'truncated length-delimited field')
                slice:unpack('*', position)
                slice:enter()
                if packed then
                    while #slice > 0 do scalar(expected, fieldType) end
                elseif category == 'message' then inspect(fieldType, depth + 1)
                else slice:unpack('+', #slice) end
                slice:leave()
            else scalar(wire, fieldType) end
        end
    end
    inspect('SaveState', 0)
end

function M.decode(bytes, strict)
    if strict then M.compatible(bytes) end
    pb.option(strict and 'no_default_values' or 'auto_default_values')
    local ok, decoded = pcall(pb.decode, 'SaveState', bytes)
    pb.option('auto_default_values')
    assert(ok and decoded, 'invalid state protobuf: ' .. tostring(decoded))
    assert(decoded.save_state_info and decoded.save_state_info.version == 4,
        'unsupported/missing state version')
    return decoded
end

function M.read(path, strict)
    local file = assert(io.open(path, 'rb'))
    local bytes = assert(file:read(67108865)); file:close()
    assert(#bytes <= 67108864, 'state exceeds 64 MiB')
    assert(bytes:sub(1, 2) ~= '\31\139', 'gzip GUI state: supply raw API protobuf')
    return M.decode(bytes, strict)
end

function M.select(value, path, missing)
    local kind, repeated = 'SaveState', false
    if path == '' then return value, kind, repeated end
    local consumed = ''
    for part in path:gmatch('[^.]+') do
        local name, index = part:match('^([%a_][%w_]*)%[(%d+)%]$')
        name = name or part:match('^([%a_][%w_]*)$')
        assert(name and not repeated, 'invalid query path: ' .. path)
        local found, _, fieldType, _, label = pb.field(kind, name)
        assert(found, 'unknown schema field: ' .. part)
        value, kind, repeated = value and value[name], fieldType, label == 'repeated' or label == 'packed'
        assert(value ~= nil or missing, 'field absent in state: ' .. part)
        if index then
            assert(repeated, 'index requires a repeated field')
            assert(tonumber(index) >= 1 and tonumber(index) <= 1000000, 'invalid state index')
            value = value and value[tonumber(index)]
            assert(value ~= nil or missing, 'state index out of range (indices start at 1)')
            repeated = false
        end
        consumed = consumed == '' and part or consumed .. '.' .. part
    end
    assert(consumed == path, 'invalid query path: ' .. path)
    return value, kind, repeated
end

function M.validate(value)
    for field, size in pairs({ram = 8388608, rom = 524288, exp1 = 8388608,
        hardware = 65536, sram = 2097152}) do
        assert(value.memory and type(value.memory[field]) == 'string' and
            #value.memory[field] == size, 'invalid saved memory region: ' .. field)
    end
    for field, count in pairs({gpr = 34, cp0 = 32, cp2d = 32, cp2c = 32}) do
        assert(value.registers and type(value.registers[field]) == 'table' and
            #value.registers[field] == count, 'invalid saved register bank: ' .. field)
    end
    assert(value.spu and #value.spu.channel == 24 and #value.spu.ram == 524288,
        'invalid saved SPU state')
end

function M.describe(value, kind, repeated, depth, budget)
    budget = budget or {remaining = 4096}
    budget.remaining = budget.remaining - 1
    assert(budget.remaining >= 0, 'query exceeds 4096 nodes; narrow the path/depth')
    if kind == 'bytes' and not repeated then
        local preview = value:sub(1, 32):gsub('.', function(c) return string.format('%02x', c:byte()) end)
        return {type = 'bytes', bytes = #value, preview_hex = preview, preview_truncated = #value > 32}
    end
    if type(value) ~= 'table' then return value end
    if repeated then
        if depth == 0 then return {type = kind, count = #value} end
        local items = require('support').array()
        for i, item in ipairs(value) do items[i] = M.describe(item, kind, false, depth - 1, budget) end
        return {type = kind, count = #value, items = items}
    end
    local fields = {}
    for name, number, fieldType, _, label in pb.fields(kind) do
        local item = {name = name, number = number, type = fieldType, label = label,
            present = value[name] ~= nil}
        if depth > 0 and value[name] ~= nil then
            item.value = M.describe(value[name], fieldType, label == 'repeated' or label == 'packed',
                depth - 1, budget)
        end
        fields[#fields + 1] = item
    end
    table.sort(fields, function(a, b) return a.number < b.number end)
    return {type = kind, fields = fields}
end

return M
