-- CD read/play identities and their scheduler/buffer/feed ancestry. Call after
-- controller.push and scheduler.push on each validated wire row. Media outcomes,
-- audio gates/queues and complete capture acceptance belong to other consumers.
local ffi, bit, m, events = require 'ffi', require 'bit', require 'support', require 'events'
local M = {}
local zero=ffi.new('uint64_t',0)
local function word(bytes,offset)
    assert(#bytes>=offset+4,'short CD stream payload')
    local a,b,c,d=bytes:byte(offset+1,offset+4);return a+b*256+c*65536+d*16777216
end
local function identity(bytes,offset)
    return ffi.new('uint64_t',word(bytes,offset+4))*4294967296+word(bytes,offset)
end
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function meta(row)
    return {sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
end
function M.new(controller,scheduler)
    local tracker, known = {}, {}
    local active, commands, native = {[0]=zero,zero},{[0]=zero,zero},{}
    local serial,mutation=zero,zero
    local first,final,expectCancel
    local result={transitions=m.array(),mutations=m.array(),feeds=m.array(),decisions=m.array()}
    local function check(id,kind)
        if kind==2 or kind==3 then
            local stream=known[events.exact(id)]
            assert(id==0 or (stream and stream.kind==kind-2),'unknown or wrong-kind CD stream owner')
        elseif kind==1 then controller.command(id)
        else assert((kind==0 and id==0) or kind==4,'invalid CD stream ancestry kind') end
    end
    local function current()
        assert(first and not final,'CD stream operation outside context')
        return assert(controller.current(),'CD stream operation lacks controller scope')
    end
    local function effective(kind)
        local frame=current()
        return frame.ownerKind==kind+2 and frame.owner or active[kind]
    end
    local function flags(read,play)
        assert(read==native[0] and play==native[1],'CD native stream state changed without a transition')
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        if expectCancel then
            assert(kind==37 and phase==3 and d[1]==2 and row.related==expectCancel,
                'active CD read stop lacks adjacent cancellation')
        end
        if kind==34 and phase<2 then
            local read,play=identity(payload,160),identity(payload,168)
            local changed=identity(payload,184)
            if phase==0 then
                assert(not first and read==0 and play==0 and changed==0,'invalid CD stream initial identities')
                native[0],native[1]=word(payload,56),word(payload,60)
                assert(native[0]<=255 and native[1]<=1,'invalid initial CD native stream flags')
                first=true
            else
                assert(phase==1 and first and not final and read==active[0] and play==active[1] and changed==mutation,
                    'CD stream final identity mismatch')
                flags(word(payload,56),word(payload,60));final=true
            end
        elseif kind==45 and (phase==4 or phase==5) then
            current();flags(math.floor(d[8]/256)%256,math.floor(d[8]/65536)%256)
        elseif kind==44 then
            local frame=current();local operation=assert(controller.root(),'CD stream transition lacks root')
            assert((phase==0 or phase==1) and frame.stage==4 and #payload==40 and d[1]<=1 and d[2]<=3 and
                identity(payload,8)==frame.id and identity(payload,0)==active[phase] and
                identity(payload,16)==mutation and d[3]==native[phase],'CD stream transition state mismatch')
            local owner,ownerKind=scheduler.owner(phase==1 and 14 or 3)
            assert(identity(payload,24)==owner and d[8]==ownerKind,'CD stream pending ancestry mismatch')
            local trigger=d[2]==0 and (operation.role==5 and operation.owner or controller.identities().latest) or zero
            assert(identity(payload,32)==trigger,'CD stream triggering command mismatch');controller.command(trigger)
            local entry=meta(row);entry.kind=phase;entry.start=d[1]==1;entry.reason=d[2]
            entry.id=events.exact(row.request);entry.command=events.exact(row.related);entry.trigger=events.exact(trigger)
            entry.previous=events.exact(active[phase]);entry.access=events.exact(frame.id);entry.mutation=events.exact(mutation)
            entry.pending_owner=events.exact(owner);entry.pending_kind=ownerKind;entry.raw=m.array(d)
            if d[1]==1 then
                assert(operation.role==5 and d[2]==0 and row.request==serial+1 and row.related==trigger,
                    'CD stream start identity/command mismatch')
                serial=row.request;active[phase],commands[phase]=row.request,trigger
                known[entry.id]={kind=phase,command=trigger};native[phase]=1
            else
                assert(row.request==active[phase] and row.related==commands[phase],
                    'CD stream stop lost initiating command')
                assert(d[2]==0 or (phase==1 and (d[2]==1 or d[2]==2) and frame.role==7) or
                    (d[2]==3 and (frame.role==8 or frame.role==9)), 'CD stream stop reason/operation mismatch')
                if phase==0 and native[0]~=0 then expectCancel=frame.id end
                active[phase],commands[phase],native[phase]=zero,zero,0
            end
            result.transitions[#result.transitions+1]=entry
        elseif kind==37 then
            check(row.request,d[6])
            if d[1]==0 then
                local frame=current();local operation=controller.root()
                local owner,ownerKind=frame.owner,frame.ownerKind
                if operation.role==5 then owner,ownerKind=operation.owner,1 end
                if phase==2 then owner,ownerKind=controller.identities().queued,1
                elseif phase==3 then owner,ownerKind=active[0],2
                elseif (phase==14 or phase==12) and native[1]~=0 then owner,ownerKind=active[1],3
                elseif phase==10 then owner,ownerKind=row.request,4 end -- exact DMA join belongs to scheduler
                assert(row.request==owner and d[6]==ownerKind,'CD schedule selected wrong stream/context owner')
            elseif d[1]==2 then
                assert(expectCancel and row.related==expectCancel,'CD read cancellation lacks active stop')
                expectCancel=nil
            end
        elseif kind==41 and phase==1 then
            local frame=current()
            assert(row.request==mutation+1 and identity(payload,8)==mutation and identity(payload,0)==frame.id and
                identity(payload,16)==effective(0) and identity(payload,24)==effective(1),
                'CD buffer mutation stream ancestry mismatch')
            mutation=row.request
            local entry=meta(row);entry.id=events.exact(mutation);entry.read=events.exact(effective(0))
            entry.play=events.exact(effective(1));entry.access=events.exact(frame.id)
            result.mutations[#result.mutations+1]=entry
        elseif kind==43 and phase==0 then
            local streamKind=d[1];assert(streamKind<=1,'invalid CD feed stream kind')
            local frame=current();local owner=effective(streamKind)
            assert(row.related==frame.id and identity(payload,8)==mutation and identity(payload,16)==owner,
                'CD feed stream ancestry mismatch')
            check(owner,streamKind+2)
            local entry=meta(row);entry.feed=events.exact(row.request);entry.kind=streamKind
            entry.stream=events.exact(owner);entry.mutation=events.exact(mutation);entry.access=events.exact(frame.id)
            result.feeds[#result.feeds+1]=entry
        elseif kind==43 and phase>=2 then
            local frame=current();local streamKind=d[8];assert(streamKind<=1,'invalid CD decision stream kind')
            local owner=effective(streamKind)
            assert(row.request==mutation and identity(payload,0)==frame.id and identity(payload,8)==owner,
                'CD decision stream ancestry mismatch')
            assert(bit.band(d[1],256)==native[1]*256 and
                (bit.band(d[1],512)~=0)==(native[0]~=0),'CD decision native stream flags mismatch')
            check(owner,streamKind+2)
            local entry=meta(row);entry.phase=phase;entry.kind=streamKind;entry.stream=events.exact(owner)
            entry.mutation=events.exact(mutation);entry.access=events.exact(frame.id)
            result.decisions[#result.decisions+1]=entry
        end
    end
    function tracker.finish()
        assert(first and final and not expectCancel,'CD stream capture boundaries incomplete')
        result.scope='read/play and mutation/feed ancestry; media outcomes, audio gates/queues and full capture required separately'
        return result
    end
    return tracker
end
return M
