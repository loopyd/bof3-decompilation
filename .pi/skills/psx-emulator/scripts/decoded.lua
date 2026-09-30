-- CD decoded-buffer callback. SPU values are evaluated native predicates;
-- this consumer does not validate DSP progress or hardware buffer timing.
local bit,m,events=require 'bit',require 'support',require 'events'
local M={}
function M.new(controller)
    local tracker,active,clock={}
    local result={callbacks=m.array(),inactive=0,disabled=0,outside=0,asserted=0}
    local function finish(frame)
        local cursor=0
        local function take(kind)
            cursor=cursor+1;local t=active.tape[cursor]
            assert(t and t.kind==kind,'CD decode expected '..kind..' at body item '..cursor)
            return t.data
        end
        local entry={access=events.exact(frame.id)}
        if frame.before_context.words[16]==0 then entry.classification='inactive'
        else
            local control=take('control')[1]
            assert(control<=65535,'CD decode control exceeds register width');entry.control=control
            if bit.band(control,64)==0 then entry.classification='disabled'
            else
                local address=take('address')[1]
                assert(address<=65535,'CD decode address exceeds register width');entry.address=address
                if address>=256 then entry.classification='outside'
                else
                    assert(take('assert')[1]==512,'CD decode asserted wrong IRQ')
                    local delay=math.floor(clock/44100)*256
                    assert(take('schedule')[2]==delay,'CD decode exact delay mismatch')
                    entry.classification,entry.delay='asserted',delay
                end
            end
        end
        assert(cursor==#active.tape,'CD decode has unexplained body work')
        local before,after=frame.before_context,frame.after_context
        for n=1,56 do assert(before.words[n]==after.words[n],'CD decode changed context word '..n) end
        for _,name in ipairs({'parameters_hex','response_hex','subq_hex'}) do
            assert(before[name]==after[name],'CD decode changed '..name)
        end
        result[entry.classification]=result[entry.classification]+1
        result.callbacks[#result.callbacks+1]=entry;active=nil
    end
    function tracker.push(row)
        local kind,phase=tonumber(row.kind),tonumber(row.device)
        local frame=controller.current()
        if kind==34 and phase==2 then
            local value=tonumber(row.data[1])
            assert(not clock and value>0 and value<=100000000,'invalid CD decode clock');clock=value
        elseif kind==34 and phase==3 and frame and frame.role==12 then
            assert(clock and not active,'CD decode missing environment or overlapping callback')
            active={id=frame.id,cycle=row.cycle,tape={}}
        elseif active then
            assert(frame and frame.id==active.id and row.cycle==active.cycle,'CD decode scope/cycle changed')
            if kind==34 and phase==4 then finish(frame);return end
            if kind==45 and (phase==3 or phase==5) then return end
            local d={};for n=0,7 do d[n+1]=tonumber(row.data[n]) end
            local t={data=d}
            if kind==46 and phase==8 then t.kind='control'
            elseif kind==46 and phase==9 then t.kind='address'
            elseif kind==12 and phase==0 then t.kind='assert'
            elseif kind==37 and phase==12 and d[1]==0 then t.kind='schedule'
            elseif kind==10 and phase==12 then return -- Scheduler consumer verifies adjacency/scaling.
            else error('unexpected CD decode body record') end
            active.tape[#active.tape+1]=t
        end
    end
    function tracker.finish()
        assert(not active,'unfinished CD decode callback')
        result.scope='decoded-buffer play/control/address gates, IRQ and exact reschedule; unchanged controller bytes; SPU values are evaluated predicates, not DSP/buffer timing evidence'
        return result
    end
    return tracker
end
return M
