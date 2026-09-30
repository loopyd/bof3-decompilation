-- interrupt() opcode-local writes and required effects. Submission and native
-- media/SubQ arithmetic have separate owners; preserve native partial writes.
local bit,image=require 'bit',require 'image'
local M={}
local function clear(n,mask) return bit.band(n,bit.bnot(mask))%4294967296 end
local function bcd(n) return (math.floor(n/10)*16+n%10)%256 end
local function decimal(n) return math.floor(n/16)*10+n%16 end
local function lba(n) return (n%256*60+math.floor(n/256)%256)*75+math.floor(n/65536)%256 end
local function track(g,msf)
    local n=1
    while n<image.tn(g) and lba(image.td(g,n+1))-lba(msf)<150 do n=n+1 end
    return n
end
function M.apply(x,op)
    local c,r,p,g=x.words,x.reply,x.params,x.image
    local base=math.floor(x.clock/75)
    local exempt,rotate=false,false
    local function errorReply(code)
        x.resize(2);r[1],r[2],c[2]=bit.bor(c[3],1),code,5
    end
    local function complete() c[2],c[24]=2,1 end
    local function position()
        if c[25]~=0 then c[27],c[25],c[26]=c[28],0,1 end
    end
    local function play()
        x.stop(1)
        if c[17]==0 then c[17]=1 end
        position()
        if c[9]~=0 and p[1]~=0 then
            local n=decimal(p[1]);if n<=image.tn(g) then c[22]=n end
            c[27]=image.td(g,c[22]%256)
        end
        c[22]=track(g,c[27]);x.lookup(c[27]);c[50]=0;x.deferred[50]=nil
        c[3]=clear(c[3],64);r[1]=c[3];c[3]=bit.bor(c[3],128)
        x.start(1);x.schedule(14,base);rotate=true
    end
    if op==1 then
        if c[31]~=1 then c[3]=clear(c[3],16) end
        exempt=true
    elseif op==2 or op==29 then
        -- Setloc effects occur on submission; GetQ is a native no-op.
    elseif op==3 then play()
    elseif op==4 or op==5 then
        complete();local n=op==4 and 32 or 33
        c[n]=c[n]==0 and 2 or (c[n]+1)%256;c[op==4 and 33 or 32]=0
    elseif op==7 then
        if c[31]~=4 then errorReply(32)
        else x.queue(263,math.floor(base*125/2));rotate=true end
    elseif op==263 or op==266 or op==274 or op==286 then
        complete();exempt=op==286
    elseif op==8 then
        if c[16]~=0 then c[27]=image.td(g,c[22]%256) end
        x.stop(1);x.stop(0)
        local delay=c[31]==0 and math.floor(base*30/2) or 2048
        c[31]=4;x.queue(264,delay)
    elseif op==264 then
        c[3]=clear(c[3],2);r[1]=c[3];complete()
    elseif op==9 then
        local delay=7000
        if c[31]~=0 then
            local speed=bit.band(c[18],128)~=0
            delay=speed and 2000000 or 1000000
            x.schedule(14,speed and math.floor(base/2) or base)
        end
        x.queue(265,delay);c[1]=bit.bor(c[1],128)
    elseif op==265 then
        c[3]=clear(c[3],32);r[1]=c[3];complete()
    elseif op==10 then
        c[23],c[18]=0,32;x.queue(266,4100000);exempt,rotate=true,true
    elseif op==11 or op==12 then c[23]=op==11 and 1 or 0
    elseif op==13 then c[19],c[20]=p[1],p[2]
    elseif op==14 then exempt=true
    elseif op==15 then
        x.resize(5);r[2],r[3],r[4],r[5]=c[18]%256,0,c[19]%256,c[20]%256;exempt=true
    elseif op==16 then
        x.resize(8);for n=1,8 do r[n]=x.header:byte(n) end
    elseif op==17 then
        assert(c[52]==255,'CD GetlocP lacks complete SubQ provenance')
        x.resize(8);for n=1,8 do r[n]=x.subq[n] end
        if c[16]==0 then
            local q=x.predicate(5)
            assert(q.data[1]==0 and q.payload==string.char(r[6],r[7],r[8]),
                'CD GetlocP has unsupported or inconsistent SBI result')
        end
        if c[16]==0 and c[15]==0 then r[2]=0 end
    elseif op==18 then x.queue(274,math.floor(base*290/4));rotate=true
    elseif op==19 or op==20 then
        local missing=x.predicate(6,op==19 and 1 or 2).data[2]==1
        if missing then c[2]=5;r[1]=bit.bor(r[1],1)
        elseif op==19 then x.resize(3);c[2]=3;r[2],r[3]=1,bcd(image.tn(g))
        else
            c[51]=decimal(p[1]);x.resize(4);c[2]=3
            local msf=image.td(g,c[51]);r[1],r[2],r[3]=c[3],bcd(msf%256),bcd(math.floor(msf/256)%256)
            -- Byte3 is deliberately stale despite a four-byte response.
        end
    elseif op==21 or op==22 then
        x.stop(1);x.stop(0);c[3]=bit.bor(c[3],64)
        x.schedule(14,c[17]==1 and 2048 or base*4);c[17]=0;rotate=true
    elseif op==25 then
        local bytes
        if p[1]==32 then x.resize(4);bytes={0x98,6,16,0xc3}
        elseif p[1]==34 then x.resize(8);bytes={0x66,0x6f,0x72,0x20}
        elseif p[1]==35 or p[1]==36 then x.resize(8);bytes={0x43,0x58,0x44,0x32} end
        if bytes then for n=1,4 do r[n]=bytes[n] end end
        exempt=true
    elseif op==26 then x.queue(282,20480)
    elseif op==282 then
        x.resize(8)
        if x.predicate(6,3).data[2]==1 then
            r[1],r[2],c[2]=8,64,5;for n=3,8 do r[n]=0 end
        else
            r[1],r[2],r[3],r[4]=c[3],0,0,0
            local status=x.predicate(7).data
            assert(status[1]==0 or status[1]==1,'CD ID lid predicate is not boolean')
            local playing=c[16]~=0
            local kind=playing and 2 or image.track(g,1).type
            assert(status[2]==kind and status[3]==status[1]*16+(playing and 128 or 0)
                and status[4]==c[27],'CD ID status contradicts controller or image state')
            if kind==0 or kind==255 then r[2]=192 elseif kind==2 then r[2]=144 end
            r[1]=bit.bor(r[1],bit.band(bit.rshift(r[2],4),8))
            r[5],r[6],r[7],r[8]=80,67,83,88;complete()
        end
    elseif op==28 then
        c[3],c[31]=bit.bor(c[3],16),2;x.schedule(13,20480);exempt,rotate=true,true
    elseif op==30 then x.queue(286,math.floor(base*180/4));exempt,rotate=true,true
    elseif op==6 or op==27 then
        position();c[22]=track(g,c[27])
        if bit.band(c[18],1)~=0 and c[22]>1 then play()
        else
            x.start(0);c[21]=1;x.lookup(c[27])
            if x.predicate(2,1).data[2]==1 then x.mutate() end
            c[3]=clear(bit.bor(c[3],32),64)
            x.schedule(3,bit.band(c[18],128)~=0 and base or base*2)
            r[1]=c[3];rotate=true
        end
    else errorReply(64) end
    if c[31]==4 and rotate then c[31]=0;c[3]=bit.bor(c[3],2) end
    if not exempt and (c[31]==1 or c[31]==2 or c[31]==3) then errorReply(128) end
end
return M
