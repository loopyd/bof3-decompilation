-- Inspect/edit SPU state or capture bounded native PCM and register activity.
local m, s = require 'support', require 'snapshot'
m.begin('action,target,voice,offset,length,address,value,frames,savestate,scratch,identity,stop,seconds,samples,blocks,events,tail', function()
    local action = m.argument('action', 'inspect')
    if action == 'capture' then require('sound').run(); return end
    for _, name in ipairs({'identity', 'stop', 'seconds', 'samples', 'blocks', 'events', 'tail'}) do
        assert(not m.argument(name), name .. ' applies only to capture')
    end
    assert(action == 'inspect' or action == 'export' or action == 'write' or action == 'trace', 'unknown SPU action')
    local voice = m.integer('voice', 0, 0, 24)
    local save = m.integer('savestate', 0, 0, 1) == 1
    local function summary(state)
        local result = {ctrl = state.spu.ctrl, stat = state.spu.stat, irq = state.spu.irq,
            addr = state.spu.addr, noise_clock = state.spu.noiseClock,
            noise_count = state.spu.noiseCount, noise_value = state.spu.noiseVal,
            cycles = state.registers.cycle, voices = {}, ports = {}}
        for i = 1, 24 do
            if voice == 0 or voice == i then
                local ch = state.spu.channel[i]
                result.voices[#result.voices + 1] = {voice = i, data = ch.data,
                    adsr = ch.adsr, adsr_ex = ch.adsr_ex}
            end
        end
        local ports = assert(state.spu.ports)
        assert(#ports == 512, 'unexpected SPU register image')
        for i = 1, #ports, 2 do
            result.ports[#result.ports + 1] = {address = 0x1f801c00 + i - 1,
                value = ports:byte(i) + ports:byte(i + 1) * 256}
        end
        return result
    end
    local function finish(report)
        m.report('spu.json', report)
        if save then m.write('state.pbuf', tostring(PCSX.createSaveState())) end
        m.finish()
    end
    if action == 'inspect' or action == 'export' then
        assert(not save, 'read-only action does not create a new machine state')
        s.observe(function(state, source)
            if action == 'export' then
                local offset = m.integer('offset', 0, 0, 524288)
                local length = m.integer('length', 524288 - offset, 0, 524288 - offset)
                assert(#state.spu.ram == 524288, 'unexpected SPU RAM size')
                m.write('spu.bin', state.spu.ram:sub(offset + 1, offset + length))
                finish({schema = 'psx.runtime-spu/v1', action = action, source = source,
                    offset = offset, bytes = length})
            else finish({schema = 'psx.runtime-spu/v1', action = action, source = source, state = summary(state)}) end
        end)
        return
    end
    m.atTarget(function()
        local before = summary(s.live())
        if action == 'write' then
            local address = m.integer('address', nil, 0x1f801c00, 0x1f801dfe)
            assert(address % 2 == 0, 'SPU address must be halfword aligned')
            local value = m.integer('value', nil, 0, 65535)
            require('bus').write(16, {{address, value}}, function(probe)
                finish({schema = 'psx.runtime-spu/v1', action = action, source = 'live', probe = probe,
                    address = address, value = value, before = before, after = summary(s.live())})
            end)
        else
            local limit, events = m.integer('frames', 1, 1, 120), {}
            m.event('GPU::Vsync', function()
                events[#events + 1] = summary(s.live())
                if #events == limit then
                    PCSX.pauseEmulator()
                    PCSX.nextTick(m.guard(function()
                        finish({schema = 'psx.runtime-spu/v1', action = action, source = 'live',
                            before = before, events = events, sampling = 'GPU Vsync; not PCM or bus trace'})
                    end))
                end
            end)
            PCSX.resumeEmulator()
        end
    end)
end)
