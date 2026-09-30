-- Synthetic media/controller correlation; no disc conformance or native live claim.
local ffi, media, m = require 'ffi', require 'media', require 'support'
local function uint(n) return ffi.new('uint64_t',n) end
local zero=uint(0);local base=uint(9007199254740992)+1
local function words(values)
    local bytes={}
    for _,n in ipairs(values) do
        bytes[#bytes+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(bytes)
end
local function idbytes(n) return words({tonumber(n%4294967296),tonumber(n/4294967296)}) end
local function ids(values)
    local out={};for _,id in ipairs(values) do out[#out+1]=idbytes(id) end;return table.concat(out)
end
local function lba(msf) return ((msf%256)*60+math.floor(msf/256)%256)*75+math.floor(msf/65536) end
local rows,previous,serial,backing,latest,mutation,current
local function emit(kind,phase,values,request,related,payload)
    local d={};for n=0,7 do d[n]=(values or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=d,request=request or zero,related=related or zero,payload=payload or '',
        sequence=uint(#rows+1),cycle=base,frame=current}
    rows[#rows+1]=row;return row
end
local function context(phase)
    local w={};for n=1,56 do w[n]=0 end;w[52]=254;w[30]=previous
    return emit(34,phase,nil,nil,nil,words(w)..string.rep('\0',32))
end
local function reset(msf)
    rows,previous,serial,backing,latest,mutation,current={},msf or 0,base,zero,{},nil,nil
    context(0);current={id=uint(1),stage=4,role=6}
end
local function lookup(kind,msf,reason,count,disposition,success,flags,sector)
    serial=serial+1;local id=serial
    local prior=kind==1 and backing or zero
    local begin=emit(40,0,{kind,msf,lba(msf),tonumber(prior%4294967296),tonumber(prior/4294967296)},id,current.id)
    local backend
    if reason then
        local value=ffi.cast('uint64_t',ffi.new('int64_t',count or 0))
        backend=emit(40,2,{reason,tonumber(value%4294967296),tonumber(value/4294967296),
            kind==2 and 1 or 0,kind==2 and reason>=3 and 1 or 0,sector or 0,flags or 0,0},id,current.id)
        if flags and math.floor(flags/4)%2==1 then backing=id end
    end
    local result=emit(40,1,{kind,success and 1 or 0,disposition or 0},id,current.id)
    if kind==1 and disposition~=1 then previous=success and msf or 0 end
    latest[kind]={id=id,msf=msf};return begin,backend,result
end
local function mutate(reason,bytes)
    local source=latest[reason==4 and 2 or 1]
    if reason==5 then source=mutation.source end
    local prior=mutation and mutation.id or zero
    local id=prior+1;local count=(reason==4 or reason==5) and 2352 or (reason==1 and 8 or 2340)
    local row=emit(41,1,{0,count,reason,source.msf,lba(source.msf),reason==5 and 2 or 1},id,
        reason==5 and prior or source.id,ids({current.id,prior,zero,zero})..(bytes or string.rep('\0',count)))
    mutation={id=id,source=source};return row
end
local function feed(kind)
    local source=latest[kind+1]
    return emit(43,0,{kind,44100,588,1},base+100,current.id,ids({source.id,mutation.id,zero}))
end
local function decision(kind)
    return emit(43,kind==1 and 5 or 2,{0,0,0,0,0,0,0,kind},mutation.id,latest[kind+1].id,ids({current.id,zero}))
end
local function finish() current=nil;return context(1) end
local function scan()
    local controller={current=function() return current end}
    local tracker=media.new(controller)
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
local function replaceId(payload,offset,id) return payload:sub(1,offset)..idbytes(id)..payload:sub(offset+9) end

-- Short nonnegative data read remains native success, then cache data survives
-- an unrelated compressed CDDA backing-selection attempt.
reset()
local begin,backend=lookup(1,0x200,7,12,0,true,12,0)
local header=mutate(1)
lookup(2,0x300,4,2352,0,true,13,1)
local cache,_,cached=lookup(1,0x200,nil,nil,1,true)
local delivered=mutate(2);local gated=decision(0);local fed=feed(0);local final=finish()
local result=scan()
assert(#result.lookups==3 and result.lookups[1].outcome.count=='12' and result.lookups[3].disposition==1)
assert(result.lookups[3].prior_backing=='9007199254740995')
assert(result.mutations[2].lookup=='9007199254740996' and type(m.json(result))=='string')
corrupt(begin,2,0,'address/backing identity mismatch')
corrupt(cache,3,0,'address/backing identity mismatch')
corrupt(backend,0,6,'return/result mismatch')
corrupt(cached,2,0,'outcome/disposition mismatch')
rejects(delivered,'related',begin.request,'latest completed lookup')
corrupt(delivered,3,0x300,'mutation media address/access mismatch')
rejects(fed,'payload',replaceId(fed.payload,0,begin.request),'latest completed lookup')
rejects(gated,'request',zero,'audio media/mutation mismatch')
rejects(final,'payload',string.rep('\0',216),'final cache address mismatch')

-- Data failures can be cached at the reset MSF, preserving failure and bytes.
reset();local failed,negative=lookup(1,0x200,6,-1,0,false,12,0)
mutate(3);lookup(1,0,nil,nil,1,false);mutate(3);finish();result=scan()
assert(result.lookups[1].outcome.count=='-1' and not result.lookups[2].success)
corrupt(negative,2,0,'return/result mismatch')

-- Missing handle, rejected data-on-CDDA, and optional native pregap adjustment.
reset();lookup(1,0x200,5,0,0,false);mutate(3)
lookup(1,0x300,nil,nil,2,false);mutate(1)
lookup(1,0x400,7,2352,0,true,44,0);mutate(2);finish();scan()

-- Every synthesized or failed CDDA read must produce zero raw samples.
for reason=1,3 do
    reset();local start,backend=lookup(2,0x300,reason,reason==3 and 100 or 0,0,reason~=3,reason==3 and 8 or 0,0)
    local raw=mutate(4);decision(1);finish();result=scan()
    assert(result.lookups[1].outcome.reason==reason)
    rejects(raw,'payload',raw.payload:sub(1,32)..'x'..raw.payload:sub(34),'contains nonzero samples')
    if reason==3 then corrupt(backend,6,10,'byte-swap mismatch') end
end

-- Successful byte-swapped CDDA can contain arbitrary raw bytes; attenuation
-- retains its original lookup even though its direct source is another mutation.
reset();lookup(2,0x300,4,2352,0,true,10,10)
local raw=mutate(4,string.rep('x',2352));local attenuated=mutate(5,string.rep('y',2352))
decision(1);feed(1);finish();result=scan()
assert(result.mutations[2].lookup==result.mutations[1].lookup)
rejects(attenuated,'related',zero,'attenuation source mismatch')

-- Outcomes cannot be duplicated, omitted, interleaved or moved out of their scope.
reset();local start,backend,done=lookup(1,0x200,7,2352,0,true,12,0);finish();scan()
rejects(backend,'request',base+10,'unique lookup')
rejects(done,'related',zero,'lookup access mismatch')
rejects(done,'cycle',done.cycle+1,'boundaries interrupted')
local backendIndex
for n,row in ipairs(rows) do if row==backend then backendIndex=n end end
table.remove(rows,backendIndex);assert(not pcall(scan));table.insert(rows,backendIndex,backend)
table.insert(rows,backendIndex+1,backend);assert(not pcall(scan));table.remove(rows,backendIndex+1)
local extra={kind=45,device=2,data={[0]=0,0,0,0,0,0,0,0},cycle=base,request=zero,related=zero,payload='',frame=current}
table.insert(rows,backendIndex,extra);assert(not pcall(scan));table.remove(rows,backendIndex)

-- Wire/controller composition on an empty capture retains the initial cache MSF.
reset(0x201);finish()
for phase,row in ipairs(rows) do row.data={[0]=2,0,7,0,0,0,56,0} end
local environment={kind=34,device=2,data={[0]=2,33868800,14,15,0,0,1,1},cycle=base,
    request=zero,related=zero,payload=string.rep('\0',172)}
table.insert(rows,1,environment)
local size=0
for n,row in ipairs(rows) do row.sequence=uint(n);row.offset=size;row.length=#row.payload;size=size+row.length end
local controller=require('controller').new({version=1,state=2,events=#rows,bytes=size,failures=0,dropped=0,
    beginCycle=base,endCycle=base})
local joined=media.new(controller)
for _,row in ipairs(rows) do controller.push(row,row.payload);joined.push(row,row.payload) end
assert(#controller.finish().accesses==0 and #joined.finish().lookups==0)
print('CD media checks: cache/backing ancestry, signed outcomes, short reads, failures, silence, mutations and corruption passed')

-- Descriptor wire/geometry checks are separate from native capture acceptance.
do
    local image=require 'image'
    local function descriptor(count,nodes,changes)
        local w={};for n=1,716 do w[n]=0 end
        w[1],w[2],w[3],w[5],w[8],w[9],w[15]=count or 0,100,1,1,1,#nodes>0 and 1 or 0,1
        for n,value in pairs(changes or {}) do w[n]=value end
        local parts={words(w)}
        for n,node in ipairs(nodes) do
            local path=node.path or ''
            parts[#parts+1]=words({node.id or n,node.kind or 1,node.parent or 0,#path})..
                idbytes(node.start or zero)..idbytes(node.size or uint(4096))..path
        end
        local bytes=table.concat(parts)
        return {kind=47,device=0,data={[0]=2,716,#nodes,100,0,0,0,0},request=uint(1),related=zero,length=#bytes},bytes
    end
    local function parse(count,nodes,changes)
        local row,payload=descriptor(count,nodes,changes)
        return image.decode(row,payload),row,payload
    end
    local function fails(callback,needle)
        local ok,err=pcall(callback)
        assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
    end
    local empty=parse(0,{},{[3]=0,[8]=0})
    assert(#empty.tracks==100 and image.tn(empty)==1 and image.track(empty,99).slot==99)
    assert(image.td(empty,0)==0 and image.td(empty,1)==512 and image.length(empty,1)==0)
    assert(image.pregap(empty,2)==512 and image.pregap(parse(1,{}),2)==512)
    assert(#image.bind(empty,{}).roots==0)

    -- One root backing two different slices: slot99 remains visible despite rawcount2.
    local nodes={{path='disc/track01.bin',size=uint(100000)},
        {kind=2,parent=1,start=uint(2352),size=uint(4704)},
        {kind=2,parent=1,start=uint(7056),size=uint(2352)}}
    local function slot(n,field) return 17+n*7+field end
    local changes={[slot(1,0)]=1,[slot(1,1)]=512,[slot(1,3)]=65536*2,[slot(1,6)]=2,
        [slot(2,0)]=2,[slot(2,1)]=768,[slot(2,2)]=65536,[slot(2,3)]=65536*4,[slot(2,6)]=3,
        [slot(99,6)]=3}
    local value,row,payload=parse(2,nodes,changes)
    assert(value.files[2].start=='2352' and value.files[3].start=='7056' and value.files[2].parent==1)
    assert(image.td(value,2)==768 and image.td(value,99)==512 and image.track(value,99).file==3)
    assert(image.pregap(value,1)==0 and image.pregap(value,2)==65536 and image.length(value,2)==262144)
    assert(image.pregap(value,99)==512 and image.pregap(value,255)==512)
    local pin={key='disc:track01.bin',path_hex=value.files[1].path_hex,bytes=100000,sha256=string.rep('a',64)}
    local joined=image.bind(value,{pin})
    assert(#joined.roots==1 and joined.roots[1].input==pin.key and #joined.unused==0)
    local mounted=parse(1,{{path='/capture/disc/track01.bin',size=uint(100000)}})
    local staged=require('mount').parse('/capture','disc:track01.bin\t100000\t'..pin.sha256..'\n'..
        'disc:track02.bin\t2352\t'..pin.sha256..'\n')
    local binding=image.bind(mounted,staged)
    assert(binding.roots[1].input=='disc:track01.bin' and binding.unused[1]=='disc:track02.bin')
    fails(function() image.bind(value,staged) end,'absent from receipt') -- No basename fallback.
    fails(function() image.bind(value,{}) end,'absent from receipt')
    pin.bytes=99999;fails(function() image.bind(value,{pin}) end,'size differs');pin.bytes=100000
    fails(function() image.bind(value,{pin,pin}) end,'duplicate')
    pin.sha256='a';fails(function() image.bind(value,{pin}) end,'SHA-256');pin.sha256=string.rep('a',64)
    fails(function() image.bind(value,{[0]=pin}) end,'dense array')
    fails(function() image.bind(value,{[1]=pin,[3]=pin}) end,'dense array')
    local path=pin.path_hex;pin.path_hex='780079';fails(function() image.bind(value,{pin}) end,'contains NUL');pin.path_hex=path
    row.device=1;assert(image.decode(row,payload).phase==1);row.device=0
    row.request=uint(2);fails(function() image.decode(row,payload) end,'image record');row.request=uint(1)
    row.data[0]=1;fails(function() image.decode(row,payload) end,'image layout');row.data[0]=2
    row.length=#payload+1;fails(function() image.decode(row,payload) end,'payload length');row.length=#payload
    fails(function() image.decode(row,payload:sub(1,-2)) end,'payload length')
    row.length=#payload+1;fails(function() image.decode(row,payload..'x') end,'trailing payload')

    -- Preserve every bit above double precision and arbitrary Unix path bytes.
    local exact=uint(4294967295)*4294967296+4294967295
    local huge=parse(1,{{path='disc/\255.bin',size=exact}})
    assert(huge.files[1].bytes=='18446744073709551615' and huge.files[1].path_hex:find('ff',1,true))
    assert(not m.json(huge):find('\255',1,true))
    local receipt={key='native',path_hex=huge.files[1].path_hex,bytes='18446744073709551615',sha256=string.rep('b',64)}
    assert(image.bind(huge,{receipt}).roots[1].bytes==receipt.bytes)
    receipt.bytes=9007199254740992;fails(function() image.bind(huge,{receipt}) end,'inexact')
    receipt.bytes='18446744073709551616';fails(function() image.bind(huge,{receipt}) end,'binding size')
    receipt.bytes='01';fails(function() image.bind(huge,{receipt}) end,'binding size')

    local chain={{path='disc/root.bin'}}
    for n=2,5 do chain[n]={kind=2,parent=n-1,start=uint(n),size=uint(20)} end
    assert(parse(1,chain,{[slot(1,6)]=5}).files[5].depth==4)
    chain[6]={kind=2,parent=5};fails(function() parse(1,chain,{[slot(1,6)]=6}) end,'four slices')
    fails(function() parse(1,{{path='x'},{kind=2,parent=2}},{[slot(1,6)]=2}) end,'file parent')
    fails(function() parse(1,{{path='x'},{path='y'}}) end,'unreachable')
    fails(function() parse(1,{{path='x'},{path='y'}},{[9]=2,[slot(1,6)]=1}) end,'native traversal')
    fails(function() parse(1,{{path='x\0y'}}) end,'contains NUL')
    fails(function() parse(1,{{path=string.rep('x',4097)}}) end,'file descriptor')
    fails(function() parse(1,{{path='x'}},{[slot(99,1)]=16777216}) end,'reserved byte')
    fails(function() parse(100,{}) end,'image geometry')
    fails(function() parse(1,{{path='x'}},{[10]=1}) end,'image geometry')
    fails(function() parse(1,{{path='x'}},{[slot(1,5)]=2}) end,'track descriptor')
    fails(function() image.track(value,100) end,'allocated table')
    fails(function() image.td(value,256) end,'uint8')

    -- Raw slot values and native helper fallbacks/normalization are distinct.
    local unusual=parse(2,{}, {[slot(2,1)]=0x00503d01,[slot(2,2)]=0x000000ff})
    assert(image.track(unusual,2).start==0x00503d01 and image.td(unusual,2)==0x00050202)
    local wrapped=parse(0,{}, {[slot(0,2)]=65536})
    assert(image.td(wrapped,0)==0x002d0a45) -- Native uint32 underflow then uint8 truncation.
    print('CD image checks: all slots, exact sizes/path bytes, slice bounds/order, receipt association and native table helpers passed')
end
