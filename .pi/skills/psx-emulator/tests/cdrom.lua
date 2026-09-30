-- Exercise the installed CD profile and joined-recorder guards without media I/O.
local m,events,transactions,ffi = require 'support',require 'events',require 'transactions',require 'ffi'
m.begin('',function()
    PCSX.pauseEmulator()
    local bindings=require('mount').read()
    local h,a=assert(PCSX.History),assert(PCSX.Audio)
    assert(type(h.beginCD)=='function','native CD history binding required')
    for _,args in ipairs({{0,16384,false},{52,16384,false},{65537,16384,false},{256,5307,false},{256,8388609,false},
        {256,16384,0},{1.5,16384,false}}) do
        assert(not pcall(h.beginCD,unpack(args)),'invalid CD profile budget accepted')
    end
    assert(not pcall(h.beginCD,256,16384,true),'joined CD began without active Audio')
    PCSX.resumeEmulator()
    assert(not pcall(h.beginCD,256,16384,false),'CD capture began while running')
    PCSX.pauseEmulator()
    h.begin(256,0);h.stop()
    for n=0,tonumber(h.status().events)-1 do assert(h.record(n).kind<34,'ordinary profile gained CD records') end
    assert(h.status().failures==0,'ordinary zero-payload profile changed');h.clear()
    h.beginCD(256,16384,false)
    assert(not pcall(h.beginCD,256,16384,false) and not pcall(h.clear),'active CD capture replaced')
    assert(not pcall(m.requireIdleCapture) and not pcall(h.record,0),'active capture exposed mutable evidence')
    h.stop()
    assert(h.status().failures==0 and not pcall(h.beginCD,256,16384,false),'frozen CD capture replaced')
    local unjoined=transactions.scan(h,{mutations=8,bytes=8},nil,bindings)
    assert(unjoined.complete and not unjoined.audio.joined and #unjoined.controller.contexts==2)
    assert(not pcall(transactions.scan,h,{mutations=8,bytes=8},nil,nil),
        'missing explicit media identities accepted')
    if #unjoined.image.initial.binding.roots>0 then
        assert(not pcall(transactions.scan,h,{mutations=8,bytes=8},nil,{}),
            'unmatched native image root accepted')
        local altered={}
        for n,entry in ipairs(bindings) do
            altered[n]={key=entry.key,path_hex=entry.path_hex,bytes=tostring(tonumber(entry.bytes)+1),sha256=entry.sha256}
        end
        assert(not pcall(transactions.scan,h,{mutations=8,bytes=8},nil,altered),
            'native size disagreement with staged input accepted')
    end
    h.clear()
    local failures=m.array()
    for _,restart in ipairs({false,true}) do
        a.begin(44100,1024,1024);h.beginCD(256,16384,true)
        local epoch=a.status().epoch
        a.stop()
        if restart then a.clear();a.begin(44100,1024,1024);assert(a.status().epoch~=epoch) end
        h.stop()
        assert(h.status().failures~=0,'joined Audio lifecycle change concealed without a feed')
        failures[#failures+1]={restart=restart,bits=tonumber(h.status().failures)}
        if restart then a.stop() end
        a.clear();h.clear()
    end
    -- Correctly enclosed empty capture exports through the same real consumers
    -- as the mission. It establishes binding/lifecycle support, not CD playback.
    a.begin(44100,1024,1024);h.beginCD(256,16384,true)
    local epoch=a.status().epoch
    h.stop();a.stop()
    local observed
    for n=0,tonumber(h.status().events)-1 do
        local row=h.record(n)
        if row.kind==34 and row.device==0 then
            assert(row.data[1]==1)
            observed=events.exact(ffi.new('uint64_t',row.data[5])*4294967296+row.data[4])
        end
    end
    assert(observed==epoch,'CD context did not retain exact Audio epoch')
    local native=events.export();local sound=require('pcm').export()
    local joined=transactions.export(h,{mutations=8,bytes=8},native,{report=sound,event=a.event},bindings)
    assert(native.complete and sound.complete and joined.complete,joined.error)
    m.report('checks.json',{schema='psx.skill-checks/v1',passed=9,failures=failures,
        checks={'invalid budgets','running guard','ordinary profile compatibility','active ownership/drain',
            'frozen ownership','unjoined correlation','staged identities','joined discontinuities','joined exact epoch/export'},
        scope='empty native CD profile and lifecycle; no command/media/audio playback claim'})
    m.finish()
end)
