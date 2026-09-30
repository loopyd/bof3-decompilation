-- Explicit leaf-command vectors, including stale bytes and post-effect errors.
package.loaded.image={tn=function() return 2 end,td=function(_,n) return n==0 and 0x003b1200 or 512 end,
    track=function(g,n) assert(n==1);return {type=g.kind or 1} end}
local opcode=require 'opcode'
local function fixture(code,options)
    options=options or {};local c,r,effects={},{},{}
    for n=1,56 do c[n]=0 end
    c[2],c[3],c[10],c[12],c[52]=3,0x12,1,1,255
    for n=1,16 do r[n]=0xa0+n-1 end;r[1]=c[3]
    for n,v in pairs(options.words or {}) do c[n]=v end
    local x={words=c,reply=r,params=options.params or {0,0},clock=33868800,image={kind=options.kind},
        deferred={},header='\1\2\3\4\5\6\7\8',subq={1,2,3,4,5,6,7,8}}
    local function effect(value) effects[#effects+1]=value end
    x.resize=function(n) effect('resize:'..n);c[10],c[11],c[12]=n,0,1 end
    x.schedule=function(slot,delay) effect('schedule:'..slot..':'..delay) end
    x.queue=function(op,delay) effect('queue:'..op..':'..delay);c[6],c[8]=op,delay end
    x.predicate=function(selector,site)
        effect('predicate:'..selector..':'..tostring(site))
        return {data=(options.predicates or {})[selector] or options.predicate or {site or 0,0},
            payload='\6\7\8'}
    end
    opcode.apply(x,code)
    return c,r,table.concat(effects,',')
end
local c,r,e=fixture(1)
assert(c[3]==2 and r[1]==0x12 and c[2]==3 and e=='')
c,r=fixture(4,{words={[32]=255,[33]=4}})
assert(c[32]==0 and c[33]==0 and c[2]==2 and c[24]==1)
c,r=fixture(5,{words={[33]=0,[32]=4}})
assert(c[33]==2 and c[32]==0)
c,r=fixture(13,{words={[9]=0},params={0xfe,0xfd}})
assert(c[19]==0xfe and c[20]==0xfd) -- Stored bytes apply even with zero count.
c,r,e=fixture(15,{words={[18]=0x140,[19]=3,[20]=4,[31]=1}})
assert(c[2]==3 and c[10]==5 and r[2]==0x40 and r[3]==0 and r[4]==3 and r[5]==4 and r[6]==0xa5)
c,r,e=fixture(16)
assert(c[10]==8 and r[1]==1 and r[8]==8 and r[9]==0xa8)
c,r,e=fixture(20,{params={0,0}})
assert(c[10]==4 and c[51]==0 and r[2]==0 and r[3]==0x18 and r[4]==0xa3)
c,r,e=fixture(19,{predicate={1,1}})
assert(c[2]==5 and c[10]==1 and r[1]==0x13 and e=='predicate:6:1')
for _,p in ipairs({0x22,0x23,0x24}) do
    c,r,e=fixture(25,{params={p,0}})
    assert(c[10]==8 and r[1]==(p==0x22 and 0x66 or 0x43) and r[5]==0xa4 and r[8]==0xa7)
end
c,r,e=fixture(25,{params={0x20,0}})
assert(c[10]==4 and r[1]==0x98 and r[4]==0xc3 and r[5]==0xa4)
c,r,e=fixture(25,{params={0xff,0},words={[31]=2}})
assert(c[2]==3 and c[10]==1 and r[1]==0x12 and e=='')
c,r,e=fixture(7,{words={[31]=1}})
assert(c[2]==5 and r[2]==128 and e=='resize:2,resize:2') -- NOTREADY replaces INVALIDARG.
c,r,e=fixture(18,{words={[31]=1}})
assert(c[6]==274 and c[8]==32739840 and c[2]==5 and r[2]==128)
assert(e=='queue:274:32739840,resize:2') -- Error does not undo the queue.
c,r,e=fixture(263,{words={[31]=2}})
assert(c[24]==1 and c[2]==5 and r[2]==128)
c,r,e=fixture(286,{words={[31]=2}})
assert(c[24]==1 and c[2]==2 and c[10]==1 and e=='')
c,r,e=fixture(9,{words={[31]=4,[18]=128}})
assert(c[1]==128 and c[6]==265 and c[8]==2000000 and
    e=='schedule:14:225792,queue:265:2000000')
c,r,e=fixture(10,{words={[31]=4,[23]=1}})
assert(c[23]==0 and c[18]==32 and c[31]==0 and c[3]==0x12 and r[1]==0x12 and c[8]==4100000)
c,r,e=fixture(17,{words={[15]=0,[16]=0}})
assert(r[1]==1 and r[2]==0 and r[8]==8 and e=='resize:8,predicate:5:nil')
c,r,e=fixture(282,{predicate={3,1}})
assert(c[2]==5 and c[10]==8 and r[1]==8 and r[2]==64 and r[8]==0 and r[9]==0xa8)
for _,kind in ipairs({0,1,2,255}) do
    for _,play in ipairs({0,1}) do
        for _,lid in ipairs({0,1}) do
            local status={lid,play==1 and 2 or kind,lid*16+play*128,512}
            c,r,e=fixture(282,{kind=kind,words={[16]=play,[27]=512},predicates={[7]=status}})
            local expected=play==1 or kind==2
            assert(c[2]==2 and c[24]==1 and r[2]==(expected and 144 or (kind==0 or kind==255) and 192 or 0))
            assert(r[5]==80 and r[8]==88 and r[9]==0xa8)
            for index=1,4 do
                local bad={unpack(status)};bad[index]=bad[index]+1
                local ok,err=pcall(fixture,282,{kind=kind,words={[16]=play,[27]=512},predicates={[7]=bad}})
                assert(not ok and tostring(err):find('CD ID',1,true),'inconsistent ID field accepted')
            end
        end
    end
end
c,r,e=fixture(0xff)
assert(c[2]==5 and c[10]==2 and r[1]==0x13 and r[2]==64 and r[3]==0xa2)
print('CD opcode checks: stale writes, shared parameters, counter wrap, delayed stages and drive overrides passed')
