-- Correlate frozen native GPU buffers with structural packet decoding.
local m, events, packets = require 'support', require 'events', require 'packets'
local M = {}
local function u32(value)
    return string.char(value%256, math.floor(value/256)%256,
        math.floor(value/65536)%256, math.floor(value/16777216)%256)
end
local function values(record)
    local data={}; for n=0,7 do data[n+1]=tonumber(record.data[n]) end; return data
end

function M.requireBinding(h)
    assert(h.status().state == 0, 'GPU history recorder must be empty')
    h.begin(128, 0); h.stop()
    local status, phases=h.status(),{}
    assert(status.failures == 0, 'GPU history capability probe failed')
    for n=0,tonumber(status.events)-1 do
        local row=h.record(n)
        if row.kind == 30 then phases[tonumber(row.device)]=true end
    end
    assert(phases[0] and phases[1], 'native GPU history binding required')
    h.clear()
end

function M.scan(h, limits)
    local status=h.status()
    assert(status.version==1 and status.state==2 and status.failures==0, 'GPU history must be complete and frozen')
    assert(status.events<=65536 and status.bytes<=8388608, 'GPU history exceeds native bounds')
    local result={buffers=m.array(), packets=m.array(), reads=m.array(), contexts=m.array(), environments=m.array(),
        headers=m.array(), terminals=m.array()}
    local contexts, environments, active, last_id, header={}, {}, nil, nil, nil
    local chunks, pending, spans, size, pending_size, words, count={}, {}, {}, 0, 0, 0, 0
    local previous, payload=status.beginCycle,0
    local function decode(bytes, port, sources)
        local decoded=packets[port](bytes, limits)
        assert(count+#decoded<=limits.packets, 'GPU capture packet limit exceeded')
        local cursor=1
        for _, packet in ipairs(decoded) do
            local first, last=packet.offset,packet.offset+packet.words*4
            local used=m.array()
            while sources[cursor] and sources[cursor].offset+sources[cursor].bytes<=first do cursor=cursor+1 end
            local index=cursor
            while sources[index] and sources[index].offset<last do
                local span=sources[index]
                if first<span.offset+span.bytes and last>span.offset then
                    used[#used+1]={buffer=span.buffer, request=span.request,
                        first_byte=math.max(first,span.offset)-span.offset,
                        bytes=math.min(last,span.offset+span.bytes)-math.max(first,span.offset),
                        input_sequence=span.input_sequence, result_sequence=span.result_sequence,
                        input_cycle=span.input_cycle, result_cycle=span.result_cycle}
                end
                index=index+1
            end
            assert(#used>0, 'decoded packet has no native buffer')
            packet.port,packet.sources=port,used
            packet.completion_sequence=used[#used].result_sequence
            packet.ordinal=#result.packets+1
            packet.offset=port=='gp0' and size-#bytes+packet.offset or nil
            result.packets[#result.packets+1]=packet
        end
        count=count+#decoded
    end
    for n=0,tonumber(status.events)-1 do
        local row=h.record(n); local kind=tonumber(row.kind); local d=values(row)
        assert(row.sequence==n+1 and row.cycle>=previous and row.cycle<=status.endCycle,
            'GPU history sequence/cycle mismatch')
        assert(row.offset==payload and payload+row.length<=status.bytes, 'GPU history payload mismatch')
        payload=payload+tonumber(row.length); previous=row.cycle
        if kind==30 or kind==31 then
            local phase=tonumber(row.device)
            assert((phase==0 or phase==1) and not active, 'GPU context inside a buffer')
            local seen=kind==30 and contexts or environments
            assert(not seen[phase] and (phase==0 or seen[0]), 'GPU context phase mismatch')
            seen[phase]=true
            local list=kind==30 and result.contexts or result.environments
            list[#list+1]={phase=phase,sequence=events.exact(row.sequence),cycle=events.exact(row.cycle),raw=m.array(d)}
            if kind==30 then
                assert(d[6]==1 and d[1]==0, 'GPU capture boundary has incomplete parser state')
                assert(phase==0 or #pending==0, 'GPU capture ends with an incomplete packet')
            end
        elseif kind==5 and row.device==2 then
            assert(not header and not active and d[3]==math.floor(d[2]/16777216), 'invalid GPU chain header')
            header={address=d[1],raw=d[2],words=d[3],request=events.exact(row.request),
                sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
            result.headers[#result.headers+1]=header
        elseif kind==21 and row.device==2 then
            assert(not header and not active, 'GPU chain ended inside a buffer')
            result.terminals[#result.terminals+1]={address=d[1],request=events.exact(row.request),
                sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
        elseif kind==32 then
            assert(contexts[0] and environments[0] and not contexts[1] and not active,
                'GPU input lacks initial context or overlaps another buffer')
            assert(row.device==2 and row.length==0 and d[1]<=5 and d[8]<=1,
                'unsupported GPU input source or boundary')
            assert(row.related~=0 and (not last_id or row.related>last_id), 'GPU buffer identity is not increasing')
            last_id=row.related
            active={buffer=events.exact(row.related),request=events.exact(row.request),source=d[1],origin=d[2],
                words=d[3],value=d[4],before_processor=d[5],before_fifo_bytes=d[6],pc=d[7],before_ready=d[8],
                input_sequence=events.exact(row.sequence),input_cycle=events.exact(row.cycle),raw=d}
            if active.source==3 then
                assert(header and header.address==active.origin and header.words==active.words and
                    header.request==active.request, 'GPU chain buffer lacks its visited header')
                active.header_sequence=header.sequence; header=nil
            else assert(not header, 'GPU chain header has no matching buffer') end
        elseif kind==4 and row.device==2 then
            assert(active and (active.source==2 or active.source==3 or active.source==4) and not active.payload,
                'GPU DMA payload lacks a unique buffer')
            assert(events.exact(row.request)==active.request and d[3]==row.length and
                row.length==active.words*4 and d[1]==active.origin+(active.source==3 and 4 or 0) and
                d[4]==(active.source==4 and 0 or 1), 'GPU DMA payload identity/count mismatch')
            active.payload={offset=tonumber(row.offset),bytes=tonumber(row.length)}
        elseif kind==33 then
            assert(active and events.exact(row.related)==active.buffer and events.exact(row.request)==active.request,
                'GPU result lacks matching input')
            assert(row.device==2 and row.length==0 and d[8]<=1, 'invalid GPU result boundary')
            for _,i in ipairs({1,2,3,7}) do assert(d[i]==active.raw[i], 'GPU result identity mismatch') end
            if active.source~=5 then assert(d[4]==active.value, 'GPU result changed its input operand') end
            active.result_sequence,active.result_cycle=events.exact(row.sequence),events.exact(row.cycle)
            active.after_processor,active.after_fifo_bytes,active.after_ready=d[5],d[6],d[8]
            local source=active.source
            if source==2 or source==3 or source==4 then assert(active.payload, 'GPU DMA buffer has no retained payload')
            else assert(active.words==1, 'CPU GPU buffer must contain one word') end
            if source==0 or source==2 or source==3 then
                assert((#pending>0) == (active.before_ready==0), 'GPU packet context was lost between buffers')
                local bytes=source==0 and u32(active.value) or h.bytes(active.payload.offset,active.payload.bytes)
                assert(#bytes==active.words*4, 'short GPU payload')
                words=words+active.words; assert(words<=limits.words, 'GPU capture word limit exceeded')
                if #bytes>0 then
                    pending[#pending+1]=bytes; chunks[#chunks+1]=bytes; size=size+#bytes
                    spans[#spans+1]={offset=pending_size,bytes=#bytes,buffer=active.buffer,request=active.request,
                        input_sequence=active.input_sequence,result_sequence=active.result_sequence,
                        input_cycle=active.input_cycle,result_cycle=active.result_cycle}
                    pending_size=pending_size+#bytes
                end
                if active.after_ready==1 and #pending>0 then
                    decode(table.concat(pending),'gp0',spans); pending,spans,pending_size={},{},0
                end
            elseif source==1 then
                words=words+1; assert(words<=limits.words, 'GPU capture word limit exceeded')
                assert(not (#pending>0 and math.floor(active.value/16777216)==0),
                    'GP1 reset interrupted a partial GP0 packet')
                decode(u32(active.value),'gp1',{{offset=0,bytes=4,buffer=active.buffer,request=active.request,
                    input_sequence=active.input_sequence,result_sequence=active.result_sequence,
                    input_cycle=active.input_cycle,result_cycle=active.result_cycle}})
            else
                result.reads[#result.reads+1]={buffer=active.buffer,source=source,value=source==5 and d[4] or nil,
                    payload=active.payload,input_cycle=active.input_cycle,result_cycle=active.result_cycle}
            end
            active.raw=nil; result.buffers[#result.buffers+1]=active; active=nil
        end
    end
    assert(not active and not header and #pending==0 and contexts[0] and contexts[1] and environments[0] and environments[1],
        'GPU capture lacks complete context/buffer boundaries')
    assert(payload==status.bytes, 'GPU capture payload was not fully accounted')
    -- A complete prefix may share a native buffer with an unfinished packet.
    -- Decoding that group later must not place intervening GP1 ahead of its prefix.
    table.sort(result.packets,function(a,b)
        local left,right=tonumber(a.completion_sequence),tonumber(b.completion_sequence)
        if left==right then return a.ordinal<b.ordinal end
        return left<right -- Sequence numbers are bounded by the 65536-record ABI.
    end)
    for _,packet in ipairs(result.packets) do packet.ordinal=nil end
    result.words,result.packet_count,result.bytes=words,count,table.concat(chunks)
    result.order='packet completion order; source sequences preserve interleaved GP1 and split GP0 inputs'
    result.timing='observed native buffer boundaries, not per-word bus timing or rasterization cycles'
    return result
end

function M.export(h, limits)
    local ok,result=pcall(M.scan,h,limits)
    if not ok then
        m.report('ingress.json',{schema='psx.runtime-gpu-ingress/v1',complete=false,error=tostring(result)})
        return {complete=false,error=tostring(result)}
    end
    m.write('commands.bin',result.bytes); result.bytes=nil
    result.schema,result.complete='psx.runtime-gpu-ingress/v1',true
    assert(#m.json(result)<=16777216, 'GPU decoded export exceeds 16 MiB')
    m.report('ingress.json',result)
    return result
end
return M
