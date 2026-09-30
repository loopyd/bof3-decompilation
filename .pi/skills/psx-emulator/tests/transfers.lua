-- Reconcile raw native records with independent fixture bytes and completion order.
local m, events, ffi, bit = require 'support', require 'events', require 'ffi', require 'bit'
m.begin('scenario,stop', function()
    local scenario, stop = m.argument('scenario'), m.address('stop')
    local h, started, timer = PCSX.History, false, luv.new_timer()
    local function check()
        timer:stop(); timer:close()
        h.stop()
        local report = events.export()
        assert(report.complete, 'native fixture incomplete')
        local records, payload = {}, h.bytes(0, report.payload_bytes)
        for i = 0, report.events - 1 do records[#records + 1] = h.record(i) end
        local function select(kind, channel)
            local values = {}
            for _, r in ipairs(records) do
                if r.kind == kind and (channel == nil or r.device == channel) then values[#values + 1] = r end
            end
            return values
        end
        local function bytes(r) return payload:sub(r.offset + 1, r.offset + r.length) end
        local function word(value)
            local out = {}
            for i = 0, 3 do out[#out + 1] = string.char(bit.band(bit.rshift(value, i * 8), 255)) end
            return table.concat(out)
        end
        local channel = assert(({otc=6, gpu=2, gpuread=2, gpuports=2, spu=4, cd=3})[scenario], 'unknown transfer fixture')
        local gpu_read = scenario == 'gpuread' or scenario == 'gpuports'
        local data, starts, completions = select(4, channel), select(3, channel), select(6, channel)
        assert(#starts == (scenario == 'otc' and 1 or (gpu_read and 3 or 2)) and #completions == #starts)
        for i, start in ipairs(starts) do
            local completion = completions[i]
            assert(start.request == completion.request and completion.sequence > start.sequence)
            assert(bit.band(completion.data[0], 0x1000000) ~= 0 and
                bit.band(completion.data[1], 0x1000000) == 0)
            local schedule, dispatch
            for _, r in ipairs(records) do
                if r.request == start.request then
                    if r.kind == 10 then schedule = r elseif r.kind == 11 then dispatch = r end
                end
            end
            assert(schedule and dispatch and dispatch.related == schedule.related and
                dispatch.cycle >= schedule.related and dispatch.sequence < completion.sequence)
            local irq = select(7, channel)
            assert(irq[i * 2 - 1].sequence > completion.sequence and
                irq[i * 2 - 1].data[0] == 0 and irq[i * 2].data[0] == 1)
        end
        local ram = ffi.cast('uint8_t*', PCSX.getMemPtr())
        if scenario == 'otc' then
            assert(#data == 5 and #payload == 20)
            for i = 1, 4 do
                assert(data[i].data[0] == 0x30010 - i * 4)
                assert(bytes(data[i]) == word(0x3000c - i * 4))
            end
            assert(data[5].data[0] == 0x30000 and bytes(data[5]) == word(0xffffff))
            assert(ffi.string(ram + 0x30000, 16) == word(0xffffff) .. word(0x30000) ..
                word(0x30004) .. word(0x30008))
            local assertions, masked = select(12), 0
            for _, r in ipairs(assertions) do if r.data[0] == 16 then masked = masked + 1 end end
            assert(masked > 1 and #select(14) == 0, 'masked timer assertion/CPU gate differs')
            local reads = select(17)
            assert(#reads == 1 and reads[1].data[0] == 4 and reads[1].data[3] == 0x1f801104 and
                reads[1].data[4] == 16, 'native timer read context differs')
        elseif scenario == 'gpu' or gpu_read then
            assert(#data == (gpu_read and 3 or 2) and #payload == (gpu_read and 40 or 32))
            assert(bytes(data[1]) == word(0x020000ff) .. word(0) .. word(0x00100010) .. word(0))
            assert(bytes(data[2]) == word(0x0200ff00) .. word(0x00200020) .. word(0x00100010) .. word(0))
            local headers = select(5, 2)
            assert(#headers == 1 and headers[1].data[0] == 0x30100 and
                headers[1].data[1] == 0x04ffffff and headers[1].sequence < data[2].sequence)
            assert(#select(21, 2) == 1)
            if gpu_read then
                assert(data[3].data[3] == 0 and bytes(data[3]) == word(0x001f001f) .. word(0x001f001f))
                assert(ffi.string(ram + 0x30200, 8) == bytes(data[3]))
            end
            if scenario == 'gpuports' then
                local reads = {}
                for _, r in ipairs(select(33, 2)) do
                    if r.data[0] == 5 then reads[#reads + 1] = r end
                end
                assert(#reads == 2 and reads[1].data[3] == 0x001f001f and reads[2].data[3] == 0)
                assert(ffi.string(ram + 0x35000, 8) == word(reads[1].data[3]) .. word(reads[2].data[3]),
                    'native GP0 returned values differ from CPU-observed RAM')
            end
        elseif scenario == 'spu' then
            assert(#data == 16 and #payload == 32, 'SPU read/write count differs')
            for i = 1, 8 do
                assert(data[i].data[1] == (0x7fff8 + (i - 1) * 2) % 0x80000)
                assert(data[i + 8].data[1] == data[i].data[1])
                assert(data[i].data[3] == 1 and data[i + 8].data[3] == 0)
                assert(bytes(data[i]) == bytes(data[i + 8]))
            end
            assert(ffi.string(ram + 0x30200, 16) == ffi.string(ram + 0x30000, 16))
        elseif scenario == 'cd' then
            assert(#data == 2056 and #payload == 2056)
            for i, r in ipairs(data) do
                local source = i <= 8 and (2056 + i - 1) % 2060 or 12 + i - 9
                local address = i <= 8 and 0x31000 + i - 1 or 0x32000 + i - 9
                assert(r.data[0] == address and r.data[1] == source and r.data[4] == address)
                assert(r.length == 1 and bytes(r) == string.char(source % 251))
                assert(ffi.string(ram + address, 1) == bytes(r))
            end
            assert(starts[2].data[2] == 0, 'zero BCR fallback fixture missing')
        end
        m.write('state.pbuf', tostring(PCSX.createSaveState()))
        m.report('checks.json', {schema = 'psx.skill-checks/v1', scenario = scenario, passed = 3,
            checks = {'raw transfer bytes and addresses', 'request/schedule/dispatch/completion order',
                'native memory/device effects'}})
        m.finish()
    end
    m.retained[#m.retained + 1] = timer
    timer:start(15000, 0, m.guard(function() error('transfer fixture timeout') end))
    m.breakpoint(stop, 'Exec', 4, function()
        assert(started)
        PCSX.pauseEmulator()
        timer:stop(); timer:start(1, 0, m.guard(check))
        return false
    end)
    m.atTarget(function()
        PCSX.pauseEmulator()
        PCSX.nextTick(m.guard(function() h.begin(8192, 1048576); started = true; PCSX.resumeEmulator() end))
    end)
end)
