-- Required CD DMA issuance work; consume after controller and scheduler.
-- Callback CHCR snapshots establish immediate and scheduled busy eligibility.
local bit, m, events = require 'bit', require 'support', require 'events'
local M={}
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
function M.new(controller)
    local tracker,pending,active,completion={},nil,nil,nil
    local current,completed=0,{}
    local result={requests=m.array(),completions=m.array(),callbacks=m.array()}
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        local frame=controller.current()
        if completion then
            local c=completion
            if c.stage==0 then
                assert(kind==7 and phase==3 and d[1]==0 and row.request==c.owner and row.related==c.current,
                    'CD DMA completion lacks adjacent IRQ entry')
                c.before=d[2];c.after=d[2];c.assertion=false
                if bit.band(d[2],0x880000)==0x880000 then
                    c.after=bit.bor(d[2],0x88000000)%4294967296
                    c.assertion=bit.band(d[2],0x80000000)==0
                end
                c.stage=c.assertion and 1 or 2
            elseif c.stage==1 then
                assert(kind==12 and phase==0 and d[1]==8 and d[3]==bit.bor(d[2],8)%4294967296,
                    'CD DMA completion lacks required IRQ assertion');c.stage=2
            else
                assert(kind==7 and phase==3 and d[1]==1 and d[2]==c.after and
                    row.request==c.owner and row.related==c.current,
                    'CD DMA completion IRQ result mismatch')
                result.completions[#result.completions+1]={owner=events.exact(c.owner),current=events.exact(c.current),
                    dicr_before=c.before,dicr_after=c.after,asserted=c.assertion}
                current=0;completion=nil
            end
        elseif kind==6 and phase==3 then
            assert(frame and frame.role==11 and frame.stage==4 and not completed[events.exact(frame.id)] and
                frame.before_context and d[1]==frame.before_context.words[54] and
                row.request==frame.owner and row.related==current and bit.band(d[1],0x01000000)~=0 and
                d[2]==bit.band(d[1],bit.bnot(0x01000000))%4294967296,
                'CD DMA completion busy transition mismatch')
            completed[events.exact(frame.id)]=true
            completion={owner=row.request,current=row.related,stage=0}
        elseif kind==7 and phase==3 then error('CD DMA IRQ lacks completion') end
        if kind==34 and phase==4 and frame and frame.role==11 then
            local before=assert(frame.before_context,'CD DMA callback lacks CHCR snapshot').words[54]
            local after=assert(frame.after_context,'CD DMA callback lacks final CHCR snapshot').words[54]
            local busy=bit.band(before,0x01000000)~=0
            assert((completed[events.exact(frame.id)]==true)==busy,
                'CD DMA callback completion differs from busy eligibility')
            assert(after==bit.band(before,bit.bnot(0x01000000))%4294967296,
                'CD DMA callback final CHCR differs from busy clear')
            result.callbacks[#result.callbacks+1]={access=events.exact(frame.id),owner=events.exact(frame.owner),
                chcr_before=before,chcr_after=after,busy=busy,completed=busy}
        end
        local started=false
        if pending then
            assert(kind==45 and phase==0 and d[1]==10 and row.related==0 and frame,
                'CD DMA start lacks adjacent root operation')
            active={id=frame.id,start=pending,bytes=0,schedules=0,children=0,completion=0,irqs=0}
            pending=nil;started=true
        end
        if kind==45 and phase==0 and d[1]==10 then
            assert(started,'CD DMA root operation lacks adjacent start')
        end
        if kind==3 and phase==3 then
            assert(not active and row.request~=0 and row.related==current and
                (d[4]==0x11000000 or d[4]==0x11400100) and
                d[2]==bit.band(d[1],0x7ffffc) and bit.band(d[6],0x8000)~=0,
                'invalid supported CD DMA start')
            pending={id=row.request,address=d[2],bcr=d[3],chcr=d[4]}
            current=row.request
        elseif active then
            local a=active
            assert(frame and (frame.id==a.id or (frame.role==11 and frame.parent==a.id)),
                'CD DMA issuance scope interrupted')
            if kind==45 and phase==4 and frame.id==a.id then
                a.ready=frame.before_mode[8]%256
                local size=bit.band(frame.before_mode[1],48)
                a.expected=(a.start.bcr%65536)*4
                if a.expected==0 then a.expected=size==32 and 2340 or (size==16 and 2328 or 2048) end
            elseif kind==45 and phase==0 and frame.role==11 then
                assert(a.ready==0,'ready CD DMA completed inline');a.children=a.children+1
            elseif kind==42 and phase==1 then
                assert(frame.id==a.id and a.ready~=0 and a.schedules==0 and
                    d[2]==(a.start.address+a.bytes)%4294967296,
                    'CD DMA byte count/address/order mismatch')
                a.bytes=a.bytes+1;assert(a.bytes<=a.expected,'CD DMA transferred beyond BCR size')
            elseif kind==37 and d[1]==0 then
                local delay=a.start.chcr==0x11400100 and math.floor(a.expected/16) or a.expected/4
                assert(frame.id==a.id and phase==10 and row.request==a.start.id and
                    a.bytes==a.expected and d[2]==delay,'CD DMA lacks exact post-transfer schedule')
                a.schedules=a.schedules+1
            elseif kind==6 and phase==3 then
                assert(a.ready==0 and frame.role==11 and a.completion==0 and a.irqs==0 and
                    row.request==a.start.id and row.related==a.start.id and d[1]==a.start.chcr and
                    d[2]==bit.band(a.start.chcr,bit.bnot(0x01000000)),
                    'CD immediate DMA completion mismatch')
                a.completion=1
            elseif kind==7 and phase==3 then
                assert(a.ready==0 and frame.role==11 and a.completion==1 and a.irqs<2 and
                    row.request==a.start.id and row.related==a.start.id and d[1]==a.irqs,
                    'CD immediate DMA IRQ boundaries mismatch')
                a.irqs=a.irqs+1
            elseif kind==45 and phase==5 and frame.id==a.id then
                if a.ready==0 then
                    assert(a.bytes==0 and a.schedules==0 and a.children==1 and a.completion==1 and a.irqs==2 and
                        frame.after[8]==frame.before[8] and frame.after_mode[8]==frame.before_mode[8],
                        'not-ready CD DMA lacks immediate completion without transfer')
                else
                    assert(a.bytes==a.expected and a.schedules==1 and a.children==0 and
                        a.completion==0 and a.irqs==0,'ready CD DMA lacks complete transfer/schedule')
                end
                result.requests[#result.requests+1]={id=events.exact(a.start.id),access=events.exact(a.id),
                    address=a.start.address,bcr=a.start.bcr,chcr=a.start.chcr,bytes=a.bytes,
                    completion=a.ready==0 and 'immediate' or 'scheduled'}
                active=nil
            end
        end
    end
    function tracker.finish()
        assert(not pending and not active and not completion,'unfinished CD DMA issuance/completion')
        result.scope='DMA issuance counts, addresses and schedules; callback CHCR busy eligibility and completion/IRQ transitions'
        return result
    end
    return tracker
end
return M
