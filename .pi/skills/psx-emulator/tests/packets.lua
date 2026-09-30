-- Standalone framing/address checks; native ingress and rendered effects need live evidence.
local packets=require 'packets'
local function encode(values)
    local out={}
    for _,v in ipairs(values) do
        out[#out+1]=string.char(v%256,math.floor(v/256)%256,math.floor(v/65536)%256,math.floor(v/16777216))
    end
    return table.concat(out)
end
local function reject(fn,pattern)
    local okay,err=pcall(fn); assert(not okay and tostring(err):find(pattern),tostring(err))
end
local input=encode({0xe10001e5,0xe2008421,0xe3000402,0xe4000803,0xe53fffff,0xe6000003,
    0x020000ff,0x00010002,0x00030004,0})
local parsed=packets.gp0(input)
assert(#parsed==8 and parsed[1].page.x_words==320 and parsed[1].page.native_bpp==16)
assert(parsed[2].window.mask_x==1 and parsed[2].window.offset_y==1)
assert(parsed[3].position.x==2 and parsed[3].position.y==1)
assert(parsed[5].position.x==-1 and parsed[5].position.y==-1)
assert(parsed[6].set_mask==1 and parsed[6].check_mask==1)
assert(parsed[7].color==255 and parsed[7].position.x==2 and parsed[7].size.height==3)
assert(parsed[8].kind=='nop' and parsed[8].offset==36)
local textured=packets.gp0(encode({0x340000ff,0x00000000,0x00400201,
    0x0000ff00,0x000107ff,0x00800403,0x00ff0000,0x00020002,0x00000605}))[1]
assert(textured.words==9 and textured.shaded and textured.textured and #textured.vertices==3)
assert(textured.vertices[2].x==-1 and textured.vertices[2].color==65280)
assert(textured.clut.y==1 and textured.page.native_bpp==8)
local lines=packets.gp0(encode({0x480000ff,0,0x10001,0x50005000,
    0x580000ff,0,0x0000ff00,0x10001,0x50005000}))
assert(#lines==2 and #lines[1].vertices==2 and lines[2].vertices[2].color==65280)
local upload=encode({0xa0000000,0x00010002,0x00010003,0x03e0001f,0x7fff7c00,0xe6000000})
parsed=packets.gp0(upload)
assert(#parsed==2 and parsed[1].pixel_words==2 and parsed[1].pixels==3 and parsed[2].offset==20)
local rectangles=packets.gp0(encode({0x640000ff,0,0x00400000,0x00050006,0x780000ff,0}))
assert(rectangles[1].size.width==6 and rectangles[1].clut.y==1 and rectangles[2].size.width==16)
local controls=packets.gp1(encode({0x03000000,0x04000002,0x05000402,0x0800003f,0x10000004}))
assert(controls[1].enabled and controls[2].direction==2 and controls[3].position.y==1)
assert(controls[4].mode.interlace==1 and controls[5].query==4)
reject(function() packets.gp0('\0') end,'stream size')
reject(function() packets.gp0(encode({0x02000000,0})) end,'truncated')
reject(function() packets.gp0(upload:sub(1,16)) end,'truncated')
reject(function() packets.gp0(encode({0xa0000000,0,0})) end,'zero%-area')
reject(function() packets.gp0(encode({0x480000ff,0,0x50005000})) end,'two vertices')
reject(function() packets.gp0(encode({0x580000ff,0,0x00ff00,0x50005000})) end,'coordinate slot')
reject(function() packets.gp0(encode({0x58005000,0,0x00ff00,0x10001,0x50005000})) end,'command matches')
reject(function() packets.gp0(encode({0x480000ff,0x50005000,0x10001,0x50005000})) end,'first coordinate')
reject(function() packets.gp0(encode({0x1f000000})) end,'unsupported native GP0')
reject(function() packets.gp1(encode({0x09000000})) end,'unsupported native GP1')
reject(function() packets.gp0(input,{words=9}) end,'stream size')
reject(function() packets.gp0(input,{packets=1}) end,'packet limit')
local ram=string.rep('\0',2097152)
local function put(offset,values)
    local bytes=encode(values); ram=ram:sub(1,offset)..bytes..ram:sub(offset+#bytes+1)
end
put(0x100,{0x02000200,0x020000ff,0x00010002})
put(0x200,{0x01800000,0x00030004})
local chain=packets.chain(ram,0xa0200100)
local chain_ram=ram
assert(#chain.nodes==2 and chain.nodes[1].physical==0x100 and chain.words==3 and chain.terminal==0x800000)
assert(packets.gp0(chain.bytes)[1].kind=='fill')
reject(function() packets.chain(ram,0x1f801810) end,'RAM alias')
reject(function() packets.chain(ram,0x80000101) end,'unaligned')
reject(function() packets.chain(ram,0x100,{packets=1}) end,'node limit')
reject(function() packets.chain(ram,0x100,{words=2}) end,'word limit')
put(0x200,{0x00000100}); reject(function() packets.chain(ram,0x100) end,'cyclic')
put(0x200,{0x00200100}); reject(function() packets.chain(ram,0x100) end,'cyclic')
put(0x1ffffc,{0x01ffffff}); reject(function() packets.chain(ram,0x1ffffc) end,'boundary')
reject(function() packets.chain(ram:sub(2),0) end,'2 MiB')
if arg[1] then
    local f=assert(io.open(arg[1],'wb')); assert(f:write(input)); assert(f:close())
end
if arg[2] then
    local f=assert(io.open(arg[2],'wb')); assert(f:write(chain_ram)); assert(f:close())
end
print('GPU packet checks passed: primitives, textures, transfers, controls, bounds and aliased/cyclic ordering tables')
