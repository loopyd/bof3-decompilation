-- Correlate CD slot ancestry with native scheduler records. Feed validated wire
-- rows after controller.push; stream/media/Audio semantics remain separate.
local ffi, bit, m, events = require 'ffi', require 'bit', require 'support', require 'events'
local M = {}
local zero = ffi.new('uint64_t',0)
local slots = {[2]=5,[3]=6,[10]=11,[12]=12,[13]=9,[14]=7}
local mask = 0x740c
local function wide(lo,hi) return ffi.new('uint64_t',hi)*4294967296+lo end
local function word(bytes,offset)
    assert(#bytes>=offset+4,'short CD scheduler payload')
    local a,b,c,d=bytes:byte(offset+1,offset+4);return a+b*256+c*65536+d*16777216
end
local function identity(bytes,offset) return wide(word(bytes,offset),word(bytes,offset+4)) end
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function meta(row)
    return {sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
end
function M.new(controller)
    local tracker, native, owners, initial, final = {}, {}, {}, {}, {}
    local result={boundaries=m.array(),schedules=m.array(),dispatches=m.array(),cancellations=m.array()}
    local initialCount, finalCount, pending = 0,0,0
    local fullInitial, context, ended, scales, previous, awaiting, readPass
    local function requireOwner(id,kind)
        assert(kind>=0 and kind<=4 and (kind~=0 or id==0),'invalid CD scheduler owner kind')
        if kind==1 then controller.command(id) end
    end
    local function pendingMatches(bits)
        assert(bit.band(bits,bit.bnot(0x7fff))==0 and bit.band(bits,mask)==pending,
            'CD scheduler pending bits mismatch')
    end
    local function active(slot)
        assert(context and not ended and initialCount==6 and initial[slot],
            'CD scheduler operation outside complete boundaries')
        return assert(native[slot],'CD scheduler slot lacks native initial state')
    end
    local function callback(row,d)
        local slot=tonumber(row.device)
        local state=active(slot)
        local frame=assert(controller.current(),'CD dispatch has no controller scope')
        assert(d[2]==slots[slot] and frame.role==d[2] and row.related==frame.id,
            'CD dispatch callback mismatch')
        assert(wide(d[3],d[4])==state.target,'CD dispatch target mismatch')
        pendingMatches(d[5]);requireOwner(row.request,d[6])
        local entry=meta(row);entry.slot=slot;entry.access=events.exact(row.related)
        entry.owner=events.exact(row.request);entry.owner_kind=d[6];entry.nested=d[7]==1
        entry.target=events.exact(state.target)
        if d[7]==0 then
            assert(awaiting and awaiting.stage==1 and awaiting.slot==slot and awaiting.access==frame.id and
                previous and previous.kind==45 and previous.device==0 and previous.request==frame.id and
                awaiting.cycle==row.cycle,'CD callback lacks adjacent native dispatch')
            local owner=owners[slot]
            assert(row.request==owner.id and d[6]==owner.kind,'CD dispatch consumed wrong slot owner')
            if slot==10 then assert(row.request==awaiting.request,'CD DMA dispatch ancestry mismatch') end
            owners[slot]={id=zero,kind=0};entry.native_sequence=events.exact(awaiting.sequence);awaiting=nil
        else
            local parent=assert(controller.root(),'nested CD dispatch lacks root')
            assert(frame.parent~=0 and (frame.role==9 or frame.role==11) and
                row.request==parent.owner and d[6]==parent.ownerKind,
                'nested CD callback changed inherited ancestry')
            assert(previous and previous.kind==45 and previous.device==0 and previous.request==frame.id,
                'nested CD callback lacks adjacent scope')
        end
        result.dispatches[#result.dispatches+1]=entry
    end
    function tracker.push(row,payload)
        local kind,slot,d=tonumber(row.kind),tonumber(row.device),data(row)
        if readPass and readPass.cycle~=row.cycle then readPass=nil end
        -- Any other native dispatch either precedes CDR in a new pass or proves
        -- that this pass has already checked CDREAD, including non-CD devices.
        if kind==11 and slot~=2 and slot~=3 then readPass=nil end
        if previous and previous.kind==10 and slots[tonumber(previous.device)] then
            assert(kind==37 and slot==tonumber(previous.device) and d[1]==0,
                'native CD schedule lacks adjacent owner record')
        end
        if awaiting then
            if awaiting.stage==0 then
                assert(kind==45 and slot==0 and d[1]==slots[awaiting.slot] and row.related==0 and
                    row.cycle==awaiting.cycle,'native CD dispatch lacks adjacent root callback')
                awaiting.stage=1;awaiting.access=row.request
            else
                assert(kind==37 and slot==awaiting.slot and d[1]==1 and d[7]==0,
                    'native CD dispatch lacks callback ancestry')
            end
        end
        if kind==34 and slot==2 then
            assert(not scales and not context and #payload>=172,'invalid CD scheduler environment')
            scales={}
            local storage=ffi.new('uint32_t[1]')
            for n=0,14 do
                storage[0]=word(payload,112+n*4)
                local value=tonumber(ffi.cast('float*',storage)[0])
                assert(value==value and value>0 and value<=16,'invalid CD interrupt scale')
                scales[n]=value
            end
        elseif kind==24 then
            assert(not context,'native scheduler initial record after CD context')
            if slot==0xffffffff then
                assert(not fullInitial,'duplicate native scheduler initial mask')
                for n=0,14 do
                    local state=assert(native[n],'missing native scheduler initial slot')
                    assert(state.pending==bit.band(bit.rshift(d[1],n),1),'native initial slot/mask mismatch')
                end
                assert(bit.band(d[1],bit.bnot(0x7fff))==0,'unsupported native scheduler bits')
                pending=bit.band(d[1],mask);fullInitial=true
            else
                assert(slot<15 and not native[slot] and not fullInitial and d[1]<=1,
                    'duplicate or invalid native scheduler initial slot')
                native[slot]={target=row.related,pending=d[1],dma=zero}
            end
        elseif kind==34 and slot==0 then
            assert(fullInitial and scales and not context,'CD scheduler initial context is incomplete');context=true
        elseif kind==34 and slot==1 then
            assert(context and initialCount==6 and not ended and not awaiting,'CD scheduler final context is incomplete');ended=true
        elseif kind==10 and slots[slot] then
            local state=active(slot)
            pendingMatches(d[2])
            assert(wide(d[3],d[4])==state.target and wide(d[5],d[6])==state.dma,
                'native CD schedule replaced wrong target or DMA owner')
            assert(slot==10 or row.request==0,'non-DMA CD schedule carries a DMA owner')
            -- C++ multiplies float32 operands before conversion to uint64.
            local delay=tonumber(ffi.new('float',tonumber(ffi.new('float',d[1]))*scales[slot]))
            assert(row.related==row.cycle+ffi.new('uint64_t',delay),'native CD schedule target/scale mismatch')
            state.target,state.dma=row.related,row.request;pending=bit.bor(pending,bit.lshift(1,slot))
        elseif kind==11 and slots[slot] then
            local state=active(slot)
            assert(row.related==state.target and row.request==state.dma,'native CD dispatch target/DMA mismatch')
            assert(ffi.cast('int64_t',state.target-row.cycle)<=0,'native CD dispatch is not due')
            local eligible=bit.band(pending,bit.lshift(1,slot))~=0
            if slot==3 and readPass then eligible=readPass.pending;readPass=nil end
            assert(eligible,'native CD dispatch has no pending eligibility')
            if slot==2 then
                -- branchTest checks CDR before CDREAD using one pending-bit snapshot.
                -- A command callback can cancel that selected read in the same cycle.
                readPass={cycle=row.cycle,pending=bit.band(pending,8)~=0}
            elseif slot~=3 then readPass=nil end
            pending=bit.band(pending,bit.bnot(bit.lshift(1,slot)));state.dma=zero
            awaiting={slot=slot,stage=0,cycle=row.cycle,sequence=row.sequence,request=row.request}
        elseif kind==37 then
            assert(slots[slot],'unsupported CD scheduler slot')
            local phase=d[1]
            if phase==3 or phase==4 then
                assert(context and fullInitial and row.related==0 and (phase==4)==(ended==true),
                    'CD scheduler snapshot outside boundary')
                local state=assert(native[slot],'CD scheduler snapshot lacks native slot')
                pendingMatches(d[5]);assert(wide(d[3],d[4])==state.target,'CD scheduler snapshot target mismatch')
                if phase==3 then
                    assert(not initial[slot] and row.request==0 and d[6]==0,'duplicate or owned initial CD slot')
                    initial[slot]=true;initialCount=initialCount+1;owners[slot]={id=zero,kind=0}
                else
                    assert(not final[slot] and row.request==owners[slot].id and d[6]==owners[slot].kind,
                        'CD scheduler final ancestry mismatch')
                    final[slot]=true;finalCount=finalCount+1
                end
                local entry=meta(row);entry.phase=phase;entry.slot=slot;entry.owner=events.exact(row.request)
                entry.owner_kind=d[6];entry.target=events.exact(state.target);entry.pending_mask=d[5]
                result.boundaries[#result.boundaries+1]=entry
            elseif phase==0 then
                local state=active(slot);local frame=assert(controller.current(),'CD schedule lacks access')
                assert(previous and previous.kind==10 and previous.device==slot and row.cycle==previous.cycle and
                    row.related==frame.id and frame.stage==4,'CD schedule lacks adjacent native operation')
                local before=data(previous);local owner=owners[slot]
                assert(d[2]==before[1] and wide(d[3],d[4])==state.target and
                    d[5]==bit.bor(before[2],bit.lshift(1,slot)) and #payload==8 and
                    identity(payload,0)==owner.id and d[7]==owner.kind,'CD schedule replacement mismatch')
                pendingMatches(d[5]);requireOwner(row.request,d[6])
                if slot==2 then
                    assert(d[6]==1 and row.request==controller.identities().queued,'CD schedule lost queued command owner')
                elseif slot==3 then assert(d[6]==2,'CD read schedule lacks stream owner kind')
                elseif slot==10 then
                    assert(d[6]==4 and row.request==previous.request,'CD DMA schedule owner mismatch')
                end
                owners[slot]={id=row.request,kind=d[6]}
                local entry=meta(row);entry.slot=slot;entry.owner=events.exact(row.request);entry.owner_kind=d[6]
                entry.replaced=events.exact(owner.id);entry.replaced_kind=owner.kind;entry.access=events.exact(row.related)
                entry.native_sequence=events.exact(previous.sequence);entry.delay=d[2];entry.target=events.exact(state.target)
                result.schedules[#result.schedules+1]=entry
            elseif phase==1 then callback(row,d)
            elseif phase==2 then
                local state=active(slot);local owner=owners[slot]
                local frame=assert(controller.current(),'CD cancellation lacks access')
                assert(slot==3 and row.related==frame.id and frame.stage==4 and row.request==owner.id and
                    d[6]==owner.kind and wide(d[3],d[4])==state.target,'CD cancellation ancestry mismatch')
                pending=bit.band(pending,bit.bnot(bit.lshift(1,slot)));pendingMatches(d[5])
                local entry=meta(row);entry.owner=events.exact(row.request);entry.owner_kind=d[6]
                entry.access=events.exact(row.related);entry.target=events.exact(state.target)
                result.cancellations[#result.cancellations+1]=entry
            else error('unknown CD scheduler phase') end
        end
        previous=row
    end
    function tracker.owner(slot)
        local owner=assert(owners[slot],'uninitialized CD scheduler slot');return owner.id,owner.kind
    end
    function tracker.finish()
        assert(context and ended and initialCount==6 and finalCount==6 and not awaiting and
            not (previous and previous.kind==10 and slots[tonumber(previous.device)]),
            'CD scheduler capture boundaries incomplete')
        result.scope='native CD slot/target/dispatch ancestry; stream/media/Audio semantics required separately'
        return result
    end
    return tracker
end
return M
