-- Observe a bounded interval of guest DMA, IRQ and timer execution.
local m, events, uv = require 'support', require 'events', assert(luv)
m.begin('target,identity,frames,stop,seconds,events,bytes', function()
    local h = assert(PCSX.History, 'native history binding required')
    local identity = assert(m.argument('identity'), 'identity is required')
    assert(#identity > 0 and #identity <= 160, 'identity must be 1..160 bytes')
    local frames = m.integer('frames', 1, 1, 36000)
    local seconds = m.integer('seconds', 20, 1, 300)
    local event_limit = m.integer('events', 16384, 1, 65536)
    local byte_limit = m.integer('bytes', 1048576, 0, 8388608)
    local stop = m.argument('stop') and m.address('stop')
    local started, closing, count, timer = false, false, 0, uv.new_timer()
    local function finish(reason, success)
        if closing then return end
        closing = true
        PCSX.pauseEmulator()
        timer:stop(); timer:close()
        local drain = uv.new_timer()
        m.retained[#m.retained + 1] = drain
        drain:start(1, 0, m.guard(function()
            drain:stop(); drain:close()
            local result
            if started then
                h.stop()
                result = events.export()
                success = success and result.complete
                m.write('final.pbuf', tostring(PCSX.createSaveState()))
            end
            m.report('history.json', {schema = 'psx.runtime-history/v1', identity = identity,
                identity_authority = 'caller label; input and tool hashes in harness receipt',
                reason = reason, success = success, started = started, frames = count,
                frame_limit = frames, seconds = seconds, stop = stop,
                event_limit = event_limit, byte_limit = byte_limit,
                native_complete = result and result.complete or false})
            if success then m.finish() else error('incomplete history: ' .. reason) end
        end))
    end
    timer:start(seconds * 1000, 0, m.guard(function() finish('wall timeout', false) end))
    m.retained[#m.retained + 1] = timer
    m.event('GPU::Vsync', function()
        if not started or closing then return end
        count = count + 1
        if h.status().failures ~= 0 then finish('native recorder failure', false)
        elseif count >= frames then finish(stop and 'frame limit before stop' or 'frame limit', not stop) end
    end)
    m.event('ExecutionFlow::Pause', function()
        if started and not closing then finish('unexpected native pause', false) end
    end)
    if stop then m.breakpoint(stop, 'Exec', 4, function()
        if started and not closing then finish('stop reached', true); return false end
        return true
    end) end
    m.atTarget(function()
        PCSX.pauseEmulator()
        PCSX.nextTick(m.guard(function()
            if closing then return end
            m.write('initial.pbuf', tostring(PCSX.createSaveState()))
            h.begin(event_limit, byte_limit)
            started = true
            if h.status().failures ~= 0 then finish('initial recorder failure', false)
            elseif stop and tonumber(PCSX.getRegisters().pc) == stop then finish('stop already reached', true)
            else PCSX.resumeEmulator() end
        end))
    end)
end)
