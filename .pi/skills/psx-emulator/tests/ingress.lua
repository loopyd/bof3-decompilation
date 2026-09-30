-- Synthetic native records test correlation, not native GPU execution.
local ffi, ingress = require 'ffi', require 'ingress'
local rows, payload, status, cycle
local function uint(n) return ffi.new('uint64_t',n) end
local function words(values)
    local bytes={}
    for _,n in ipairs(values) do
        bytes[#bytes+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(bytes)
end
local function row(kind,device,values,request,related,bytes)
    local data={}; for i=0,7 do data[i]=(values or {})[i+1] or 0 end
    local result={sequence=uint(#rows+1),cycle=cycle,kind=kind,device=device,data=data,
        request=uint(request or 0),related=uint(related or 0),offset=#payload,length=bytes and #bytes or 0}
    rows[#rows+1]=result; payload=payload..(bytes or ''); return result
end
local function scope(kind,source,origin,count,value,processor,ready,id,request)
    return row(kind,2,{source,origin,count,value,processor,0,0x80010000,ready},request,id)
end
local fixture={}
local function reset(prefix)
    rows,payload={},'';cycle=uint(9007199254740992)+1
    fixture.context=row(30,0,{0,0,1024,0x1ffffc,0,1});row(31,0)
    if prefix then
        fixture.first=scope(32,2,0x300,2,0,0,1,1,6)
        row(4,2,{0x300,0,8,1},6,0,words({0,0x020000ff}))
        scope(33,2,0x300,2,0,2,0,1,6)
    else
        fixture.first=scope(32,0,0x1f801810,1,0x020000ff,0,1,1)
        scope(33,0,0x1f801810,1,0x020000ff,2,0,1)
    end
    fixture.control=scope(32,1,0x1f801814,1,0x08000001,2,0,2)
    fixture.control_result=scope(33,1,0x1f801814,1,0x08000001,2,0,2)
    row(5,2,{0x100,0x02800000,2},7)
    fixture.chain=scope(32,3,0x100,2,0,2,0,3,7)
    fixture.data=row(4,2,{0x104,0,8,1},7,0,words({0x00010000,0x00010010}))
    fixture.result=scope(33,3,0x100,2,0,0,1,3,7)
    row(21,2,{0x800000},7)
    scope(32,2,0x200,1,0,0,1,4,8)
    row(4,2,{0x200,0,4,1},8,0,words({0}))
    scope(33,2,0x200,1,0,0,1,4,8)
    scope(32,5,0x1f801810,1,0,0,1,5)
    scope(33,5,0x1f801810,1,0x12345678,0,1,5)
    fixture.final=row(30,1,{0,0,1024,0x1ffffc,0,1});row(31,1)
    status={version=1,state=2,events=#rows,bytes=#payload,failures=0,beginCycle=cycle,endCycle=cycle}
end
local h={status=function() return status end,record=function(n) return rows[n+1] end,
    bytes=function(offset,count) return payload:sub(offset+1,offset+count) end}
local function scan(options) return ingress.scan(h,options or {words=32,packets=16}) end
reset()
local result=scan()
assert(#result.buffers==5 and result.words==5 and result.packet_count==3)
assert(result.packets[1].port=='gp1' and result.packets[1].kind=='display-mode')
local fill=result.packets[2]
assert(fill.kind=='fill' and fill.size.width==16 and fill.size.height==1 and fill.offset==0)
assert(#fill.sources==2 and fill.sources[1].buffer=='1' and fill.sources[2].buffer=='3')
assert(fill.sources[1].bytes==4 and fill.sources[2].bytes==8 and fill.sources[2].request=='7')
assert(fill.sources[1].input_cycle=='9007199254740993')
assert(result.headers[1].address==0x100 and result.terminals[1].address==0x800000)
assert(result.reads[1].value==0x12345678 and result.packets[3].offset==12)
assert(result.bytes==words({0x020000ff,0x00010000,0x00010010,0}))
reset(true); result=scan()
assert(result.packet_count==4 and result.words==6)
assert(result.packets[1].kind=='nop' and result.packets[1].offset==0)
assert(result.packets[2].port=='gp1' and result.packets[3].kind=='fill' and result.packets[3].offset==4)
assert(result.packets[4].kind=='nop' and result.packets[4].offset==16)
local function fails(change,fragment,options)
    reset(); change()
    local ok,err=pcall(scan,options)
    assert(not ok and tostring(err):find(fragment,1,true),tostring(err))
end
fails(function() fixture.context.data[5]=0 end,'incomplete parser state')
fails(function() fixture.context.kind=25 end,'lacks initial context')
fails(function() fixture.result.related=uint(99) end,'matching input')
fails(function() fixture.data.request=uint(8) end,'payload identity/count')
fails(function() fixture.data.data[0]=0x100 end,'payload identity/count')
fails(function() fixture.chain.related=uint(1) end,'identity is not increasing')
fails(function() fixture.chain.data[7]=1 end,'context was lost')
fails(function() fixture.result.data[2]=3 end,'identity mismatch')
fails(function() fixture.control.data[3]=0; fixture.control_result.data[3]=0 end,'reset interrupted')
fails(function() fixture.final.data[5]=0 end,'incomplete parser state')
fails(function() fixture.first.data[0]=6 end,'unsupported GPU input')
fails(function() status.failures=1 end,'complete and frozen')
fails(function() end,'word limit exceeded',{words=3,packets=16})
fails(function() end,'packet limit exceeded',{words=32,packets=2})
fails(function() rows[4].cycle=cycle-1 end,'sequence/cycle mismatch')
reset(); local state=0
local probe={status=function() return {state=state,events=2,failures=0} end,
    begin=function() assert(state==0);state=1 end,stop=function() assert(state==1);state=2 end,
    record=function(n) return {kind=30,device=n} end,clear=function() assert(state==2);state=0 end}
ingress.requireBinding(probe); assert(state==0)
probe.record=function() return {kind=25,device=0} end
local ok,err=pcall(ingress.requireBinding,probe)
assert(not ok and tostring(err):find('native GPU history binding required',1,true) and state==2)
print('GPU ingress checks passed: split CPU/DMA packets, control interleaving, exact identities, payloads and failures')
