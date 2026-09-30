-- Exercise display comparison and terminal conditions without game fixtures.
local m = require 'support'
m.begin('', function()
    PCSX.pauseEmulator()
    local display, condition, timeline = require 'display', require 'condition', require 'timeline'
    local checks = m.array()
    local function check(name, callback) callback(); checks[#checks + 1] = name end
    local function parse(text)
        local words = {}
        for word in text:gmatch('%S+') do words[#words + 1] = word end
        return condition.parse(words)
    end
    local function reject(text)
        assert(not pcall(parse, text), 'invalid condition accepted: ' .. text)
    end
    check('equal and unavailable displays retain explicit availability', function()
        local empty = {width = 0, height = 0, format = 'psx-bgr555-le', bytes = 0, available = false}
        local result = display.compare('', '', empty, empty)
        assert(result.equal and not result.image_comparison_available)
        local pixels, info = display.capture()
        assert(display.compare(pixels, pixels, info, info).equal)
    end)
    check('pixel changes and metadata changes are independent', function()
        local a = {width = 2, height = 1, format = 'psx-bgr555-le', bytes = 4, available = true}
        local b = {width = 1, height = 2, format = 'psx-bgr555-le', bytes = 4, available = true}
        local result = display.compare('\0\0\0\0', '\0\1\0\0', a, a)
        assert(not result.equal and result.pixels.changed_bytes == 1 and #result.metadata == 0)
        result = display.compare('1234', '1234', a, b)
        assert(not result.equal and result.pixels.equal and #result.metadata == 2)
        b.format, b.available = 'rgb24', false
        result = display.compare('1234', '', a, b)
        assert(not result.equal and not result.image_comparison_available)
    end)
    check('pixel difference overflow rejects', function()
        local ok = pcall(display.compare, 'aaaa', 'baba', {}, {}, {limit = 1})
        assert(not ok)
    end)
    check('PC and register conditions inspect exact native values', function()
        local pc = tonumber(PCSX.getRegisters().pc)
        assert(condition.observe(parse('9 stop pc ' .. pc)).matched)
        assert(condition.observe(parse('9 stop register 0 0')).matched)
        assert(not condition.observe(parse('9 stop register 0 1')).matched)
    end)
    check('RAM condition bounds and exact bytes', function()
        local ffi = require 'ffi'
        local bytes = ffi.string(PCSX.getMemPtr(), 4)
        local hex = bytes:gsub('.', function(c) return string.format('%02x', c:byte()) end)
        assert(condition.observe(parse('9 stop memory 0 ' .. hex)).matched)
        reject('9 stop memory 2097151 0000')
        reject('9 stop memory 0 z0')
        reject('9 stop memory 0 0')
        reject('9 stop memory 0 ' .. string.rep('00', 65))
    end)
    check('unknown, malformed and executable expressions reject', function()
        for _, text in ipairs({'9 stop lua print(1)', '9 stop pc 1', '9 stop pc -4',
            '9 stop pc 4294967296', '9 stop register 32 0', '9 stop register 1 1+2',
            '9 stop pc', '9 stop pc 4 extra'}) do reject(text) end
    end)
    check('screen and conditional schedule retains source order', function()
        local file = assert(io.open('schedule.txt', 'wb'))
        file:write('psx.schedule/v1\n0 screen\n1 buttons 1 -\n9 stop memory 0 0000\n'); file:close()
        local result = timeline.read('schedule.txt')
        assert(result.samples[1].action == 'screen' and result.events[3].condition.kind == 'memory')
    end)
    m.report('checks.json', {schema = 'psx.skill-checks/v1', checks = checks, passed = #checks})
    m.finish()
end)
