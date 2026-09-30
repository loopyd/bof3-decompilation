-- Exercise the installed history ABI and exporter through real native lifecycle guards.
local m, events, ffi = require 'support', require 'events', require 'ffi'
m.begin('', function()
    PCSX.pauseEmulator()
    local h = assert(PCSX.History)
    assert(ffi.sizeof('HistoryStatus') == 48 and ffi.sizeof('HistoryRecord') == 80)
    for _, args in ipairs({{0, 0}, {65537, 0}, {1, 8388609}, {1.5, 0}, {1, -1}}) do
        assert(not pcall(h.begin, args[1], args[2]), 'invalid limits accepted')
    end
    assert(not pcall(h.stop) and not pcall(h.record, 0), 'empty recorder accepted drain')
    PCSX.resumeEmulator()
    assert(not pcall(h.begin, 128, 0), 'running begin accepted')
    PCSX.pauseEmulator()
    h.begin(128, 0)
    assert(not pcall(m.requireIdleCapture), 'host edit guard accepted active capture')
    assert(not pcall(h.begin, 128, 0) and not pcall(h.clear), 'active evidence was replaceable')
    assert(not pcall(h.record, 0) and not pcall(h.bytes, 0, 0), 'active drain accepted')
    h.stop()
    assert(not pcall(h.begin, 128, 0), 'frozen evidence overwritten without clear')
    local status, record = h.status(), h.record(0)
    assert(status.failures == 0 and status.events >= 30 and record.sequence == 1)
    assert(h.bytes(0, 0) == '' and not pcall(h.bytes, 0, 1))
    assert(not pcall(h.record, tonumber(status.events)), 'out-of-range event accepted')
    PCSX.resumeEmulator()
    assert(not pcall(h.record, 0) and not pcall(h.clear), 'running drain accepted')
    PCSX.pauseEmulator()
    h.clear()
    assert(status.state == 2 and record.sequence == 1, 'native return values are borrowed')
    collectgarbage(); collectgarbage()
    assert(record.sequence == 1)
    assert(events.exact(ffi.new('uint64_t', 9007199254740992) + 1) == '9007199254740993')
    m.write('seed.pbuf', tostring(PCSX.createSaveState()))
    h.begin(128, 0)
    local state = Support.File.open('seed.pbuf')
    PCSX.loadSaveState(state); state:close()
    h.stop()
    assert(require('bit').band(h.status().failures, 32) ~= 0, 'state restore discontinuity concealed')
    h.clear()
    for _, reset in ipairs({PCSX.softResetEmulator, PCSX.hardResetEmulator}) do
        h.begin(128, 0); reset(); PCSX.pauseEmulator(); h.stop()
        assert(require('bit').band(h.status().failures, 32) ~= 0, 'native reset discontinuity concealed')
        h.clear()
    end
    h.begin(1, 0); h.stop()
    local result = events.export()
    assert(not result.complete and result.events == 1 and result.failure_bits == 1 and
        result.dropped_attempts > 0, 'initial event overflow concealed')
    h.clear(); m.requireIdleCapture()
    m.report('checks.json', {schema = 'psx.skill-checks/v1', passed = 10,
        checks = {'ABI and invalid budgets', 'empty/running lifecycle guards',
            'active replacement and drain guards', 'host edit guard',
            'frozen evidence retention and bounds', 'owning FFI copies',
            'uint64 precision', 'state restore discontinuity', 'native soft/hard reset',
            'native initial overflow export'}})
    m.finish()
end)
