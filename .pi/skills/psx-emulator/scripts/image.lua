-- Decode native CD image geometry and bind file views to caller-verified media.
-- This does not read files or establish controller/stream completion.
local ffi, m, events = require 'ffi', require 'support', require 'events'
local M = {}
local marker = {}
local function integer(value, maximum, label)
    assert(type(value)=='number' and value>=0 and value<=maximum and value%1==0, label)
    return value
end
local function word(bytes, offset)
    assert(offset+4<=#bytes, 'short CD image word')
    local a,b,c,d=bytes:byte(offset+1,offset+4)
    return a+b*256+c*65536+d*16777216
end
local function wide(bytes, offset)
    return ffi.new('uint64_t',word(bytes,offset+4))*4294967296+word(bytes,offset)
end
local function hex(bytes)
    return (bytes:gsub('.',function(c) return string.format('%02x',c:byte()) end))
end
local function decimal(value)
    if type(value)=='number' then
        integer(value,9007199254740991,'inexact CD binding size')
        value=string.format('%.0f',value)
    end
    assert(type(value)=='string' and value:match('^%d+$') and
        (#value==1 or value:sub(1,1)~='0') and
        (#value<20 or (#value==20 and value<='18446744073709551615')),
        'invalid CD binding size')
    return value
end
local function image(value)
    assert(type(value)=='table' and getmetatable(value)==marker, 'decoded CD image required')
    return value
end
local function msf(value)
    assert(value<16777216, 'CD track MSF reserved byte is nonzero')
    return value
end
local function lba(value)
    return ((value%256)*60+math.floor(value/256)%256)*75+math.floor(value/65536)
end
-- Native MSF(uint32) truncates each member before subtracting its contribution.
local function packed(value)
    value=value%4294967296
    local minute=math.floor(value/4500)%256
    value=value-minute*4500
    local second=math.floor(value/75)%256
    return minute+second*256+(value-second*75)%256*65536
end

function M.decode(row, payload)
    assert(tonumber(row.kind)==47 and (tonumber(row.device)==0 or tonumber(row.device)==1) and
        row.request==ffi.new('uint64_t',1) and row.related==ffi.new('uint64_t',0), 'invalid CD image record')
    local d={}
    for n=0,7 do d[n+1]=integer(tonumber(row.data[n]),4294967295,'invalid CD image data word') end
    assert(d[1]==2 and d[2]==716 and d[3]<=202 and d[4]==100 and
        d[5]==0 and d[6]==0 and d[7]==0 and d[8]==0, 'unsupported CD image layout')
    assert(type(payload)=='string' and #payload==tonumber(row.length) and
        #payload>=2864 and #payload<=836720, 'invalid CD image payload length')
    local h={}
    for n=0,15 do h[n+1]=word(payload,n*4) end
    assert(h[1]<=99 and h[2]==100 and h[3]<=1 and (h[3]==1 or h[4]==0) and
        h[5]<=1 and h[6]<=1 and h[7]<=1 and h[8]<=2 and h[9]<=d[3] and
        h[10]==0 and h[11]==0 and h[12]==0 and h[13]==0 and h[14]<=1 and h[15]==1 and h[16]==0,
        'unsupported CD image geometry')
    assert(h[9]==0 or (h[8]~=0 and h[3]==1), 'CD main file lacks initialized backend')
    local result=setmetatable({schema='psx.cd-image/v2',phase=tonumber(row.device),identity='1',
        header={tracks=h[1],slots=100,pregap_known=h[3]==1,pregap_offset=h[4],multifile=h[5]==1,
            mode1=h[6]==1,big_endian=h[7]==1,backend=h[8],main=h[9],sbi_known=h[14]==1},
        tracks=m.array(),files=m.array()},marker)
    for slot=0,99 do
        local start=64+slot*28
        local values={};for n=0,6 do values[n+1]=word(payload,start+n*4) end
        assert(values[1]<=2 and values[6]<=1 and values[7]<=d[3], 'invalid CD track descriptor')
        result.tracks[slot+1]={slot=slot,type=values[1],start=msf(values[2]),pregap=msf(values[3]),
            length=msf(values[4]),offset=values[5],cdda=values[6],file=values[7]}
    end
    local cursor=2864
    for ordinal=1,d[3] do
        assert(cursor+32<=#payload, 'short CD file descriptor')
        local id,kind,parent,count=word(payload,cursor),word(payload,cursor+4),
            word(payload,cursor+8),word(payload,cursor+12)
        local start,size=wide(payload,cursor+16),wide(payload,cursor+24)
        assert(id==ordinal and (kind==1 or kind==2) and count<=4096 and cursor+32+count<=#payload,
            'invalid CD file descriptor')
        local path=payload:sub(cursor+33,cursor+32+count)
        assert(not path:find('%z'), 'CD file path contains NUL')
        local depth=0
        if kind==1 then
            assert(parent==0 and start==0 and count>0, 'invalid CD root file')
        else
            assert(parent>=1 and parent<ordinal and count==0, 'invalid CD file parent')
            depth=result.files[parent].depth+1
            assert(depth<=4, 'CD file nesting exceeds four slices')
        end
        result.files[ordinal]={ordinal=ordinal,kind=kind,parent=parent,start=events.exact(start),
            bytes=events.exact(size),path_hex=hex(path),depth=depth}
        cursor=cursor+32+count
    end
    assert(cursor==#payload, 'CD image has trailing payload')
    -- Native preparation interns main, then slots0..99, parents before children.
    local seen,nextid={},1
    local function visit(id)
        if id==0 or seen[id] then return end
        local node=assert(result.files[id], 'CD file reference is missing')
        visit(node.parent)
        assert(id==nextid, 'CD file order differs from native traversal')
        seen[id],nextid=true,nextid+1
    end
    visit(result.header.main)
    for _,track in ipairs(result.tracks) do visit(track.file) end
    assert(nextid==#result.files+1, 'CD image has unreachable files')
    return result
end

function M.bind(value, bindings)
    image(value)
    assert(type(bindings)=='table' and #bindings<=202, 'invalid CD receipt bindings')
    local entries=0
    for key in pairs(bindings) do
        assert(type(key)=='number' and key%1==0 and key>=1 and key<=#bindings,
            'CD receipt bindings must be a dense array')
        entries=entries+1
    end
    assert(entries==#bindings, 'CD receipt bindings must be a dense array')
    local paths,keys,used={},{},{}
    for _,entry in ipairs(bindings) do
        assert(type(entry)=='table' and type(entry.path_hex)=='string' and #entry.path_hex>0 and
            #entry.path_hex<=8192 and #entry.path_hex%2==0 and entry.path_hex:match('^[0-9a-f]+$'),
            'invalid CD binding path bytes')
        for byte in entry.path_hex:gmatch('..') do assert(byte~='00', 'CD binding path contains NUL') end
        assert(type(entry.key)=='string' and #entry.key>0 and #entry.key<=128 and entry.key:match('^[%w][%w:_.-]*$') and
            not keys[entry.key] and not paths[entry.path_hex], 'duplicate or invalid CD receipt binding')
        assert(type(entry.sha256)=='string' and #entry.sha256==64 and entry.sha256:match('^[0-9a-f]+$'),
            'invalid CD binding SHA-256')
        paths[entry.path_hex]={key=entry.key,bytes=decimal(entry.bytes),sha256=entry.sha256}
        keys[entry.key]=true
    end
    local result={roots=m.array(),unused=m.array(),
        authority='exact path/size association to declared inputs; verify final receipt; no file I/O or hashing'}
    for _,file in ipairs(value.files) do
        if file.kind==1 then
            local entry=assert(paths[file.path_hex], 'CD native root is absent from receipt bindings')
            assert(file.bytes==entry.bytes, 'CD native file size differs from receipt')
            used[entry.key]=true
            result.roots[#result.roots+1]={ordinal=file.ordinal,input=entry.key,
                bytes=entry.bytes,sha256=entry.sha256}
        end
    end
    for key in pairs(keys) do if not used[key] then result.unused[#result.unused+1]=key end end
    table.sort(result.unused)
    return result
end

function M.track(value, slot)
    return image(value).tracks[integer(slot,99,'CD track slot outside allocated table')+1]
end
function M.tn(value) return math.max(image(value).header.tracks,1) end
function M.td(value, track)
    integer(track,255,'CD track argument must be uint8');image(value)
    if track==0 then
        local last=M.track(value,value.header.tracks)
        return packed(lba(last.start)+lba(last.length)-lba(last.pregap))
    elseif value.header.tracks>1 and track<=value.header.tracks then
        return packed(lba(M.track(value,track).start))
    end
    return 512
end
function M.length(value, track)
    integer(track,255,'CD track argument must be uint8');image(value)
    if track==0 then
        local last=M.track(value,value.header.tracks)
        return packed(lba(last.start)+lba(last.length))
    elseif value.header.tracks>0 and track<=value.header.tracks then return M.track(value,track).length end
    return 0
end
function M.pregap(value, track)
    integer(track,255,'CD track argument must be uint8');image(value)
    if track<=1 then return 0 end
    if value.header.tracks>0 and track<=value.header.tracks then return M.track(value,track).pregap end
    return 512
end
return M
