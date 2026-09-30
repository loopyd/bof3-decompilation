-- Stream correlation fixtures; mock interfaces isolate native relationship
-- checks, followed by a composed controller/scheduler/stream boundary fixture.
local ffi, bit, streams, m = require 'ffi', require 'bit', require 'streams', require 'support'
local function uint(n) return ffi.new('uint64_t',n) end
local zero=uint(0);local base=uint(9007199254740992)+1
local function words(values)
    local bytes={}
    for _,n in ipairs(values) do
        bytes[#bytes+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(bytes)
end
local function idbytes(value) return words({tonumber(value%4294967296),tonumber(value/4294967296)}) end
local function ids(values)
    local out={};for _,id in ipairs(values) do out[#out+1]=idbytes(id) end;return table.concat(out)
end
local rows, active, native, owners, initiators, latest, queued, current, root, serial, mutation, commands, nextAccess
local function emit(kind,phase,values,request,related,payload)
    local d={};for n=0,7 do d[n]=(values or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=d,request=request or zero,related=related or zero,
        payload=payload or '',cycle=base,sequence=uint(#rows+1),frame=current,root=root,latest=latest,queued=queued,
        owners={[3]=owners[3],[14]=owners[14]}}
    rows[#rows+1]=row;return row
end
local function context(phase)
    local w={};for n=1,56 do w[n]=0 end;w[52]=254
    w[15],w[16]=native[0],native[1]
    for index,id in pairs({[41]=active[0],[43]=active[1],[47]=mutation}) do
        w[index],w[index+1]=tonumber(id%4294967296),tonumber(id/4294967296)
    end
    return emit(34,phase,nil,nil,nil,words(w)..string.rep('\0',32))
end
local function reset(read,play)
    rows,active,native,owners,initiators={},{[0]=zero,zero},{[0]=read or 0,play or 0},
        {[3]={zero,0},[14]={zero,0}},{[0]=zero,zero}
    current,root,latest,queued,serial,mutation,commands,nextAccess=nil,nil,zero,zero,zero,zero,{},0
    context(0)
end
local function body(role,owner,ownerKind)
    nextAccess=nextAccess+1
    current={id=uint(nextAccess),role=role,owner=owner or zero,ownerKind=ownerKind or 0,stage=4}
    root=current;return current
end
local function flags(phase)
    return emit(45,phase,{0,0,0,0,0,0,0,native[0]*256+native[1]*65536},current.id,root.id)
end
local function transition(kind,start,reason)
    reason=reason or 0
    local previous=active[kind];local owner=owners[kind==1 and 14 or 3]
    local trigger=reason==0 and (root.role==5 and root.owner or latest) or zero
    local command=start and trigger or initiators[kind]
    if start then serial=serial+1 end
    local row=emit(44,kind,{start and 1 or 0,reason,native[kind],0,0,0,0,owner[2]},
        start and serial or previous,command,ids({previous,current.id,mutation,owner[1],trigger}))
    local was=native[kind]
    active[kind],initiators[kind],native[kind]=start and serial or zero,start and trigger or zero,start and 1 or 0
    if not start and kind==0 and was~=0 then
        emit(37,3,{2,0,0,0,0,owner[2]},owner[1],current.id)
    end
    return row
end
local function schedule(slot,id,kind)
    owners[slot]={id,kind};return emit(37,slot,{0,0,0,0,0,kind},id,current.id)
end
local function mutate()
    local read=current.ownerKind==2 and current.owner or active[0]
    local play=current.ownerKind==3 and current.owner or active[1]
    local previous=mutation;mutation=mutation+1
    return emit(41,1,{},mutation,zero,ids({current.id,previous,read,play}))
end
local function feed(kind)
    local owner=current.ownerKind==kind+2 and current.owner or active[kind]
    return emit(43,0,{kind,44100,588,1},base+99,current.id,ids({zero,mutation,owner}))
end
local function decision(kind)
    local owner=current.ownerKind==kind+2 and current.owner or active[kind]
    local bits=(native[0]~=0 and 512 or 0)+native[1]*256
    return emit(43,kind==1 and 5 or 2,{bits,0,0,0,0,0,0,kind},mutation,zero,ids({current.id,owner}))
end
local function finish() current,root=nil,nil;return context(1) end
local function scan()
    local controller,scheduler={},{}
    function controller.current() return current end
    function controller.root() return root end
    function controller.identities() return {latest=latest,queued=queued} end
    function controller.command(id) assert(id==0 or commands[tostring(id)],'unknown command') end
    function scheduler.owner(slot) return unpack(owners[slot]) end
    local tracker=streams.new(controller,scheduler)
    for n,row in ipairs(rows) do
        row.sequence=uint(n);current,root,latest,queued,owners=row.frame,row.root,row.latest,row.queued,row.owners
        tracker.push(row,row.payload)
    end
    return tracker.finish()
end
local function rejects(row,key,value,needle)
    local old=row[key];row[key]=value
    local ok,err=pcall(scan);row[key]=old
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
local function corrupt(row,index,value,needle)
    local old=row.data[index];row.data[index]=value
    local ok,err=pcall(scan);row.data[index]=old
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
local function replaceId(payload,offset,id) return payload:sub(1,offset)..idbytes(id)..payload:sub(offset+9) end

-- Prehistory can be active without a known stream ID, and inactive stop is valid.
reset(1,1);body(4);flags(4)
transition(0,false);transition(0,false);transition(1,false);flags(5);finish()
local result=scan();assert(#result.transitions==3 and result.transitions[1].id=='0')

-- Replacement does not erase an earlier stream needed by a scheduled callback.
reset();latest=base;commands[tostring(base)]=true;body(5,base,1);flags(4)
local first=transition(0,true);local readId=active[0]
local play=transition(1,true);local playId=active[1]
local scheduled=schedule(3,readId,2)
local replacement=transition(0,true);flags(5)
body(4);latest=base+1;commands[tostring(latest)]=true
local stopped=transition(0,false);flags(5)
body(6,readId,2);local mutationRow=mutate();local readDecision=decision(0);local readFeed=feed(0)
body(7,playId,3);local playStop=transition(1,false,2)
local raw=mutate();local playDecision=decision(1);local playFeed=feed(1);flags(5)
local final=finish();result=scan()
assert(result.transitions[4].command=='9007199254740993' and result.transitions[4].trigger=='9007199254740994')
assert(result.mutations[1].read=='1' and result.mutations[1].play=='2')
assert(result.feeds[1].stream=='1' and result.feeds[2].stream=='2')
assert(result.decisions[2].stream=='2' and result.transitions[5].trigger=='0')
assert(type(m.json(result))=='string')
rejects(first,'request',base,'start identity/command mismatch')
rejects(replacement,'payload',replaceId(replacement.payload,0,zero),'transition state mismatch')
rejects(stopped,'related',base+1,'lost initiating command')
rejects(stopped,'payload',replaceId(stopped.payload,32,base),'triggering command mismatch')
rejects(stopped,'payload',replaceId(stopped.payload,24,zero),'pending ancestry mismatch')
rejects(mutationRow,'payload',replaceId(mutationRow.payload,16,zero),'mutation stream ancestry mismatch')
rejects(raw,'payload',replaceId(raw.payload,24,zero),'mutation stream ancestry mismatch')
rejects(readFeed,'payload',replaceId(readFeed.payload,16,zero),'feed stream ancestry mismatch')
rejects(playFeed,'payload',replaceId(playFeed.payload,8,zero),'feed stream ancestry mismatch')
rejects(playDecision,'payload',replaceId(playDecision.payload,8,zero),'decision stream ancestry mismatch')
corrupt(playDecision,0,256,'decision native stream flags mismatch')
corrupt(playStop,1,3,'stop reason/operation mismatch')
rejects(scheduled,'request',playId,'wrong-kind CD stream owner')
rejects(final,'payload',replaceId(final.payload,160,readId),'final identity mismatch')
for n,row in ipairs(rows) do
    if row.kind==37 and row.data[0]==2 then
        table.remove(rows,n);assert(not pcall(scan),'missing read-stop cancellation must reject');table.insert(rows,n,row);break
    end
end

-- Schedule ancestry derives from active play, queued command, or callback context.
reset();commands[tostring(base)]=true;body(5,base,1)
transition(1,true);local activePlay=active[1]
schedule(14,activePlay,3);schedule(12,activePlay,3);schedule(13,base,1)
queued=base;schedule(2,queued,1)
body(7,activePlay,3);transition(1,false,1)
local inherited=schedule(12,activePlay,3);finish();scan()
rejects(inherited,'request',zero,'selected wrong stream/context owner')

-- A stopped read schedules unknown active ancestry, while its executing owner survives.
reset();commands[tostring(base)]=true;body(5,base,1);transition(0,true)
local oldRead=active[0];body(4);transition(0,false)
body(6,oldRead,2);local empty=schedule(3,zero,2);mutate();finish();scan()
rejects(empty,'request',oldRead,'selected wrong stream/context owner')

-- Mode records detect missing native start/stop observations.
reset();body(4);local changed=flags(5);finish();scan()
corrupt(changed,7,256,'changed without a transition')
reset();body(4);emit(37,3,{2,0,0,0,0,0},zero,current.id);finish()
assert(not pcall(scan),'unexplained cancellation must reject')

-- Compose real controller and scheduler on an empty CD capture with prehistory
-- active streams. Unknown IDs remain zero; active native flags are preserved.
reset(1,1);finish()
local contexts={rows[1],rows[2]};rows={}
for slot=0,14 do emit(24,slot,{0},zero,base) end
emit(24,0xffffffff,{0},zero,base)
local scales={};for n=1,15 do scales[n]=0x3f800000 end
emit(34,2,{2,33868800,14,15,0,0,1,1},zero,zero,string.rep('\0',112)..words(scales))
for phase=0,1 do
    local contextRow=contexts[phase+1]
    contextRow.data={[0]=2,0,7,0,0,0,56,0};rows[#rows+1]=contextRow
    for _,slot in ipairs({2,3,14,13,12,10}) do
        emit(37,slot,{phase==0 and 3 or 4,0,tonumber(base%4294967296),tonumber(base/4294967296),0,0})
    end
end
local size=0
for n,row in ipairs(rows) do row.sequence=uint(n);row.offset=size;row.length=#row.payload;size=size+row.length end
local controller=require('controller').new({version=1,state=2,events=#rows,bytes=size,failures=0,dropped=0,
    beginCycle=base,endCycle=base})
local scheduler=require('scheduler').new(controller);local joined=streams.new(controller,scheduler)
for _,row in ipairs(rows) do
    controller.push(row,row.payload);scheduler.push(row,row.payload);joined.push(row,row.payload)
end
assert(#controller.finish().accesses==0 and #scheduler.finish().boundaries==12 and #joined.finish().transitions==0)
print('CD stream checks: prehistory, replacement, initiating commands, stopped callback ancestry, native flags, scheduling and corruption passed')
