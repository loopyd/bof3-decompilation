-- Command callback branches and opcode-local effects; submission, media/SubQ
-- arithmetic and identity joins are explicitly separate consumers.
local ffi,bit,m,events,opcode=require 'ffi',require 'bit',require 'support',require 'events',require 'opcode'
local M={}
local function wide(lo,hi) return ffi.new('uint64_t',hi)*4294967296+lo end
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function bytes(hex)
    local r={};for pair in hex:gmatch('..') do r[#r+1]=tonumber(pair,16) end;return r
end
local function clear(n,mask) return bit.band(n,bit.bnot(mask))%4294967296 end
function M.new(controller,sectors,geometry)
    local tracker,active,pending,clock,istat={}
    local result={callbacks=m.array(),busy=0,retried=0,executed=0}
    local function finish(frame)
        local a=assert(active);local before,after=frame.before_context,frame.after_context
        local b,c=before.words,{};for n=1,56 do c[n]=b[n] end
        local x={words=c,reply=bytes(before.response_hex),params=bytes(before.parameters_hex),
            subq=bytes(before.subq_hex),header=a.header,image=geometry(),clock=clock,deferred={}}
        local cursor=0
        local function take(kind)
            cursor=cursor+1;local t=a.tape[cursor]
            assert(t and t.kind==kind,'CD command expected '..kind..' at body item '..cursor)
            return t
        end
        function x.resize(count)
            assert(take('resize').data[1]==count,'CD command response resize mismatch')
            c[10],c[11],c[12]=count,0,1
        end
        function x.schedule(slot,delay)
            local t=take('schedule')
            assert(t.phase==slot and t.data[2]==delay,'CD command exact schedule mismatch')
        end
        function x.queue(op,delay)
            local d=take('queue').data
            assert(d[1]==c[6] and d[2]==c[8] and d[3]==c[7] and d[4]==op and d[5]==delay and d[6]==0,
                'CD command delayed-stage queue mismatch')
            c[6],c[8]=op,delay;x.deferred[39],x.deferred[40]=true,true
            x.schedule(2,delay)
        end
        function x.stop(kind)
            local t=take('stream');local n=kind==1 and 16 or 15
            assert(t.phase==kind and t.data[1]==0 and t.data[2]==0 and t.data[3]==c[n],
                'CD command stream stop mismatch')
            if kind==1 then
                if c[16]~=0 then c[3]=clear(c[3],128);c[16],c[32],c[33]=0,0,0 end
                x.deferred[43],x.deferred[44]=true,true
            else
                if c[15]~=0 then take('cancel');c[15]=0 end
                c[3]=clear(c[3],96);x.deferred[41],x.deferred[42]=true,true
            end
        end
        function x.start(kind)
            local t=take('stream');local n=kind==1 and 16 or 15
            assert(t.phase==kind and t.data[1]==1 and t.data[2]==0 and t.data[3]==c[n],
                'CD command stream start mismatch')
            c[n]=1
            local offset=kind==1 and 43 or 41;x.deferred[offset],x.deferred[offset+1]=true,true
        end
        function x.predicate(selector,site)
            local t=take('predicate')
            assert(t.phase==selector and (not site or t.data[1]==site),'CD command predicate selector/site mismatch')
            return t
        end
        function x.lookup(msf)
            local t=take('lookup');local d=t.data
            assert(d[1]==1 and d[2]==msf and t.outcome,'CD command data lookup mismatch')
            if t.outcome[3]==1 then
                assert((c[24]~=0)==(t.outcome[2]==1),'CD command cached success mismatch')
            else c[24]=t.outcome[2];c[30]=c[24]~=0 and msf or 0 end
            -- Track/SubQ generation and backend selection remain media obligations.
            for _,n in ipairs({22,50,52,53}) do x.deferred[n]=true end
            x.media=true
        end
        function x.mutate()
            local d=take('mutation').data
            assert(d[1]==0 and d[2]==8 and d[3]==1 and d[4]==c[27],
                'CD command header-copy mismatch')
            x.deferred[47],x.deferred[48]=true,true
        end
        local execution=take('execute').data
        for n,slot in ipairs({6,5,9,2,8,7}) do
            assert(execution[n]==b[slot],'CD command execution snapshot mismatch')
        end
        local branch
        if b[2]~=0 then branch='busy';x.schedule(2,256);result.busy=result.busy+1
        else
            c[1]=clear(c[1],128);x.resize(1);x.reply[1]=b[3];c[2]=3
            x.deferred[45],x.deferred[46]=true,true
            if b[7]~=0 then c[7]=0 end
            if b[7]~=0 and ffi.new('uint64_t',b[8])>a.target-a.cycle then
                branch='retry';x.schedule(2,b[8]);result.retried=result.retried+1
            else
                branch='executed';c[6]=0;opcode.apply(x,b[6]);result.executed=result.executed+1
            end
            local irq=take('irq').data
            local asserted=bit.band(c[2],c[4])~=0
            assert(irq[1]==c[2] and irq[2]==c[4] and irq[3]==(asserted and 1 or 0) and irq[4]==1,
                'CD command terminal IRQ mismatch')
            if asserted then assert(take('assert').data[1]==4,'CD command asserted wrong native IRQ') end
            c[9]=0
            local p=take('publish')
            assert(p.data[1]==c[10] and p.payload==string.char(unpack(x.reply,1,c[10])),
                'CD command published response mismatch')
        end
        assert(cursor==#a.tape,'CD command has unexplained body work')
        for n=1,56 do
            assert(x.deferred[n] or after.words[n]==c[n],'CD command final context mismatch at word '..n)
        end
        assert(after.parameters_hex==before.parameters_hex,'CD command changed shared parameter bytes')
        assert(after.response_hex==(string.char(unpack(x.reply)):gsub('.',function(ch)
            return string.format('%02x',ch:byte()) end)),'CD command response buffer mismatch')
        if not x.media then assert(after.subq_hex==before.subq_hex,'CD command changed unrelated SubQ bytes') end
        result.callbacks[#result.callbacks+1]={access=events.exact(frame.id),opcode=b[6],classification=branch,
            target=events.exact(a.target),media_semantics_deferred=x.media or false}
        active=nil
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        local frame=controller.current()
        if kind==25 then assert(istat==nil,'duplicate initial IRQ latch');istat=d[1]
        elseif kind==12 or kind==13 then
            assert(istat~=nil and d[2]==istat and d[3]==(kind==12 and bit.bor(istat,d[1])%4294967296 or
                clear(istat,d[1])),'native IRQ latch transition mismatch')
            istat=d[3]
        end
        if kind==34 and phase==2 then
            assert(not clock and d[2]>0 and d[2]<=100000000,'invalid CD command clock');clock=d[2]
        elseif kind==37 and phase==2 and d[1]==1 then
            assert(not pending and not active,'overlapping CD command dispatch')
            pending={id=row.related,target=wide(d[3],d[4]),cycle=row.cycle}
        elseif kind==34 and phase==3 and frame and frame.role==5 then
            assert(pending and pending.id==frame.id and pending.cycle==row.cycle and clock and istat,
                'CD command lacks dispatch/environment')
            active={id=frame.id,cycle=row.cycle,target=pending.target,tape={},header=sectors.header()};pending=nil
        elseif active then
            assert(frame and frame.id==active.id and row.cycle==active.cycle,'CD command scope/cycle changed')
            if kind==34 and phase==4 then finish(frame);return end
            if kind==45 and (phase==3 or phase==5) then return end
            local t={data=d,phase=phase,payload=payload,request=row.request}
            if kind==40 and phase==0 then
                t.kind='lookup';active.medium=t
            elseif kind==40 then
                local medium=assert(active.medium,'CD command media work outside lookup')
                if phase==1 then
                    assert(not medium.outcome and row.request==medium.request,'CD command lookup outcome mismatch')
                    medium.outcome=d
                else assert(phase==2,'unexpected CD command media phase') end
                return
            elseif kind==46 and (phase==3 or phase==4 or phase==10 or phase==11) then
                assert(active.medium,'CD command media predicate outside lookup');return
            else
                if active.medium then assert(active.medium.outcome,'unfinished CD command lookup');active.medium=nil end
                if kind==36 and phase==1 then t.kind='execute'
                elseif kind==36 and phase==2 then t.kind='queue'
                elseif kind==38 and phase==0 then t.kind='resize'
                elseif kind==38 and phase==1 then t.kind='publish'
                elseif kind==37 and d[1]==0 then t.kind='schedule'
                elseif kind==37 and phase==3 and d[1]==2 then t.kind='cancel'
                elseif kind==44 then t.kind='stream'
                elseif kind==46 and (phase==2 or phase==5 or phase==6 or phase==7) then t.kind='predicate'
                elseif kind==41 and phase==1 then t.kind='mutation'
                elseif kind==39 and phase==0 then t.kind='irq'
                elseif kind==12 and phase==0 then t.kind='assert'
                elseif kind==10 and (phase==2 or phase==3 or phase==13 or phase==14) then
                    return -- Scheduler requires adjacent typed ancestry.
                else error('unexpected CD command body record') end
            end
            active.tape[#active.tape+1]=t
        end
    end
    function tracker.finish()
        assert(not active and not pending,'unfinished CD command callback')
        result.scope='callback branch, opcode-local state/response writes and required effects; submission and native media/SubQ arithmetic are separate obligations'
        return result
    end
    return tracker
end
return M
