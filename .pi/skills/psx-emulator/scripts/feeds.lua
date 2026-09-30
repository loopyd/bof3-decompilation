-- CD feed outcomes and exact forward Audio joins. Consume validated wire rows
-- after controller.push; audio, when supplied, is {report=pcm.export(),event=...}
-- from the frozen recorder. Eligibility/filter/decoder decisions remain separate.
local ffi, m, events = require 'ffi', require 'support', require 'events'
local M={}
local zero=ffi.new('uint64_t',0)
local settings={'volume','interpolation','reverb','mute','mono','streaming','null_sync',
    'irq_wait','decode_irq','scaler','forced_irq','voice_mute','voice_solo','xa'}
local function word(bytes,offset)
    assert(#bytes>=offset+4,'short CD feed payload')
    local a,b,c,d=bytes:byte(offset+1,offset+4);return a+b*256+c*65536+d*16777216
end
local function wide(lo,hi) return ffi.new('uint64_t',hi)*4294967296+lo end
local function identity(bytes,offset) return wide(word(bytes,offset),word(bytes,offset+4)) end
local function decimal(value)
    assert(type(value)=='string' and value:match('^%d+$') and
        (value=='0' or value:sub(1,1)~='0') and
        (#value<20 or (#value==20 and value<='18446744073709551615')),'invalid Audio uint64 decimal')
    local result=zero;for n=1,#value do result=result*10+value:byte(n)-48 end;return result
end
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function meta(row) return {sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)} end
function M.new(controller,audio)
    local tracker,seen,joinedEvents={},{},{}
    local environment,first,final,joined,epoch,historyEpoch,pending,native
    local result={feeds=m.array()}
    local function current()
        assert(first and not final,'CD feed outside capture context')
        local frame=assert(controller.current(),'CD feed lacks controller scope')
        assert(frame.stage==4,'CD feed outside controller body');return frame
    end
    local function verifyAudio(row,d)
        assert(audio and audio.report and audio.report.complete==true and type(audio.event)=='function',
            'joined CD feed requires complete frozen Audio export')
        native=audio.report.native
        assert(native and native.version==1 and native.state==2 and native.failures==0 and native.dropped_events==0 and
            native.dropped_frames=='0' and native.streaming_dropped=='0' and
            decimal(native.epoch)==epoch and epoch~=0 and decimal(native.begin_cycle)<=row.cycle,
            'joined Audio status/epoch mismatch')
        assert(native.events>=0 and native.events<=65536 and native.events%1==0,'invalid joined Audio event count')
        for i,name in ipairs(settings) do
            assert(ffi.cast('int64_t',environment.values[i])==native.settings[name],
                'joined Audio environment mismatch: '..name)
        end
        assert(native.settings.configured_backend==environment.backend and native.settings.configured_device==environment.device,
            'joined Audio device selection mismatch')
    end
    local function join(row,d,entry)
        if not joined then return end
        assert(d[6]==1 and identity(entry.payload,0)==epoch,'CD feed changed joined Audio lifecycle')
        if d[1]<6 then return end
        assert(d[7]==2 and row.related>0 and row.related<=native.events,'CD feed lacks retained Audio event')
        local id=events.exact(row.related)
        assert(not joinedEvents[id],'Audio feed event joined more than once')
        local event=audio.event(tonumber(row.related)-1)
        assert(event and event.id==tonumber(row.related) and event.kind==3 and decimal(event.parent)==0 and
            decimal(event.cycle)==row.cycle and event.source_rate==d[2] and event.source_frames==d[3] and
            event.stereo==d[4] and event.output_frames==d[5] and event.accepted==(d[1]==6 and 1 or 0) and
            event.cdda==d[8],'CD feed/Audio event mismatch')
        joinedEvents[id]=true;entry.audio_event=id
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        if pending and kind==45 and (phase==1 or phase==3) and row.request==pending.access then
            error('CD feed left controller body without outcome')
        end
        if kind==34 and phase==2 then
            assert(not environment and not first and #payload>=172,'duplicate or short CD feed environment')
            environment={values={},backend=payload:sub(173,172+d[5]),device=payload:sub(173+d[5],172+d[5]+d[6])}
            for n=0,13 do environment.values[n+1]=identity(payload,n*8) end
            assert(ffi.cast('int64_t',environment.values[10])>=100,'unsupported CD feed scaler')
            assert(environment.values[6]<=1,'invalid CD streaming setting')
        elseif kind==34 and phase<2 then
            assert(environment and not pending,'CD feed context lacks environment or has unfinished feed')
            if phase==0 then
                assert(not first and d[2]<=1,'duplicate CD feed initial context')
                joined=d[2]==1;epoch=wide(d[5],d[6]);historyEpoch=wide(d[3],d[4])
                assert(historyEpoch~=0 and ((joined and epoch~=0) or (not joined and epoch==0)),
                    'invalid CD feed capture profile')
                if joined then verifyAudio(row,d) else assert(not audio,'Audio export supplied to an unjoined CD profile') end
                first=true
            else
                assert(phase==1 and first and not final and joined==(d[2]==1) and epoch==wide(d[5],d[6]) and
                    historyEpoch==wide(d[3],d[4]),'CD feed final profile mismatch')
                if joined then assert(decimal(native.end_cycle)>=row.cycle,'Audio capture ended before CD capture') end
                final=true
            end
        elseif kind==43 and phase==0 then
            local frame=current();local id=events.exact(row.request)
            assert(not pending and not seen[id] and row.request~=0 and row.related==frame.id and d[1]<=1 and
                d[4]<=1 and #payload==24 and frame.role==(d[1]==1 and 7 or 6),'invalid CD feed entry')
            if d[1]==1 then assert(d[2]==44100 and d[3]==588 and d[4]==1,'invalid native CDDA feed metadata') end
            seen[id]=true
            pending={id=row.request,access=frame.id,cycle=row.cycle,raw=d,begin=meta(row),
                media=events.exact(identity(payload,0)),mutation=events.exact(identity(payload,8)),
                stream=events.exact(identity(payload,16))}
        elseif kind==43 and phase==1 then
            local frame=current()
            assert(pending and row.request==pending.id and frame.id==pending.access and #payload==16 and
                identity(payload,8)==frame.id and row.cycle==pending.cycle and d[8]==pending.raw[1],
                'CD feed outcome identity mismatch')
            local reason,rate,frames,stereo=d[1],pending.raw[2],pending.raw[3],pending.raw[4]
            assert(reason>=1 and reason<=7 and reason~=2 and d[6]<=2 and d[7]<=3,
                'invalid or unreachable CD feed outcome')
            local disabled=d[8]==0 and environment.values[6]==0
            assert((reason==1)==disabled,'CD feed streaming suppression mismatch')
            if not disabled then assert((reason==3)==(d[8]==0 and rate==0),'CD feed zero-frequency outcome mismatch') end
            if reason<=4 then
                assert(d[2]==0 and d[3]==0 and d[4]==0 and d[5]==0,'early CD feed outcome inspected metadata')
                assert(d[8]==0 or reason==4,'CDDA used an XA-only suppression path')
            else
                assert(rate>0 and rate<=2147483647 and frames<=48695 and d[2]==rate and d[3]==frames and d[4]==stereo,
                    'CD feed outcome metadata mismatch or unsupported arithmetic')
                local expected=math.floor(44100*frames/rate)
                assert(expected<=32768 and d[5]==expected and ((reason==5)==(expected==0)),
                    'CD feed output frame calculation mismatch')
            end
            local submitted=reason>=6
            local disposition=not submitted and 0 or (d[6]~=1 and 1 or (row.related~=0 and 2 or 3))
            assert(d[7]==disposition and (submitted and d[6]==1 or row.related==0),
                'CD feed Audio event disposition mismatch')
            local entry={id=events.exact(pending.id),access=events.exact(pending.access),media=pending.media,
                mutation=pending.mutation,stream=pending.stream,begin=pending.begin,finish=meta(row),
                input=m.array(pending.raw),outcome=m.array(d),audio_epoch=events.exact(identity(payload,0)),
                observed_audio_event=events.exact(row.related),payload=payload}
            join(row,d,entry);entry.payload=nil
            result.feeds[#result.feeds+1]=entry;pending=nil
        end
    end
    function tracker.finish()
        assert(environment and first and final and not pending,'CD feed capture boundaries incomplete')
        result.joined=joined;result.audio_epoch=events.exact(epoch)
        result.scope='feed outcomes and exact forward Audio joins; gates/decoder, full capture and audible timing require separate evidence'
        return result
    end
    return tracker
end
return M
