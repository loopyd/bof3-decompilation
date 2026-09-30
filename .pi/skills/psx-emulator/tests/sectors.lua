-- Synthetic records verify buffer correlation, not native CD execution.
local ffi, sectors = require 'ffi', require 'sectors'
local function uint(n) return ffi.new('uint64_t',n) end
local high=uint(9007199254740992)+1
local function wide(value)
    value=uint(value); local bytes={}
    for n=1,8 do bytes[n]=string.char(tonumber(value%256)); value=value/256 end
    return table.concat(bytes)
end
local rows, fixture
local function row(kind,phase,data,request,related,payload)
    local values={};for n=0,7 do values[n]=(data or {})[n+1] or 0 end
    local r={kind=kind,device=phase,data=values,request=uint(request or 0),related=uint(related or 0),
        length=#payload,sequence=uint(#rows+1),cycle=high}
    rows[#rows+1]={row=r,payload=payload};return rows[#rows]
end
local function mutation(n,reason,bytes,source)
    return row(41,1,{0,#bytes,reason,0,150,reason==5 and 2 or 1},n,source or high,
        wide(high)..wide(n-1)..wide(0)..wide(0)..bytes)
end
local function reset()
    rows,fixture={},{}
    row(41,0,{0,2352},0,0,string.rep('z',2352))
    local bytes={};for n=0,2339 do bytes[#bytes+1]=string.char(n%251) end;bytes=table.concat(bytes)
    fixture.delivery=mutation(1,2,bytes)
    mutation(2,1,'header!!')
    fixture.pio=row(42,0,{8,0x1f801802,8,1,9,1,0},1,high,wide(0))
    fixture.transfer=row(4,3,{0x39000,2339,1,0},high,0,string.char(2339%251))
    fixture.wrap=row(42,1,{2339,0x39000,2339%251,1,0,0,1},1,high,wide(high))
    row(4,3,{0x39001,0,1,0},high,0,'h')
    fixture.dma=row(42,1,{0,0x39001,string.byte('h'),0,1,0,0},2,high,wide(high))
    fixture.empty=row(42,0,{2350,0x1f801802,0,0,2350,0,0},0,high,wide(0))
    fixture.tail=row(42,0,{2340,0x1f801802,string.byte('z'),1,1,1,1},0,high,wide(0))
    mutation(3,4,string.rep('a',2352))
    fixture.attenuation=mutation(4,5,string.rep('b',2352),3)
    mutation(5,3,string.rep('\0',2340))
    fixture.final=row(41,2,{0,2352},0,0,string.rep('\0',2340)..string.rep('b',12))
end
local function scan(limits)
    local tracker=sectors.new(limits or {mutations=8,bytes=8})
    for _,entry in ipairs(rows) do tracker.push(entry.row,entry.payload) end
    return tracker.finish()
end
reset();local result=scan()
assert(#result.mutations==5 and #result.reads==5)
assert(result.reads[1].mutation=='1' and result.reads[2].wrapped and result.reads[3].mutation=='2')
assert(result.reads[3].dma=='9007199254740993' and result.reads[3].cycle=='9007199254740993')
assert(result.reads[4].mutation=='0' and result.mutations[4].source=='3')
assert(result.reads[5].mutation=='0' and result.reads[5].value==string.byte('z'))
assert(result.final==string.rep('\0',2340)..string.rep('b',12))
local function fails(change,fragment,limits)
    reset();change();local ok,err=pcall(scan,limits)
    assert(not ok and tostring(err):find(fragment,1,true),tostring(err))
end
fails(function() fixture.pio.row.request=uint(2) end,'origin/value mismatch')
fails(function() fixture.wrap.row.data[6]=0 end,'cursor/wrap mismatch')
fails(function() fixture.dma.payload=wide(0) end,'owner/address mismatch')
fails(function() fixture.transfer.row.request=uint(4) end,'DMA consumption mismatch')
fails(function() fixture.transfer.row.data[0]=0x39004 end,'DMA consumption mismatch')
fails(function() fixture.empty.row.data[2]=1 end,'fabricated data')
fails(function() fixture.tail.row.request=uint(1) end,'origin/value mismatch')
fails(function() fixture.final.payload=string.rep('a',2352) end,'differs from mutations')
fails(function() fixture.delivery.row.data[1]=2339 end,'unsupported CD mutation')
fails(function() fixture.delivery.row.request=uint(2) end,'identity or payload mismatch')
fails(function() fixture.delivery.row.data[2]=3 end,'error fill is not zero')
fails(function() fixture.attenuation.row.related=uint(2) end,'source mismatch')
fails(function() rows[#rows]=nil end,'boundaries incomplete')
fails(function() end,'mutation limit exceeded',{mutations=3,bytes=8})
fails(function() end,'byte limit exceeded',{mutations=8,bytes=3})
print('CD buffer checks: partial headers, exact origins, DMA wrap/read-ready, CDDA derivation, uint64 IDs and malformed records passed')
