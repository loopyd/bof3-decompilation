-- READ callback branches, local state, response work and unscaled delays.
-- Scheduler scaling, XA decisions and native media/SubQ semantics have owners.
local bit, m, events = require 'bit', require 'support', require 'events'
local M = {}
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function nextMSF(msf)
    local minute,second,frame=msf%256,math.floor(msf/256)%256,math.floor(msf/65536)%256
    frame=(frame+1)%256
    if frame>=75 then
        frame=0;second=(second+1)%256
        if second>=60 then second=0;minute=(minute+1)%256 end
    end
    return minute+second*256+frame*65536
end
function M.new(controller)
    local tracker, active={},nil
    local clock
    local result={callbacks=m.array(),delivered=0,errors=0,inactive=0,busy=0,retried=0}
    local function stable(before,after,changes)
        for n=1,56 do
            if not changes[n] then assert(before.words[n]==after.words[n],
                'CD READ changed unrelated context word '..n) end
        end
        assert(before.parameters_hex==after.parameters_hex,'CD READ changed shared parameters')
    end
    local function delay(a,expected)
        assert(#a.schedules==1 and a.schedules[1].delay==expected,'CD READ exact delay mismatch')
        return a.schedules[1].sequence
    end
    local function finish(frame)
        local a=assert(active,'CD READ delivery boundary missing')
        local before,after=frame.before_context,assert(frame.after_context,'CD READ lacks final context')
        local b,c=before.words,after.words
        local entry={access=events.exact(frame.id),classification=a.branch}
        assert(a.branch,'CD READ lacks evaluated IRQ predicate')
        if a.branch~='read' then
            assert(not a.mutation and #a.builders==0 and #a.publications==0 and #a.lookups==0 and
                #a.irqs==0 and not a.buffer,'CD READ early return fabricated delivery work')
            local changes={}
            if a.branch=='inactive' then
                assert(#a.schedules==0 and #a.body==0,'inactive CD READ performed work')
                result.inactive=result.inactive+1
            elseif a.branch=='busy' then
                entry.delay=256;delay(a,entry.delay)
                assert(#a.body==2 and a.body[1]=='schedule' and a.body[2]=='ancestry',
                    'busy CD READ has unexpected work')
                result.busy=result.busy+1
            else
                entry.delay=math.floor(math.floor(clock/75)/2);delay(a,entry.delay)
                assert(#a.body==3 and a.body[1]=='irq' and a.body[2]=='schedule' and a.body[3]=='ancestry',
                    'IRQ-delay CD READ has unexpected work')
                assert(c[49]==1,'CD READ retry flag was not set');changes[49]=true
                result.retried=result.retried+1
            end
            stable(before,after,changes)
            assert(before.response_hex==after.response_hex and before.subq_hex==after.subq_hex,
                'CD READ early return changed response/SubQ bytes')
        else
            local mutation=a.mutation
            assert(mutation and a.buffer and #a.lookups>=1 and a.lookups[1].success~=nil,
                'CD READ lacks required lookup/buffer/mutation')
            local succeeded=a.lookups[1].success and a.buffer.present
            assert(mutation.reason==(succeeded and 2 or 3), 'CD READ mutation differs from evaluated success/buffer')
            assert(#a.builders==1 and a.irq.sequence<a.builders[1] and a.builders[1]<a.lookups[1].sequence and
                a.lookups[1].sequence<mutation.sequence and #a.publications==1,
                'CD delivery lacks ordered response construction/publication')
            assert(a.lookups[1].msf==b[27] and a.lookups[1].finished<a.buffer.sequence and
                a.buffer.sequence<mutation.sequence,'CD READ lookup/buffer order or sector mismatch')
            local publication=a.publications[1]
            assert(publication.count==1 and publication.sequence>mutation.sequence,
                'CD delivery response has wrong size/order')
            assert(bit.band(publication.byte,0x62)==0x22,
                'CD delivery response has wrong READ/ROTATING/SEEK status')
            local status=bit.band(bit.bor(b[3],0x22),bit.bnot(0x40))%4294967296
            assert(publication.byte==(succeeded and status or bit.bor(status,1)) and c[3]==status and
                c[17]==1 and c[10]==1 and c[11]==0 and c[12]==1,
                'CD READ status/seek/result state mismatch')
            local changed={[1]=true,[2]=true,[3]=true,[10]=true,[11]=true,[12]=true,[14]=true,[17]=true,
                [22]=true,[24]=true,[26]=true,[27]=true,[30]=true,
                [45]=true,[46]=true,[47]=true,[48]=true,[49]=true,[50]=true,[52]=true,[53]=true}
            if succeeded then changed[19],changed[20],changed[21]=true,true,true end
            stable(before,after,changed)
            assert(before.response_hex:sub(3)==after.response_hex:sub(3),
                'CD READ changed unused response bytes')
            if mutation.reason==2 then
                assert(#a.schedules==1 and mutation.sequence<a.schedules[1].sequence,
                    'CD delivered read lacks mandatory next-read schedule')
                local interval=math.floor(clock/75)
                if bit.band(b[18],128)~=0 then interval=math.floor(interval/2) end
                if b[26]~=0 then interval=interval*30 end
                entry.delay=interval;local scheduled=delay(a,interval)
                assert(#a.lookups==2 and scheduled<a.lookups[2].sequence and
                    a.lookups[2].sequence<publication.sequence and
                    a.lookups[2].msf==nextMSF(a.lookups[1].msf),
                    'CD delivered read lacks ordered next-sector lookup')
                assert(frame.after_mode[8]%256==0 and bit.band(frame.after[1],64)~=0,
                    'CD delivered read has wrong transfer-ready state')
                assert(c[1]==bit.bor(b[1],64)%4294967296 and c[14]==0 and c[49]==0 and c[26]==0 and
                    c[27]==nextMSF(b[27]), 'CD delivered READ movement/flags mismatch')
                local irq=bit.band(frame.before_mode[1],64)==0 or bit.band(mutation.submode,4)==0
                assert(#a.irqs==(irq and 1 or 0) and publication.irq==(irq and 1 or 0),
                    'CD delivered read has wrong DataReady obligation')
                if irq then
                    assert(a.irqs[1].irq==1 and scheduled<a.irqs[1].sequence and
                        a.irqs[1].sequence<a.lookups[2].sequence,
                        'CD DataReady is outside delivery order')
                end
                assert(c[2]==(irq and 1 or 0),'CD delivered READ final IRQ mismatch')
                result.delivered=result.delivered+1;entry.classification='delivered'
            else
                assert(#a.lookups==1 and #a.schedules==0 and #a.irqs==1 and
                    a.irqs[1].irq==5 and mutation.sequence<a.irqs[1].sequence and
                    a.irqs[1].sequence<publication.sequence and publication.irq==5 and
                    bit.band(publication.byte,1)==1,
                    'CD error-zero read lacks terminal DiskError path')
                assert(c[1]==b[1] and c[2]==5 and c[14]==b[14] and c[49]==b[49] and
                    c[26]==b[26] and c[27]==b[27] and c[24]==0,
                    'CD error-zero READ moved sector or changed transfer flags')
                result.errors=result.errors+1;entry.classification='error-zero'
            end
            local cache=b[30]
            for _,lookup in ipairs(a.lookups) do
                assert(lookup.success~=nil,'CD READ lookup lacks result')
                if lookup.disposition~=1 then cache=lookup.success and lookup.msf or 0 end
            end
            assert(c[30]==cache and c[24]==(succeeded and a.lookups[#a.lookups].success and 1 or 0),
                'CD READ final cache/success differs from lookup')
        end
        result.callbacks[#result.callbacks+1]=entry;active=nil
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        local frame=controller.current()
        if kind==34 and phase==2 then
            assert(not clock and d[2]>0 and d[2]<=100000000,'invalid CD READ clock');clock=d[2]
        elseif kind==34 and phase==3 and frame and frame.role==6 then
            assert(not active,'nested CD READ delivery')
            local b=assert(frame.before_context,'CD READ lacks initial context').words
            assert(clock,'CD READ lacks clock environment')
            active={id=frame.id,cycle=row.cycle,builders={},publications={},lookups={},schedules={},irqs={},body={},
                branch=b[15]==0 and 'inactive' or ((b[6]~=0 or b[2]~=0) and 'busy' or nil)}
        elseif active then
            assert(frame and frame.id==active.id,'CD READ delivery scope interrupted')
            assert(row.cycle==active.cycle,'CD READ callback changed CPU cycle')
            if active.assertion then
                assert(kind==12 and phase==0 and d[1]==4 and d[2]==active.irq.istat and
                    d[3]==bit.bor(d[2],4)%4294967296,
                    'CD READ lacks adjacent native IRQ assertion')
                active.assertion=nil;return
            end
            if kind==34 and phase==4 then finish(frame);return end
            if kind==45 and (phase==3 or phase==5) then return end
            local token='other'
            if kind==46 and phase==1 then token='irq'
            elseif kind==10 and phase==3 then token='schedule'
            elseif kind==37 and phase==3 and d[1]==0 then token='ancestry' end
            active.body[#active.body+1]=token
            if kind==46 and phase==1 then
                assert(not active.branch and not active.irq and #active.body==1,'unexpected CD READ IRQ predicate')
                active.irq={sequence=row.sequence,istat=d[1],imask=d[2]}
                local b=frame.before_context.words
                active.branch=bit.band(d[1],d[2],4)~=0 and b[49]==0 and 'irq-delay' or 'read'
            elseif kind==46 and phase==2 then
                assert(active.branch=='read' and d[1]==2 and not active.buffer and #active.lookups==1,
                    'CD READ buffer predicate site/order mismatch')
                active.buffer={sequence=row.sequence,present=d[2]==1}
                active.medium=nil
            elseif kind==46 then
                assert(active.branch=='read' and (phase==3 or phase==4 or phase==10 or phase==11),
                    'unexpected CD READ predicate')
                assert(active.medium,'CD READ media predicate outside lookup work')
            elseif kind==38 and phase==0 then
                assert(d[1]==1,'CD READ response builder has wrong size')
                active.builders[#active.builders+1]=row.sequence
            elseif kind==38 and phase==1 then
                active.medium=nil
                active.publications[#active.publications+1]={sequence=row.sequence,count=d[1],irq=d[5],byte=payload:byte(1)}
            elseif kind==40 and phase==0 then
                assert(d[1]==1,'CD READ used non-data lookup')
                active.medium=true
                active.lookups[#active.lookups+1]={id=row.request,sequence=row.sequence,msf=d[2]}
            elseif kind==40 and phase==1 then
                local lookup=active.lookups[#active.lookups]
                assert(lookup and lookup.id==row.request and lookup.success==nil,'CD READ lookup result is unpaired')
                lookup.success=d[2]==1;lookup.disposition=d[3];lookup.finished=row.sequence
            elseif kind==41 and phase==1 then
                assert(not active.mutation and (d[3]==2 or d[3]==3) and #active.lookups==1,
                    'CD READ has unexpected buffer mutation')
                active.mutation={sequence=row.sequence,reason=d[3],submode=payload:byte(39)}
                active.medium=nil
            elseif kind==37 and d[1]==0 then
                assert(phase==3,'CD READ scheduled wrong callback')
                active.schedules[#active.schedules+1]={sequence=row.sequence,delay=d[2]}
            elseif kind==39 and phase==0 then
                assert(d[2]==frame.before_context.words[4] and
                    d[3]==(bit.band(d[1],d[2])~=0 and 1 or 0),
                    'CD READ IRQ mask/assertion differs from context')
                active.irqs[#active.irqs+1]={sequence=row.sequence,irq=d[1]}
                active.assertion=d[3]==1
            elseif kind==10 and phase==3 then
                -- Paired with the adjacent CD schedule by the scheduler consumer.
            elseif kind==40 and phase==2 then
                assert(active.medium,'CD READ media span outside lookup work')
            elseif kind==43 and phase>=0 and phase<=4 then
                -- Eligibility/feeds own exact decision order and identities.
                assert(active.mutation and active.mutation.reason==2 and #active.schedules==0 and
                    #active.lookups==1 and #active.publications==0,
                    'CD READ XA work outside delivered sector window')
            else
                error('unexpected CD READ body record')
            end
        end
    end
    function tracker.finish()
        assert(not active,'unfinished CD READ delivery')
        result.scope='READ branch selection, local state, required response work and unscaled delays; media/SubQ, XA and scaled scheduler targets require their consumers'
        return result
    end
    return tracker
end
return M
