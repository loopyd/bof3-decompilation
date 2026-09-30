-- Controller scopes, shared parameters and response publication. This component
-- does not accept media/stream/Audio relationships or a whole CD capture.
local ffi, bit, m, events = require 'ffi', require 'bit', require 'support', require 'events'
local M = {}
local function wide(low, high) return ffi.new('uint64_t',high)*4294967296+low end
local function word(bytes, offset)
    assert(offset>=0 and #bytes>=offset+4, 'short CD controller payload')
    local a,b,c,d=bytes:byte(offset+1,offset+4);return a+b*256+c*65536+d*16777216
end
local function identity(bytes,offset) return wide(word(bytes,offset),word(bytes,offset+4)) end
local function hex(bytes) return (bytes:gsub('.',function(c) return string.format('%02x',c:byte()) end)) end
local function data(row)
    local result={};for n=0,7 do result[n+1]=tonumber(row.data[n]) end;return result
end
local function equal(a,b,indices,message)
    for _,i in ipairs(indices) do assert(a[i]==b[i],message) end
end
local indices={1,2,3,4,5,6,7,8}
local slots={[5]=2,[6]=3,[7]=14,[9]=13,[11]=10,[12]=12}
local nesting={[1]=3,[2]=4,[8]=9,[10]=11}
local zero=ffi.new('uint64_t',0)

function M.new(status)
    assert(status.version==1 and status.state==2 and status.failures==0 and status.dropped==0 and
        status.events>=1 and status.events<=65536 and status.bytes<=8388608 and
        status.beginCycle<=status.endCycle, 'CD controller requires complete frozen history')
    local result={accesses=m.array(),commands=m.array(),responses=m.array(),irqs=m.array(),contexts=m.array()}
    local stack, commands={}, {}
    local count, offset, previous=0,0,status.beginCycle
    local serial, responseSerial, latest, queued, published=zero,zero,zero,zero,zero
    local first, final, environment, pair, builder
    local state, mode, parameters, responseBytes
    local dma=zero
    local tracker={}
    local function current()
        return assert(stack[#stack], 'CD controller record has no enclosing access')
    end
    local function root() return assert(stack[1], 'CD controller record has no operation') end
    local function claim(id)
        assert(id==serial+1, 'CD access/media/feed identity is not contiguous');serial=id
    end
    local function command(id)
        assert(id==0 or commands[events.exact(id)], 'unknown CD command owner')
    end
    local function metadata(row)
        return {sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
    end
    local function context(row,payload,d)
        local phase=tonumber(row.device)
        assert(d[1]==2, 'invalid CD controller context version')
        if phase==2 then
            assert(#stack==0 and not builder and row.request==0 and row.related==0 and
                not environment and not first and d[3]==14 and d[4]==15 and
                d[5]<=1024 and d[6]<=1024 and d[7]<=1 and d[8]<=1 and #payload==172+d[5]+d[6],
                'invalid CD controller environment')
            environment={raw=m.array(d),payload={offset=tonumber(row.offset),bytes=#payload}};return
        end
        assert((phase==0 or phase==1 or phase==3 or phase==4) and #payload==256 and d[2]<=1 and d[7]==56 and d[8]==0,
            'invalid CD controller context layout')
        local words={};for n=0,55 do words[n+1]=word(payload,n*4) end
        local snapshot={words[1],words[2],words[4],words[9],words[10],words[11],words[12],words[13]}
        local extended={words[18],words[19],words[20],words[21],words[23],words[34],words[35],
            words[14]+words[15]*256+words[16]*65536}
        local record=metadata(row);record.phase=phase;record.words=m.array(words);record.raw=m.array(d)
        local params,contents=payload:sub(225,232),payload:sub(233,248)
        record.parameters_hex,record.response_hex=hex(params),hex(contents)
        record.subq_hex=hex(payload:sub(249,256))
        assert(words[49]<=255 and words[50]<=1 and words[51]<=255 and
            (words[52]==254 or words[52]==255) and words[53]<=3 and
            (words[52]==254)==(words[53]==0) and (words[52]==255 or payload:byte(249)==0) and
            words[55]<=255 and words[56]<=255, 'invalid CD callback provenance/state')
        assert(snapshot[4]<=8 and snapshot[5]<=16 and snapshot[6]<=255 and snapshot[7]<=1 and
            snapshot[8]<2352, 'invalid CD context counters')
        if phase>=3 then
            local frame=current()
            assert(first and not final and row.request==frame.id and row.related==root().id,
                'CD callback context has wrong owner')
            equal(d,first.raw,indices,'CD callback epochs/profile changed')
            local before=phase==3
            assert(frame.stage==(before and 4 or 5) and not frame[before and 'before_context' or 'after_context'],
                'CD callback context is unpaired')
            equal(snapshot,before and frame.before or frame.after,indices,'CD callback basic state mismatch')
            equal(extended,before and frame.before_mode or frame.after_mode,indices,'CD callback mode state mismatch')
            local expected=parameters
            if not before and frame.role==4 and frame.raw[2]==0x1f801802 and frame.before[1]%4==0 and frame.before[4]<8 then
                local n=frame.before[4];expected=expected:sub(1,n)..string.char(frame.raw[4])..expected:sub(n+2)
            end
            assert(params==expected, 'CD callback parameter bytes mismatch')
            assert(wide(words[37],words[38])==latest and wide(words[39],words[40])==queued and
                wide(words[45],words[46])==published, 'CD callback command/response identity mismatch')
            if before then
                if not root().response_dirty then
                    assert(contents==responseBytes, 'CD callback response continuity mismatch')
                end
            elseif frame.publication then
                assert(contents:sub(1,frame.publication[1])==responseBytes,
                    'CD callback response publication bytes mismatch')
            elseif not frame.response_dirty then
                assert(hex(contents)==frame.before_context.response_hex, 'CD callback changed response bytes')
            end
            if not before then responseBytes=contents end
            frame[before and 'before_context' or 'after_context']=record
            return
        end
        assert(#stack==0 and not builder and row.request==0 and row.related==0,
            'invalid CD controller context boundary')
        result.contexts[#result.contexts+1]=record
        if phase==0 then
            assert(environment and not first and not final and wide(d[3],d[4])~=0,
                'missing environment or duplicate CD initial context')
            assert((d[2]==0 and wide(d[5],d[6])==0) or (d[2]==1 and wide(d[5],d[6])~=0),
                'CD Audio join epoch mismatch')
            for n=37,48 do assert(words[n]==0, 'CD initial context invented owner identity') end
            first=record;state,mode=snapshot,extended;parameters,responseBytes=params,contents
        else
            assert(first and not final, 'duplicate or unpaired CD final context')
            equal(d,first.raw,indices,'CD capture epochs/profile changed')
            equal(snapshot,state,indices,'CD final controller state mismatch')
            equal(extended,mode,indices,'CD final mode/filter state mismatch')
            assert(params==parameters, 'CD final parameter bytes mismatch')
            assert(wide(words[37],words[38])==latest and wide(words[39],words[40])==queued and
                wide(words[45],words[46])==published, 'CD final command/response identity mismatch')
            assert(contents==responseBytes,
                'CD final response bytes mismatch')
            final=record
        end
    end
    local function scope(row,d)
        local role,phase=d[1],tonumber(row.device)
        assert(first and not final and phase<=1 and d[7]>=1 and d[7]<=16 and d[8]==0,
            'CD access outside valid context')
        assert((role<=4)==(row.kind==35), 'CD access kind/role mismatch')
        if phase==0 then
            claim(row.request)
            local parent=stack[#stack]
            assert(d[6]==0 and d[7]==#stack+1 and row.related==(parent and parent.id or zero),
                'CD access parent/depth mismatch')
            assert(role>=1 and role<=12 and (not parent or nesting[parent.role]==role) and
                (parent or (role~=3 and role~=4)), 'unsupported CD access nesting')
            if role<=4 then
                assert(d[3]==8 and bit.band(d[2],0x1fffffff)>=0x1f801800 and
                    bit.band(d[2],0x1fffffff)<=0x1f801803, 'invalid CD port access')
                if parent then
                    assert(d[2]==bit.band(parent.raw[2],0x1fffffff) and d[3]==parent.raw[3] and
                        d[5]==parent.raw[5] and (role==3 or d[4]==parent.raw[4]%256), 'CD handler/CPU access mismatch')
                end
            end
            local frame={id=row.request,role=role,raw=d,parent=row.related,children=0,
                enter=metadata(row),owner=parent and parent.owner or zero,ownerKind=parent and parent.ownerKind or 0}
            if role==10 or role==11 then frame.owner,frame.ownerKind=dma,4 end
            if parent then parent.children=parent.children+1 end
            stack[#stack+1]=frame
        else
            local frame=current()
            assert(frame.id==row.request and row.related==frame.parent and d[7]==#stack and d[1]==frame.role,
                'unmatched CD access exit')
            equal(d,frame.raw,{1,2,3,5,7,8},'CD access exit changed identity')
            local read=role==1 or role==3
            assert(d[6]==(read and 1 or 0) and (read or d[4]==frame.raw[4]), 'CD access return/operand mismatch')
            if role<=2 then
                assert(frame.children==1 and frame.child_return==(read and d[4] or d[4]%256),
                    'CD CPU access lacks its matching handler')
            else
                assert(frame.stage==5 and frame.before_context and frame.after_context and
                    (not slots[role] or frame.dispatch), 'CD access lacks state/dispatch boundaries')
                assert(role~=5 or frame.executing, 'CD command callback entry is missing')
                if role==3 and (d[2]==0x1f801801 or d[2]==0x1f801802) then
                    assert(frame.returned~=nil, 'CD read handler lacks its FIFO/data observation')
                end
                if role==4 and d[2]==0x1f801801 and frame.before[1]%4==0 then
                    assert(frame.submitted, 'CD command handler lacks its submission')
                end
                if role==4 and frame.before[1]%4==1 and (d[2]==0x1f801802 or d[2]==0x1f801803) then
                    assert(frame.irq_write and (d[2]~=0x1f801802 or frame.irq_assert), 'CD IRQ handler lacks its observations')
                end
                if frame.returned~=nil then assert(frame.returned==d[4], 'CD handler returned a different byte') end
                if frame.consumption then
                    assert(frame.after[8]==frame.consumption.index and
                        frame.after_mode[8]%256==frame.consumption.ready,
                        'CD consumed data differs from final controller cursor/readiness')
                    if frame.empty then assert(bit.band(frame.after[1],64)==0, 'CD empty FIFO retained DRQ') end
                end
                if frame.cursor~=nil then
                    assert(frame.after[6]==frame.cursor and frame.after[7]==frame.ready and frame.after[5]==frame.before[5],
                        'CD response FIFO result-state mismatch')
                end
                if frame.publication then
                    local p=frame.publication
                    assert(frame.after[5]==p[1] and frame.after[6]==p[4] and frame.after[7]==p[3] and frame.after[2]==p[5],
                        'CD response publication/result-state mismatch')
                end
                if frame.mask then assert(frame.after[3]==frame.mask, 'CD IRQ mask result-state mismatch') end
                if frame.ack then assert(frame.after[2]==frame.ack, 'CD IRQ acknowledgment result-state mismatch') end
                if role==4 and frame.raw[2]==0x1f801803 and frame.before[1]%4==0 then
                    local ready,cursor=frame.before_mode[8]%256,frame.before[8]
                    if bit.band(frame.raw[4],128)~=0 and ready==0 then
                        ready=1
                        local size=bit.band(frame.before_mode[1],48)
                        cursor=(size==0 or size==16) and 12 or 0
                    end
                    assert(frame.after[8]==cursor and frame.after_mode[8]==frame.before_mode[8]-
                        frame.before_mode[8]%256+ready, 'CD transfer-enable cursor/readiness mismatch')
                    equal(frame.after,frame.before,{1,2,3,4,5,6,7},'CD transfer-enable changed unrelated state')
                    equal(frame.after_mode,frame.before_mode,{1,2,3,4,5,6,7},'CD transfer-enable changed mode')
                end
            end
            assert(not builder or builder.operation~=frame.id, 'CD response builder left unpublished')
            if role==4 and frame.raw[2]==0x1f801802 and frame.before[1]%4==0 and frame.before[4]<8 then
                local n=frame.before[4];parameters=parameters:sub(1,n)..string.char(frame.raw[4])..parameters:sub(n+2)
            end
            frame.exit=metadata(row);frame.value=d[4]
            table.remove(stack)
            local parent=stack[#stack]
            if parent and parent.role<=2 then parent.child_return=d[4] end
            result.accesses[#result.accesses+1]={id=events.exact(frame.id),parent=events.exact(frame.parent),role=role,
                owner=events.exact(frame.owner),owner_kind=frame.ownerKind,enter=frame.enter,exit=frame.exit,
                raw=m.array(frame.raw),value=d[4],before=frame.before and m.array(frame.before),
                after=frame.after and m.array(frame.after),before_mode=frame.before_mode and m.array(frame.before_mode),
                after_mode=frame.after_mode and m.array(frame.after_mode),
                before_context=frame.before_context,after_context=frame.after_context}
        end
    end
    local function transition(row,d)
        local frame=current();local phase=tonumber(row.device)
        assert(frame.role>=3 and row.request==frame.id and row.related==root().id,
            'CD state has wrong access owner')
        if phase==2 or phase==3 then
            assert(d[4]<=8 and d[5]<=16 and d[6]<=255 and d[7]<=1 and d[8]<2352, 'invalid CD controller counters')
        end
        if phase==2 then
            assert(not frame.stage, 'duplicate CD before-state')
            assert(not slots[frame.role] or frame.dispatch, 'CD state precedes callback dispatch')
            if #stack==1 or stack[1].role<=2 then equal(d,state,indices,'CD state changed outside an observed operation') end
            frame.before=d;frame.stage=2;pair={phase=4,id=frame.id,cycle=row.cycle}
        elseif phase==4 then
            assert(frame.stage==2, 'CD mode before-state is unpaired')
            if #stack==1 or stack[1].role<=2 then equal(d,mode,indices,'CD mode changed outside an observed operation') end
            frame.before_mode=d;frame.stage=4;pair={kind=34,phase=3,id=frame.id,cycle=row.cycle}
        elseif phase==3 then
            assert(frame.stage==4, 'CD after-state is unpaired')
            frame.after=d;frame.stage=3;state=d;pair={phase=5,id=frame.id,cycle=row.cycle}
        elseif phase==5 then
            assert(frame.stage==3, 'CD mode after-state is unpaired')
            frame.after_mode=d;frame.stage=5;mode=d;pair={kind=34,phase=4,id=frame.id,cycle=row.cycle}
        else error('unknown CD state phase') end
    end
    local function commandsRow(row,payload,d)
        local frame,operation=current(),root();local phase=tonumber(row.device)
        assert(frame.stage==4, 'CD command outside controller body')
        if phase==0 then
            assert(frame.role==4 and frame.raw[2]==0x1f801801 and frame.before[1]%4==0 and
                row.request==frame.id and row.related==operation.id and not commands[events.exact(row.request)] and
                d[1]==frame.raw[4] and d[2]==frame.before[4] and payload==parameters, 'CD command submission mismatch')
            latest=row.request;frame.submitted=true
            local entry=metadata(row);entry.id=events.exact(latest);entry.opcode=d[1];entry.parameters_hex=hex(payload)
            entry.parameter_count=d[2];entry.executions=m.array();commands[entry.id]=entry
            result.commands[#result.commands+1]=entry
        elseif phase==1 then
            assert(frame.role==5 and frame.dispatch and not frame.executing and row.related==operation.id and
                row.request==frame.owner and payload==parameters and d[3]==frame.before[4],
                'CD command callback owner/parameters mismatch')
            command(row.request);frame.executing=true
            local entry=metadata(row);entry.parameters_hex=hex(payload);entry.raw=m.array(d)
            if row.request~=0 then
                local target=commands[events.exact(row.request)];target.executions[#target.executions+1]=entry
            else result.prehistory_executions=result.prehistory_executions or m.array()
                result.prehistory_executions[#result.prehistory_executions+1]=entry end
        elseif phase==2 then
            assert(row.related==frame.id and wide(d[7],d[8])==queued, 'CD command replaced wrong queue owner')
            local repeated=d[1]~=0 and (d[4]==d[1] or d[4]+256==d[1])
            assert(d[6]==(repeated and 1 or 0), 'CD repeated-command flag mismatch')
            local owner=repeated and queued or (operation.role==5 and operation.owner or latest)
            assert(row.request==owner, 'CD queued command owner mismatch');command(owner);queued=owner
        else error('unknown CD command phase') end
    end
    local function response(row,payload,d)
        local frame,operation=current(),root();local phase=tonumber(row.device)
        if phase==0 then
            assert(frame.stage==4 and row.related==operation.id and d[1]<=16, 'CD response construction owner/range mismatch')
            for _,scope in ipairs(stack) do scope.response_dirty=true end
            if builder then
                assert(row.request==builder.id and builder.operation==operation.id and d[2]==builder.revision+1 and
                    wide(d[3],d[4])==0, 'CD response revision mismatch')
            else
                assert(row.request==responseSerial+1 and d[2]==1 and wide(d[3],d[4])==published,
                    'CD response creation/displacement mismatch')
                responseSerial=row.request;published=zero
                builder={id=row.request,operation=operation.id,creation=metadata(row),displaced=events.exact(wide(d[3],d[4]))}
            end
            builder.revision,builder.size=d[2],d[1]
        elseif phase==1 then
            assert(builder and row.request==builder.id and row.related==builder.operation and operation.id==builder.operation and
                #stack==1 and frame.stage==4 and d[1]==builder.size and d[2]==builder.revision and #payload==builder.size,
                'CD response publication mismatch')
            local entry=metadata(row);entry.id=events.exact(row.request);entry.operation=events.exact(row.related)
            entry.revision,entry.data_hex,entry.byte_count,entry.irq=d[2],hex(payload),#payload,d[5];entry.creation=builder.creation
            entry.displaced=builder.displaced;result.responses[#result.responses+1]=entry
            published=row.request;responseBytes=payload;builder=nil;frame.publication=d
        elseif phase==2 then
            assert(frame.stage==4 and frame.returned==nil and frame.role==3 and frame.raw[2]==0x1f801801 and row.related==frame.id and row.request==published and
                d[1]==frame.before[6] and d[2]==d[1]%16 and d[3]==frame.before[5] and d[4]==frame.before[7] and
                d[5]==(d[2]<d[3] and 1 or 0), 'CD response FIFO identity/cursor mismatch')
            local value=d[5]==1 and assert(responseBytes:byte(d[2]+1),'CD response bytes unavailable') or 0
            assert(d[6]==value, 'CD response FIFO byte mismatch');frame.returned=value
            frame.cursor=(d[1]+1)%256;frame.ready=frame.cursor==d[3] and 0 or d[4]
        else error('unknown CD response phase') end
    end
    function tracker.push(row,payload)
        count=count+1
        assert(count<=status.events and row.sequence==count and row.cycle>=previous and row.cycle<=status.endCycle and
            row.offset==offset and #payload==tonumber(row.length) and offset+#payload<=status.bytes,
            'CD journal sequence/cycle/payload mismatch')
        previous,offset=row.cycle,offset+#payload
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        if pair then
            assert(kind==(pair.kind or 45) and phase==pair.phase and row.request==pair.id and
                row.cycle==pair.cycle, 'CD controller state pair was interrupted');pair=nil
        end
        if kind==34 then context(row,payload,d)
        elseif kind==46 then
            local frame=current()
            assert(frame.stage==4 and frame.before_context and row.request==frame.id and row.related==root().id,
                'CD predicate has wrong access owner')
        elseif kind==35 or (kind==45 and phase<=1) then scope(row,d)
        elseif kind==45 then transition(row,d)
        elseif kind==36 then commandsRow(row,payload,d)
        elseif kind==38 then response(row,payload,d)
        elseif kind==39 then
            local frame=current();assert(frame.stage==4 and row.related==frame.id, 'CD IRQ has wrong access owner')
            if phase==0 then
                assert(row.request==(builder and builder.id or published) and d[4]==(builder and 1 or 0) and
                    d[3]==(bit.band(d[1],d[2])~=0 and 1 or 0), 'CD IRQ response/assertion mismatch')
                frame.irq_assert=true
            else
                assert(not frame.irq_write and row.request==0 and frame.role==4 and ((phase==1 and frame.raw[2]==0x1f801802) or
                    (phase==2 and frame.raw[2]==0x1f801803)), 'CD IRQ write owner mismatch')
                assert(frame.before[1]%4==1, 'CD IRQ write used wrong register index')
                assert(d[1]==frame.before[2], 'CD IRQ write changed prior status');frame.irq_write=true
                if phase==1 then
                    assert(d[2]==frame.before[3] and d[3]==frame.raw[4], 'CD IRQ mask write mismatch');frame.mask=d[3]
                else
                    assert(d[2]==frame.raw[4] and d[3]==bit.band(d[1],bit.bnot(d[2])) and d[4]==frame.before[3],
                        'CD IRQ acknowledgment mismatch');frame.ack=d[3]
                end
            end
            local entry=metadata(row);entry.phase=phase;entry.raw=m.array(d);entry.response=events.exact(row.request)
            entry.access=events.exact(row.related);result.irqs[#result.irqs+1]=entry
        elseif kind==37 and d[1]==1 then
            local frame=current();assert(not frame.stage and not frame.dispatch and row.related==frame.id and d[2]==frame.role and
                phase==slots[frame.role] and d[7]==(#stack>1 and 1 or 0), 'CD callback dispatch mismatch')
            frame.dispatch=true;frame.owner,frame.ownerKind=row.request,d[6]
        elseif kind==37 then
            if d[1]==0 or d[1]==2 then
                assert(row.related==current().id and current().stage==4, 'CD schedule has wrong access owner')
            else
                assert(#stack==0 and row.related==0 and ((d[1]==3 and first and not final) or (d[1]==4 and final)),
                    'CD scheduler snapshot outside boundary')
            end
        elseif kind==40 then
            assert(row.related==current().id and current().stage==4, 'CD media has wrong access owner')
            if phase==0 then claim(row.request) end
        elseif kind==41 and phase==1 then
            assert(identity(payload,0)==current().id and current().stage==4, 'CD mutation has wrong access owner')
        elseif kind==42 then
            local frame=current()
            assert(row.related==frame.id and frame.stage==4, 'CD data has wrong access owner')
            local before=frame.consumption or {index=frame.before[8],ready=frame.before_mode[8]%256}
            assert(d[1]==before.index and d[4]==before.ready,
                'CD consumption differs from controller cursor/readiness')
            local cursor,ready,wrapped=before.index,before.ready,0
            if phase==1 or ready~=0 then
                local size=bit.band(frame.before_mode[1],48)
                local limit=(size==16 or size==32) and 2340 or 2060
                cursor=cursor+1
                if cursor>=limit then cursor=cursor-limit;wrapped=1 end
                if cursor==0 then ready=0 end
            end
            assert(d[5]==cursor and d[6]==ready and d[7]==wrapped,
                'CD consumption differs from native mode wrap/readiness')
            if (phase==0 and before.ready==0) or cursor==0 then frame.empty=true end
            frame.consumption={index=d[5],ready=d[6]}
            if phase==0 then
                assert(frame.returned==nil and frame.role==3 and frame.raw[2]==0x1f801802,
                    'CD PIO data lacks a unique read handler');frame.returned=d[3]
            else
                assert(phase==1 and frame.role==10 and identity(payload,0)==frame.owner, 'CD DMA data owner mismatch')
            end
        elseif kind==43 then
            local access=phase==0 and row.related or identity(payload,phase==1 and 8 or 0)
            assert(access==current().id and current().stage==4, 'CD feed/decision has wrong access owner')
            if phase==0 then claim(row.request) end
        elseif kind==44 then
            assert(identity(payload,8)==current().id and current().stage==4, 'CD stream has wrong access owner')
        elseif kind==3 and phase==3 then dma=row.request
        elseif kind==7 and phase==3 and d[1]==1 then dma=zero
        end
    end
    function tracker.current() return stack[#stack] end
    function tracker.root() return stack[1] end
    function tracker.command(id) command(id);return commands[events.exact(id)] end
    function tracker.identities() return {latest=latest,queued=queued,published=published} end
    function tracker.finish()
        assert(count==status.events and offset==status.bytes and first and final and environment and
            #stack==0 and not builder and not pair, 'CD controller capture boundaries incomplete')
        result.environment=environment
        result.scope='controller identities and response publication; media/stream/Audio correlation required separately'
        return result
    end
    return tracker
end
return M
