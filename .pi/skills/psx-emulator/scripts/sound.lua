-- Bound native audio observations without serializing state during recording.
local m, pcm = require 'support', require 'pcm'
local M = {}

function M.run()
    m.begin('action,target,identity,frames,stop,seconds,samples,blocks,events,tail', function()
        local h, uv = assert(PCSX.Audio, 'native audio capture binding required'), assert(luv)
        local identity = assert(m.argument('identity'), 'identity is required')
        assert(#identity > 0 and #identity <= 160, 'identity must be 1..160 bytes')
        if m.input('state', false) then
            assert(os.getenv('PSX_RUNTIME_ORIGIN_VALIDATED') == '1',
                'audio state capture requires a matching origin receipt')
        end
        local frames = m.integer('frames', 60, 1, 36000)
        local tail = m.integer('tail', 0, 0, 3600)
        local seconds = m.integer('seconds', 20, 1, 300)
        local samples = m.integer('samples', 441000, 1, 2646000)
        local blocks = m.integer('blocks', 16384, 1, 65536)
        local events = m.integer('events', 16384, 1, 65536)
        local stop = m.argument('stop') and m.address('stop')
        local started, closing, count, tail_count = false, false, 0, 0
        local primary, timer = nil, uv.new_timer()
        local function finish(reason, success)
            if closing then return end
            closing = true
            PCSX.pauseEmulator()
            timer:stop(); timer:close()
            -- A separate timer survives nextTick dispatch and leaves device/Pause callbacks.
            local deferred = uv.new_timer()
            m.retained[#m.retained + 1] = deferred
            deferred:start(1, 0, m.guard(function()
                deferred:stop(); deferred:close()
                local result, capture_error
                local ok, err = pcall(function()
                    if started then
                        h.stop()
                        result = pcm.export()
                        m.write('final.pbuf', tostring(PCSX.createSaveState()))
                    end
                end)
                if not ok then capture_error = tostring(err) end
                success = success and ok and result ~= nil and result.complete
                m.report('sound.json', {schema = 'psx.runtime-sound/v1', identity = identity,
                    identity_authority = 'caller label; input and tool hashes in harness receipt',
                    reason = reason, success = success, started = started,
                    primary = primary, frames = count, frame_limit = frames,
                    tail_frames = tail_count, tail_limit = tail, seconds = seconds, stop = stop,
                    sample_limit = samples, block_limit = blocks, event_limit = events,
                    native_complete = result and result.complete or false, capture_error = capture_error,
                    boundary = 'pause then deferred native freeze; host callback may append PCM between them',
                    tail = 'additional guest Vsync interval; no injected key-off, queue drain or DSP-end claim'})
                if success then m.finish() else error('incomplete audio capture: ' .. reason) end
            end))
        end
        local function reached(reason)
            if primary or closing then return end
            primary = {reason = reason, frames = count, observed_pcm_frames = h.status().frames}
            if tail == 0 then finish(reason, true) end
        end
        timer:start(seconds * 1000, 0, m.guard(function() finish('wall timeout', false) end))
        m.retained[#m.retained + 1] = timer
        m.event('GPU::Vsync', function()
            if not started or closing then return end
            count = count + 1
            if h.status().failures ~= 0 then finish('native recorder failure', false)
            elseif primary then
                tail_count = tail_count + 1
                if tail_count == tail then finish('tail interval reached', true) end
            elseif count >= frames then
                if stop then finish('frame limit before stop', false) else reached('frame limit') end
            end
        end)
        m.event('ExecutionFlow::Pause', function()
            if started and not closing then finish('unexpected native pause', false) end
        end)
        if stop then m.breakpoint(stop, 'Exec', 4, function()
            if started and not closing then reached('stop reached'); return false end
            return true
        end) end
        m.atTarget(function()
            PCSX.pauseEmulator()
            PCSX.nextTick(m.guard(function()
                if closing then return end
                m.requireIdleCapture()
                assert(h.status().state == 0, 'audio recorder must be empty before initial state capture')
                m.write('initial.pbuf', tostring(PCSX.createSaveState()))
                h.begin(samples, blocks, events)
                started = true
                if h.status().failures ~= 0 then finish('initial recorder failure', false)
                else
                    if stop and tonumber(PCSX.getRegisters().pc) == stop then reached('stop already reached') end
                    if not closing then PCSX.resumeEmulator() end
                end
            end))
        end)
    end)
end

return M
