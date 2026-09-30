-- Measure repeated native continuation; differences are evidence, never masked.
local m = require 'support'
m.begin('runs,depth,limit', function()
    PCSX.pauseEmulator()
    assert(os.getenv('PSX_RUNTIME_ORIGIN_VALIDATED') == '1', 'replay requires --origin capture receipt')
    assert(not os.getenv('PSX_RUNTIME_ENTRY'), 'replay restores a state without loading an EXE')
    local snapshot, difference, timeline = require 'snapshot', require 'difference', require 'timeline'
    local original = snapshot.read(m.input('state', true), true)
    snapshot.validate(original)
    local schedule = timeline.read(m.input('schedule', true))
    local alternatePath = m.input('alternate', false)
    local alternate = alternatePath and timeline.read(alternatePath) or schedule
    timeline.compatible(schedule, alternate)
    for _, event in ipairs(schedule.samples) do
        if event.action == 'sample' then snapshot.select(original, event.path, true)
        else require('display') end
    end
    original = nil
    local runs = m.integer('runs', 2, 2, 8)
    local options = {depth = m.integer('depth', 16, 0, 64), limit = m.integer('limit', 1024, 1, 65536)}
    local result = {schema = 'psx.runtime-replay/v1', runs_requested = runs,
        identical_schedule = not alternatePath, runs = m.array(), checkpoints = m.array(),
        equal = true, timing = 'GPU Vsync boundaries; cycles from native state',
        settings = 'receipt.json settings records fixed launch flags, fresh portable defaults and unpinned host behavior',
        limitations = {'Finite checkpoint comparison, not proof of determinism between observations.',
            'Host audio queues and physical controller input are not isolated or compared.',
            'Native serialization/restoration may affect host scheduling; no PCM capture in this action.',
            'Display pixels are compared only at explicit screen checkpoints; no continuous GPU history.'}}
    local baselines, baselineButtons, baselineCycles, current, frame, index = {}, {}, {}, 0, 0, 1
    local held = {{}, {}}
    local active, run
    local artifactBytes = 0
    local function capture(name, bytes)
        artifactBytes = artifactBytes + #bytes
        assert(artifactBytes <= 120 * 1024 * 1024, 'replay capture budget exceeded')
        m.write(name, bytes)
    end
    local function clear()
        for slot = 1, 2 do
            for _, button in pairs(PCSX.CONSTS.PAD.BUTTON) do
                PCSX.SIO0.slots[slot].pads[1].clearOverride(button)
            end
            held[slot] = {}
        end
    end
    local function buttons()
        local observed = {{}, {}}
        for slot = 1, 2 do
            for name, button in pairs(PCSX.CONSTS.PAD.BUTTON) do
                observed[slot][name] = PCSX.SIO0.slots[slot].pads[1].getButton(button)
            end
        end
        return m.json(observed)
    end
    local function divergence(boundary, detail, bytes)
        result.equal = false
        if result.first_divergence then return end
        capture('divergence.pbuf', bytes or tostring(PCSX.createSaveState()))
        detail.run, detail.frame, detail.boundary = current, frame, boundary
        detail.capture = 'divergence.pbuf'
        result.first_divergence = detail
    end
    local function sample(event)
        local bytes = tostring(PCSX.createSaveState())
        local state = snapshot.decode(bytes, true)
        local value, kind, repeated = snapshot.select(state, event.path, true)
        local observed = buttons()
        if current == 1 then
            local name = string.format('baseline-%03d.pbuf', event.index)
            capture(name, bytes)
            baselines[event.index], baselineButtons[event.index] = name, observed
            baselineCycles[event.index] = state.registers.cycle
            result.checkpoints[#result.checkpoints + 1] = {index = event.index, path = event.path,
                frame = frame, capture = name, cycles = state.registers.cycle, buttons = observed}
        else
            local baseline = snapshot.read(baselines[event.index], true)
            local before = snapshot.select(baseline, event.path, true)
            local report = difference.compare(before, value, kind, repeated, event.path, options)
            local buttonsEqual = observed == baselineButtons[event.index]
            local cyclesEqual = state.registers.cycle == baselineCycles[event.index]
            run.comparisons[#run.comparisons + 1] = {index = event.index, frame = frame,
                cycles = state.registers.cycle, cycles_equal = cyclesEqual, state = report,
                buttons_equal = buttonsEqual, buttons = observed}
            if not report.equal or not buttonsEqual or not cyclesEqual then
                divergence('checkpoint', {index = event.index, path = event.path,
                    state_equal = report.equal, buttons_equal = buttonsEqual, cycles_equal = cyclesEqual}, bytes)
            end
        end
    end
    local screens = {}
    local function screen(event)
        local display = require 'display'
        local bytes, metadata = display.capture()
        local observed, cycles = buttons(), tostring(PCSX.getCPUCycles())
        if current == 1 then
            local name = string.format('baseline-%03d.bin', event.index)
            capture(name, bytes)
            screens[event.index] = {name = name, metadata = metadata, cycles = cycles, buttons = observed}
            result.checkpoints[#result.checkpoints + 1] = {index = event.index, kind = 'display',
                frame = frame, capture = name, cycles = cycles, buttons = observed, metadata = metadata}
        else
            local baseline = screens[event.index]
            local file = assert(io.open(baseline.name, 'rb'))
            local before, problem = file:read(1572865); file:close()
            assert(not problem, problem)
            before = before or '' -- Empty native displays have an empty capture.
            local report = display.compare(before, bytes, baseline.metadata, metadata, options)
            local buttonsEqual, cyclesEqual = observed == baseline.buttons, cycles == baseline.cycles
            run.comparisons[#run.comparisons + 1] = {index = event.index, frame = frame, cycles = cycles,
                cycles_equal = cyclesEqual, display = report, buttons_equal = buttonsEqual, buttons = observed}
            if not report.equal or not buttonsEqual or not cyclesEqual then
                if not result.first_divergence then
                    capture('divergence.bin', bytes)
                    m.report('divergence.json', metadata)
                end
                divergence('checkpoint', {index = event.index, kind = 'display',
                    display_equal = report.equal, buttons_equal = buttonsEqual, cycles_equal = cyclesEqual})
            end
        end
    end
    local advance, begin
    begin = function()
        clear()
        current, frame, index = current + 1, 0, 1
        active = current == 1 and schedule or alternate
        assert(m.restore())
        run = {number = current, comparisons = m.array(), start_cycles = tostring(PCSX.getCPUCycles())}
        result.runs[#result.runs + 1] = run
        if current > 1 then
            run.start_cycles_equal = run.start_cycles == result.runs[1].start_cycles
            if not run.start_cycles_equal then divergence('start', {cycles_equal = false}) end
        end
        advance()
    end
    local function stop(event, observation)
        run.stop_frame, run.stop_cycles = frame, tostring(PCSX.getCPUCycles())
        run.reason = observation and 'condition matched' or 'scheduled stop'
        run.condition = observation
        if current > 1 then
            run.stop_cycles_equal = run.stop_cycles == result.runs[1].stop_cycles
            run.stop_frame_equal = frame == result.runs[1].stop_frame
            if not run.stop_cycles_equal or not run.stop_frame_equal then
                divergence('stop', {cycles_equal = run.stop_cycles_equal, frame_equal = run.stop_frame_equal})
            end
        end
        clear()
        -- Already outside Vsync dispatch; nested nextTick callbacks are discarded.
        if current < runs then begin()
        else
            result.runs_completed = current
            m.write('state.proto', snapshot.schema)
            m.report('replay.json', result)
            m.finish()
        end
    end
    local function condition(event)
        if not event.condition then return nil end
        return require('condition').observe(event.condition)
    end
    advance = function()
        while index <= #active.events and active.events[index].frame == frame do
            local event = active.events[index]
            index = index + 1
            if event.action == 'sample' then sample(event)
            elseif event.action == 'screen' then screen(event)
            elseif event.action == 'buttons' then
                local pad = PCSX.SIO0.slots[event.slot].pads[1]
                for _, button in ipairs(held[event.slot]) do pad.clearOverride(button) end
                held[event.slot] = event.buttons
                for _, button in ipairs(event.buttons) do pad.setOverride(button) end
            else
                local observation = condition(event)
                if observation and not observation.matched then
                    clear()
                    m.report('condition.json', {schema = 'psx.runtime-condition/v1', frame = frame,
                        run = current, observation = observation, registers = m.registers()})
                    error('stop condition unmet at frame deadline')
                end
                stop(event, observation)
                return
            end
        end
        local nextEvent = active.events[index]
        if nextEvent and nextEvent.action == 'stop' and nextEvent.condition and frame >= 1 then
            local observation = condition(nextEvent)
            if observation.matched then stop(nextEvent, observation); return end
        end
        PCSX.resumeEmulator()
    end
    m.event('GPU::Vsync', function()
        PCSX.pauseEmulator()
        frame = frame + 1
        assert(frame <= active.frames, 'replay frame bound exceeded')
        PCSX.nextTick(m.guard(advance))
    end)
    begin()
end)
