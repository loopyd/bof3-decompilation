-- Synthetic native/CD scheduler joins. The controller interface is isolated;
-- this suite does not establish complete capture or live native acceptance.
local ffi, bit, scheduler, m = require 'ffi', require 'bit', require 'scheduler', require 'support'
local function uint(n) return ffi.new('uint64_t',n) end
local base=uint(9007199254740992)+1
local zero=uint(0)
local function words(values)
    local bytes={}
    for _,n in ipairs(values) do
        bytes[#bytes+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(bytes)
end
local function halves(n) return tonumber(n%4294967296),tonumber(n/4294967296) end
local function idbytes(n) local lo,hi=halves(n);return words({lo,hi}) end
local slots={2,3,14,13,12,10}
local roles={[2]=5,[3]=6,[14]=7,[13]=9,[12]=12,[10]=11}
local rows, states, pending, current, root, queued, commands, cycle, serial
local function emit(kind,slot,values,request,related,payload)
    local d={};for n=0,7 do d[n]=(values or {})[n+1] or 0 end
    local row={kind=kind,device=slot,data=d,request=request or zero,related=related or zero,
        cycle=cycle,sequence=uint(#rows+1),payload=payload or '',frame=current,root=root,queued=queued}
    rows[#rows+1]=row;return row
end
local function body(role,owner,ownerKind,parent)
    serial=serial+1
    current={id=uint(serial),role=role or 4,owner=owner or zero,ownerKind=ownerKind or 0,
        parent=parent and parent.id or zero,stage=4}
    root=parent or current;return current
end
local function snapshot(phase)
    for _,slot in ipairs(slots) do
        local s=states[slot];local lo,hi=halves(s.target)
        emit(37,slot,{phase,0,lo,hi,pending,s.kind},s.owner)
    end
end
local function reset(bits,scale)
    rows,states,pending,commands,cycle,serial={}, {},bits or 0,{},base,0
    current,root,queued=nil,nil,zero
    for n=0,14 do
        states[n]={target=base,owner=zero,kind=0,dma=zero}
        emit(24,n,{bit.band(bit.rshift(pending,n),1)},nil,base)
    end
    emit(24,0xffffffff,{pending},nil,base)
    local storage=ffi.new('float[1]',scale or 1);local bitscale=tonumber(ffi.cast('uint32_t*',storage)[0])
    local floats={};for n=1,15 do floats[n]=bitscale end
    emit(34,2,{},nil,nil,string.rep('\0',112)..words(floats))
    emit(34,0);snapshot(3)
end
local function schedule(slot,owner,ownerKind,delay,scale,dma)
    local s=states[slot];local lo,hi=halves(s.target);local dlo,dhi=halves(s.dma)
    local target=cycle+uint(tonumber(ffi.new('float',tonumber(ffi.new('float',delay))*(scale or 1))))
    local native=emit(10,slot,{delay,pending,lo,hi,dlo,dhi},dma,nil)
    native.related=target
    pending=bit.bor(pending,bit.lshift(1,slot));lo,hi=halves(target)
    local cd=emit(37,slot,{0,delay,lo,hi,pending,ownerKind,s.kind,0},owner,current.id,idbytes(s.owner))
    s.target,s.owner,s.kind,s.dma=target,owner,ownerKind,dma or zero
    return native,cd
end
local function cancel()
    local s=states[3];local lo,hi=halves(s.target)
    pending=bit.band(pending,bit.bnot(8))
    return emit(37,3,{2,0,lo,hi,pending,s.kind},s.owner,current.id)
end
local function dispatch(slot,nested,parent,atCycle)
    local s=states[slot];local owner,kind=s.owner,s.kind
    if nested then owner,kind=parent.owner,parent.ownerKind
    else
        cycle=atCycle or s.target+10
        pending=bit.band(pending,bit.bnot(bit.lshift(1,slot)))
        emit(11,slot,{},s.dma,s.target);s.dma=zero
    end
    local frame=body(roles[slot],owner,kind,nested and parent or nil)
    frame.stage=nil
    local enter=emit(45,0,{frame.role,0,0,0,0,0,nested and 2 or 1},frame.id,frame.parent)
    local lo,hi=halves(s.target)
    local cd=emit(37,slot,{1,frame.role,lo,hi,pending,kind,nested and 1 or 0},owner,frame.id)
    if not nested then s.owner,s.kind=zero,0 end
    return cd,enter
end
local function finish()
    current,root=nil,nil;emit(34,1);snapshot(4)
end
local function scan()
    local controller={}
    function controller.current() return current end
    function controller.root() return root end
    function controller.command(id) assert(id==0 or commands[tostring(id)],'unknown command') end
    function controller.identities() return {queued=queued} end
    local tracker=scheduler.new(controller)
    for n,row in ipairs(rows) do
        row.sequence=uint(n);current,root,queued=row.frame,row.root,row.queued
        tracker.push(row,row.payload)
    end
    return tracker.finish(),tracker
end
local function rejects(row,key,value,needle)
    local old=row[key];row[key]=value
    local ok,err=pcall(scan);row[key]=old
    assert(not ok and tostring(err):find(needle,1,true),'expected rejection: '..needle..'; got '..tostring(err))
end
local function corrupt(row,index,value,needle)
    local old=row.data[index];row.data[index]=value
    local ok,err=pcall(scan);row.data[index]=old
    assert(not ok and tostring(err):find(needle,1,true),'expected rejection: '..needle..'; got '..tostring(err))
end

-- Pending prehistory has unknown ancestry. Dispatch consumes no invented ID.
reset(8);dispatch(3);finish()
local result=scan();assert(result.dispatches[1].owner=='0' and #result.boundaries==12)
assert(type(m.json(result))=='string')

-- Replacement, cancellation and already-selected callback keep stream ancestry.
reset(4);body()
local firstNative,first=schedule(3,base+1,2,128)
local secondNative,second=schedule(3,base+2,2,256)
dispatch(2,false,nil,states[3].target+10);current.stage=4
local canceled=cancel();local dispatched,entered=dispatch(3);finish()
result=scan()
assert(result.schedules[2].replaced=='9007199254740994' and result.dispatches[2].owner=='9007199254740995')
assert(result.boundaries[8].owner=='0' and #result.cancellations==1)
rejects(second,'payload',idbytes(zero),'replacement mismatch')
corrupt(second,5,1,'unknown command')
corrupt(secondNative,1,0,'pending bits mismatch')
corrupt(secondNative,2,0,'replaced wrong target')
corrupt(second,1,257,'replacement mismatch')
corrupt(canceled,4,8,'pending bits mismatch')
rejects(dispatched,'request',zero,'consumed wrong slot owner')
corrupt(dispatched,2,0,'dispatch target mismatch')
rejects(entered,'related',uint(1),'adjacent root callback')
rejects(first,'cycle',first.cycle+1,'adjacent native operation')
local boundaryIndex
for n,row in ipairs(rows) do if row.kind==37 and row.device==10 and row.data[0]==3 then boundaryIndex=n end end
local old=table.remove(rows,boundaryIndex)
local ok=pcall(scan);table.insert(rows,boundaryIndex,old);assert(not ok,'missing boundary must reject')
-- Remove only the CD schedule after its native predecessor.
for n,row in ipairs(rows) do
    if row==first then table.remove(rows,n);assert(not pcall(scan));table.insert(rows,n,row);break end
end

-- Retained owners do not grant dispatch permission, and future work is not due.
reset();dispatch(3);finish();assert(not pcall(scan),'unscheduled dispatch must reject')
reset();body();schedule(3,base,2,500);dispatch(3,false,nil,base+499);finish()
assert(not pcall(scan),'future dispatch must reject')
reset();body();schedule(3,base,2,500);cancel();dispatch(3);finish()
assert(not pcall(scan),'port cancellation must not grant snapshot eligibility')
reset(4);body();schedule(3,base,2,500);dispatch(2,false,nil,base+510);current.stage=4
cancel();dispatch(3,false,nil,base+511);finish()
assert(not pcall(scan),'cancellation eligibility must expire on cycle change')
reset(4);dispatch(2);current.stage=4;schedule(3,base,2,0)
dispatch(3,false,nil,cycle);finish()
assert(not pcall(scan),'new pending bit was absent from the current dispatch snapshot')
for _,slot in ipairs({4,5,6,8,9}) do
    reset(4);body();schedule(3,base,2,500);dispatch(2,false,nil,base+510);current.stage=4
    cancel();emit(11,slot,{},zero,base);dispatch(3,false,nil,cycle);finish()
    local accepted,err=pcall(scan)
    assert(not accepted and tostring(err):find('no pending eligibility',1,true),
        'a later native dispatch must expire read snapshot eligibility')
end

-- Native float multiplication rounds before uint64 conversion; no double-only approximation.
reset(0,1.25);body();local rounded=schedule(3,base,2,16777217,1.25);finish();result=scan()
assert(result.schedules[1].target==tostring(base+20971520):gsub('ULL$',''))
rejects(rounded,'related',rounded.related+1,'target/scale mismatch')

-- Queued command ownership persists independently of slot consumption.
reset();body();queued=uint(77);commands[tostring(queued)]=true
local _,command=schedule(2,queued,1,100);dispatch(2);finish();result=scan()
assert(result.dispatches[1].owner=='77')
rejects(command,'request',zero,'lost queued command owner')

-- Nested lid helper inherits enclosing ancestry without consuming a queued slot.
reset();body();commands[tostring(uint(77))]=true
schedule(13,uint(77),1,100)
commands[tostring(uint(55))]=true
local parent=body(8,uint(55),1)
local helper=dispatch(13,true,parent);finish();result=scan()
assert(result.dispatches[1].nested and result.dispatches[1].owner=='55')
assert(result.boundaries[10].owner=='77')
rejects(helper,'request',uint(77),'changed inherited ancestry')

-- Scheduled and nested DMA completion both preserve exact DMA request IDs.
reset();body(10,base,4);local dmaNative,dma=schedule(10,base,4,32,1,base)
dispatch(10);finish();result=scan();assert(result.dispatches[1].owner=='9007199254740993')
rejects(dma,'request',base+1,'DMA schedule owner mismatch')
corrupt(dmaNative,3,0,'replaced wrong target')
reset();local parentDma=body(10,base,4);dispatch(10,true,parentDma);finish();result=scan()
assert(result.dispatches[1].nested and result.dispatches[1].owner=='9007199254740993')

-- Outstanding scheduled work at freeze is valid, but its ancestry must survive.
reset();body();schedule(3,base,2,500);finish();result=scan()
assert(#result.dispatches==0 and result.boundaries[8].owner=='9007199254740993')
local final=rows[#rows-4]
rejects(final,'request',zero,'final ancestry mismatch')
table.remove(rows);assert(not pcall(scan),'missing final slot must reject')

-- Compose the real controller and scheduler on a prehistory read callback.
-- Media/stream/feed consumers are intentionally absent from this component test.
reset(8);dispatch(3)
local frame=current
local state={0,0,31,0,0,0,0,0};local mode={0,0,0,0,0,0,0,0}
emit(45,2,state,frame.id,frame.id);emit(45,4,mode,frame.id,frame.id);emit(34,3,{},frame.id,frame.id)
emit(45,3,state,frame.id,frame.id);emit(45,5,mode,frame.id,frame.id);emit(34,4,{},frame.id,frame.id)
emit(45,1,{6,0,0,0,0,0,1},frame.id);finish()
local w={};for n=1,56 do w[n]=0 end;w[52]=254;w[4]=31
local contextBytes=words(w)..string.rep('\0',32)
local size=0
for n,row in ipairs(rows) do
    if row.kind==34 then
        local values=row.device==2 and {2,33868800,14,15,0,0,1,1} or {2,0,7,0,0,0,56,0}
        for i=0,7 do row.data[i]=values[i+1] end
        if row.device~=2 then row.payload=contextBytes end
    end
    row.sequence=uint(n);row.offset=size;row.length=#row.payload;size=size+row.length
end
local controller=require('controller').new({version=1,state=2,failures=0,dropped=0,
    events=#rows,bytes=size,beginCycle=base,endCycle=cycle})
local joined=scheduler.new(controller)
for _,row in ipairs(rows) do controller.push(row,row.payload);joined.push(row,row.payload) end
assert(#controller.finish().accesses==1 and #joined.finish().dispatches==1)
print('CD scheduler checks: native joins, float targets, prehistory, replacement, canceled dispatch, nested helpers, DMA ancestry and corruption passed')
