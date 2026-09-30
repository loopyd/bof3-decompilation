-- Lid/seek callback control flow. Host-time openness is an evaluated predicate;
-- mounted-media rescan and CD label/ID writes need further native observations.
local bit,m,events,image=require 'bit',require 'support',require 'events',require 'image'
local M={}
local function clear(n,mask) return bit.band(n,bit.bnot(mask))%4294967296 end
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function compare(before,after,c,delegated)
    for n=1,56 do assert(delegated[n] or after.words[n]==c[n],'CD lid context mismatch at word '..n) end
    for _,name in ipairs({'parameters_hex','response_hex','subq_hex'}) do
        assert(before[name]==after[name],'CD lid changed '..name)
    end
end
function M.new(controller,geometry)
    local tracker,stack,clock={},{}
    local result={callbacks=m.array(),interrupts=0,seeks=0,rescans=0}
    local function finish(active,frame)
        local before,c,delegated=frame.before_context,{},{}
        for n=1,56 do c[n]=before.words[n] end
        local cursor=0
        local function take(kind)
            cursor=cursor+1;local t=active.tape[cursor]
            assert(t and t.kind==kind,'CD lid expected '..kind..' at body item '..cursor);return t
        end
        local entry={access=events.exact(frame.id),role=frame.role,check_deferred=false}
        local function status()
            local d=take('status').data
            local playing=c[16]~=0
            assert(d[1]<=1 and d[2]==(playing and 2 or image.track(geometry(),1).type) and
                d[3]==d[1]*16+(playing and 128 or 0) and d[4]==c[27],'CD lid status predicate mismatch')
            entry.open=d[1]==1;return entry.open
        end
        local function schedule(delay)
            assert(take('schedule').data[2]==delay,'CD lid exact delay mismatch');entry.delay=delay
        end
        local function stop(kind)
            local t=take('stop');local n=kind==1 and 16 or 15
            assert(t.phase==kind and t.data[1]==0 and t.data[2]==3 and t.data[3]==c[n],
                'CD lid stream stop mismatch')
            if kind==1 then
                if c[16]~=0 then c[3]=clear(c[3],128);c[16],c[32],c[33]=0,0,0 end
                delegated[43],delegated[44]=true,true
            else
                if c[15]~=0 then take('cancel');c[15]=0 end
                c[3]=clear(c[3],96);delegated[41],delegated[42]=true,true
            end
        end
        if frame.role==8 then
            c[29]=image.td(geometry(),0);stop(1)
            local child=take('child').frame
            assert(child.before_context and child.after_context,'CD lid missing completed nested seek')
            compare(before,child.before_context,c,delegated)
            compare(child.after_context,frame.after_context,child.after_context.words,{})
            entry.classification='interrupt';entry.child=events.exact(child.id)
            result.interrupts=result.interrupts+1
        else
            local drive,base=c[31],math.floor(clock/75)
            if drive==1 then
                local open=status()
                if bit.band(c[3],16)==0 then
                    stop(0);c[3]=bit.bor(c[3],16);schedule(base*30);entry.classification='opening'
                elseif bit.band(c[3],2)~=0 then
                    c[3]=clear(c[3],2);schedule(base*3);entry.classification='spindown'
                elseif not open then
                    assert(geometry().header.main==0,'CD lid mounted-media rescan is unsupported')
                    c[31]=2;schedule(base*105);entry.classification='closed';entry.check_deferred=true
                    result.rescans=result.rescans+1
                else schedule(base*3);entry.classification='waiting' end
            elseif drive==2 then
                c[3],c[31]=bit.bor(c[3],2),3;schedule(base*150);entry.classification='rescan'
            elseif drive==3 then
                c[3],c[31]=bit.bor(c[3],64),0;schedule(base*26);entry.classification='prepare'
            else
                c[3]=clear(c[3],64)
                if status() then
                    stop(1);c[31]=1;schedule(2048);entry.classification='opened'
                else entry.classification='standby' end
            end
            compare(before,frame.after_context,c,delegated);result.seeks=result.seeks+1
        end
        assert(cursor==#active.tape,'CD lid has unexplained body work')
        result.callbacks[#result.callbacks+1]=entry
    end
    function tracker.push(row)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        local frame,active=controller.current(),stack[#stack]
        if kind==34 and phase==2 then
            assert(not clock and d[2]>0 and d[2]<=100000000,'invalid CD lid clock');clock=d[2]
        elseif kind==34 and phase==3 and frame and (frame.role==8 or frame.role==9) then
            assert(clock and (not active or (active.child==frame.id and active.cycle==row.cycle)),
                'CD lid missing environment or overlapping callback')
            stack[#stack+1]={id=frame.id,role=frame.role,cycle=row.cycle,tape={}}
        elseif active then
            assert(row.cycle==active.cycle,'CD lid cycle changed')
            if kind==45 and phase==0 and d[1]==9 then
                assert(active.role==8 and not active.child and frame and frame.parent==active.id,
                    'CD lid unexpected nested callback')
                active.child=frame.id;active.tape[#active.tape+1]={kind='child',frame=frame};return
            elseif active.child then
                if kind==45 and phase==1 and row.request==active.child then
                    assert(frame and frame.id==active.id,'CD lid nested exit lost parent');active.child=nil;return
                end
                assert(frame and frame.id==active.child and
                    ((kind==37 and phase==13 and d[1]==1) or (kind==45 and (phase==2 or phase==4))),
                    'CD lid unexpected nested boundary');return
            end
            assert(frame and frame.id==active.id,'CD lid scope changed')
            if kind==34 and phase==4 then finish(active,frame);table.remove(stack);return end
            if kind==45 and (phase==3 or phase==5) then return end
            local t={data=d,phase=phase}
            if kind==46 and phase==7 then t.kind='status'
            elseif kind==44 then t.kind='stop'
            elseif kind==37 and phase==13 and d[1]==0 then t.kind='schedule'
            elseif kind==37 and phase==3 and d[1]==2 then t.kind='cancel'
            elseif kind==10 and phase==13 then return -- Scheduler owns native adjacency/scaling.
            else error('unexpected CD lid body record') end
            active.tape[#active.tape+1]=t
        end
    end
    function tracker.finish()
        assert(#stack==0,'unfinished CD lid callback')
        result.scope='lid/seek local state, stream stops and exact schedules; openness is an evaluated host-time predicate; rescan ID/label writes remain unobserved and mounted-media closure unsupported'
        return result
    end
    return tracker
end
return M
