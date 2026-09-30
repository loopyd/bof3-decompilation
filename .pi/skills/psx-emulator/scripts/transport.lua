-- Bound a native CD journal, optionally enclosed by a native PCM capture.
local m, events, transactions = require 'support', require 'events', require 'transactions'
local M = {}

function M.run()
    m.begin('action,target,identity,frames,stop,seconds,events,bytes,mutations,reads,audio,samples,blocks,spuevents', function()
        local h,uv=assert(PCSX.History,'native CD history binding required'),assert(luv)
        assert(type(h.beginCD)=='function','native CD history binding required')
        local bindings=require('mount').read()
        local identity=assert(m.argument('identity'),'identity is required')
        assert(#identity>0 and #identity<=160,'identity must be 1..160 bytes')
        if m.input('state',false) then
            assert(os.getenv('PSX_RUNTIME_ORIGIN_VALIDATED')=='1','CD state capture requires a matching origin receipt')
        end
        local joined=m.integer('audio',0,0,1)==1
        local a=joined and assert(PCSX.Audio,'native audio capture binding required') or nil
        local pcm=joined and require('pcm') or nil
        local frames=m.integer('frames',60,1,36000)
        local seconds=m.integer('seconds',20,1,300)
        local event_limit=m.integer('events',65536,53,65536)
        local byte_limit=m.integer('bytes',8388608,11116,8388608)
        local limits={mutations=m.integer('mutations',4096,1,65536),bytes=m.integer('reads',65536,1,65536)}
        local samples,blocks,spuevents
        if joined then
            samples=m.integer('samples',441000,1,2646000)
            blocks=m.integer('blocks',16384,1,65536)
            spuevents=m.integer('spuevents',16384,1,65536)
        else
            assert(not m.argument('samples') and not m.argument('blocks') and not m.argument('spuevents'),
                'samples, blocks and spuevents require audio=1')
        end
        local stop=m.argument('stop') and m.address('stop')
        local started,audio_started,closing,count=false,false,false,0
        local timer=uv.new_timer()
        local function finish(reason,success,start_error)
            if closing then return end
            closing=true;PCSX.pauseEmulator();timer:stop();timer:close()
            -- Leave breakpoint/Vsync/device scopes before freezing. History
            -- must freeze while joined Audio is still active.
            local deferred=uv.new_timer();m.retained[#m.retained+1]=deferred
            deferred:start(1,0,m.guard(function()
                deferred:stop();deferred:close()
                local failures=m.array()
                if start_error then failures[#failures+1]=start_error end
                local function attempt(label,callback)
                    local ok,value=pcall(callback)
                    if not ok then failures[#failures+1]=label..': '..tostring(value);return nil end
                    return value
                end
                if started then attempt('history freeze',function() h.stop();return true end) end
                if audio_started then attempt('audio freeze',function() a.stop();return true end) end
                local native,sound,decoded
                if started then native=attempt('history export',events.export) end
                if audio_started then sound=attempt('audio export',pcm.export) end
                if started then
                    decoded=attempt('CD correlation',function()
                        return transactions.export(h,limits,native,joined and {report=sound,event=a.event} or nil,bindings)
                    end)
                end
                if started or audio_started then
                    attempt('final state',function()
                        m.requireIdleCapture()
                        m.write('final.pbuf',tostring(PCSX.createSaveState()));return true
                    end)
                end
                success=success and #failures==0 and native~=nil and native.complete and
                    decoded~=nil and decoded.complete and (not joined or (sound~=nil and sound.complete))
                m.report('transport.json',{schema='psx.runtime-cd-capture/v1',identity=identity,
                    identity_authority='caller label; input/tool hashes in receipt',reason=reason,success=success,
                    started=started,audio_started=audio_started,audio_joined=joined,frames=count,frame_limit=frames,
                    seconds=seconds,stop=stop,event_limit=event_limit,byte_limit=byte_limit,limits=limits,
                    sample_limit=samples,block_limit=blocks,spu_event_limit=spuevents,
                    native_complete=native and native.complete or false,
                    correlation_complete=decoded and decoded.complete or false,
                    audio_complete=sound and sound.complete or false,errors=failures,
                    boundary='paused initial state; Audio then History begin; deferred History then Audio freeze; final state',
                    scope='bounded observations and correlation; no command-completion, disc-conformance or audible-fidelity claim'})
                if success then m.finish() else error('incomplete CD capture: '..reason) end
            end))
        end
        timer:start(seconds*1000,0,m.guard(function() finish('wall timeout',false) end))
        m.retained[#m.retained+1]=timer
        m.event('GPU::Vsync',function()
            if not started or closing then return end
            count=count+1
            if h.status().failures~=0 or (joined and a.status().failures~=0) then finish('native recorder failure',false)
            elseif count>=frames then finish(stop and 'frame limit before stop' or 'frame limit',not stop) end
        end)
        m.event('ExecutionFlow::Pause',function()
            if started and not closing then finish('unexpected native pause',false) end
        end)
        if stop then m.breakpoint(stop,'Exec',4,function()
            if started and not closing then finish('stop reached',true);return false end
            return true
        end) end
        m.atTarget(function()
            PCSX.pauseEmulator()
            PCSX.nextTick(m.guard(function()
                if closing then return end
                m.requireIdleCapture()
                assert(h.status().state==0 and (not joined or a.status().state==0),'CD capture recorders must be empty')
                m.write('initial.pbuf',tostring(PCSX.createSaveState()))
                local ok,err=pcall(function()
                    if joined then a.begin(samples,blocks,spuevents);audio_started=true end
                    h.beginCD(event_limit,byte_limit,joined);started=true
                end)
                if not ok then
                    -- A binding may fail after recorder allocation. Freeze any
                    -- active recorder before exporting or serializing state.
                    started=h.status().state~=0
                    audio_started=joined and a.status().state~=0
                    finish('recorder startup failed',false,tostring(err))
                elseif h.status().failures~=0 or (joined and a.status().failures~=0) then
                    finish('initial recorder failure',false)
                elseif stop and tonumber(PCSX.getRegisters().pc)==stop then finish('stop already reached',true)
                else PCSX.resumeEmulator() end
            end))
        end)
    end)
end
return M
