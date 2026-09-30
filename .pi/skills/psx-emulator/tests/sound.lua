-- Simulate mission dispatch/lifecycle; live SDL and CPU behavior need native validation.
local m = require 'support'
local options, callbacks, timers, next_tick, outputs, reports, native, paused, quit, stop_hook, export_ok, write_fault
local state_input, origin, getenv = nil, nil, os.getenv
os.getenv = function(name)
    if name == 'PSX_RUNTIME_ORIGIN_VALIDATED' then return origin end
    return getenv(name)
end
package.loaded.pcm = {export=function()
    assert(native.state == 2 and paused)
    return {complete=export_ok and native.failures == 0}
end}
local sound = require 'sound'
m.argument = function(name, default) return options[name] or default end
m.input = function(name) return name == 'state' and state_input or nil end
m.write = function(name, value)
    if name == write_fault then error('injected output failure') end
    outputs[name] = value
end
m.report = function(name, value) reports[name] = value end
m.event = function(name, callback) callbacks[name] = m.guard(callback) end
m.breakpoint = function(_, _, _, callback) stop_hook = m.guard(callback) end
m.atTarget = function(callback) callback() end
m.finish = function() outputs['completion.json'] = true; quit = 0; m.done = true end
luv = {new_timer=function()
    local timer = {}
    function timer:start(ms, _, callback) self.ms, self.callback = ms, callback end
    function timer:stop() self.stopped = true end
    function timer:close() self.closed = true end
    timers[#timers+1] = timer
    return timer
end}
PCSX = {History={status=function() return {state=0} end}, Audio={begin=function()
    assert(paused and native.state == 0 and outputs['initial.pbuf'])
    native.state = 1
end, stop=function() assert(paused and native.state == 1); native.state = 2 end,
    status=function() return native end},
    pauseEmulator=function() paused=true; if callbacks['ExecutionFlow::Pause'] then callbacks['ExecutionFlow::Pause']() end end,
    resumeEmulator=function() assert(native.state == 1); paused=false end,
    nextTick=function(callback) next_tick=callback end,
    createSaveState=function() assert(native.state ~= 1, 'state captured while recording'); return 'state' end,
    getRegisters=function() return {pc=0x80010000} end,
    quit=function(code) quit=code end}
local function reset(args)
    options = args or {}; options.identity = 'fixture'; options.frames = options.frames or '2'
    callbacks, timers, outputs, reports = {}, {}, {}, {}
    native, paused, quit, next_tick, stop_hook = {state=0,failures=0,frames=0}, true, nil, nil, nil
    export_ok, write_fault, m.done, m.retained = true, nil, false, {}
    sound.run()
end
local function frame() callbacks['GPU::Vsync']() end
local function freeze()
    assert(#timers == 2 and timers[1].closed)
    timers[2].callback()
    assert(timers[2].closed)
    return reports['sound.json']
end
reset(); next_tick(); assert(not paused); frame(); assert(not quit); frame()
assert(paused and native.state == 1 and not outputs['final.pbuf'])
local result=freeze(); assert(result.success and quit==0 and outputs['completion.json'] and native.state==2)
reset({tail='2'}); next_tick(); frame(); frame(); assert(not paused); frame(); assert(not paused); frame()
result=freeze(); assert(result.success and result.frames==4 and result.primary.frames==2 and result.tail_frames==2)
reset({stop='0x80010000',tail='1'}); next_tick(); assert(not paused); frame()
result=freeze(); assert(result.success and result.primary.reason=='stop already reached' and result.frames==1)
reset({stop='0x80010004'}); next_tick(); stop_hook()
result=freeze(); assert(result.success and result.reason=='stop reached')
reset({stop='0x80010004'}); next_tick(); frame(); frame()
result=freeze(); assert(not result.success and quit==1 and not outputs['completion.json'])
reset(); next_tick(); native.failures=1; frame()
result=freeze(); assert(not result.success and result.reason=='native recorder failure' and outputs['final.pbuf'])
reset(); next_tick(); PCSX.pauseEmulator()
result=freeze(); assert(not result.success and result.reason=='unexpected native pause')
reset(); timers[1].callback(); next_tick()
result=freeze(); assert(not result.started and native.state==0 and not outputs['initial.pbuf'])
reset(); next_tick(); timers[1].callback()
result=freeze(); assert(not result.success and result.reason=='wall timeout' and native.state==2)
reset(); next_tick(); export_ok=false; frame(); frame()
result=freeze(); assert(not result.success and not outputs['completion.json'])
reset(); next_tick(); write_fault='final.pbuf'; frame(); frame()
result=freeze(); assert(not result.success and result.capture_error:find('injected output failure'))
reset({samples='2646001'}); assert(quit==1 and native.state==0 and #timers==0)
reset(); native.state=1; next_tick(); assert(quit==1 and not outputs['initial.pbuf'])
reset(); native.state=2; next_tick(); assert(quit==1 and not outputs['initial.pbuf'])
local audio=PCSX.Audio; PCSX.Audio=nil
reset(); assert(quit==1 and #timers==0); PCSX.Audio=audio
state_input='state.pbuf'; reset(); assert(quit==1 and #timers==0 and not outputs['initial.pbuf'])
origin='1'; reset(); next_tick(); frame(); frame(); result=freeze(); assert(result.success)
state_input,origin=nil,nil
PCSX.History={status=function() return {state=0} end}
native.state=0; m.requireIdleCapture()
native.state=1; assert(not pcall(m.requireIdleCapture))
native.state=2; m.requireIdleCapture()
PCSX.History.status=function() return {state=1} end
assert(not pcall(m.requireIdleCapture))
PCSX.History.status=function() return {state=0} end
PCSX.Audio=nil; m.requireIdleCapture()
os.getenv=getenv
print('Sound mission contract checks passed: deferred lifecycle, stops, tails, timeout, failures and missing binding')
