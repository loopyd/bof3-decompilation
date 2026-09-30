-- Submission vectors focus on preserved state and ordered native side effects.
local ffi,submission=require 'ffi',require 'submission'
local function id(n) return ffi.new('uint64_t',n) end
local rows,frame,marks
local function fixture(op,options)
    options=options or {};rows,marks={},{}
    local b,c={},{};for n=1,56 do b[n]=0 end
    b[1],b[2],b[3],b[5],b[8],b[9],b[10],b[11],b[12]=64,5,0xf2,99,777,0,8,7,1
    b[17],b[27],b[28],b[32],b[33],b[35]=1,512,0x332211,3,4,0x11223344
    for n,v in pairs(options.before or {}) do b[n]=v end
    for n=1,56 do c[n]=b[n] end
    local bank=b[1]%4
    if bank==0 then
        c[1],c[5],c[6],c[8],c[12]=b[1]+128,op,op,2048,0
    end
    for n,v in pairs(options.after or {}) do c[n]=v end
    local params=options.params or '000200aabbccddee'
    local function context(w) return {words=w,parameters_hex=params,response_hex=string.rep('aa',16),subq_hex='1234567890abcdef'} end
    frame={id=id(2),role=4,raw={4,0x1f801801,8,op},before_context=context(b),after_context=context(c)}
    local function add(kind,phase,d)
        local data={};for n=0,7 do data[n]=(d or {})[n+1] or 0 end
        local row={kind=kind,device=phase,data=data,cycle=id(32),request=id(2),related=id(1)}
        rows[#rows+1]=row;return row
    end
    marks.before=add(34,3)
    if bank==0 then
        marks.submit=add(36,0,{op,b[9],b[6],b[2],b[12]})
        marks.queue=add(36,2,{b[6],b[8],b[7],op,2048,options.repeated and 1 or 0})
        add(10,2);marks.schedule=add(37,2,{0,2048})
    end
    for _,effect in ipairs(options.effects or {}) do
        if effect=='play' then marks.play=add(44,1,{0,0,b[16]})
        elseif effect=='read' then marks.read=add(44,0,{0,0,b[15]})
        elseif effect=='cancel' then marks.cancel=add(37,3,{2}) end
    end
    add(45,3);add(45,5);marks.after=add(34,4)
end
local function scan()
    local tracker=submission.new({current=function() return frame end})
    for _,row in ipairs(rows) do tracker.push(row,'') end
    return tracker.finish()
end
local function check(op,options)
    fixture(op,options);local result=scan();assert(#result.writes==1);return result
end
for _,op in ipairs({0,1,255}) do assert(check(op).commands==1) end
for _,bank in ipairs({1,2}) do assert(check(6,{before={[1]=64+bank}}).commands==0) end
check(0xfe,{before={[1]=67},after={[35]=0xfe223344}})
assert(check(1,{before={[6]=1,[7]=255},after={[6]=1,[7]=1,[8]=777},repeated=true}).repeated==1)
check(9,{before={[6]=265,[7]=0},after={[6]=265,[7]=1,[8]=777,[3]=0x92},
    repeated=true,effects={'play','read'}})
check(0,{before={[6]=0,[7]=255},after={[7]=255}})
check(4,{before={[6]=7,[7]=1},after={[7]=1}})
for _,case in ipairs({
    {'000216aabbccddee',16,1}, {'000217aabbccddee',17,0},
    {'000174aabbccddee',-1,1}, {'995974aabbccddee',449849,0},
}) do
    local msf=case[1]=='995974aabbccddee' and 99+59*256+74*65536 or
        (case[2]<0 and 256+74*65536 or 512+case[2]*65536)
    local result=check(2,{params=case[1],after={[25]=1,[28]=msf,[17]=case[3]}})
    assert(result.writes[1].location_valid)
end
check(2,{before={[17]=0},after={[25]=1,[28]=512,[17]=0}})
for _,params in ipairs({'9a0200','a00200','006000','005a00','000275','00020a'}) do
    assert(not check(2,{params=params..'aabbccddee'}).writes[1].location_valid)
end
for _,op in ipairs({6,27,9,10,28}) do
    check(op,{before={[17]=0},after={[3]=0x92,[17]=(op==10 or op==28) and 1 or 0},effects={'play','read'}})
    check(op,{before={[16]=1,[15]=2,[17]=0},after={[3]=0x12,[15]=0,[16]=0,[32]=0,[33]=0,
        [17]=(op==10 or op==28) and 1 or 0},effects={'play','read','cancel'}})
end
for _,case in ipairs({{0,64,true},{64,64,false},{65,64,true},{0,192,false}}) do
    local result=check(14,{before={[18]=case[1]},after={[18]=case[2]},params=string.format('%02x',case[2])..'0200aabbccddee'})
    assert(result.writes[1].decoder_reset_deferred==case[3])
end
check(14,{before={[16]=1},after={[18]=1},params='010200aabbccddee'})
check(14,{before={[16]=1},after={[18]=64,[16]=0,[3]=0x72,[32]=0,[33]=0},
    params='400200aabbccddee',effects={'play'}})
local function rejects(edit,needle)
    fixture(1);edit();local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
rejects(function() marks.submit.data[3]=0 end,'snapshot')
rejects(function() marks.queue.data[4]=777 end,'queue mismatch')
rejects(function() marks.schedule.data[1]=777 end,'schedule mismatch')
rejects(function() marks.after.cycle=id(33) end,'scope/cycle')
rejects(function() marks.queue.kind=38 end,'unexpected CD submission')
for _,n in ipairs({2,3,4,7,9,10,11,13,14,15,16,17,18,25,27,28,32,33,35,45,47,49,55,56}) do
    rejects(function() frame.after_context.words[n]=frame.after_context.words[n]+1 end,'word '..n)
end
for _,name in ipairs({'parameters_hex','response_hex','subq_hex'}) do
    rejects(function() frame.after_context[name]='00' end,'changed '..name)
end
print('CD submission checks: banks, queue retention, stored BCD, seek threshold, stops, mode/reset conditions and corruptions passed')
