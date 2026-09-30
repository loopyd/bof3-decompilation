-- Transfer-buffer provenance only. The CD mission separately validates controller,
-- media, stream, scope and Audio joins; this module cannot accept a whole capture.
local ffi, events, m = require 'ffi', require 'events', require 'support'
local M = {}
local function id(bytes, offset)
    assert(#bytes >= offset+8, 'short CD identity payload')
    local value=ffi.new('uint64_t',0)
    for n=7,0,-1 do value=value*256+bytes:byte(offset+n+1) end
    return value
end
function M.new(limits)
    assert(limits and limits.mutations>=1 and limits.mutations<=65536 and
        limits.mutations%1==0 and limits.bytes>=1 and limits.bytes<=65536 and
        limits.bytes%1==0, 'invalid CD buffer limits')
    local result={mutations=m.array(), reads=m.array()}
    local initial, current, final, origins, latest=nil,nil,nil,{},ffi.new('uint64_t',0)
    local last_reason
    local pending_dma
    local tracker={}
    function tracker.push(row, payload)
        local kind, phase=tonumber(row.kind),tonumber(row.device)
        if pending_dma then
            assert(kind==42 and phase==1, 'CD DMA byte lacks adjacent consumption')
        end
        if kind==4 and phase==3 then
            assert(initial and not final and row.length==1 and #payload==1 and row.data[2]==1 and
                row.data[3]==0 and row.request~=0, 'invalid CD DMA byte payload')
            pending_dma={request=row.request,address=tonumber(row.data[0]),offset=tonumber(row.data[1]),
                value=payload:byte(1)}
            return
        end
        if kind~=41 and kind~=42 then return end
        local d={}; for n=0,7 do d[n+1]=tonumber(row.data[n]) end
        assert(#payload==tonumber(row.length), 'CD buffer payload length mismatch')
        if kind==41 and phase~=1 then
            assert((phase==0 or phase==2) and d[1]==0 and d[2]==2352 and #payload==2352 and
                row.request==0 and row.related==0, 'invalid CD buffer boundary')
            if phase==0 then
                assert(not initial, 'duplicate initial CD buffer')
                initial,current=payload,payload
                for n=0,2351 do origins[n]=ffi.new('uint64_t',0) end
            else
                assert(initial and not final, 'CD final buffer lacks unique initial boundary')
                assert(payload==current, 'CD final buffer differs from mutations')
                final=payload
            end
        elseif kind==41 then
            assert(initial and not final, 'CD mutation outside buffer boundaries')
            local offset,count,reason=d[1],d[2],d[3]
            assert(offset==0 and ((reason==1 and count==8) or
                ((reason==2 or reason==3) and count==2340) or
                ((reason==4 or reason==5) and count==2352)), 'unsupported CD mutation range/reason')
            assert(#payload==32+count and row.request==latest+1 and id(payload,8)==latest,
                'CD mutation identity or payload mismatch')
            assert(id(payload,0)~=0, 'CD mutation has no controller access')
            assert((reason==5 and d[6]==2 and row.related==latest and last_reason==4) or
                (reason~=5 and d[6]==1 and row.related~=0), 'CD mutation source mismatch')
            assert(#result.mutations<limits.mutations, 'CD mutation limit exceeded')
            local bytes=payload:sub(33)
            if reason==3 then assert(bytes==string.rep('\0',count), 'CD error fill is not zero') end
            current=current:sub(1,offset)..bytes..current:sub(offset+count+1)
            for n=offset,offset+count-1 do origins[n]=row.request end
            latest=row.request
            last_reason=reason
            result.mutations[#result.mutations+1]={id=events.exact(row.request),source=events.exact(row.related),
                access=events.exact(id(payload,0)),read_stream=events.exact(id(payload,16)),
                play_stream=events.exact(id(payload,24)),offset=offset,bytes=bytes,reason=reason,
                sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
        else
            assert(initial and not final and (phase==0 or phase==1), 'CD data outside buffer boundaries')
            assert(#payload==8 and row.related~=0 and d[4]<=1 and d[6]<=1 and d[7]<=1,
                'invalid CD data metadata')
            assert(#result.reads<limits.bytes, 'CD byte limit exceeded')
            local dma=id(payload,0)
            assert((phase==1 and dma~=0) or (phase==0 and dma==0 and d[2]==0x1f801802),
                'CD data owner/address mismatch')
            if phase==1 then
                assert(pending_dma and pending_dma.request==dma and pending_dma.address==d[2] and
                    pending_dma.offset==d[1] and pending_dma.value==d[3], 'CD DMA consumption mismatch')
                pending_dma=nil
            end
            if phase==1 or d[4]==1 then
                assert(d[1]<2352 and row.request==origins[d[1]] and d[3]==current:byte(d[1]+1),
                    'CD byte origin/value mismatch')
                local distance=d[1]+1-d[5]
                assert((distance==0 or distance==2060 or distance==2340) and
                    d[7]==(distance==0 and 0 or 1) and d[6]==(d[5]==0 and 0 or d[4]),
                    'CD byte cursor/wrap mismatch')
            else
                assert(row.request==0 and d[3]==0 and d[5]==d[1] and d[6]==0 and d[7]==0,
                    'not-ready CD PIO fabricated data')
            end
            result.reads[#result.reads+1]={mutation=events.exact(row.request),access=events.exact(row.related),
                dma=events.exact(dma),offset=d[1],address=d[2],value=d[3],wrapped=d[7]==1,
                sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
        end
    end
    function tracker.header()
        assert(initial and not final, 'CD buffer header unavailable outside capture')
        return current:sub(1,8),latest
    end
    function tracker.finish()
        assert(initial and final and not pending_dma, 'CD buffer boundaries incomplete')
        result.initial,result.final=initial,final
        return result
    end
    return tracker
end
return M
