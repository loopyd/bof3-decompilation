-- Explicit PLAY vectors, including raw-sample report peaks and stale responses.
package.loaded.image={tn=function() return 1 end,td=function() return 512 end}
local ffi,playback=require 'ffi',require 'playback'
local function id(n) return ffi.new('uint64_t',n) end
local function hex(s) return (s:gsub('.',function(c) return string.format('%02x',c:byte()) end)) end
local raw=string.rep('\0\128\0\64',587)..'\0\128\208\138'
local rows,frame,marks
local definitions={
    resize={38,0,{1}},report={38,0,{8}},irq={39,0,{2,31,1,1}},assertion={12,0,{4,0,4}},
    data={40,0,{1,512}},dataok={40,1,{1,1,0}},cached={40,1,{1,1,1}},
    audio={40,0,{2,512}},audiook={40,1,{2,1,0}},
    raw={41,1,{0,2352,4,512},string.rep('\0',32)..raw},
    attenuation={41,1,{0,2352,5,512},string.rep('\0',2384)},
    gate={43,5},feed={43,0,{1,44100,588,1}},outcome={43,1},
    schedule={37,14,{0,451584}},subq={46,4,{1}},
    stop={44,1,{0,2,1}},endpoint={44,1,{0,1,1}},
}
local function fixture(options)
    options=options or {};rows,marks={},{}
    local b,c={},{};for n=1,56 do b[n]=0 end
    b[1],b[3],b[4],b[9],b[10],b[11],b[12]=64,0xe2,31,3,4,3,0
    b[16],b[17],b[22],b[23],b[27],b[29],b[32],b[33],b[52],b[53]=1,1,1,1,512,2048,3,4,255,3
    for n,v in pairs(options.before or {}) do b[n]=v end
    for n=1,56 do c[n]=b[n] end
    for n,v in pairs(options.after or {}) do c[n]=v end
    local function context(w,r) return {words=w,parameters_hex='abcdef1234567890',response_hex=hex(r),
        subq_hex=hex(options.subq or '\18\2\0\1\35\0\2\5')} end
    frame={id=id(1),role=7,before_context=context(b,string.rep('Z',16)),
        after_context=context(c,(options.reply or '')..string.rep('Z',16-#(options.reply or '')))}
    local function emit(kind,phase,d,payload)
        local values={};for n=0,7 do values[n]=(d or {})[n+1] or 0 end
        local row={kind=kind,device=phase,data=values,payload=payload or '',cycle=id(9007199254740992)+1,
            request=id(1),related=id(1)}
        rows[#rows+1]=row;return row
    end
    emit(34,2,{2,options.clock or 33868800});marks.before=emit(34,3)
    for _,step in ipairs(options.steps or {}) do
        local name=type(step)=='string' and step or step[1]
        local def=assert(definitions[name]);local d={unpack(def[3] or {})}
        if type(step)=='table' then for n,v in pairs(step[2] or {}) do d[n]=v end end
        marks[name]=emit(def[1],def[2],d,def[4])
    end
    if options.reply then
        marks.publish=emit(38,1,{c[10],1,c[12],c[11],c[2]},options.reply)
    end
    emit(45,3);emit(45,5);marks.after=emit(34,4)
end
local function scan()
    local current;local tracker=playback.new({current=function() return current end},function() return {} end)
    for _,row in ipairs(rows) do if row==marks.before then current=frame end;tracker.push(row,row.payload) end
    return tracker.finish()
end
local function check(options,classification)
    fixture(options);local result=scan();assert(result.callbacks[1].classification==classification);return result
end
check({before={[16]=0}},'inactive')
check({before={[17]=0,[2]=3},steps={{'schedule',{[2]=256}}}},'busy')
local seek={before={[17]=0,[16]=0},after={[3]=0xa2,[2]=2,[17]=1,[24]=1,[30]=512,[10]=1,[11]=0,[12]=1},
    steps={'resize','irq','assertion','data','dataok'},reply='\162'}
check(seek,'seek-only')
check({before={[17]=0,[16]=0,[6]=9},after={[3]=0xa2,[17]=1,[24]=1,[30]=512,[10]=1,[11]=0,[12]=1},
    steps={'resize','data','dataok'},reply='\162'},'seek-only')
check({before={[17]=0,[16]=0,[30]=512},after={[3]=0xa2,[2]=2,[17]=1,[24]=1,[10]=1,[11]=0,[12]=1},
    steps={'resize','irq','assertion','data','cached'},reply='\162'},'seek-only')
local played={after={[27]=66048,[52]=255,[53]=1},steps={'audio','audiook','raw','gate','schedule','subq'}}
check(played,'played')
for _,changes in ipairs({{[2]=3,[18]=4},{[6]=9,[18]=6},{[18]=2},{[18]=128}}) do
    check({before=changes,after=played.after,steps=played.steps},'played')
end
check({before={[23]=0},after=played.after,steps={'audio','audiook','raw','gate','attenuation','feed','outcome','schedule','subq'}},'played')
check({before={[26]=1},after={[26]=0,[27]=66048,[53]=1},
    steps={'audio','audiook','raw','gate',{'schedule',{[2]=13547520}},'subq'}},'played')
for _,case in ipairs({{255+59*256+74*65536,0},{255+59*256+255*65536,255+59*256},{254+255*256+74*65536,254}}) do
    check({before={[27]=case[1]},after={[27]=case[2],[53]=1},steps={{'audio',{[2]=case[1]}},'audiook',
        {'raw',{[4]=case[1]}},'gate','schedule','subq'}},'played')
end
local report={before={[18]=4},after={[2]=1,[10]=8,[11]=0,[12]=1,[27]=66048,[53]=1},
    steps={'audio','audiook','raw','report',{'irq',{[1]=1}},'assertion','gate','schedule','subq'},
    reply='\226\18\2\0\2\5\255\127'}
assert(check(report,'played').reports==1)
check({before=report.before,after=report.after,steps=report.steps,subq='\18\2\0\1\35\0\3\16',
    reply='\226\18\2\0\129\35\48\245'},'played')
check({before={[18]=4,[23]=0},after=report.after,reply=report.reply,
    steps={'audio','audiook','raw','report',{'irq',{[1]=1}},'assertion','gate','attenuation','feed','outcome','schedule','subq'}},'played')
local stopped={[2]=4,[16]=0,[3]=0x62,[32]=0,[33]=0}
check({before={[18]=6,[50]=1},after=stopped,steps={'audio','audiook','raw',{'irq',{[1]=4,[4]=0}},'assertion','stop','gate'}},'stopped')
check({before={[27]=2048},after={[16]=0,[3]=0x62,[32]=0,[33]=0,[50]=1},
    steps={'endpoint',{'audio',{[2]=2048}},'audiook',{'raw',{[4]=2048}},'gate'}},'stopped')
check({before={[27]=2048,[18]=6},after={[2]=4,[16]=0,[3]=0x62,[32]=0,[33]=0,[50]=1},
    steps={'endpoint',{'audio',{[2]=2048}},'audiook',{'raw',{[4]=2048}},{'irq',{[1]=4,[4]=0}},
        'assertion',{'stop',{[3]=0}},'gate'}},'stopped')
local function rejects(options,edit,needle)
    fixture(options);edit();local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
rejects(report,function() frame.after_context.response_hex='00'..frame.after_context.response_hex:sub(3) end,'response buffer')
rejects(report,function() marks.publish.payload=string.rep('Z',8) end,'publication')
rejects(report,function() marks.irq.data[3]=0 end,'builder')
rejects(played,function() marks.raw.data[2]=5 end,'mutation')
rejects(played,function() marks.schedule.data[1]=225792 end,'delay')
rejects(played,function() marks.gate.kind=18 end,'unexpected CD PLAY')
rejects(played,function() marks.after.cycle=marks.after.cycle+1 end,'scope/cycle')
rejects(played,function() frame.after_context.parameters_hex='00' end,'parameters')
local function remove(mark)
    for n,row in ipairs(rows) do if row==marks[mark] then table.remove(rows,n);return end end
    error('test row missing: '..mark)
end
rejects(played,function() remove('gate') end,'expected gate')
rejects(played,function() remove('subq') end,'expected subq')
rejects(report,function() remove('assertion') end,'expected assert')
rejects(report,function() remove('report') end,'expected resize')
rejects(played,function()
    local a,b
    for n,row in ipairs(rows) do if row==marks.gate then a=n elseif row==marks.schedule then b=n end end
    rows[a],rows[b]=rows[b],rows[a]
end,'expected gate')
for _,n in ipairs({1,2,3,4,6,9,10,11,12,14,15,16,17,18,21,23,24,25,26,28,29,30,32,33,34,35,49,51,52,53,54,55,56}) do
    rejects(played,function() frame.after_context.words[n]=frame.after_context.words[n]+1 end,'word '..n)
end
print('CD PLAY checks: seek/IRQ branches, endpoint/autopause order, raw reports, mute, scheduling, uint8 carries and corruptions passed')
