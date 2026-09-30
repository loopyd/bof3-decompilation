-- write1 handler semantics. Controller/scheduler/streams own journal identity
-- joins; XA predictor reset requires decoder-state evidence outside profile 2.
local bit,m,events=require 'bit',require 'support',require 'events'
local M={}
local function data(row)
    local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end;return d
end
local function clear(n,mask) return bit.band(n,bit.bnot(mask))%4294967296 end
local function lba(msf) return (msf%256*60+math.floor(msf/256)%256)*75+math.floor(msf/65536)%256 end
local function decimal(n) return math.floor(n/16)*10+n%16 end
function M.new(controller)
    local tracker,active={}
    local result={writes=m.array(),commands=0,repeated=0,decoder_reset_conditions=0}
    local function finish(frame)
        local before,after=frame.before_context,frame.after_context
        local b,c,delegated=before.words,{},{}
        for n=1,56 do c[n]=b[n] end
        local params={};for pair in before.parameters_hex:gmatch('..') do params[#params+1]=tonumber(pair,16) end
        local cursor=0
        local function take(kind)
            cursor=cursor+1;local t=active.tape[cursor]
            assert(t and t.kind==kind,'CD submission expected '..kind..' at body item '..cursor)
            return t
        end
        local function stop(kind)
            local t=take('stop');local n=kind==1 and 16 or 15
            assert(t.phase==kind and t.data[1]==0 and t.data[2]==0 and t.data[3]==c[n],
                'CD submission stream stop mismatch')
            if kind==1 then
                if c[16]~=0 then c[3]=clear(c[3],128);c[16],c[32],c[33]=0,0,0 end
                delegated[43],delegated[44]=true,true
            else
                if c[15]~=0 then take('cancel');c[15]=0 end
                c[3]=clear(c[3],96);delegated[41],delegated[42]=true,true
            end
        end
        local bank,operand=b[1]%4,frame.raw[4]
        local entry={access=events.exact(frame.id),bank=bank,operand=operand,decoder_reset_deferred=false}
        if bank==3 then c[35]=c[35]%16777216+operand*16777216
        elseif bank==0 then
            local submitted=take('submit').data
            for n,value in ipairs({operand,b[9],b[6],b[2],b[12]}) do
                assert(submitted[n]==value,'CD submission snapshot mismatch')
            end
            c[5],c[12],c[1]=operand,0,bit.bor(c[1],128)%4294967296
            delegated[37],delegated[38],delegated[39],delegated[40]=true,true,true,true
            local repeated=c[6]~=0 and (operand==c[6] or operand+256==c[6])
            local queued=take('queue').data
            for n,value in ipairs({c[6],c[8],c[7],operand,2048,repeated and 1 or 0}) do
                assert(queued[n]==value,'CD submission queue mismatch')
            end
            if repeated then c[7]=1;result.repeated=result.repeated+1
            else c[6],c[8]=operand,2048 end -- An old repeated flag survives replacement.
            local schedule=take('schedule')
            assert(schedule.phase==2 and schedule.data[2]==2048,'CD submission schedule mismatch')
            result.commands=result.commands+1;entry.repeated=repeated
            if operand==2 then
                local a,s,f=params[1],params[2],params[3]
                local valid=a%16<=9 and a<=0x99 and s%16<=9 and s<0x60 and f%16<=9 and f<0x75
                entry.location_valid=valid
                if valid then
                    local location=decimal(a)+decimal(s)*256+decimal(f)*65536
                    if math.abs(lba(c[27])-lba(location))>16 then c[17]=0 end
                    c[28],c[25]=location,1
                end
            elseif operand==6 or operand==27 or operand==9 or operand==10 or operand==28 then
                if operand==10 or operand==28 then c[17]=1 end
                stop(1);stop(0)
            elseif operand==14 then
                entry.decoder_reset_deferred=c[18]~=64 and params[1]==64
                if entry.decoder_reset_deferred then result.decoder_reset_conditions=result.decoder_reset_conditions+1 end
                c[18]=params[1]
                if c[16]~=0 and bit.band(c[18],1)==0 then stop(1) end
            end
        end
        assert(cursor==#active.tape,'CD submission has unexplained body work')
        for n=1,56 do
            assert(delegated[n] or after.words[n]==c[n],'CD submission final context mismatch at word '..n)
        end
        for _,name in ipairs({'parameters_hex','response_hex','subq_hex'}) do
            assert(after[name]==before[name],'CD submission changed '..name)
        end
        result.writes[#result.writes+1]=entry;active=nil
    end
    function tracker.push(row,payload)
        local kind,phase,d=tonumber(row.kind),tonumber(row.device),data(row)
        local frame=controller.current()
        if kind==34 and phase==3 and frame and frame.role==4 and frame.raw[2]==0x1f801801 then
            assert(not active,'overlapping CD submission');active={id=frame.id,cycle=row.cycle,tape={}}
        elseif active then
            assert(frame and frame.id==active.id and row.cycle==active.cycle,'CD submission scope/cycle changed')
            if kind==34 and phase==4 then finish(frame);return end
            if kind==45 and (phase==3 or phase==5) then return end
            local t={data=d,phase=phase}
            if kind==36 and phase==0 then t.kind='submit'
            elseif kind==36 and phase==2 then t.kind='queue'
            elseif kind==37 and phase==2 and d[1]==0 then t.kind='schedule'
            elseif kind==37 and phase==3 and d[1]==2 then t.kind='cancel'
            elseif kind==44 then t.kind='stop'
            elseif kind==10 and phase==2 then return -- Scheduler checks adjacent typed ancestry.
            else error('unexpected CD submission body record') end
            active.tape[#active.tape+1]=t
        end
    end
    function tracker.finish()
        assert(not active,'unfinished CD submission')
        result.scope='write1 bank selection, queue/seek/mode/stream effects; controller and scheduler own identities; XA predictor resets are deferred'
        return result
    end
    return tracker
end
return M
