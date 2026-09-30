-- Data-only schedule validation through the native mission environment.
local m = require 'support'
m.begin('', function()
    PCSX.pauseEmulator()
    local timeline = require 'timeline'
    local checks = m.array()
    local function read(text)
        local file = assert(io.open('schedule.txt', 'wb'))
        assert(file:write(text)); assert(file:close())
        return timeline.read('schedule.txt')
    end
    local function check(name, callback)
        callback(); checks[#checks + 1] = name
    end
    local function reject(text, pattern)
        local ok, problem = pcall(read, text)
        assert(not ok and tostring(problem):find(pattern, 1, true), tostring(problem))
    end
    local header, tail = 'psx.schedule/v1\n', '1 stop\n'
    check('ordered events, empty holds and CRLF', function()
        local value = read('psx.schedule/v1\r\n0 buttons 1 START,SELECT\r\n0 sample registers\r\n1 buttons 1 -\r\n1 stop\r\n')
        assert(#value.events == 4 and #value.events[1].buttons == 2)
        assert(value.samples[1].frame == 0 and value.samples[1].path == 'registers')
        assert(#value.events[3].buttons == 0 and value.frames == 1)
    end)
    check('header, unknown action and missing boundaries', function()
        reject('wrong\n', 'unsupported schedule')
        reject(header .. '0 magic\n', 'unknown schedule action')
        reject(header .. '0 sample .\n', 'requires samples')
        reject(header .. tail, 'requires samples')
        reject(header .. '0 sample .\n0 stop\n', 'stop requires')
    end)
    check('frame syntax and order', function()
        for _, frame in ipairs({'-1', '1.5', '0x10', '36001'}) do
            reject(header .. frame .. ' sample .\n' .. tail, 'monotonic integers')
        end
        reject(header .. '2 sample .\n' .. tail, 'monotonic integers')
        reject(header .. '0 sample .\n' .. tail .. '2 sample .\n', 'events after stop')
    end)
    check('button slots, names and duplicates', function()
        reject(header .. '0 buttons 3 START\n', 'buttons requires')
        reject(header .. '0 buttons 1 UNKNOWN\n', 'unknown button')
        reject(header .. '0 buttons 1 START,START\n', 'duplicate button')
        reject(header .. '0 buttons 1 ,START\n', 'invalid comma')
        reject(header .. '0 buttons 1 START,\n', 'invalid comma')
    end)
    check('file, event and checkpoint bounds', function()
        reject(header .. string.rep(' ', 65537), '64 KiB')
        reject(header .. '\0', 'NUL')
        reject(header .. string.rep('0 sample .\n', 5) .. tail, 'four checkpoints')
        reject(header .. string.rep('0 buttons 1 -\n', 513), '512 events')
    end)
    check('alternate schedules require equivalent checkpoints', function()
        local a = read(header .. '0 sample registers\n' .. tail)
        local b = read(header .. '0 buttons 2 START\n0 sample registers\n' .. tail)
        timeline.compatible(a, b)
        for _, text in ipairs({'0 sample memory\n1 stop\n', '1 sample registers\n1 stop\n',
                '0 sample registers\n2 stop\n', '0 sample registers\n0 sample memory\n1 stop\n'}) do
            local ok, problem = pcall(timeline.compatible, a, read(header .. text))
            assert(not ok and tostring(problem):find('checkpoint boundaries', 1, true))
        end
    end)
    m.report('checks.json', {schema = 'psx.skill-checks/v1', checks = checks, passed = #checks})
    m.finish()
end)
