-- Feed outcome/join fixtures. Gate/decoder and media ancestry are separate tests.
local ffi, feeds, m = require 'ffi', require 'feeds', require 'support'
local function uint(n) return ffi.new('uint64_t',n) end
local zero=uint(0);local base=uint(9007199254740992)+1;local epoch=base+10
local names={'volume','interpolation','reverb','mute','mono','streaming','null_sync',
    'irq_wait','decode_irq','scaler','forced_irq','voice_mute','voice_solo','xa'}
local function words(values)
    local bytes={}
    for _,n in ipairs(values) do
        bytes[#bytes+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(bytes)
end
local function halves(n) return tonumber(n%4294967296),tonumber(n/4294967296) end
local function idbytes(n) local lo,hi=halves(n);return words({lo,hi}) end
local function exact(n) return tostring(n):gsub('ULL$','') end
local rows,audio,current,joined,serial,settings
local function emit(kind,phase,values,request,related,payload)
    local d={};for n=0,7 do d[n]=(values or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=d,request=request or zero,related=related or zero,payload=payload or '',
        sequence=uint(#rows+1),cycle=base,frame=current}
    rows[#rows+1]=row;return row
end
local function context(phase)
    local lo,hi=halves(joined and epoch or zero)
    return emit(34,phase,{2,joined and 1 or 0,7,0,lo,hi,56,0},phase>=3 and uint(1) or nil,phase>=3 and uint(1) or nil,
        string.rep('\0',204)..words({254})..string.rep('\0',48))
end
local function reset(useAudio,streaming)
    rows,joined,current,serial={},useAudio,nil,uint(1)
    settings={0,0,0,0,0,streaming==false and 0 or 1,0,0,0,100,0,0,0,1}
    local bytes={};for _,n in ipairs(settings) do bytes[#bytes+1]=idbytes(uint(n)) end
    local scales={};for n=1,15 do scales[n]=0x3f800000 end
    emit(34,2,{2,33868800,14,15,3,3,1,1},nil,nil,table.concat(bytes)..words(scales)..'sdlout')
    context(0)
    if joined then
        local snapshot={configured_backend='sdl',configured_device='out'}
        for n,name in ipairs(names) do snapshot[name]=settings[n] end
        audio={events={},report={complete=true,native={version=1,state=2,failures=0,dropped_events=0,
            dropped_frames='0',streaming_dropped='0',epoch=exact(epoch),begin_cycle=exact(base),end_cycle=exact(base),
            events=0,settings=snapshot}}}
        audio.event=function(index) return audio.events[index+1] end
    else audio=nil end
    current={id=uint(1),role=6,stage=4}
end
local function feed(cdda,rate,frames,stereo,reason,state,eventId)
    current={id=current.id,role=cdda and 7 or 6,stage=4}
    serial=serial+1
    local begin=emit(43,0,{cdda and 1 or 0,rate,frames,stereo},serial,current.id,idbytes(zero)..idbytes(zero)..idbytes(zero))
    local output=reason>=5 and math.floor(44100*frames/rate) or 0
    local nativeState=state or (joined and 1 or 0)
    local event=eventId or (joined and reason>=6 and uint(#audio.events+1) or zero)
    local disposition=reason<6 and 0 or (nativeState~=1 and 1 or (event~=0 and 2 or 3))
    local outcome=emit(43,1,{reason,reason>=5 and rate or 0,reason>=5 and frames or 0,
        reason>=5 and stereo or 0,output,nativeState,disposition,cdda and 1 or 0},serial,event,
        idbytes(joined and epoch or zero)..idbytes(current.id))
    if joined and reason>=6 and not eventId then
        audio.events[#audio.events+1]={id=#audio.events+1,kind=3,parent='0',cycle=exact(base),source_rate=rate,
            source_frames=frames,stereo=stereo,output_frames=output,accepted=reason==6 and 1 or 0,cdda=cdda and 1 or 0}
        audio.report.native.events=#audio.events
    end
    return begin,outcome
end
local function finish() current=nil;return context(1) end
local function scan()
    local controller={current=function() return current end}
    local tracker=feeds.new(controller,audio)
    for n,row in ipairs(rows) do current=row.frame;row.sequence=uint(n);tracker.push(row,row.payload) end
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

-- Streaming suppression, zero rate, closed SPU and zero output retain distinct metadata.
reset(false,false);feed(false,37800,4032,0,1);finish();assert(scan().feeds[1].outcome[1]==1)
reset(false);feed(false,0,0,0,3);feed(false,37800,4032,0,4);feed(false,37800,0,0,5);finish()
local result=scan();assert(#result.feeds==3 and result.feeds[1].outcome[2]==0)
reset(false,false);feed(true,44100,588,1,6);finish();assert(scan().feeds[1].outcome[5]==588)
reset(false);feed(false,37800,4032,0,7);finish();assert(scan().feeds[1].outcome[7]==1)

-- Exact forward joins allow unrelated Audio prefix/suffix records at the same cycle.
reset(true)
audio.events[1]={id=1,kind=3,parent='0',cycle=exact(base),source_rate=44100,source_frames=588,
    output_frames=588,stereo=1,accepted=1,cdda=1};audio.report.native.events=1
local begin,xa=feed(false,37800,4032,0,6)
local _,cdda=feed(true,44100,588,1,6)
audio.events[4]={id=4,kind=3,parent='0',cycle=exact(base),source_rate=44100,
    source_frames=588,output_frames=588,stereo=1,accepted=1,cdda=1};audio.report.native.events=4
local final=finish();result=scan()
assert(result.joined and result.audio_epoch=='9007199254741003' and #result.feeds==2)
assert(result.feeds[1].audio_event=='2' and result.feeds[2].audio_event=='3')
assert(result.feeds[1].outcome[5]==4704 and type(m.json(result))=='string')
rejects(xa,'payload',idbytes(epoch+1)..idbytes(uint(1)),'changed joined Audio lifecycle')
rejects(xa,'related',uint(1),'feed/Audio event mismatch')
rejects(xa,'related',uint(5),'lacks retained Audio event')
rejects(cdda,'related',uint(2),'joined more than once')
rejects(xa,'cycle',base+1,'outcome identity mismatch')
corrupt(xa,4,4703,'frame calculation mismatch')
corrupt(xa,2,4031,'outcome metadata mismatch')
corrupt(xa,5,0,'event disposition mismatch')
local event=audio.events[2]
rejects(event,'source_frames',4031,'feed/Audio event mismatch')
rejects(event,'cycle','9007199254740994','feed/Audio event mismatch')
rejects(event,'parent','1','feed/Audio event mismatch')
rejects(audio.report.native,'epoch',exact(epoch+1),'status/epoch mismatch')
rejects(audio.report.native,'end_cycle',exact(base-1),'ended before CD capture')
rejects(audio.report.native.settings,'streaming',0,'environment mismatch')
rejects(audio.report,'complete',false,'complete frozen Audio export')
rejects(audio.report.native,'streaming_dropped','1','status/epoch mismatch')
rejects(audio.report.native,'epoch','18446744073709551616','invalid Audio uint64')

-- Early joined suppression has no Audio event but still pins its active epoch.
reset(true,false);feed(false,0,0,0,1);finish();result=scan()
assert(result.feeds[1].observed_audio_event=='0' and not result.feeds[1].audio_event)

-- Disposition 3 preserves an append failure in an unjoined capture, never a join.
reset(false);feed(false,37800,4032,0,6,1,zero);finish();assert(scan().feeds[1].outcome[7]==3)
reset(true);feed(false,37800,4032,0,6,1,zero);finish();assert(not pcall(scan))

-- Impossible path selection, missing outcomes and early metadata fabrication reject.
reset(false,false);local _,early=feed(false,37800,4032,0,1);finish();scan()
corrupt(early,1,37800,'early CD feed outcome inspected metadata')
reset(false);feed(false,37800,4032,0,2);finish();assert(not pcall(scan))
reset(false,false);feed(false,37800,4032,0,6);finish();assert(not pcall(scan))
reset(false);local _,outcome=feed(false,0,0,0,3);finish();scan()
corrupt(outcome,0,4,'zero-frequency outcome mismatch')
reset(false);local _,outcome=feed(false,37800,4032,0,6);finish();scan()
local position
for n,row in ipairs(rows) do if row==outcome then position=n end end
table.remove(rows,position);assert(not pcall(scan));table.insert(rows,position,outcome)
table.insert(rows,position+1,outcome);assert(not pcall(scan));table.remove(rows,position+1)

-- Compose a real controller read callback and a History-only XA feed. This
-- isolates outcome integration; scheduler/media/eligibility consumers are absent.
reset(false);local env,initial=rows[1],rows[2];rows={env,initial}
emit(45,0,{6,0,0,0,0,0,1},uint(1));emit(37,3,{1,6,0,0,0,0,0},zero,uint(1))
emit(45,2,{0,0,0,0,0,0,0,0},uint(1),uint(1));emit(45,4,{},uint(1),uint(1));context(3)
feed(false,37800,4032,0,6)
emit(45,3,{},uint(1),uint(1));emit(45,5,{},uint(1),uint(1));context(4);emit(45,1,{6,0,0,0,0,0,1},uint(1))
finish();local size=0
for n,row in ipairs(rows) do row.sequence=uint(n);row.offset=size;row.length=#row.payload;size=size+row.length end
local controller=require('controller').new({version=1,state=2,events=#rows,bytes=size,failures=0,dropped=0,
    beginCycle=base,endCycle=base})
local tracker=feeds.new(controller)
for _,row in ipairs(rows) do controller.push(row,row.payload);tracker.push(row,row.payload) end
assert(#controller.finish().accesses==1 and #tracker.finish().feeds==1)
print('CD feed checks: suppression, metadata, output frames, exact Audio epoch/event joins, lifecycle and corruption passed')
