-- Validate retained MDEC ownership across CHCR clears, quantization and partial output.
local m, events, ffi = require 'support', require 'events', require 'ffi'
m.begin('target,stop,precapture', function()
    local stop, pre = m.address('stop'), m.integer('precapture', 0, 0, 1) == 1
    local h, timer = PCSX.History, luv.new_timer()
    m.retained[#m.retained + 1] = timer
    timer:start(15000, 0, m.guard(function() error('MDEC fixture timeout') end))
    local function check()
        timer:stop(); timer:close(); h.stop()
        local report = events.export()
        assert(report.complete)
        local function select(kind, channel)
            local values = {}
            for i = 0, report.events - 1 do
                local r = h.record(i)
                if r.kind == kind and (channel == nil or r.device == channel) then values[#values + 1] = r end
            end
            return values
        end
        local input = pre and 0 or 2
        local quant = pre and 1 or 3
        local output = pre and 2 or 4
        local data, quant_bytes, decode_bytes = select(4, 0), 0, 0
        for _, r in ipairs(data) do
            if r.request == quant then
                assert(r.length == 1 and h.bytes(r.offset, r.length) == '\1')
                quant_bytes = quant_bytes + tonumber(r.length)
            else
                assert(r.request == input and r.length == 2)
                decode_bytes = decode_bytes + tonumber(r.length)
            end
        end
        assert(quant_bytes == 128 and decode_bytes == (pre and 0 or 24))
        local out, buffer = select(4, 1), select(27, 1)
        assert(#out == (pre and 1 or 2) and #buffer == (pre and 0 or 1))
        if not pre then
            assert(out[1].request == 1 and out[1].related == input and out[1].length == 128)
            assert(buffer[1].request == 1 and buffer[1].related == input and buffer[1].length == 512)
            local deferred = select(8, 1)
            assert(#deferred == 1 and deferred[1].request == 1)
        end
        local last = out[#out]
        assert(last.request == output and last.related == input and last.length == 384)
        local completion = select(6, 0)
        assert(#completion == 2 and completion[1].request == quant and completion[2].request == input,
            'nested completion rebound retained input to newer DMA0 request')
        local done = select(6, 1)
        assert(#done == 1 and done[1].request == output and done[1].sequence > completion[2].sequence)
        local schedules, replaced = select(10), false
        for _, r in ipairs(schedules) do
            if r.request == output then
                assert(r.data[4] == (pre and 0 or 1) and r.data[5] == 0)
                replaced = true
            end
        end
        assert(replaced, 'replaced scheduler ownership absent')
        local ram = ffi.cast('uint8_t*', PCSX.getMemPtr())
        assert(ffi.string(ram + 0x30400, 512) == string.rep('\16\66', 256), 'decoded pixels differ')
        if pre then
            local initial, found = select(26, 0), false
            for _, r in ipairs(initial) do
                if r.data[0] == 0 then assert(r.data[6] == 128 and r.related == 0); found = true end
            end
            assert(found, 'pre-capture partial buffer not described')
        end
        m.report('checks.json', {schema = 'psx.skill-checks/v1', precapture = pre, passed = 5,
            checks = {'compressed/quantization byte ownership', 'partial buffer output identity',
                'nested DMA0/DMA1 completion identity', 'replaced callback owner', 'exact decoded pixels'}})
        m.write('state.pbuf', tostring(PCSX.createSaveState())); m.finish()
    end
    m.breakpoint(stop, 'Exec', 4, function()
        PCSX.pauseEmulator(); timer:stop(); timer:start(1, 0, m.guard(check)); return false
    end)
    m.atTarget(function()
        PCSX.pauseEmulator()
        PCSX.nextTick(m.guard(function() h.begin(8192, 1048576); PCSX.resumeEmulator() end))
    end)
end)
