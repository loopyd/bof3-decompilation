-- Bound native GPU history and retain the surrounding VRAM/display observations.
local m, events, ingress = require 'support', require 'events', require 'ingress'
local snapshot, display, difference = require 'snapshot', require 'display', require 'difference'
local M = {}

function M.run()
    m.begin('action,target,identity,frames,stop,seconds,events,bytes,words,packets,limit', function()
        local h, uv=assert(PCSX.History,'native GPU history binding required'),assert(luv)
        local identity=assert(m.argument('identity'),'identity is required')
        assert(#identity>0 and #identity<=160,'identity must be 1..160 bytes')
        assert(not m.input('words',false) and not m.input('ram',false),'capture does not accept offline GPU inputs')
        if m.input('state',false) then
            assert(os.getenv('PSX_RUNTIME_ORIGIN_VALIDATED')=='1','GPU state capture requires a matching origin receipt')
        end
        local frames=m.integer('frames',1,1,36000)
        local seconds=m.integer('seconds',20,1,300)
        local event_limit=m.integer('events',16384,1,65536)
        local byte_limit=m.integer('bytes',1048576,0,8388608)
        local limits={words=m.integer('words',65536,1,1048576),packets=m.integer('packets',4096,1,65536)}
        local comparison={limit=m.integer('limit',1024,1,65536)}
        local stop=m.argument('stop') and m.address('stop')
        local started,closing,count,before=false,false,0,nil
        local timer=uv.new_timer()
        local function boundary(name)
            local raw=tostring(PCSX.createSaveState())
            m.write(name..'.pbuf',raw)
            local state=snapshot.decode(raw)
            local vram=assert(state.gpu and state.gpu.vram,'GPU state lacks VRAM')
            assert(#vram==1048576,'unexpected VRAM size')
            m.write(name..'-vram.bin',vram)
            local screen,metadata=display.capture()
            m.write(name..'-screen.bin',screen)
            m.report(name..'-screen.json',metadata)
            return {vram=vram,screen=screen,metadata=metadata}
        end
        local function finish(reason,success)
            if closing then return end
            closing=true; PCSX.pauseEmulator(); timer:stop(); timer:close()
            local deferred=uv.new_timer(); m.retained[#m.retained+1]=deferred
            deferred:start(1,0,m.guard(function()
                deferred:stop(); deferred:close()
                local native,decoded,capture_error
                local ok,err=pcall(function()
                    if started then
                        h.stop()
                        native=events.export()
                        local after=boundary('final')
                        decoded=ingress.export(h,limits)
                        m.report('effects.json',{schema='psx.runtime-gpu-effects/v1',
                            vram=difference.compare(before.vram,after.vram,'bytes',false,'gpu.vram',comparison),
                            display=display.compare(before.screen,after.screen,before.metadata,after.metadata,comparison),
                            scope='whole-interval before/after storage; no per-command rasterization or causality claim'})
                    end
                end)
                if not ok then capture_error=tostring(err) end
                success=success and ok and native~=nil and native.complete and decoded~=nil and decoded.complete
                m.report('video.json',{schema='psx.runtime-video/v1',identity=identity,
                    identity_authority='caller label; input/tool hashes in receipt',reason=reason,success=success,
                    started=started,frames=count,frame_limit=frames,seconds=seconds,stop=stop,
                    event_limit=event_limit,byte_limit=byte_limit,limits=limits,difference_limit=comparison.limit,
                    native_complete=native and native.complete or false,
                    decoded_complete=decoded and decoded.complete or false,capture_error=capture_error,
                    boundary='CPU paused, deferred native freeze, then state/display observation'})
                if success then m.finish() else error('incomplete GPU capture: '..reason) end
            end))
        end
        timer:start(seconds*1000,0,m.guard(function() finish('wall timeout',false) end))
        m.retained[#m.retained+1]=timer
        m.event('GPU::Vsync',function()
            if not started or closing then return end
            count=count+1
            if h.status().failures~=0 then finish('native recorder failure',false)
            elseif count>=frames then finish(stop and 'frame limit before stop' or 'frame limit',not stop) end
        end)
        m.event('ExecutionFlow::Pause',function()
            if started and not closing then finish('unexpected native pause',false) end
        end)
        if stop then m.breakpoint(stop,'Exec',4,function()
            if started and not closing then finish('stop reached',true); return false end
            return true
        end) end
        m.atTarget(function()
            PCSX.pauseEmulator()
            PCSX.nextTick(m.guard(function()
                if closing then return end
                m.requireIdleCapture()
                ingress.requireBinding(h)
                before=boundary('initial')
                h.begin(event_limit,byte_limit); started=true
                if h.status().failures~=0 then finish('initial recorder failure',false)
                elseif stop and tonumber(PCSX.getRegisters().pc)==stop then finish('stop already reached',true)
                else PCSX.resumeEmulator() end
            end))
        end)
    end)
end
return M
