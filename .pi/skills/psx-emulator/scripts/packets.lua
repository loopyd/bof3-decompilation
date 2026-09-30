-- Bounded packet interpretation for the pinned Redux GPU, not a hardware rasterizer.
local bit = require 'bit'
local M = {}
local function integer(value, low, high, name)
    assert(type(value) == 'number' and value % 1 == 0 and value >= low and value <= high,
        'invalid ' .. name)
    return value
end
local function bounds(options)
    options = options or {}
    return integer(options.words or 65536, 1, 1048576, 'word limit'),
        integer(options.packets or 4096, 1, 65536, 'packet limit')
end
local function word(bytes, offset)
    assert(offset >= 0 and offset % 4 == 0 and offset + 4 <= #bytes, 'truncated GPU word')
    local a,b,c,d = bytes:byte(offset+1,offset+4)
    return a + b*256 + c*65536 + d*16777216
end
local function field(value, shift, bits) return math.floor(value / 2^shift) % 2^bits end
local function signed(value) value=value%2048; return value>=1024 and value-2048 or value end
local function xy(value) return {x=signed(value), y=signed(math.floor(value/65536))} end
local function unsigned(value) return {x=value%65536, y=math.floor(value/65536)} end
local function size(value) return {width=value%65536, height=math.floor(value/65536)} end
local function texture(value)
    local depth=field(value,7,2)
    return {raw=value, x_words=field(value,0,4)*64, y=field(value,4,1)*256,
        blend=field(value,5,2), depth_bits=depth, native_bpp=({4,8,16,16})[depth+1],
        dither=field(value,9,1), draw_to_display=field(value,10,1), disable=field(value,11,1),
        flip_x=field(value,12,1), flip_y=field(value,13,1)}
end
local function uv(value)
    return {u=value%256, v=field(value,8,8), upper=math.floor(value/65536)}
end
local function clut(value) return {raw=value, x_words=value%64*16, y=field(value,6,9)} end

function M.gp0(bytes, options)
    local maxwords, maxpackets=bounds(options)
    assert(type(bytes)=='string' and #bytes%4==0 and #bytes/4<=maxwords, 'invalid GPU stream size')
    local result, offset={},0
    while offset<#bytes do
        assert(#result<maxpackets, 'GPU packet limit exceeded')
        local start, raw=offset,{}
        local function take()
            assert(#raw<maxwords, 'GPU packet word limit exceeded')
            local value=word(bytes,offset); offset=offset+4; raw[#raw+1]=value; return value
        end
        local first=take()
        local opcode=math.floor(first/16777216)
        local group=math.floor(opcode/32)
        local p={offset=start, opcode=opcode, raw=raw}
        local shaded, textured=bit.band(opcode,16)~=0,bit.band(opcode,4)~=0
        local function vertex(color)
            local v=xy(take()); v.color=color
            if textured then v.texture=uv(take()) end
            return v
        end
        if group==0 then
            if first==0 then p.kind='nop'
            elseif opcode==1 then p.kind='clear-cache'
            elseif opcode==2 then
                p.kind,p.color,p.position,p.size='fill',first%16777216,unsigned(take()),size(take())
            else error(string.format('unsupported native GP0 opcode 0x%02x at byte %d',opcode,start)) end
        elseif group==1 then
            p.kind,p.shaded,p.textured,p.semitransparent,p.raw_texture='polygon',shaded,textured,
                bit.band(opcode,2)~=0,bit.band(opcode,1)~=0
            p.vertices={}
            local color=first%16777216
            for n=1,(bit.band(opcode,8)~=0 and 4 or 3) do
                if n>1 and shaded then color=take()%16777216 end
                p.vertices[n]=vertex(color)
            end
            if textured then
                p.clut=clut(p.vertices[1].texture.upper)
                p.page=texture(bit.band(p.vertices[2].texture.upper,0x9ff))
            end
        elseif group==2 then
            p.kind,p.shaded,p.semitransparent='line',shaded,bit.band(opcode,2)~=0
            p.polyline,p.vertices=bit.band(opcode,8)~=0,{}
            if p.polyline then
                assert(bit.band(first,0xf000f000)~=0x50005000,
                    'polyline command matches the native terminator pattern')
                assert(bit.band(word(bytes,offset),0xf000f000)~=0x50005000,
                    'polyline first coordinate matches the native terminator pattern')
            end
            local color=first%16777216
            local function line_vertex()
                local v=xy(take()); v.color=color; p.vertices[#p.vertices+1]=v
            end
            line_vertex()
            if not p.polyline then
                if shaded then color=take()%16777216 end
                line_vertex()
            else
                while true do
                    local value=word(bytes,offset)
                    if bit.band(value,0xf000f000)==0x50005000 then
                        assert(#p.vertices>=2,'polyline has fewer than two vertices')
                        p.terminator=take(); break
                    end
                    if shaded then
                        color=take()%16777216
                        assert(bit.band(word(bytes,offset),0xf000f000)~=0x50005000,
                            'polyline terminator in coordinate slot is unsupported')
                    end
                    line_vertex()
                end
            end
        elseif group==3 then
            p.kind,p.textured,p.semitransparent,p.raw_texture='rectangle',textured,
                bit.band(opcode,2)~=0,bit.band(opcode,1)~=0
            p.vertex=vertex(first%16777216)
            if textured then p.clut=clut(p.vertex.texture.upper) end
            local kind=field(opcode,3,2)
            if kind==0 then p.size=size(take())
            else local n=({1,8,16})[kind]; p.size={width=n,height=n} end
        elseif group==4 then
            p.kind,p.source,p.destination,p.size='copy',xy(take()),xy(take()),size(take())
        elseif group==5 or group==6 then
            p.kind=group==5 and 'upload' or 'readback'
            p.position,p.size=unsigned(take()),size(take())
            if group==5 then
                local count=math.floor((p.size.width*p.size.height+1)/2)
                assert(count>0,'zero-area native upload has a deferred boundary and is unsupported')
                assert(count+3<=maxwords and count*4<=#bytes-offset,'truncated or oversized GPU upload')
                p.pixel_offset,p.pixel_words,p.pixels=offset,count,p.size.width*p.size.height
                for _=1,count do take() end
            end
        else
            local names={[0xe1]='page',[0xe2]='window',[0xe3]='area-start',
                [0xe4]='area-end',[0xe5]='offset',[0xe6]='mask'}
            p.kind=assert(names[opcode],string.format('unsupported native GP0 opcode 0x%02x',opcode))
            if opcode==0xe1 then p.page=texture(first%16777216)
            elseif opcode==0xe2 then p.window={mask_x=field(first,0,5),mask_y=field(first,5,5),
                offset_x=field(first,10,5),offset_y=field(first,15,5)}
            elseif opcode==0xe3 or opcode==0xe4 then p.position={x=field(first,0,10),y=field(first,10,9)}
            elseif opcode==0xe5 then p.position={x=signed(first),y=signed(math.floor(first/2048))}
            else p.set_mask,p.check_mask=field(first,0,1),field(first,1,1) end
        end
        p.words=#raw; result[#result+1]=p
    end
    return result
end

function M.gp1(bytes, options)
    local maxwords,maxpackets=bounds(options)
    assert(type(bytes)=='string' and #bytes%4==0 and #bytes/4<=math.min(maxwords,maxpackets),
        'invalid GPU control stream size')
    local names={[0]='reset',[1]='clear-fifo',[2]='irq-ack',[3]='display-enable',
        [4]='dma',[5]='display-start',[6]='horizontal-range',[7]='vertical-range',
        [8]='display-mode',[16]='query'}
    local result={}
    for offset=0,#bytes-1,4 do
        local value=word(bytes,offset); local op=math.floor(value/16777216)
        local p={offset=offset,raw={value},words=1,opcode=op,
            kind=assert(names[op],string.format('unsupported native GP1 opcode 0x%02x',op))}
        if op==3 then p.enabled=value%2==0
        elseif op==4 then p.direction=value%4
        elseif op==5 then p.position={x=field(value,0,10),y=field(value,10,9)}
        elseif op==6 then p.range={first=field(value,0,12),last=field(value,12,12)}
        elseif op==7 then p.range={first=field(value,0,10),last=field(value,10,10)}
        elseif op==8 then
            p.mode={horizontal_bits=field(value,0,2),vertical_bit=field(value,2,1),pal=field(value,3,1),
                depth24=field(value,4,1),interlace=field(value,5,1),alternate_horizontal=field(value,6,1),
                reverse=field(value,7,1)}
        elseif op==16 then p.query=value%8 end
        result[#result+1]=p
    end
    return result
end

function M.chain(ram, address, options)
    local maxwords,maxpackets=bounds(options)
    assert(type(ram)=='string' and #ram==2097152,'ordering table requires exact physical 2 MiB RAM')
    integer(address,0,4294967295,'ordering-table address')
    local segment=math.floor(address/0x20000000)
    assert((segment==0 or segment==4 or segment==5) and address%0x20000000<0x800000,
        'ordering-table address is not a supported RAM alias')
    local nodes,chunks,seen,total={}, {}, {},0
    while true do
        assert(address%4==0,'unaligned ordering-table pointer')
        local physical=address%2097152
        assert(not seen[physical],'cyclic ordering table')
        assert(#nodes<maxpackets,'ordering-table node limit exceeded')
        seen[physical]=true
        local header=word(ram,physical)
        local count,next_address=math.floor(header/16777216),header%16777216
        assert(total+count<=maxwords,'ordering-table word limit exceeded')
        assert(physical+4+count*4<=#ram,'ordering-table payload crosses physical RAM boundary')
        nodes[#nodes+1]={address=address,physical=physical,header=header,next=next_address,
            words=count,stream_offset=total*4}
        chunks[#chunks+1]=ram:sub(physical+5,physical+4+count*4)
        total=total+count
        if bit.band(next_address,0x800000)~=0 then
            return {nodes=nodes,words=total,terminal=next_address,bytes=table.concat(chunks)}
        end
        address=next_address
    end
end

return M
