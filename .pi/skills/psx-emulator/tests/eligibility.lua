-- CD audio gate/filter/decoder control-flow fixtures, independent of PCM fidelity.
local ffi, bit, eligibility, m = require 'ffi', require 'bit', require 'eligibility', require 'support'
local function uint(n) return ffi.new('uint64_t',n) end
local zero=uint(0);local base=uint(9007199254740992)+1
local function words(values)
    local bytes={}
    for _,n in ipairs(values) do
        bytes[#bytes+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(bytes)
end
local rows,mode,header,mutation,current,enabled,live,access
local function emit(kind,phase,values,request)
    local d={};for n=0,7 do d[n]=(values or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=d,request=request or zero,related=zero,payload='',cycle=base,
        sequence=uint(#rows+1),frame=current,header=header,mutation=mutation}
    rows[#rows+1]=row;return row
end
local function reset(options)
    options=options or {};rows,mutation,current,access={},zero,nil,0
    mode={options.mode or 64,options.file or 1,options.channel or 1,options.first or 1,options.muted or 0}
    enabled=options.xa~=false;live=options.live or 512
    header='\0\0\0\0'..string.char(options.source_file or 1,options.source_channel or 1,options.submode or 4,options.coding or 0)
    local environment=emit(34,2);environment.payload=string.rep('\0',104)..words({enabled and 1 or 0,0})..string.rep('\0',60)
    emit(34,0)
end
local function body(role)
    access=access+1;current={id=uint(access),role=role or 6,stage=4}
    emit(45,4,mode,current.id)
end
local function mutate(reason)
    mutation=mutation+1;return emit(41,1,{0,0,reason},mutation)
end
local function decision(phase,ret)
    local file,channel,submode,coding=header:byte(5,8)
    if phase==3 and mode[4]==1 and bit.band(mode[1],8)==0 then mode[2],mode[3]=file,channel end
    local flags=(mode[5]~=0 and 1 or 0)+(bit.band(mode[1],64)~=0 and 2 or 0)+(enabled and 4 or 0)+
        (mode[4]~=0xffffffff and 8 or 0)+(mode[3]~=255 and 16 or 0)+(file==mode[2] and 32 or 0)+
        (channel==mode[3] and 64 or 0)+(bit.band(submode,4)~=0 and 128 or 0)+live
    return emit(43,phase,{flags,mode[1],mode[2],mode[3],mode[4],
        phase==5 and 0 or (file+channel*256+submode*65536+coding*16777216),ret or 0,phase==5 and 1 or 0},mutation)
end
local function feed(cdda,rate,stereo,frames)
    local begin=emit(43,0,{cdda and 1 or 0,rate or 37800,frames or 4032,stereo or 0},base)
    local outcome=emit(43,1,{},base)
    if not cdda then mode[4]=0 end
    return begin,outcome
end
local function close() emit(45,5,mode,current.id);current=nil end
local function finish() close();emit(34,1) end
local function scan()
    local controller={current=function() return current end}
    local sectors={header=function() return header,mutation end}
    local tracker=eligibility.new(controller,sectors)
    for n,row in ipairs(rows) do current,header,mutation=row.frame,row.header,row.mutation;row.sequence=uint(n);tracker.push(row,row.payload) end
    return tracker.finish()
end
local function rejects(row,key,value,needle)
    local old=row[key];row[key]=value;local ok,err=pcall(scan);row[key]=old
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
local function corrupt(row,index,value,needle)
    local old=row.data[index];row.data[index]=value;local ok,err=pcall(scan);row.data[index]=old
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end

-- Each outer gate suppression ends before filter/decode/feed, with unchanged mode.
for _,options in ipairs({{muted=1},{mode=0},{xa=false},{first=0xffffffff}}) do
    reset(options);body();mutate(2);decision(2);finish()
    local result=scan();assert(#result.decisions==1 and not result.decisions[1].eligible)
end
-- Explicit file/channel filters and channel 255 can suppress eligible audio sectors.
for _,options in ipairs({{mode=72,source_file=2},{first=0,source_channel=2},{channel=255,source_channel=255},{submode=0}}) do
    reset(options);body();mutate(2);decision(2);decision(3);finish()
    local result=scan();assert(#result.decisions==2 and not result.decisions[2].eligible)
end

-- First-sector auto-selection changes filters before decoding. Coding 5 is
-- 18900 Hz stereo; first-sector bits defaults remain the native decoder's concern.
reset({source_file=2,source_channel=3,coding=5});body();mutate(2)
local eligible=decision(2);local filter=decision(3);local decoded=decision(4)
local begin,outcome=feed(false,18900,1);finish();local result=scan()
assert(result.decisions[3].coding_applied and result.decisions[3].decoder_success and type(m.json(result))=='string')
corrupt(filter,2,1,'flags/filter/header mismatch')
corrupt(eligible,0,0,'flags/filter/header mismatch')
corrupt(decoded,6,0xffffffff,'decoder result mismatch')
corrupt(begin,1,37800,'differs from known XA decoder metadata')
corrupt(begin,3,0,'differs from known XA decoder metadata')
rejects(decoded,'request',zero,'header identity mismatch')
rejects(outcome,'request',zero,'lacks eligible feed')
for n,row in ipairs(rows) do
    if row==filter then table.remove(rows,n);assert(not pcall(scan));table.insert(rows,n,row);break end
end
for n,row in ipairs(rows) do
    if row==begin then table.remove(rows,n);assert(not pcall(scan));table.insert(rows,n,row);break end
end
local finalMode=rows[#rows-1]
corrupt(finalMode,3,1,'final mode/filter mismatch')

-- Invalid first-sector frequency fails; continuation sectors ignore their coding byte.
reset({coding=8});body();mutate(2);decision(2);decision(3);decision(4,0xffffffff)
mode[4]=0xffffffff;finish();result=scan();assert(not result.decisions[3].decoder_success)
reset({coding=8,first=0});body();mutate(2);decision(2);decision(3);decision(4)
feed(false,44100,1);finish();result=scan();assert(not result.decisions[3].coding_applied)
reset({coding=0x32});body();mutate(2);decision(2);decision(3);decision(4)
feed(false,37800,0);finish();scan() -- Native reserved stereo defaults to mono; nbits does not reject.

-- Known decoder metadata survives callbacks and coding bytes ignored by a
-- continuation. A later first-sector nbits change resets the native sample count.
reset({coding=5});body();mutate(2);decision(2);decision(3);decision(4);feed(false,18900,1);close()
header=header:sub(1,7)..string.char(8);body();mutate(2);decision(2);decision(3);decision(4)
local continuation=feed(false,18900,1);close()
mode[4]=1;header=header:sub(1,7)..string.char(0x15);body();mutate(2);decision(2);decision(3);decision(4)
local changedBits=feed(false,18900,1,2016);finish();scan()
corrupt(continuation,1,44100,'differs from known XA decoder metadata')
corrupt(continuation,3,0,'differs from known XA decoder metadata')
corrupt(continuation,2,2016,'differs from known XA decoder metadata')
corrupt(changedBits,2,4032,'differs from known XA decoder metadata')

-- CDDA needs a raw sector and gate. Stop/mute suppresses attenuation and feed.
for _,options in ipairs({{live=0},{live=256,muted=1}}) do
    reset(options);body(7);mutate(4);decision(5);finish()
    assert(not scan().decisions[1].eligible)
end
reset({live=256});body(7);local raw=mutate(4);local gate=decision(5)
local attenuated=mutate(5);feed(true,44100,1);finish();assert(scan().decisions[1].eligible)
corrupt(gate,5,4,'flags/filter/header mismatch')
for n,row in ipairs(rows) do
    if row==attenuated then table.remove(rows,n);assert(not pcall(scan));table.insert(rows,n,row);break end
end
reset({live=256});body(7);mutate(4);finish();assert(not pcall(scan),'missing CDDA gate must reject')
reset();body();feed(false);finish();assert(not pcall(scan),'unsolicited feed must reject')
reset();body();decision(2);finish();assert(not pcall(scan),'decision without data must reject')
reset();body();mutate(2);decision(2);decision(3);decision(4);finish()
assert(not pcall(scan),'missing decoded XA feed must reject')

-- The buffer getter returns retained header bytes and fails outside boundaries.
local buffer=require('sectors').new({mutations=4,bytes=4})
assert(not pcall(buffer.header))
local initial={kind=41,device=0,data={[0]=0,2352,0,0,0,0,0,0},request=zero,related=zero,length=2352}
buffer.push(initial,string.rep('z',2352));local value,id=buffer.header();assert(value=='zzzzzzzz' and id==0)
initial.device=2;buffer.push(initial,string.rep('z',2352));assert(not pcall(buffer.header))
print('CD eligibility checks: XA gates/auto-filter/decoder paths, coding continuation, CDDA stop/mute/feed order and corruption passed')
