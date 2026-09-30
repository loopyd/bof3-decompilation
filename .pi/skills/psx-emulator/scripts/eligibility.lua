-- Correlate CD audio decisions with the retained transfer header and native
-- controller modes. Feed rows after controller and sectors; feed outcomes and
-- Audio joins belong to feeds. This does not reproduce ADPCM or PCM arithmetic.
local ffi, bit, m, events = require 'ffi', require 'bit', require 'support', require 'events'
local M={}
local function word(bytes,offset)
    assert(#bytes>=offset+4,'short CD eligibility payload')
    local a,b,c,d=bytes:byte(offset+1,offset+4);return a+b*256+c*65536+d*16777216
end
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function meta(row) return {sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)} end
function M.new(controller,sectors)
    local tracker,frames={},{}
    local result={decisions=m.array()}
    local first,final,enabled,expected,feeding,decoder
    local function current()
        assert(first and not final,'CD audio decision outside capture')
        local frame=assert(controller.current(),'CD audio decision lacks controller scope')
        local state=assert(frames[events.exact(frame.id)],'CD audio decision lacks read/play mode boundary')
        return frame,state
    end
    local function expect(phase,id) expected={phase=phase,id=id} end
    local function requireExpected(phase,id)
        assert(expected and expected.phase==phase and expected.id==id,'CD audio decision order mismatch');expected=nil
    end
    local function decision(row,d)
        local frame,state=current();local phase=tonumber(row.device)
        assert(frame.stage==4 and ((phase==5 and frame.role==7 and d[8]==1) or
            (phase>=2 and phase<=4 and frame.role==6 and d[8]==0)), 'CD audio decision role/kind mismatch')
        local header,mutation=sectors.header()
        assert(#header==8 and row.request==mutation,'CD audio decision header identity mismatch')
        local file,channel,submode,coding=header:byte(5,8)
        local mode=state.mode
        if phase==3 and mode[4]==1 and bit.band(mode[1],8)==0 then mode[2],mode[3]=file,channel end
        local flags=(mode[5]~=0 and 1 or 0)+(bit.band(mode[1],64)~=0 and 2 or 0)+(enabled and 4 or 0)+
            (mode[4]~=0xffffffff and 8 or 0)+(mode[3]~=255 and 16 or 0)+
            (file==mode[2] and 32 or 0)+(channel==mode[3] and 64 or 0)+
            (bit.band(submode,4)~=0 and 128 or 0)+bit.band(d[1],768)
        assert(d[1]==flags and d[2]==mode[1] and d[3]==mode[2] and d[4]==mode[3] and d[5]==mode[4] and
            d[6]==(phase==5 and 0 or word(header,4)), 'CD audio decision flags/filter/header mismatch')
        local entry=meta(row);entry.phase=phase;entry.access=events.exact(frame.id)
        entry.mutation=events.exact(mutation);entry.raw=m.array(d)
        if phase==2 then
            requireExpected(2,frame.id)
            entry.eligible=bit.band(flags,15)==14
            if entry.eligible then expect(3,frame.id) end
        elseif phase==3 then
            requireExpected(3,frame.id)
            entry.eligible=bit.band(flags,240)==240
            if entry.eligible then expect(4,frame.id) end
        elseif phase==4 then
            requireExpected(4,frame.id)
            local frequency=math.floor(coding/4)%4
            local failed=mode[4]~=0 and frequency>=2
            assert(d[7]==(failed and 0xffffffff or 0),'CD XA decoder result mismatch')
            entry.coding_applied=mode[4]~=0;entry.decoder_success=not failed
            if failed then mode[4]=0xffffffff
            else
                -- Only first sectors apply coding. Preserve evidence across
                -- callbacks; a reset of ADPCM history does not change this tuple.
                if mode[4]~=0 then
                    local rate,stereo=frequency==0 and 37800 or 18900,coding%4==1 and 1 or 0
                    local code=math.floor(coding/16)%4;local nbits=code==0 and 4 or (code==1 and 8 or 0)
                    local count
                    if decoder then
                        if decoder.rate~=rate or decoder.stereo~=stereo or (decoder.nbits and decoder.nbits~=nbits) then
                            count=4032/(stereo+1)
                        elseif decoder.nbits then count=decoder.frames end
                    end
                    decoder={rate=rate,stereo=stereo,nbits=nbits,frames=count}
                end
                expect(0,frame.id)
            end
        else
            assert(state.gate and state.gate==mutation,'CDDA gate lacks raw sector mutation');state.gate=nil
            entry.eligible=bit.band(flags,257)==256
            if entry.eligible then expect('attenuation',frame.id) end
        end
        if phase~=4 then assert(d[7]==0,'CD non-decoder decision carries a result') end
        result.decisions[#result.decisions+1]=entry
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        if expected then
            if expected.phase=='attenuation' then
                assert(kind==41 and phase==1 and d[3]==5,'eligible CDDA gate lacks adjacent attenuation')
            else
                assert(kind==43 and phase==expected.phase,'eligible CD audio path lacks adjacent decision/feed')
            end
        end
        if kind==34 and phase==2 then
            assert(enabled==nil and not first,'duplicate CD eligibility environment')
            assert(#payload>=112 and word(payload,108)==0 and word(payload,104)<=1,'invalid XA enable setting')
            enabled=word(payload,104)==1
        elseif kind==34 and phase<2 then
            assert(enabled~=nil,'CD eligibility lacks environment')
            if phase==0 then assert(not first,'duplicate CD eligibility context');first=true
            else
                assert(phase==1 and first and not final and not next(frames) and not expected and not feeding,
                    'CD eligibility context ended with unfinished decisions');final=true
            end
        elseif kind==45 and phase==4 then
            local frame=assert(controller.current(),'CD mode boundary lacks controller scope')
            if frame.role==6 or frame.role==7 then
                local key=events.exact(frame.id);assert(not frames[key],'duplicate CD audio mode boundary')
                frames[key]={mode={d[1],d[2],d[3],d[4],d[5]}}
            end
        elseif kind==45 and phase==5 then
            local frame=assert(controller.current(),'CD mode boundary lacks controller scope')
            if frame.role==6 or frame.role==7 then
                local key=events.exact(frame.id);local state=assert(frames[key],'missing CD audio mode boundary')
                assert(not state.gate and not expected and not feeding,'CD audio body left an unfinished path')
                for n=1,5 do assert(d[n]==state.mode[n],'CD audio final mode/filter mismatch') end
                frames[key]=nil
            end
        elseif kind==41 and phase==1 then
            if d[3]==2 then local frame=current();expect(2,frame.id)
            elseif d[3]==4 then
                local frame,state=current();assert(frame.role==7 and not state.gate,'duplicate CDDA raw sector')
                local _,mutation=sectors.header();assert(row.request==mutation,'CDDA raw mutation identity mismatch')
                state.gate=mutation
            elseif d[3]==5 then
                local frame=current();requireExpected('attenuation',frame.id);expect(0,frame.id)
            end
        elseif kind==43 and phase>=2 then decision(row,d)
        elseif kind==43 and phase==0 then
            local frame,state=current();requireExpected(0,frame.id)
            assert(not feeding and d[1]==(frame.role==7 and 1 or 0),'unexpected CD audio feed kind')
            if frame.role==6 then
                if decoder then
                    assert(d[2]==decoder.rate and d[4]==decoder.stereo and (not decoder.frames or d[3]==decoder.frames),
                        'CD feed differs from known XA decoder metadata')
                    decoder.frames=d[3]
                else decoder={rate=d[2],stereo=d[4],frames=d[3]} end
            end
            feeding={id=row.request,access=frame.id}
        elseif kind==43 and phase==1 then
            local frame,state=current()
            assert(feeding and row.request==feeding.id and frame.id==feeding.access,'CD outcome lacks eligible feed')
            if frame.role==6 then state.mode[4]=0 end
            feeding=nil
        end
    end
    function tracker.finish()
        assert(first and final and not next(frames) and not expected and not feeding,'CD eligibility boundaries incomplete')
        result.scope='native XA/CDDA eligibility and decoder control flow; prehistory coding may be unknown; observed metadata persists; no ADPCM/PCM fidelity claim'
        return result
    end
    return tracker
end
return M
