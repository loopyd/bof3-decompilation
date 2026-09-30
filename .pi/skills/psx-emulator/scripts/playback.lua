-- PLAY callback control flow and response semantics. Media/SubQ arithmetic,
-- attenuation, feed contents and scheduler scaling retain their own obligations.
local bit,m,events,image=require 'bit',require 'support',require 'events',require 'image'
local M={}
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function bytes(hex)
    local b={};for pair in hex:gmatch('..') do b[#b+1]=tonumber(pair,16) end;return b
end
local function clear(n,mask) return bit.band(n,bit.bnot(mask))%4294967296 end
local function lba(n) return (n%256*60+math.floor(n/256)%256)*75+math.floor(n/65536)%256 end
local function nextMSF(n)
    local minute,second,frame=n%256,math.floor(n/256)%256,(math.floor(n/65536)%256+1)%256
    if frame>=75 then frame=0;second=(second+1)%256
        if second>=60 then second=0;minute=(minute+1)%256 end
    end
    return minute+second*256+frame*65536
end
function M.new(controller,geometry)
    local tracker,active,clock={}
    local result={callbacks=m.array(),busy=0,inactive=0,seeks=0,played=0,stopped=0,reports=0}
    local function finish(frame)
        local b,c=frame.before_context.words,{}
        for n=1,56 do c[n]=b[n] end
        local reply,q=bytes(frame.before_context.response_hex),bytes(frame.before_context.subq_hex)
        local cursor,builder,media,subq=0,false,false,false
        local delegated={}
        local entry={access=events.exact(frame.id),seek=false,report=false}
        local function take(kind)
            cursor=cursor+1;local t=active.tape[cursor]
            assert(t and t.kind==kind,'CD PLAY expected '..kind..' at body item '..cursor);return t
        end
        local function resize(n)
            assert(take('resize').data[1]==n,'CD PLAY response size mismatch')
            c[10],c[11],c[12]=n,0,1;builder=true;delegated[45],delegated[46]=true,true
        end
        local function irq()
            local d=take('irq').data;local asserted=bit.band(c[2],c[4])~=0
            assert(d[1]==c[2] and d[2]==c[4] and d[3]==(asserted and 1 or 0) and d[4]==(builder and 1 or 0),
                'CD PLAY IRQ status/mask/builder mismatch')
            -- The command consumer checks continuity of every global latch change.
            if asserted then assert(take('assert').data[1]==4,'CD PLAY asserted wrong IRQ') end
        end
        local function schedule(delay)
            assert(take('schedule').data[2]==delay,'CD PLAY exact delay mismatch');entry.delay=delay
        end
        local function stop(reason)
            local d=take('stop').data
            assert(d[1]==0 and d[2]==reason and d[3]==c[16],'CD PLAY stream stop mismatch')
            if c[16]~=0 then c[3]=clear(c[3],128);c[16],c[32],c[33]=0,0,0 end
            delegated[43],delegated[44]=true,true
        end
        local function lookup(kind)
            local t=take('lookup')
            assert(t.data[1]==kind and t.data[2]==c[27] and t.outcome,'CD PLAY lookup kind/position/result mismatch')
            if kind==1 then
                if t.outcome[3]==1 then
                    assert(t.outcome[2]==c[24],'CD PLAY cached lookup changed success')
                else
                    c[24]=t.outcome[2];c[30]=c[24]~=0 and c[27] or 0
                    for _,n in ipairs({22,50,52,53}) do delegated[n]=true end
                    subq=true
                end
            end
            media=true
        end
        local function mutation(reason)
            local t=take('mutation');local d=t.data
            assert(d[1]==0 and d[2]==2352 and d[3]==reason and d[4]==c[27] and #t.payload==2384,
                'CD PLAY full-sector mutation mismatch')
            delegated[47],delegated[48]=true,true
            return t.payload:sub(33)
        end
        if c[17]==0 and c[2]~=0 then
            schedule(256);entry.classification='busy';result.busy=result.busy+1
        else
            if c[17]==0 then
                resize(1);c[3]=clear(bit.bor(c[3],2),64);reply[1]=c[3];c[17]=1
                if c[6]==0 then c[2],c[24]=2,1;irq() end
                if c[25]~=0 then c[27],c[25],c[26]=c[28],0,1 end
                local g=geometry();c[22]=1
                while c[22]<image.tn(g) and lba(image.td(g,c[22]+1))-lba(c[27])<150 do c[22]=c[22]+1 end
                lookup(1);c[50]=0;delegated[50]=nil
                entry.seek=true;result.seeks=result.seeks+1
            end
            if c[16]==0 then
                entry.classification=entry.seek and 'seek-only' or 'inactive';result.inactive=result.inactive+1
            else
                if c[27]==c[29] then stop(1);c[50]=1;entry.endpoint=true end
                lookup(2);local raw=mutation(4)
                if c[6]==0 and c[2]==0 and bit.band(c[18],6)~=0 then
                    if bit.band(c[18],2)~=0 and c[50]~=0 then
                        c[2]=4;irq();stop(2);entry.autopause=true
                    elseif bit.band(c[18],4)~=0 then
                        assert(not entry.seek and c[52]==255,'CD PLAY report lacks current complete SubQ')
                        reply[1],reply[2],reply[3]=c[3],q[1],q[2]
                        local channel=q[7]%2;local peak=0
                        for n=0,587 do
                            local lo,hi=raw:byte(n*4+channel*2+1,n*4+channel*2+2)
                            local sample=lo+hi*256;if sample>=32768 then sample=sample-65536 end
                            peak=math.max(peak,math.abs(sample))
                        end
                        peak=math.min(peak,32767)+channel*32768
                        if bit.band(q[8],16)~=0 then reply[4],reply[5],reply[6]=q[3],bit.bor(q[4],128),q[5]
                        else reply[4],reply[5],reply[6]=q[6],q[7],q[8] end
                        reply[7],reply[8]=peak%256,math.floor(peak/256)
                        c[2]=1;resize(8);irq();entry.report=true;result.reports=result.reports+1
                    end
                end
                take('gate') -- Eligibility checks the gate's mode, flags, ancestry and exact feed condition.
                if c[16]==0 then entry.classification='stopped';result.stopped=result.stopped+1
                else
                    if c[23]==0 then
                        mutation(5)
                        local feed=take('feed').data
                        assert(feed[1]==1 and feed[2]==44100 and feed[3]==588 and feed[4]==1,'CD PLAY feed shape mismatch')
                        take('outcome')
                    end
                    c[27]=nextMSF(c[27]);schedule(math.floor(clock/75)*(c[26]~=0 and 30 or 1));c[26]=0
                    assert(take('subq').data[1]==1,'CD PLAY lacks generated final SubQ')
                    delegated[22],delegated[50]=true,true;c[52],c[53]=255,1
                    delegated[52],delegated[53]=nil,nil;subq=true
                    entry.classification='played';result.played=result.played+1
                end
            end
            if builder then
                local t=take('publish')
                assert(t.data[1]==c[10] and t.data[3]==c[12] and t.data[4]==c[11] and t.data[5]==c[2] and
                    t.payload==string.char(unpack(reply,1,c[10])),'CD PLAY publication mismatch')
            end
        end
        assert(cursor==#active.tape,'CD PLAY has unexplained body work')
        local after=frame.after_context
        for n=1,56 do assert(delegated[n] or after.words[n]==c[n],'CD PLAY final context mismatch at word '..n) end
        assert(after.parameters_hex==frame.before_context.parameters_hex,'CD PLAY changed shared parameters')
        assert(after.response_hex==(string.char(unpack(reply)):gsub('.',function(ch) return string.format('%02x',ch:byte()) end)),
            'CD PLAY response buffer mismatch')
        assert(subq or after.subq_hex==frame.before_context.subq_hex,'CD PLAY changed unrelated SubQ bytes')
        entry.media_semantics_deferred=media;entry.subq_semantics_deferred=subq
        result.callbacks[#result.callbacks+1]=entry;active=nil
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        local frame=controller.current()
        if kind==34 and phase==2 then
            assert(not clock and d[2]>0 and d[2]<=100000000,'invalid CD PLAY clock');clock=d[2]
        elseif kind==34 and phase==3 and frame and frame.role==7 then
            assert(clock and not active,'CD PLAY missing environment or overlapping callback')
            active={id=frame.id,cycle=row.cycle,tape={}}
        elseif active then
            assert(frame and frame.id==active.id and row.cycle==active.cycle,'CD PLAY scope/cycle changed')
            if kind==34 and phase==4 then finish(frame);return end
            if kind==45 and (phase==3 or phase==5) then return end
            local t={data=d,phase=phase,payload=payload,request=row.request}
            if kind==40 and phase==0 then
                assert(not active.medium or active.medium.outcome,'overlapping CD PLAY media lookup')
                t.kind='lookup';active.medium=t
            elseif kind==40 then
                local medium=assert(active.medium,'CD PLAY media record outside lookup')
                if phase==1 then
                    assert(not medium.outcome and row.request==medium.request,'CD PLAY lookup outcome mismatch');medium.outcome=d
                else assert(phase==2,'unexpected CD PLAY media phase') end
                return
            elseif kind==46 and active.medium and (phase==3 or phase==4 or phase==10 or phase==11) then return
            else
                if active.medium then assert(active.medium.outcome,'unfinished CD PLAY lookup');active.medium=nil end
                if kind==38 and phase==0 then t.kind='resize'
                elseif kind==38 and phase==1 then t.kind='publish'
                elseif kind==39 and phase==0 then t.kind='irq'
                elseif kind==12 and phase==0 then t.kind='assert'
                elseif kind==44 and phase==1 then t.kind='stop'
                elseif kind==41 and phase==1 then t.kind='mutation'
                elseif kind==43 and phase==5 then t.kind='gate'
                elseif kind==43 and phase==0 then t.kind='feed'
                elseif kind==43 and phase==1 then t.kind='outcome'
                elseif kind==37 and phase==14 and d[1]==0 then t.kind='schedule'
                elseif kind==10 and phase==14 then return
                elseif kind==46 and phase==4 then t.kind='subq'
                else error('unexpected CD PLAY body record') end
            end
            active.tape[#active.tape+1]=t
        end
    end
    function tracker.finish()
        assert(not active,'unfinished CD PLAY callback')
        result.scope='PLAY seek, stop/report and local state/response/scheduling; media/SubQ arithmetic, attenuation and feed contents remain separate obligations'
        return result
    end
    return tracker
end
return M
