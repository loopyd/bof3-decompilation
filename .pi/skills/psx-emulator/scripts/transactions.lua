-- Compose CD journal consumers. Correlation validates observed native records;
-- mission completion, media conformance and audible fidelity remain separate.
local m, events, image = require 'support', require 'events', require 'image'
local M = {}

local function reference(row, skip)
    return {file='payload.bin',offset=tonumber(row.offset)+skip,bytes=tonumber(row.length)-skip,
        sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)}
end

function M.scan(h, limits, audio, bindings)
    local status=h.status()
    local controller=require('controller').new(status)
    local scheduler=require('scheduler').new(controller)
    local sectors=require('sectors').new(limits)
    local images,geometry,previous={},nil,nil
    -- Every row reaches each tracker, including unrelated device events which
    -- can invalidate adjacency or scheduler-pass assumptions.
    local consumers={controller,scheduler,require('streams').new(controller,scheduler),
        require('media').new(controller),sectors,require('feeds').new(controller,audio),
        require('eligibility').new(controller,sectors),require('delivery').new(controller),
        require('transfers').new(controller),
        require('command').new(controller,sectors,function() return assert(images[0]).descriptor end),
        require('submission').new(controller),
        require('playback').new(controller,function() return assert(images[0]).descriptor end),
        require('decoded').new(controller),
        require('lid').new(controller,function() return assert(images[0]).descriptor end)}
    local names={'controller','scheduler','streams','media','sectors','audio','eligibility','delivery','transfers','command','submission','playback','decoded','lid'}
    local boundaries,mutations={},{}
    for index=0,tonumber(status.events)-1 do
        local row=h.record(index)
        events.describe(row)
        -- Check payload bounds before asking the binding to copy them.
        local offset,length=tonumber(row.offset),tonumber(row.length)
        assert(offset>=0 and length>=0 and offset%1==0 and length%1==0 and
            offset+length<=tonumber(status.bytes),'CD payload lies outside frozen history')
        local payload=h.bytes(offset,length)
        assert(type(payload)=='string' and #payload==length,'short CD history payload copy')
        if row.kind==47 then
            local phase=tonumber(row.device)
            assert(not controller.current() and not images[phase], 'duplicate or nested CD image boundary')
            local decoded=image.decode(row,payload)
            if phase==0 then
                assert(previous and previous.kind==34 and previous.device==2 and previous.cycle==row.cycle,
                    'CD initial image lacks adjacent environment')
                geometry=payload
            else
                assert(images[0] and geometry==payload, 'CD image changed across capture')
            end
            images[phase]={descriptor=decoded,payload=reference(row,0),binding=image.bind(decoded,bindings)}
        elseif row.kind==34 and row.device==0 then
            assert(images[0] and not images[1], 'CD initial controller lacks image boundary')
        elseif row.kind==34 and row.device==1 then
            assert(images[1] and previous and previous.kind==47 and previous.device==1 and previous.cycle==row.cycle,
                'CD final controller lacks adjacent image')
        elseif images[1] then
            assert((row.kind==41 and row.device==2) or (row.kind==37 and row.data[0]==4),
                'CD activity follows final image')
        end
        for _,consumer in ipairs(consumers) do consumer.push(row,payload) end
        if row.kind==41 then
            if row.device==1 then
                mutations[events.exact(row.request)]=reference(row,32)
            else boundaries[tonumber(row.device)]=reference(row,0) end
        end
        previous=row
    end
    assert(images[0] and images[1], 'CD image boundaries incomplete')
    local result={schema='psx.runtime-cd-transactions/v1',complete=true,
        begin_cycle=events.exact(status.beginCycle),end_cycle=events.exact(status.endCycle),
        events=tonumber(status.events),payload_bytes=tonumber(status.bytes)}
    result.image={initial=images[0],final=images[1],
        scope='native geometry associated with staged input identities; verify the passed harness receipt'}
    for index,consumer in ipairs(consumers) do result[names[index]]=consumer.finish() end
    -- The sector tracker keeps binary shadows for byte comparisons. Export only
    -- references into the raw journal, never arbitrary bytes through JSON.
    local buffers=result.sectors
    assert(boundaries[0] and boundaries[2] and #buffers.initial==boundaries[0].bytes and
        #buffers.final==boundaries[2].bytes,'CD buffer export boundaries mismatch')
    buffers.initial,buffers.final=boundaries[0],boundaries[2]
    for _,mutation in ipairs(buffers.mutations) do
        local payload=assert(mutations[mutation.id],'CD mutation export lacks payload')
        assert(#mutation.bytes==payload.bytes,'CD mutation export length mismatch')
        mutation.payload,mutation.bytes=payload,nil
    end
    result.scope='controller, scheduler, stream, media-source, buffer and feed correlation within one frozen capture'
    result.limits= {mutations=limits.mutations,bytes=limits.bytes}
    result.exclusions=m.array({'mission stop or command completion','complete native callback-transition semantics',
        'track-table/media-format conformance',
        'ADPCM/PCM arithmetic fidelity','audible onset or queue drain','per-byte bus timing'})
    return result
end

-- The caller exports events (and joined PCM, if selected) before correlation.
-- A failed consumer retains raw evidence and writes a separate failure report.
function M.export(h, limits, native, audio, bindings)
    local ok,result,encoded=pcall(function()
        local status=h.status()
        assert(native and native.schema=='psx.native-history/v1' and native.complete and
            native.failure_bits==0 and native.dropped_attempts==0 and
            native.events==tonumber(status.events) and native.payload_bytes==tonumber(status.bytes) and
            native.begin_cycle==events.exact(status.beginCycle) and native.end_cycle==events.exact(status.endCycle),
            'CD correlation requires matching complete raw history export')
        local value=M.scan(h,limits,audio,bindings)
        local bytes=m.json(value)
        assert(#bytes<=16777216,'CD correlation export exceeds 16 MiB')
        return value,bytes
    end)
    if not ok then
        result={schema='psx.runtime-cd-transactions/v1',complete=false,error=tostring(result)}
        encoded=m.json(result)
    end
    m.write('transactions.json',encoded..'\n')
    return result
end
return M
