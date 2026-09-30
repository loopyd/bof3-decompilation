-- Check surviving callbacks, MDEC cancellation and MSAN rejection through native execution.
local m, events, ffi = require 'support', require 'events', require 'ffi'
m.begin('scenario,stop', function()
    local scenario, stop = m.argument('scenario'), m.address('stop')
    local h, timer = PCSX.History, luv.new_timer()
    m.retained[#m.retained + 1] = timer
    timer:start(15000, 0, m.guard(function() error('ownership fixture timeout') end))
    local function check()
        timer:stop(); timer:close(); h.stop()
        local report = events.export()
        local function select(kind, device)
            local rows = {}
            for i = 0, report.events - 1 do
                local r = h.record(i)
                if r.kind == kind and (device == nil or r.device == device) then rows[#rows + 1] = r end
            end
            return rows
        end
        if scenario == 'msan' then
            assert(not report.complete and report.failure_bits == 8, 'MSAN activation accepted')
            h.clear()
            assert(not pcall(h.begin, 128, 0), 'MSAN-enabled begin accepted')
        else
            assert(report.complete)
            if scenario == 'survivor' then
                local done = select(6, 0)
                assert(#done == 2 and done[1].request == 1 and done[1].related == 2,
                    'older callback lost owner or touched-channel identity')
                assert(done[2].request == 2 and done[2].related == 0)
                local output = select(4, 1)
                assert(#output == 1 and output[1].request == 3 and output[1].related == 2 and output[1].length == 512)
                assert(ffi.string(ffi.cast('uint8_t*', PCSX.getMemPtr()) + 0x30400, 512) == string.rep('\16\66', 256))
            elseif scenario == 'cancel' then
                local cancelled = select(28, 1)
                assert(#cancelled == 3 and cancelled[2].request == 1 and cancelled[2].related == 0)
                assert(cancelled[3].request == 3 and cancelled[3].related == 2)
                local deferred, buffer, output = select(8, 1), select(27, 1), select(4, 1)
                assert(#deferred == 1 and deferred[1].request == 1)
                assert(#buffer == 1 and buffer[1].request == 3 and buffer[1].related == 2)
                assert(#output == 1 and output[1].request == 3 and output[1].length == 128)
                local done = select(6, 1)
                assert(#done == 1 and done[1].request == 3, 'reset incorrectly cancelled scheduled callback')
                local after, clears = select(26, 0), 0
                for _, r in ipairs(after) do
                    if r.data[0] == 4 then
                        assert(r.data[3] == 0 and r.data[6] == 4294967295)
                        clears = clears + 1
                    end
                end
                assert(clears == 3)
            else error('unknown ownership scenario') end
        end
        m.report('checks.json', {schema = 'psx.skill-checks/v1', scenario = scenario,
            passed = scenario == 'msan' and 2 or 3, expected_native_failure = scenario == 'msan'})
        m.finish()
    end
    m.breakpoint(stop, 'Exec', 4, function()
        PCSX.pauseEmulator(); timer:stop(); timer:start(1, 0, m.guard(check)); return false
    end)
    m.atTarget(function()
        PCSX.pauseEmulator()
        PCSX.nextTick(m.guard(function() h.begin(8192, 1048576); PCSX.resumeEmulator() end))
    end)
end)
