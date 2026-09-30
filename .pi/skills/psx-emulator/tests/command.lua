-- Callback branch checks beyond Lua's exact-number range; other consumers own
-- native scheduler due-time validation and complete journal composition.
local ffi,command=require 'ffi',require 'command'
local function uint(n) return ffi.new('uint64_t',n) end
local base=uint(9007199254740992)+1
local rows,marks,frame
local function context(w,response)
    return {words=w,parameters_hex='aabbccddeeff0011',response_hex=response,subq_hex=string.rep('00',8)}
end
local function hex(bytes) return (bytes:gsub('.',function(c) return string.format('%02x',c:byte()) end)) end
local function fixture(case)
    case=case or {};rows,marks={},{}
    local function add(kind,phase,d,payload)
        local values={};for n=0,7 do values[n]=(d or {})[n+1] or 0 end
        local row={kind=kind,device=phase,data=values,payload=payload or '',cycle=base,
            sequence=uint(#rows+1),request=uint(1),related=uint(1)}
        rows[#rows+1]=row;return row
    end
    local b,c={},{};for n=1,56 do b[n]=0 end
    b[1],b[2],b[3],b[4],b[5],b[6],b[7],b[8],b[9]=129,case.busy and 3 or 0,0x12,31,1,1,
        case.repeated or 0,case.delay or 10,3
    b[31]=case.drive or 0
    for n=1,56 do c[n]=b[n] end
    local response=hex(string.char(0x81)..string.rep('Z',15))
    frame={id=uint(1),role=5,before_context=context(b,response),after_context=context(c,response)}
    add(25,0,{0,0});add(34,2,{2,33868800})
    local target=base+(case.target or 0)
    add(37,2,{1,5,tonumber(target%4294967296),tonumber(target/4294967296)})
    marks.before=add(34,3)
    marks.execute=add(36,1,{b[6],b[5],b[9],b[2],b[8],b[7]})
    local function schedule(delay)
        add(10,2,{delay});marks.schedule=add(37,2,{0,delay})
    end
    if case.busy then schedule(256)
    else
        c[1],c[2],c[7],c[9],c[10],c[11],c[12]=1,3,0,0,1,0,1
        c[45]=1
        marks.resize=add(38,0,{1})
        if case.retry then schedule(b[8]) else c[6]=0;if c[31]~=1 then c[3]=2 end end
        marks.irq=add(39,0,{3,31,1,1});marks.assertion=add(12,0,{4,0,4})
        marks.publication=add(38,1,{1,1,1,0,3},string.char(0x12))
        frame.after_context.response_hex=hex(string.char(0x12)..string.rep('Z',15))
    end
    add(45,3);add(45,5);marks.after=add(34,4)
end
local function scan()
    local current
    local consumer=command.new({current=function() return current end},{header=function() return string.rep('\0',8) end},
        function() return {} end)
    for _,row in ipairs(rows) do
        if row==marks.before then current=frame end
        consumer.push(row,row.payload)
    end
    return consumer.finish()
end
local function check(case,branch)
    fixture(case);local r=scan();assert(#r.callbacks==1 and r.callbacks[1].classification==branch)
end
check({busy=true,repeated=1},'busy')
check({repeated=1,retry=true},'retry')
check({repeated=255,retry=true,target=5},'retry')
check({repeated=1,target=10},'executed') -- Equal difference: strict greater-than fails.
check({repeated=1,target=-1},'executed') -- Unsigned subtraction wraps.
check({repeated=1,delay=0},'executed')
check({drive=1},'executed') -- GetStat is exempt from the drive override.
local function rejects(case,edit,needle)
    fixture(case);edit();local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
rejects({busy=true},function() frame.after_context.words[9]=0 end,'word 9')
rejects({busy=true},function() marks.schedule.data[1]=257 end,'exact schedule')
rejects({repeated=1,retry=true},function() frame.after_context.words[6]=0 end,'word 6')
rejects({},function() frame.after_context.words[9]=3 end,'word 9')
rejects({},function() frame.after_context.response_hex=string.rep('00',16) end,'response buffer')
rejects({},function() marks.irq.data[1]=0 end,'terminal IRQ')
rejects({},function() marks.assertion.data[2]=0 end,'latch transition')
rejects({},function() marks.execute.data[0]=2 end,'execution snapshot')
rejects({},function() marks.resize.kind=18 end,'unexpected CD command body')
rejects({},function() marks.after.cycle=base+1 end,'scope/cycle')
print('CD command checks: busy preservation, exact uint64 retry selection, parameter reset and corruptions passed')
