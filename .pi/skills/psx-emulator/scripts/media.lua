-- Correlate native CD lookup outcomes, cache/backing selection and their buffer
-- sources. Consume validated rows after controller.push. Track-table semantics,
-- complete media conformance and Audio outcomes are outside this component.
local ffi, bit, m, events = require 'ffi', require 'bit', require 'support', require 'events'
local M={}
local zero=ffi.new('uint64_t',0)
local function word(bytes,offset)
    assert(#bytes>=offset+4,'short CD media payload')
    local a,b,c,d=bytes:byte(offset+1,offset+4);return a+b*256+c*65536+d*16777216
end
local function wide(lo,hi) return ffi.new('uint64_t',hi)*4294967296+lo end
local function identity(bytes,offset) return wide(word(bytes,offset),word(bytes,offset+4)) end
local function signed(value) return tostring(ffi.cast('int64_t',value)):gsub('LL$','') end
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function lba(msf)
    assert(msf<16777216,'CD MSF has nonzero reserved byte')
    return ((msf%256)*60+math.floor(msf/256)%256)*75+math.floor(msf/65536)
end
local function meta(row) return {sequence=events.exact(row.sequence),cycle=events.exact(row.cycle)} end
function M.new(controller)
    local tracker,lookups={},{}
    local result={lookups=m.array(),mutations=m.array(),feeds=m.array(),decisions=m.array()}
    local latest={[1]=zero,[2]=zero}
    local first,final,open,previous,mutation
    local backing=zero
    local function scope()
        assert(first and not final,'CD media operation outside context')
        local frame=assert(controller.current(),'CD media lacks controller scope')
        assert(frame.stage==4,'CD media operation outside controller body');return frame
    end
    local function lookup(id,kind)
        local entry=lookups[events.exact(id)]
        assert(entry and entry.finished and entry.kind==kind and latest[kind]==id,'CD media source is not latest completed lookup')
        return entry
    end
    local function outcome(row,d)
        assert(open and not open.outcome and row.request==open.id and row.related==open.access,
            'CD backend outcome lacks unique lookup')
        local reason,count,flags=d[1],ffi.cast('int64_t',wide(d[2],d[3])),d[7]
        assert(reason>=1 and reason<=7 and flags<64,'invalid CD backend outcome')
        if reason==1 or reason==2 or reason==5 then
            assert(count==0 and d[5]==0 and d[6]==0 and flags==0 and d[8]==0 and
                (reason~=5 or d[4]==0),'CD no-read outcome fabricated backend work')
        elseif reason==6 or reason==7 then
            local sector=d[6]>=2147483648 and d[6]-4294967296 or d[6]
            assert(d[4]==0 and d[5]==0 and d[8]==0 and bit.band(flags,14)==12 and
                (sector==open.lba-150 or sector==open.lba-300),'CD data backend layout mismatch')
            assert((reason==6 and count<0) or (reason==7 and count>=0),'CD data backend return/result mismatch')
        else
            assert(bit.band(flags,8)~=0 and (bit.band(flags,1)~=0)==(bit.band(flags,4)~=0) and
                (reason~=3 or bit.band(flags,2)==0),'CDDA backend selection/byte-swap mismatch')
            assert((reason==3 and count~=2352) or (reason==4 and count==2352),'CDDA backend return/result mismatch')
        end
        assert((open.kind==1 and reason>=5) or (open.kind==2 and reason<=4),'CD backend kind mismatch')
        if bit.band(flags,4)~=0 then backing=open.id end
        local entry=meta(row);entry.reason=reason;entry.count=signed(wide(d[2],d[3]));entry.raw=m.array(d)
        open.outcome=entry
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        if open then
            assert(((kind==40 and phase~=0) or kind==46) and row.cycle==open.cycle,'CD lookup boundaries interrupted')
        end
        if kind==34 and phase<2 then
            if phase==0 then
                assert(not first,'duplicate CD media initial context');first=true;previous=word(payload,116)
            else
                assert(phase==1 and first and not final and previous==word(payload,116),'CD final cache address mismatch')
                final=true
            end
        elseif kind==40 then
            local frame=scope();assert(row.related==frame.id,'CD lookup access mismatch')
            if phase==0 then
                assert(not open and (d[1]==1 or d[1]==2) and row.request~=0 and not lookups[events.exact(row.request)] and
                    d[3]==lba(d[2]) and wide(d[4],d[5])==(d[1]==1 and backing or zero),
                    'CD lookup address/backing identity mismatch')
                open={id=row.request,access=row.related,kind=d[1],msf=d[2],lba=d[3],cycle=row.cycle,
                    begin=meta(row),backing=events.exact(wide(d[4],d[5]))}
                lookups[events.exact(row.request)]=open
            elseif phase==2 then outcome(row,d)
            else
                assert(phase==1 and open and row.request==open.id and row.related==open.access and
                    d[1]==open.kind and d[2]<=1 and d[3]<=2,'CD lookup result mismatch')
                assert((d[3]==0)==(open.outcome~=nil) and (open.kind~=2 or d[3]==0),
                    'CD lookup outcome/disposition mismatch')
                if open.kind==1 then
                    assert((d[3]==1)==(open.msf==previous),'CD cache disposition/address mismatch')
                    if d[3]~=1 then previous=d[2]==1 and open.msf or 0 end
                    if d[3]==2 then assert(d[2]==0,'rejected CDDA-track data lookup succeeded') end
                end
                if open.outcome then
                    local reason=open.outcome.reason
                    assert(d[2]==((reason==1 or reason==2 or reason==4 or reason==7) and 1 or 0),
                        'CD backend outcome/success mismatch')
                end
                open.finished=true;open.success=d[2]==1;open.disposition=d[3];open.finish=meta(row)
                latest[open.kind]=open.id
                result.lookups[#result.lookups+1]={id=events.exact(open.id),access=events.exact(open.access),
                    kind=open.kind,packed_msf=open.msf,absolute_frame=open.lba,prior_backing=open.backing,
                    success=open.success,disposition=open.disposition,begin=open.begin,finish=open.finish,outcome=open.outcome}
                open=nil
            end
        elseif kind==41 and phase==1 then
            local frame=scope();local reason=d[3];local source
            assert(d[5]==lba(d[4]) and identity(payload,0)==frame.id,'CD mutation media address/access mismatch')
            if reason==5 then
                assert(mutation and mutation.reason==4 and row.related==mutation.id and d[6]==2 and
                    d[4]==mutation.msf,'CDDA attenuation source mismatch')
                source=mutation.source
            else
                assert(reason>=1 and reason<=4 and d[6]==1,'invalid CD mutation media source kind')
                source=lookup(row.related,reason==4 and 2 or 1)
                assert(source.msf==d[4],'CD mutation differs from source lookup address')
                if reason==2 then assert(source.success,'delivered CD data came from failed lookup') end
                if reason==4 and source.outcome.reason~=4 then
                    assert(#payload==32+2352 and payload:sub(33)==string.rep('\0',2352),
                        'CDDA silence/error mutation contains nonzero samples')
                end
            end
            mutation={id=row.request,reason=reason,msf=d[4],source=source}
            local entry=meta(row);entry.id=events.exact(row.request);entry.lookup=events.exact(source.id)
            entry.reason=reason;entry.access=events.exact(frame.id);result.mutations[#result.mutations+1]=entry
        elseif kind==43 and phase~=1 then
            local frame=scope();local cdda=phase==0 and d[1] or d[8]
            assert(cdda<=1,'invalid CD audio media kind')
            local id=phase==0 and identity(payload,0) or row.related
            local source=lookup(id,cdda+1)
            assert(mutation and mutation.source==source and
                (phase==0 and identity(payload,8) or row.request)==mutation.id,
                'CD audio media/mutation mismatch')
            local entry=meta(row);entry.lookup=events.exact(id);entry.mutation=events.exact(mutation.id)
            entry.access=events.exact(frame.id);entry.kind=cdda
            local output=phase==0 and result.feeds or result.decisions
            if phase==0 then entry.feed=events.exact(row.request) else entry.phase=phase end
            output[#output+1]=entry
        end
    end
    function tracker.finish()
        assert(first and final and not open,'CD media capture boundaries incomplete')
        result.scope='lookup/cache/backend and buffer source correlation; track-table conformance and Audio outcomes required separately'
        return result
    end
    return tracker
end
return M
