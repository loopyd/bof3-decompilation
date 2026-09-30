-- Simulate GPU mission lifecycle; native ingress and rendered effects require live checks.
local m=require 'support'
local options,callbacks,timers,next_tick,outputs,reports,native,paused,quit,stop_hook,export_ok,decode_ok,write_fault,binding
local state_input, origin, getenv=nil,nil,os.getenv
os.getenv=function(name)
    if name=='PSX_RUNTIME_ORIGIN_VALIDATED' then return origin end
    return getenv(name)
end
package.loaded.events={export=function() assert(native.state==2 and paused);return {complete=export_ok and native.failures==0} end}
package.loaded.ingress={requireBinding=function()
    assert(native.state==0,'GPU history recorder must be empty');assert(binding,'native GPU history binding required')
end,export=function() return {complete=decode_ok} end}
package.loaded.snapshot={decode=function() return {gpu={vram=string.rep('\0',1048576)}} end}
package.loaded.display={capture=function() return '',{available=false} end,
    compare=function() return {equal=true,image_comparison_available=false} end}
package.loaded.difference={compare=function() return {equal=true} end}
local video=require 'video'
m.argument=function(name,default) return options[name] or default end
m.input=function(name) return name=='state' and state_input or nil end
m.write=function(name,value) if name==write_fault then error('injected output failure') end;outputs[name]=value end
m.report=function(name,value) reports[name]=value end
m.event=function(name,callback) callbacks[name]=m.guard(callback) end
m.breakpoint=function(_,_,_,callback) stop_hook=m.guard(callback) end
m.atTarget=function(callback) callback() end
m.finish=function() outputs['completion.json']=true;quit=0;m.done=true end
luv={new_timer=function()
    local timer={}
    function timer:start(ms,_,callback) self.ms,self.callback=ms,callback end
    function timer:stop() self.stopped=true end
    function timer:close() self.closed=true end
    timers[#timers+1]=timer;return timer
end}
PCSX={History={begin=function()
    assert(paused and native.state==0 and outputs['initial.pbuf']);native.state=1
end,stop=function() assert(paused and native.state==1);native.state=2 end,status=function() return native end},
    pauseEmulator=function() paused=true;if callbacks['ExecutionFlow::Pause'] then callbacks['ExecutionFlow::Pause']() end end,
    resumeEmulator=function() assert(native.state==1);paused=false end,
    nextTick=function(callback) next_tick=callback end,
    createSaveState=function() assert(native.state~=1,'state saved during capture');return 'state' end,
    getRegisters=function() return {pc=0x80010000} end,quit=function(code) quit=code end}
local function reset(args)
    options=args or {};options.identity='fixture';options.frames=options.frames or '2'
    callbacks,timers,outputs,reports={},{},{},{}
    native,paused,quit,next_tick,stop_hook={state=0,failures=0},true,nil,nil,nil
    export_ok,decode_ok,write_fault,binding,m.done,m.retained=true,true,nil,true,false,{}
    video.run()
end
local function frame() callbacks['GPU::Vsync']() end
local function freeze()
    assert(#timers==2 and timers[1].closed);timers[2].callback();assert(timers[2].closed)
    return reports['video.json']
end
reset();next_tick();assert(not paused);frame();assert(not quit);frame()
assert(paused and native.state==1 and not outputs['final.pbuf'])
local result=freeze();assert(result.success and quit==0 and outputs['completion.json'])
assert(#outputs['initial-vram.bin']==1048576 and outputs['final-screen.bin']=='')
assert(reports['effects.json'].display.image_comparison_available==false)
reset({stop='0x80010000'});next_tick();result=freeze();assert(result.success and result.frames==0)
reset({stop='0x80010004'});next_tick();stop_hook();result=freeze();assert(result.success and result.reason=='stop reached')
reset({stop='0x80010004'});next_tick();frame();frame();result=freeze();assert(not result.success and quit==1)
reset();next_tick();native.failures=1;frame();result=freeze();assert(not result.success and outputs['final.pbuf'])
reset();next_tick();PCSX.pauseEmulator();result=freeze();assert(not result.success and result.reason=='unexpected native pause')
reset();timers[1].callback();next_tick();result=freeze();assert(not result.started and not outputs['initial.pbuf'])
reset();next_tick();timers[1].callback();result=freeze();assert(not result.success and native.state==2)
reset();next_tick();decode_ok=false;frame();frame();result=freeze();assert(not result.success and not outputs['completion.json'])
reset();next_tick();export_ok=false;frame();frame();result=freeze();assert(not result.success)
reset();next_tick();write_fault='final.pbuf';frame();frame();result=freeze();assert(not result.success and result.capture_error:find('injected output failure'))
reset({words='1048577'});assert(quit==1 and #timers==0)
reset();native.state=1;next_tick();assert(quit==1 and not outputs['initial.pbuf'])
reset();native.state=2;next_tick();assert(quit==1 and not outputs['initial.pbuf'])
reset();binding=false;next_tick();assert(quit==1 and not outputs['initial.pbuf'])
local h=PCSX.History;PCSX.History=nil;reset();assert(quit==1 and #timers==0);PCSX.History=h
state_input='state.pbuf';reset();assert(quit==1 and #timers==0)
origin='1';reset();next_tick();frame();frame();result=freeze();assert(result.success)
os.getenv=getenv
print('GPU mission checks passed: deferred capture, bounds, stops, failures, publication and binding guards')
