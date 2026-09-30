-- Isolate READ branch obligations from scheduler scaling and media/SubQ models.
local ffi, bit, delivery = require 'ffi', require 'bit', require 'delivery'
local function uint(n) return ffi.new('uint64_t',n) end
local rows,marks,frame
local function values(w)
    return {w[1],w[2],w[4],w[9],w[10],w[11],w[12],w[13]},
        {w[18],w[19],w[20],w[21],w[23],w[34],w[35],w[14]+w[15]*256+w[16]*65536}
end
local function context(w)
    return {words=w,parameters_hex=string.rep('12',8),response_hex=string.rep('34',16),subq_hex=string.rep('00',8)}
end
local function add(kind,phase,d,payload,id)
    local data={};for n=0,7 do data[n]=(d or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=data,sequence=uint(#rows+1),cycle=uint(100),
        request=uint(id or 1),related=uint(1),payload=payload or ''}
    rows[#rows+1]=row;return row
end
local function fixture(case)
    case=case or {};rows,marks={},{}
    local b,c={},{};for n=1,56 do b[n]=0 end
    b[1],b[2],b[3],b[6]=case.ctrl or 0,case.stat or 0,case.status or 0,case.irq or 0
    b[4]=case.mask or 0
    b[13],b[14],b[15],b[18]=73,case.read or 0,case.reading or 1,case.mode or 0
    b[19],b[20],b[21]=7,9,1
    b[26],b[27],b[49]=case.location or 0,case.first or 512,case.retry or 0
    for n=1,56 do c[n]=b[n] end
    frame={id=uint(1),role=6,before_context=context(b),after_context=context(c)}
    frame.before,frame.before_mode=values(b)
    add(34,2,{2,case.clock or 33868800})
    marks.before=add(34,3)
    local branch=case.branch or 'delivered'
    local function schedule(delay)
        add(10,3,{delay});marks.schedule=add(37,3,{0,delay})
    end
    local function lookup(msf,success,id)
        local first=add(40,0,{1,msf},nil,id)
        local last=add(40,1,{1,success and 1 or 0,0},nil,id)
        return first,last
    end
    local function irq(value)
        local asserted=bit.band(value,b[4])~=0
        marks.irq=add(39,0,{value,b[4],asserted and 1 or 0,1})
        if asserted then marks.assertion=add(12,0,{4,case.istat or 0,bit.bor(case.istat or 0,4)}) end
    end
    if branch=='busy' then schedule(case.delay or 256)
    elseif branch~='inactive' then
        marks.predicate=add(46,1,{case.istat or 0,case.imask or 0})
        if branch=='irq-delay' then
            schedule(case.delay or 225792);c[49]=1
        else
            marks.builder=add(38,0,{1})
            marks.lookup,marks.outcome=lookup(b[27],case.success~=false,2)
            marks.buffer=add(46,2,{2,case.buffer==false and 0 or 1})
            local reason=branch=='error-zero' and 3 or 2
            marks.mutation=add(41,1,{0,2340,reason},string.rep('\0',38)..
                string.char(case.submode or 0)..string.rep('\0',2333))
            c[3],c[17],c[10],c[11],c[12]=case.expectedStatus or 0x22,1,1,0,1
            c[45],c[47]=1,1
            local response=c[3]
            if branch=='error-zero' then
                c[2],c[24]=5,0
                c[30]=case.success==false and 0 or b[27]
                response=bit.bor(response,1)
                irq(5)
            else
                c[1],c[14],c[49],c[26]=bit.bor(b[1],64),0,0,0
                c[27]=case.following or 66048
                schedule(case.delay or 451584)
                c[2]=case.dataReady==false and 0 or 1
                if c[2]~=0 then irq(1) end
                marks.next,marks.nextOutcome=lookup(c[27],case.prefetch~=false,3)
                c[24],c[30]=case.prefetch==false and 0 or 1,case.prefetch==false and 0 or c[27]
            end
            marks.publication=add(38,1,{1,1,1,0,c[2]},string.char(response))
        end
    end
    frame.after,frame.after_mode=values(c)
    add(45,3);add(45,5);marks.after=add(34,4)
    return case
end
local function scan()
    local current
    local consumer=delivery.new({current=function() return current end})
    for n,row in ipairs(rows) do
        current=n>1 and frame or nil
        consumer.push(row,row.payload)
    end
    return consumer.finish()
end
local function check(case,classification)
    fixture(case);local result=scan()
    assert(#result.callbacks==1 and result.callbacks[1].classification==classification)
    if case.delay~=nil then assert(result.callbacks[1].delay==case.delay) end
    return result
end
check({branch='inactive',reading=0},'inactive')
check({branch='busy',stat=3},'busy')
check({branch='busy',irq=0x103,reading=2},'busy')
check({branch='irq-delay',istat=4,imask=4},'irq-delay')
check({branch='irq-delay',istat=7,imask=12,clock=225,delay=1},'irq-delay')
check({branch='irq-delay',istat=4,imask=4,clock=1,delay=0},'irq-delay')
check({retry=255,istat=4,imask=4},'delivered')
check({istat=4,imask=0},'delivered')
check({mode=128,location=1,clock=225,delay=30},'delivered')
check({mode=128,clock=1,delay=0},'delivered')
check({mode=0,location=1,delay=13547520},'delivered')
check({prefetch=false},'delivered')
check({mode=64,submode=4,dataReady=false},'delivered')
check({status=0xc1,expectedStatus=0xa3},'delivered')
check({mask=31,istat=0x81},'delivered')
check({branch='error-zero',success=false,ctrl=64,read=2,retry=3,location=1},'error-zero')
check({branch='error-zero',buffer=false,status=0xc0,expectedStatus=0xa2},'error-zero')
local function rejects(case,edit,needle)
    fixture(case);edit();local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
rejects({},function() marks.schedule.data[1]=451585 end,'exact delay mismatch')
rejects({},function() marks.buffer.data[1]=0 end,'evaluated success/buffer')
rejects({},function() frame.after_context.words[49]=1 end,'movement/flags mismatch')
rejects({},function() frame.after_context.words[13]=74 end,'unrelated context word')
rejects({},function() frame.after_context.words[24]=0 end,'final cache/success')
rejects({},function() marks.predicate.device=8 end,'unexpected CD READ predicate')
rejects({},function() marks.buffer.data[0]=1 end,'buffer predicate site/order')
rejects({},function() table.remove(rows,3) end,'buffer predicate site/order')
rejects({branch='busy',irq=1},function() marks.schedule.data[1]=257 end,'exact delay mismatch')
rejects({branch='inactive',reading=0},function() frame.after_context.words[3]=1 end,'unrelated context word')
rejects({branch='irq-delay',istat=4,imask=4},function() frame.after_context.words[49]=0 end,'retry flag was not set')
rejects({branch='error-zero',success=false},function() frame.after_context.words[19]=9 end,'unrelated context word')
rejects({},function() marks.after.cycle=uint(101) end,'changed CPU cycle')
rejects({},function() frame.after_context.response_hex='34'..string.rep('56',15) end,'unused response bytes')
rejects({branch='error-zero',success=false},function()
    frame.after_context.response_hex=string.rep('56',16)
end,'unused response bytes')
local function inject(kind,phase,d)
    local extra=add(kind,phase,d)
    table.remove(rows);table.insert(rows,#rows-3,extra)
    for n,row in ipairs(rows) do row.sequence=uint(n) end
end
rejects({},function() inject(18,0) end,'unexpected CD READ body record')
rejects({},function() inject(12,0,{4}) end,'unexpected CD READ body record')
rejects({mask=31},function() marks.assertion.kind=18 end,'lacks adjacent native IRQ assertion')
rejects({mask=31},function() marks.assertion.data[2]=0 end,'lacks adjacent native IRQ assertion')
rejects({mask=31},function() marks.irq.data[1]=0;marks.irq.data[2]=0 end,'mask/assertion differs from context')
rejects({},function() inject(43,2) end,'XA work outside delivered sector window')
-- Preserve coverage for native uint8 MSF carries, including noncanonical fields.
for _,pair in ipairs({{0x004a0200,0x00000300},{0x004a3bff,0},
    {0x00ff0200,0x00000200},{0x004aff00,0}}) do
    check({first=pair[1],following=pair[2]},'delivered')
end
rejects({first=512,following=0x004c0100},function() end,'ordered next-sector lookup')
print('CD READ checks: every branch, exact delays, evaluated outcomes, state preservation and uint8 MSF carries passed')
