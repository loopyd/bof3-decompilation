-- Full synthetic CD journals through every real consumer and raw exporter.
-- Native execution and PCM arithmetic remain separate acceptance gates.
local ffi, m, events, transactions = require 'ffi', require 'support', require 'events', require 'transactions'
local function uint(n) return ffi.new('uint64_t',n) end
local base=uint(9007199254740992)+1
local function words(values)
    local bytes={}
    for _,n in ipairs(values) do
        bytes[#bytes+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(bytes)
end
local function halves(n) return tonumber(n%4294967296),tonumber(n/4294967296) end
local function id(n) local lo,hi=halves(uint(n));return words({lo,hi}) end
local function exact(n) return events.exact(uint(n)) end
local rows,marks,state,mode,serial,payload,status,outputs,reports,audio
local slots={2,3,14,13,12,10}
local readTarget,delivered
local function emit(kind,phase,data,request,related,bytes)
    local d={};for n=0,7 do d[n]=(data or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=d,request=uint(request or 0),related=uint(related or 0),
        cycle=base,payload=bytes or ''}
    rows[#rows+1]=row;return row
end
local function context(phase,joined,request,related)
    local w={};for n=1,56 do w[n]=0 end;w[52]=254
    for n,slot in ipairs({1,2,4,9,10,11,12,13}) do w[slot]=state[n] end
    for n,slot in ipairs({18,19,20,21,23,34,35}) do w[slot]=mode[n] end
    w[14],w[15],w[16]=mode[8]%256,math.floor(mode[8]/256)%256,math.floor(mode[8]/65536)%256
    w[22]=1;w[27]=512
    if phase==1 or delivered then
        w[3],w[17],w[24]=0x22,1,1
        w[27],w[30],w[45],w[47]=66048,66048,1,1 -- next MSF, response and mutation
    end
    local lo,hi=halves(joined and base+10 or uint(0))
    return emit(34,phase,{2,joined and 1 or 0,7,0,lo,hi,56},request or 0,related or 0,
        words(w)..string.rep('\0',8)..string.char((phase==1 or delivered) and 0x22 or 0)..string.rep('\0',23))
end
local function descriptor(phase)
    local w={};for n=1,716 do w[n]=0 end
    w[2],w[15]=100,1
    return emit(47,phase,{2,716,0,100},1,0,words(w))
end
local function boundaries(phase,pending)
    for _,slot in ipairs(slots) do
        local lo,hi=halves(slot==3 and readTarget or uint(0))
        emit(37,slot,{phase,0,lo,hi,pending,(phase==4 and slot==3) and 2 or 0},0,0)
    end
end
local function pack()
    local chunks={};local size=0
    for n,row in ipairs(rows) do
        row.sequence=uint(n);row.offset=size;row.length=#row.payload
        chunks[#chunks+1]=row.payload;size=size+row.length
    end
    payload=table.concat(chunks)
    status={version=1,state=2,events=#rows,bytes=size,beginCycle=base,endCycle=base,
        failures=0,firstFailure=0,dropped=0}
end
local function fixture(joined,suppressed)
    rows,marks,serial,outputs,reports,m.captures={},{},0,{},{},{}
    readTarget=base;delivered=false
    state={0,0,31,0,0,0,0,0};mode={64,1,1,1,suppressed and 1 or 0,0x80000080,0,256}
    for slot=0,14 do emit(24,slot,{slot==3 and 1 or 0},0,slot==3 and base or 0) end
    emit(24,0xffffffff,{8})
    local settings={0,0,0,0,0,1,0,0,0,100,0,0,0,1}
    local env,scales={},{}
    for _,n in ipairs(settings) do env[#env+1]=id(n) end
    for n=1,15 do scales[n]=0x3f800000 end
    emit(34,2,{2,33868800,14,15,3,3,1,1},0,0,table.concat(env)..words(scales)..'sdlout')
    marks.image=descriptor(0);marks.initial=context(0,joined)
    local initial=string.rep('\255',2352)
    marks.buffer=emit(41,0,{0,2352},0,0,initial);boundaries(3,8)
    marks.dispatch=emit(11,3,{},0,base)
    emit(45,0,{6,0,0,0,0x80010000,0,1},1)
    local lo,hi=halves(base)
    emit(37,3,{1,6,lo,hi,0,0},0,1)
    emit(45,2,state,1,1);emit(45,4,mode,1,1);context(3,joined,1,1)
    marks.readIRQ=emit(46,1,{0,0},1,1)
    emit(38,0,{1,1,0,0},1,1);state[5],state[7]=1,1
    emit(40,0,{1,512,150,0,0},2,1)
    marks.backend=emit(40,2,{7,2340,0,0,0,0,12,0},2,1)
    marks.lookup=emit(40,1,{1,1,0},2,1)
    marks.bufferPredicate=emit(46,2,{2,1},1,1)
    local sector=string.char(128,255,0,0,1,1,4,0)..string.rep('\254',2332)
    marks.mutation=emit(41,1,{0,2340,2,512,150,1},1,2,id(1)..id(0)..id(0)..id(0)..sector)
    local flags=766+(suppressed and 1 or 0)
    marks.gate=emit(43,2,{flags,64,1,1,1,0x040101,0,0},1,2,id(1)..id(0))
    if not suppressed then
        emit(43,3,{flags,64,1,1,1,0x040101,0,0},1,2,id(1)..id(0))
        emit(43,4,{flags,64,1,1,1,0x040101,0,0},1,2,id(1)..id(0))
        marks.feed=emit(43,0,{0,37800,4032,0},3,1,id(2)..id(1)..id(0))
        marks.outcome=emit(43,1,{6,37800,4032,0,4704,joined and 1 or 0,joined and 2 or 1,0},3,
            joined and 1 or 0,id(joined and base+10 or 0)..id(1))
        mode[4]=0
    end
    -- Native readInterrupt clears m_read, schedules the next read, then looks
    -- up the next sector for GetlocP before publishing its response.
    state[1]=64;mode[8]=256
    readTarget=base+451584
    emit(10,3,{451584,0,lo,hi,0,0},0,readTarget)
    lo,hi=halves(readTarget)
    emit(37,3,{0,451584,lo,hi,8,2,0,0},0,1,id(0))
    local nextLookup=suppressed and 3 or 4
    emit(40,0,{1,66048,151,2,0},nextLookup,1)
    emit(40,2,{7,2340,0,0,0,1,12,0},nextLookup,1)
    emit(40,1,{1,1,0},nextLookup,1)
    emit(38,1,{1,1,1,0,0},1,1,string.char(0x22))
    emit(45,3,state,1,1);emit(45,5,mode,1,1);delivered=true;context(4,joined,1,1);emit(45,1,{6,0,0,0,0x80010000,0,1},1)
    -- Guest enables transfer explicitly. In 2048-byte mode the cursor starts
    -- at offset 12; readInterrupt itself did not leave transfer ready.
    serial=nextLookup+1
    emit(35,0,{2,0xbf801803,8,128,0x80010000,0,1},serial)
    emit(35,0,{4,0x1f801803,8,128,0x80010000,0,2},serial+1,serial)
    emit(45,2,state,serial+1,serial);emit(45,4,mode,serial+1,serial);context(3,joined,serial+1,serial)
    state[8]=12;mode[8]=257
    emit(45,3,state,serial+1,serial);emit(45,5,mode,serial+1,serial);context(4,joined,serial+1,serial)
    emit(35,1,{4,0x1f801803,8,128,0x80010000,0,2},serial+1,serial)
    emit(35,1,{2,0xbf801803,8,128,0x80010000,0,1},serial)
    -- PIO consumption uses the mutated byte, then updates the final cursor.
    serial=serial+2
    emit(35,0,{1,0xbf801802,8,0,0x80010000,0,1},serial)
    emit(35,0,{3,0x1f801802,8,0,0x80010000,0,2},serial+1,serial)
    emit(45,2,state,serial+1,serial);emit(45,4,mode,serial+1,serial);context(3,joined,serial+1,serial)
    marks.read=emit(42,0,{12,0x1f801802,254,1,13,1,0},1,serial+1,id(0));state[8]=13
    marks.afterRead=emit(45,3,state,serial+1,serial);marks.afterReadMode=emit(45,5,mode,serial+1,serial);context(4,joined,serial+1,serial)
    emit(35,1,{3,0x1f801802,8,254,0x80010000,1,2},serial+1,serial)
    emit(35,1,{1,0xbf801802,8,254,0x80010000,1,1},serial)
    marks.imageEnd=descriptor(1);marks.final=context(1,joined)
    marks.tail=emit(41,2,{0,2352},0,0,sector..initial:sub(2341));boundaries(4,8)
    pack()
    audio=nil
    if joined then
        local names={'volume','interpolation','reverb','mute','mono','streaming','null_sync',
            'irq_wait','decode_irq','scaler','forced_irq','voice_mute','voice_solo','xa'}
        local snapshot={configured_backend='sdl',configured_device='out'}
        for n,name in ipairs(names) do snapshot[name]=settings[n] end
        audio={report={complete=true,native={version=1,state=2,failures=0,dropped_events=0,
            dropped_frames='0',streaming_dropped='0',epoch=exact(base+10),begin_cycle=exact(base),
            end_cycle=exact(base),events=suppressed and 0 or 1,settings=snapshot}}}
        audio.event=function(index)
            assert(index==0 and not suppressed)
            return {id=1,kind=3,parent='0',cycle=exact(base),source_rate=37800,source_frames=4032,
                output_frames=4704,stereo=0,accepted=1,cdda=0}
        end
    end
end
local h={status=function() return status end,record=function(index) return rows[index+1] end,
    bytes=function(offset,length) return payload:sub(offset+1,offset+length) end}
PCSX={History=h}
local old_open,old_write,old_report=io.open,m.write,m.report
m.write=function(name,bytes) outputs[name]=bytes end
m.report=function(name,value) reports[name]=value;outputs[name]=m.json(value) end
io.open=function(name,mode)
    assert(mode=='wb');outputs[name]=''
    return {write=function(_,value) outputs[name]=outputs[name]..value;return true end,
        close=function() return true end}
end
local limits={mutations=8,bytes=8}
local function scan() return transactions.scan(h,limits,audio,{}) end
local function export()
    local raw=events.export();assert(raw.complete,raw.export_error)
    return transactions.export(h,limits,raw,audio,{})
end
for _,joined in ipairs({false,true}) do
    for _,suppressed in ipairs({false,true}) do
        fixture(joined,suppressed)
        local result=export();assert(result.complete,result.error)
        assert(result.begin_cycle=='9007199254740993' and #result.controller.accesses==5)
        assert(#result.scheduler.dispatches==1 and #result.media.lookups==2 and #result.sectors.reads==1)
        assert(#result.scheduler.schedules==1 and result.controller.responses[1].data_hex=='22')
        assert(#result.audio.feeds==(suppressed and 0 or 1) and result.audio.joined==joined)
        assert(#result.eligibility.decisions==(suppressed and 1 or 3))
        assert(result.delivery.delivered==1 and result.delivery.errors==0)
        for _,ref in ipairs({result.sectors.initial,result.sectors.final,result.sectors.mutations[1].payload}) do
            assert(ref.file=='payload.bin' and #outputs[ref.file]:sub(ref.offset+1,ref.offset+ref.bytes)==ref.bytes)
        end
        local ref=result.sectors.mutations[1].payload
        assert(outputs['payload.bin']:sub(ref.offset+1,ref.offset+ref.bytes)==marks.mutation.payload:sub(33))
        assert(not outputs['transactions.json']:find('[\128-\255]'),'binary bytes leaked into JSON')
        assert(result.sectors.mutations[1].bytes==nil and #result.exclusions==6)
    end
end
local function rejects(edit,needle)
    fixture(false,false);edit();pack()
    local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
rejects(function() marks.lookup.data[1]=0 end,'outcome/success mismatch')
rejects(function() marks.image.data[0]=1 end,'CD image layout')
rejects(function()
    marks.imageEnd.payload=marks.imageEnd.payload:sub(1,68)..words({512})..marks.imageEnd.payload:sub(73)
end,'CD image changed across capture')
rejects(function()
    for n,row in ipairs(rows) do if row==marks.image then table.remove(rows,n);break end end
end,'initial controller lacks image boundary')
rejects(function() marks.read.data[2]=127 end,'byte origin/value mismatch')
rejects(function() marks.read.data[0]=13 end,'consumption differs from controller cursor/readiness')
rejects(function() marks.read.data[3]=0 end,'consumption differs from controller cursor/readiness')
rejects(function() marks.afterRead.data[7]=14 end,'callback basic state mismatch')
rejects(function() marks.afterReadMode.data[7]=256 end,'callback mode state mismatch')
rejects(function() marks.gate.data[0]=0 end,'native stream flags mismatch')
rejects(function() marks.feed.related=uint(2) end,'wrong access owner')
rejects(function() marks.outcome.data[4]=4703 end,'frame calculation mismatch')
rejects(function() marks.tail.payload=string.rep('\0',2352) end,'final buffer differs')
rejects(function() marks.dispatch.related=base+1 end,'dispatch target/DMA mismatch')
rejects(function() marks.gate.kind=48 end,'unknown native history kind')
rejects(function() table.remove(rows,#rows) end,'scheduler capture boundaries incomplete')
rejects(function()
    for _,row in ipairs(rows) do
        if row.kind==38 and row.device==1 then row.payload=string.char(0) end
    end
    for _,row in ipairs(rows) do
        if row.kind==34 and row.device~=2 then
            row.payload=row.payload:sub(1,232)..string.char(0)..row.payload:sub(234)
        end
    end
end,'READ/ROTATING/SEEK status')
rejects(function()
    -- Remove both schedule observations and reconcile the final scheduler state:
    -- relationships alone accept this, but READ delivery requires the work.
    local lo,hi=halves(base)
    for n=#rows,1,-1 do
        local row=rows[n]
        if (row.kind==10 and row.device==3) or
            (row.kind==37 and row.device==3 and row.data[0]==0) then table.remove(rows,n)
        elseif row.kind==37 and row.data[0]==4 then
            row.data[4]=0
            if row.device==3 then row.data[2],row.data[3],row.data[5]=lo,hi,0 end
        end
    end
end,'mandatory next-read schedule')
rejects(function()
    -- A mutually consistent media record set may still omit the native
    -- trailing lookup. The final cache address must follow the retained lookup.
    for n=#rows,1,-1 do
        if rows[n].kind==40 and rows[n].request==4 then table.remove(rows,n) end
    end
end,'ordered next-sector lookup')
fixture(false,false);limits.mutations=0;assert(not pcall(scan));limits.mutations=8
fixture(false,false);status.dropped=1;assert(not pcall(scan))
fixture(false,false);marks.mutation.offset=status.bytes;assert(not pcall(scan))
fixture(false,false);marks.mutation.length=marks.mutation.length-1;assert(not pcall(scan))
fixture(false,false);local original=h.bytes
h.bytes=function(offset,length) return original(offset,math.max(0,length-1)) end
assert(not pcall(scan));h.bytes=original
fixture(true,false);audio.report.native.epoch='1';assert(not pcall(scan))
fixture(false,false);marks.read.data[2]=127
local result=export();assert(not result.complete and outputs['payload.bin']==payload)
assert(outputs['events.ndjson'] and outputs['transactions.json']:find('"complete":false',1,true))
fixture(false,false);local raw=events.export();raw.events=raw.events+1
assert(not transactions.export(h,limits,raw,nil,{}).complete)
for _,oversized in ipairs({false,true}) do
    fixture(false,false);raw=events.export()
    local encode=m.json
    m.json=function(value)
        if type(value)=='table' and value.schema=='psx.runtime-cd-transactions/v1' and value.complete then
            if oversized then return string.rep('x',16777217) end
            error('injected encoding failure')
        end
        return encode(value)
    end
    result=transactions.export(h,limits,raw,nil,{});m.json=encode
    assert(not result.complete and outputs['transactions.json']:find('"complete":false',1,true))
    assert(outputs['payload.bin']==payload and outputs['events.ndjson'])
end
io.open,m.write,m.report=old_open,old_write,old_report
print('CD transaction checks: full journal, joined/suppressed feeds, PIO provenance, binary references and corruptions passed')
