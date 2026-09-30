-- Assert production stepping observations against independently encoded fixture programs.
local m = require 'support'
local file = assert(io.open(m.input('case', true), 'rb'))
local scenario = file:read(32); file:close()
local original_json, original_report, observed = m.json, m.report, {}
-- Fault injection checks stop policy even when native exception registers are unchanged.
if scenario == 'pause' then
    local event = m.event
    m.event = function(name, callback)
        return event(name, function(value)
            if name == 'ExecutionFlow::Pause' then value.exception = true end
            return callback(value)
        end)
    end
end
local expected = {0x80010004, 0x80010008, 0x8001000c, 0x80010010, 0x80010014,
    0x80010018, 0x80010020, 0x80010024, 0x80010030, 0x80010034,
    0x80010038, 0x80010028, 0x80000080}
m.json = function(value)
    if type(value) == 'table' and value.step and value.before and value.after then
        observed[#observed + 1] = value
        if scenario == 'delays' then
            assert(value.after.pc == expected[value.step], 'unexpected native step PC')
            if value.step == 4 then
                assert(value.after.gpr[12] == 7 and value.after.gpr[11] == 42, 'load delay failed')
            end
            if value.step == 7 then assert(value.before.next_is_delay_slot, 'branch delay state absent') end
        end
    end
    return original_json(value)
end
m.report = function(name, value)
    if name == 'step.json' then
        assert(value.success and value.steps == #observed)
        if scenario == 'pause' then
            assert(value.steps == 1 and value.reason == 'native exception pause')
            assert(not observed[1].exception_context_changed)
            return original_report(name, value)
        end
        assert(value.reason == 'exception context changed')
        assert(value.context.exception.code == (scenario:match('^break') and 9 or 8))
        if scenario == 'delays' then
            assert(value.steps == 13 and value.context.exception.epc == 0x80010028)
        elseif scenario == 'nested' then
            assert(value.context.gpr[9] == 31 and value.context.exception.epc == 0x8001000c)
            assert(#value.coverage.functions == 4, 'nested/indirect/tail entry coverage differs')
        elseif scenario == 'fault' or scenario == 'fault_odd' or scenario:match('^break') then
            local odd = scenario:match('odd$') ~= nil
            assert(value.steps == (odd and 3 or 2) and value.context.exception.branch_delay)
            assert(value.context.exception.epc == (odd and 0x80010004 or 0x80010000), 'delay fault EPC must name branch')
            assert(value.context.pc == 0x80000080, 'pending branch overwrote exception PC')
        elseif scenario == 'recursion' then
            assert(value.steps == 62 and value.context.gpr[30] == 0x4000 and value.context.gpr[5] == 0)
            local found = false
            for _, fn in ipairs(value.coverage.functions) do
                if fn.address == 0x8001002c then assert(fn.hits == 5); found = true end
            end
            assert(found, 'recursive entry coverage missing')
        else error('unknown fixture scenario') end
    end
    return original_report(name, value)
end
require 'step'
