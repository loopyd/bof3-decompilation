-- Exercise synchronous native Pause/Run events from repeatedly called Lua FFI.
local jit,util=require 'jit',require 'jit.util'
-- Establish compiled code before loading support to check the flush obligation.
local total=0
for n=1,1024 do total=total+n end
assert(total==524800 and util.traceinfo(1),'fixture did not establish a host trace')
local m=require 'support'
-- Deliberate test-only violation must fail before registering callbacks.
if m.integer('reenable',0,0,1)==1 then jit.on() end
m.begin('iterations,reenable',function()
    assert(not jit.status() and not util.traceinfo(1),'mission retained host traces')
    PCSX.pauseEmulator()
    local iterations=m.integer('iterations',512,1,4096)
    local pauses,runs=0,0
    m.event('ExecutionFlow::Pause',function() pauses=pauses+1 end)
    m.event('ExecutionFlow::Run',function() runs=runs+1 end)
    -- Already-paused calls exercise the path that emits no callback until run.
    for _=1,iterations do PCSX.pauseEmulator() end
    for _=1,iterations do PCSX.resumeEmulator();PCSX.pauseEmulator() end
    assert(pauses==iterations and runs==iterations,'native callback count mismatch')
    assert(not jit.status() and not util.traceinfo(1),'mission enabled host traces')
    m.report('callbacks.json',{iterations=iterations,pauses=pauses,runs=runs,
        host_jit=false,warmed_trace_removed=true,
        scope='synchronous native event reentry; no guest instruction execution'})
    m.finish()
end)
