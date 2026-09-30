-- Deterministic capture lifecycle checks. Real journals are tested separately;
-- these mocks exercise ordering, asynchronous stops and evidence on failure.
local support={retained={},done=false}
local callbacks,timers,ticks,log,reports,writes,args,options,history,audio,failed,finished
local function record(value) log[#log+1]=value end
function support.argument(name,default) return args[name] or default end
function support.integer(name,default,minimum,maximum)
    local value=tonumber(args[name] or default)
    assert(value and value%1==0 and value>=minimum and value<=maximum,name..' out of bounds')
    return value
end
function support.address(name) return tonumber(args[name]) end
function support.input() return false end
function support.array(value) return value or {} end
function support.guard(callback)
    return function(...)
        if support.done then return false end
        local ok,value=pcall(callback,...)
        if not ok then failed=tostring(value);support.done=true;return false end
        return value
    end
end
function support.begin(_,callback) support.guard(callback)() end
function support.event(name,callback) callbacks[name]=support.guard(callback) end
function support.breakpoint(_,_,_,callback) callbacks.stop=support.guard(callback) end
function support.atTarget(callback) callback() end
function support.requireIdleCapture()
    assert(history.state~=1 and audio.state~=1,'state serialization during recording')
end
function support.write(name,bytes)
    assert(not writes[name],'duplicate capture');writes[name]=bytes;record(name)
end
function support.report(name,value) reports[name]=value;record(name) end
function support.finish() finished=true;support.done=true;record('finish') end
package.loaded.support=support
package.loaded.events={export=function()
    assert(history.state==2,'unfrozen history export');record('history export')
    writes['payload.bin']='raw';writes['events.ndjson']='events'
    if options.history_export then error('injected history export failure') end
    return {complete=history.failures==0}
end}
package.loaded.pcm={export=function()
    assert(audio.state==2,'unfrozen audio export');record('audio export')
    if options.audio_export then error('injected audio export failure') end
    writes['pcm.f32']='pcm';return {complete=audio.failures==0}
end}
local bindings={{key='fixture'}}
package.loaded.mount={read=function() return bindings end}
package.loaded.transactions={export=function(_,_,native,joined,observed)
    assert(observed==bindings, 'capture lost its staged media identities')
    record('correlation')
    local complete=native and native.complete and not options.correlation and
        (not joined or (joined.report and joined.report.complete)) or false
    reports['transactions.json']={complete=complete}
    return reports['transactions.json']
end}
local transport=require 'transport'
local function reset(arguments,failures)
    callbacks,timers,ticks,log,reports,writes={},{},{},{},{},{}
    args={identity='synthetic',frames='2',seconds='5'}
    for k,v in pairs(arguments or {}) do args[k]=v end
    options=failures or {};history={state=0,failures=0};audio={state=0,failures=0}
    failed,finished,support.done=nil,false,false;support.retained={}
    local h={status=function() return history end}
    if not options.missing then h.beginCD=function(_,_,joined)
        assert(history.state==0 and (not joined or audio.state==1));record('history begin')
        if options.begin then error('injected history begin failure') end
        history.state=1
        if options.partial_begin then error('injected partial history begin failure') end
        history.failures=options.initial and 1 or 0
    end end
    h.stop=function()
        assert(history.state==1 and (args.audio~='1' or audio.state==1));record('history freeze')
        if options.freeze then error('injected history freeze failure') end
        history.state=2
    end
    local a={status=function() return audio end,event=function() end}
    a.begin=function()
        assert(history.state==0 and audio.state==0);record('audio begin');audio.state=1
    end
    a.stop=function() assert(audio.state==1);record('audio freeze');audio.state=2 end
    PCSX={History=h,Audio=a,getRegisters=function() return {pc=100} end,
        createSaveState=function() support.requireIdleCapture();record('serialize');return 'state' end,
        nextTick=function(callback) ticks[#ticks+1]=callback end,
        pauseEmulator=function() if callbacks['ExecutionFlow::Pause'] then callbacks['ExecutionFlow::Pause']() end end,
        resumeEmulator=function() record('resume') end}
    luv={new_timer=function()
        local timer={}
        function timer:start(ms,_,callback) self.ms,self.callback,self.active=ms,callback,true end
        function timer:stop() self.active=false end
        function timer:close() self.closed=true end
        timers[#timers+1]=timer;return timer
    end}
end
local function tick()
    local current=ticks;ticks={};for _,callback in ipairs(current) do callback() end
end
local function flush()
    for _,timer in ipairs(timers) do if timer.ms==1 and timer.active then timer.callback() end end
end
local function start(arguments,failures) reset(arguments,failures);transport.run();tick() end
local function frames()
    callbacks['GPU::Vsync']();callbacks['GPU::Vsync']();flush()
end
local function ordered(names)
    local index=1
    for _,value in ipairs(log) do if value==names[index] then index=index+1 end end
    assert(index>#names,'lifecycle order mismatch: '..table.concat(log,','))
end
local function failure()
    assert(not finished and failed and reports['transport.json'] and not reports['transport.json'].success)
end
start();frames();assert(finished and not failed and reports['transport.json'].success)
ordered({'initial.pbuf','history begin','resume','history freeze','history export','correlation','final.pbuf','finish'})
start({audio='1'});frames();assert(finished and reports['transport.json'].audio_complete)
ordered({'initial.pbuf','audio begin','history begin','history freeze','audio freeze','history export','audio export',
    'correlation','final.pbuf','finish'})
start({audio='1',stop='100'});flush();assert(finished and reports['transport.json'].frames==0)
assert(reports['transport.json'].reason=='stop already reached')
start({stop='104'});callbacks.stop();flush();assert(finished)
start({stop='104'});frames();failure();assert(reports['transport.json'].reason=='frame limit before stop')
start();timers[1].callback();flush();failure();assert(reports['transport.json'].reason=='wall timeout')
start();callbacks['ExecutionFlow::Pause']();flush();failure()
start();history.failures=1;callbacks['GPU::Vsync']();flush();failure();assert(writes['payload.bin'])
start({audio='1'});audio.failures=1;callbacks['GPU::Vsync']();flush();failure()
start({audio='1'},{begin=true});flush();failure()
assert(audio.state==2 and history.state==0 and writes['pcm.f32'] and not writes['payload.bin'])
ordered({'audio begin','history begin','audio freeze','audio export','final.pbuf'})
start({audio='1'},{partial_begin=true});flush();failure()
ordered({'history begin','history freeze','audio freeze','history export','audio export','final.pbuf'})
start({audio='1'},{freeze=true});frames();failure()
assert(history.state==1 and audio.state==2 and not writes['final.pbuf'] and writes['pcm.f32'])
start({audio='1'},{history_export=true});frames();failure();assert(writes['pcm.f32'] and writes['final.pbuf'])
start({audio='1'},{audio_export=true});frames();failure();assert(writes['payload.bin'] and writes['final.pbuf'])
start({}, {correlation=true});frames();failure();assert(writes['payload.bin'] and writes['final.pbuf'])
start({}, {initial=true});flush();failure();assert(reports['transport.json'].frames==0)
start({}, {missing=true});assert(failed:find('native CD history binding required',1,true) and not next(writes))
start({samples='100'});assert(failed:find('require audio=1',1,true) and not next(writes))
start({bytes='5307'});assert(failed:find('out of bounds',1,true) and not next(writes))
-- Timeout while waiting for startup: never start recorders or publish success.
reset();transport.run();timers[1].callback();tick();flush();failure()
assert(not writes['initial.pbuf'] and not writes['final.pbuf'] and history.state==0)
print('CD capture lifecycle checks: ordered freeze/export, bounds, stops, partial startup and retained failures passed')
