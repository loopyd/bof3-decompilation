-- Record bounded native debugger steps, exact pause contexts and observed coverage.
local m, execution, uv = require 'support', require 'execution', assert(luv, 'native luv library unavailable')
m.begin('target,identity,steps,stop,seconds,exceptions,savestate', function()
    assert(PCSX.Debugger and PCSX.Debugger.stepInto, 'approved native step binding is unavailable')
    local identity = m.argument('identity')
    assert(identity and #identity > 0 and #identity <= 160, 'identity must name the executable/overlay context (1..160 bytes)')
    local limit = m.integer('steps', 1, 1, 1024)
    local stop = m.argument('stop') and m.address('stop')
    local seconds = m.integer('seconds', 20, 1, 300)
    local exceptions = m.argument('exceptions', 'stop')
    assert(exceptions == 'stop' or exceptions == 'continue', 'exceptions must be stop or continue')
    local save = m.integer('savestate', 0, 0, 1) == 1
    local coverage, observe = execution.coverage(execution.symbols(m.input('symbols', false)))
    local journal = assert(io.open('steps.ndjson', 'wb'))
    local bytes, count, started, pending, closed = 0, 0, false, false, false
    local before, timer
    local function record(value)
        local text = m.json(value) .. '\n'
        assert(bytes + #text <= 16777216, 'step journal exceeds 16 MiB')
        assert(journal:write(text)); assert(journal:flush()); bytes = bytes + #text
    end
    local function finish(reason, success, context)
        if closed then return end
        closed = true
        PCSX.pauseEmulator()
        timer:stop(); timer:close()
        assert(journal:close())
        m.captures[#m.captures + 1] = {path = 'steps.ndjson', bytes = bytes}
        m.report('step.json', {schema = 'psx.runtime-step/v1', identity = identity,
            identity_authority = 'caller label; input and executable hashes are in the harness receipt',
            boundary = 'native debugger pauses; IRQ handling may cross multiple instructions',
            coverage_scope = 'observed starting boundaries and last executed words, not a complete retired stream',
            memory_words = 'side-effect-free RAM/ROM/scratch bytes; may differ from instruction cache',
            reason = reason, success = success, steps = count, limit = limit, stop = stop,
            seconds = seconds, exceptions = exceptions, context = context, coverage = coverage})
        if save then m.write('state.pbuf', tostring(PCSX.createSaveState())) end
        if success then m.finish() else error(reason) end
    end
    local function safe(callback)
        return m.guard(function(...)
            local ok, err = pcall(callback, ...)
            if not ok and not closed then finish('execution error: ' .. tostring(err), false, before)
            elseif not ok then error(err) end
        end)
    end
    timer = uv.new_timer()
    timer:start(seconds * 1000, 0, safe(function()
        finish(started and 'step timeout' or 'target timeout', false, execution.capture())
    end))
    m.retained[#m.retained + 1] = timer
    m.event('ExecutionFlow::Pause', function(event)
        if not pending or closed then return end
        pending = false
        local native_exception = event.exception
        PCSX.nextTick(safe(function()
            local after = execution.capture()
            count = count + 1
            local entered = (not before.in_isr and after.in_isr) or
                after.exception.epc ~= before.exception.epc or after.exception.cause ~= before.exception.cause
            record({step = count, before = before, after = after,
                deltas = execution.deltas(before, after), native_exception_pause = native_exception,
                exception_context_changed = entered})
            observe(before, after)
            before = after
            if stop and after.pc == stop then finish('stop reached', true, after)
            elseif (entered or native_exception) and exceptions == 'stop' then
                finish(native_exception and 'native exception pause' or 'exception context changed', not stop, after)
            elseif count == limit then finish(stop and 'step limit before stop' or 'step count reached', not stop, after)
            else pending = true; PCSX.Debugger.stepInto() end
        end))
    end)
    m.atTarget(safe(function()
        -- A target breakpoint is still dispatching; defer until its step reset completes.
        PCSX.pauseEmulator()
        PCSX.nextTick(safe(function()
            before, started = execution.capture(), true
            record({initial = before})
            if stop and before.pc == stop then finish('stop already reached', true, before)
            else pending = true; PCSX.Debugger.stepInto() end
        end))
    end))
end)
