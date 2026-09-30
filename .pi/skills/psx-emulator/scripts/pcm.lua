-- Export frozen native PCM, callback boundaries and already-named SPU records.
local m, wave, bit = require 'support', require 'wave', require 'bit'
local M = {}
local faults = {'frames', 'blocks', 'events', 'thread', 'bounds', 'clock',
    'discontinuity', 'streaming-loss', 'output'}

local function integer(value, low, high, label)
    assert(type(value) == 'number' and value % 1 == 0 and value >= low and value <= high,
        'invalid native ' .. label)
    return value
end

local function decimal(value)
    assert(type(value) == 'string' and value:match('^%d+$') and
        (value == '0' or value:sub(1, 1) ~= '0') and
        (#value < 20 or (#value == 20 and value <= '18446744073709551615')),
        'native uint64 must be a canonical decimal string')
    return value
end

local function less(a, b)
    return #a < #b or (#a == #b and a < b)
end

local function sequence(name, count, read, validate)
    local file = assert(io.open(name, 'wb'))
    local bytes, rows = 0, 0
    local ok, err = pcall(function()
        for index = 0, count - 1 do
            local record = read(index)
            -- Retain the offending native row as evidence if semantic checks fail.
            local line = m.json(record) .. '\n'
            assert(bytes + #line <= 16777216, 'audio journal exceeds 16 MiB')
            assert(file:write(line)); bytes, rows = bytes + #line, rows + 1
            validate(record, index)
        end
    end)
    local closed, why = file:close()
    m.captures[#m.captures + 1] = {path = name, bytes = bytes}
    assert(closed, why)
    return {rows = rows, bytes = bytes, complete = ok, error = not ok and tostring(err) or nil}
end

function M.export()
    local h = assert(PCSX.Audio, 'native audio capture binding required')
    local status = h.status()
    local result = {schema = 'psx.native-audio/v1', native = status, complete = false,
        failures = m.array(), timing = {
            pcm = 'stereo frame offsets at the SDL input boundary, including underflow zero-fill',
            blocks = 'last CPU publication observed by callback; not per-sample cycles',
            events = 'CPU access cycles with observed capture cursor; not audible-effect positions',
            queues = 'begin/end snapshots; existing queued and partial mixer work is not flushed',
            stop = 'freezes output observations; does not drain queues or finish a DSP tail'}}
    local ok, err = pcall(function()
        assert(status.version == 1 and status.state == 2 and status.rate == 44100 and
            status.channels == 2 and status.format == 'f32le' and
            status.stage == 'post-mixer-mute-mono/pre-SDL-conversion', 'unsupported frozen audio ABI')
        integer(status.frame_limit, 1, 2646000, 'frame limit')
        integer(status.block_limit, 1, 65536, 'block limit')
        integer(status.event_limit, 1, 65536, 'event limit')
        integer(status.frames, 0, status.frame_limit, 'frame count')
        integer(status.blocks, 0, status.block_limit, 'block count')
        integer(status.events, 0, status.event_limit, 'event count')
        integer(status.failures, 0, 511, 'failure bits')
        integer(status.first_failure, 0, 256, 'first failure')
        integer(status.dropped_events, 0, 4294967295, 'dropped events')
        for _, key in ipairs({'begin_cycle', 'end_cycle', 'dropped_frames', 'streaming_dropped'}) do
            decimal(status[key])
        end
        for _, key in ipairs({'initial_voice_queue', 'initial_disc_queue', 'final_voice_queue', 'final_disc_queue'}) do
            integer(status[key], 0, 4294967295, key)
        end
        assert(type(status.settings) == 'table', 'missing native mixer settings')
        for _, key in ipairs({'volume', 'interpolation', 'reverb', 'mute', 'mono', 'streaming',
            'null_sync', 'irq_wait', 'decode_irq', 'scaler', 'forced_irq', 'voice_mute', 'voice_solo', 'xa'}) do
            integer(status.settings[key], 0, 4294967295, 'setting ' .. key)
        end
        for _, key in ipairs({'configured_backend', 'configured_device', 'active_driver'}) do
            assert(type(status.settings[key]) == 'string', 'missing native ' .. key)
        end
        for i, name in ipairs(faults) do
            if bit.band(status.failures, 2 ^ (i - 1)) ~= 0 then result.failures[#result.failures + 1] = name end
        end
        assert((status.failures == 0 and status.first_failure == 0) or
            (status.first_failure > 0 and bit.band(status.first_failure, status.first_failure - 1) == 0 and
            bit.band(status.failures, status.first_failure) ~= 0), 'inconsistent native first failure')
        assert(status.failures ~= 0 or (status.dropped_events == 0 and status.dropped_frames == '0' and
            status.streaming_dropped == '0'), 'native loss without failure')

        local chunks = {}
        for offset = 0, status.frames - 1, 65536 do
            local frames = math.min(65536, status.frames - offset)
            local bytes = h.samples(offset, frames)
            assert(type(bytes) == 'string' and #bytes == frames * 8, 'short native PCM copy')
            chunks[#chunks + 1] = bytes
        end
        local pcm = table.concat(chunks)
        m.write('audio.f32', pcm); m.write('audio.wav', wave.encode(pcm))
        result.pcm_bytes, result.duration = #pcm, status.frames / 44100
        local offset, previous = 0, status.begin_cycle
        local voice_fill, disc_fill = 0, 0
        result.blocks = sequence('blocks.ndjson', status.blocks, h.block, function(b, index)
            assert(b.index == index and b.offset == offset, 'noncontiguous native PCM block')
            integer(b.frames, 1, 2048, 'block frames')
            integer(b.voice_frames, 0, b.frames, 'voice frames')
            integer(b.disc_frames, 0, b.frames, 'disc frames')
            integer(b.flags, 0, 7, 'block flags')
            local cycle = decimal(b.observed_cpu_cycle)
            assert(not less(cycle, previous), 'native block clock regression')
            -- Clock failure may freeze with an end cycle before the retained prefix.
            assert(bit.band(status.failures, 32) ~= 0 or not less(status.end_cycle, cycle),
                'native block after end cycle')
            assert(bit.band(b.flags, 4) ~= 0 or bit.band(status.failures, 256) ~= 0,
                'failed SDL submission without native failure')
            offset, previous = offset + b.frames, cycle
            assert(offset <= status.frames, 'block exceeds retained PCM')
            voice_fill, disc_fill = voice_fill + b.frames - b.voice_frames, disc_fill + b.frames - b.disc_frames
        end)
        result.zero_fill = {voice_frames = voice_fill, disc_frames = disc_fill}
        previous = status.begin_cycle
        local observed = '0'
        result.events = sequence('spu.ndjson', status.events, h.event, function(e, index)
            assert(e.id == index + 1, 'noncontiguous native SPU event')
            integer(e.kind, 1, 6, 'event kind')
            local cycle, frame, parent = decimal(e.cycle), decimal(e.observed_frame), decimal(e.parent)
            assert(not less(cycle, previous) and not less(frame, observed) and
                not less(tostring(status.frames), frame), 'native event cursor regression or overflow')
            assert(bit.band(status.failures, 32) ~= 0 or not less(status.end_cycle, cycle),
                'native event after end cycle')
            assert(less(parent, tostring(e.id)), 'native event parent must be absent or earlier')
            previous, observed = cycle, frame
            if e.kind == 3 then
                for _, key in ipairs({'source_rate', 'output_frames', 'source_frames'}) do
                    integer(e[key], 0, 4294967295, key)
                end
                for _, key in ipairs({'stereo', 'accepted', 'cdda'}) do integer(e[key], 0, 1, key) end
            else
                integer(e.address, 0, 4294967295, 'event address')
                integer(e.value, 0, 4294967295, 'event value')
                assert(e.width == 8 or e.width == 16 or e.width == 32, 'invalid native SPU access width')
            end
        end)
        assert(result.blocks.complete and result.events.complete, 'incomplete native journal export')
        assert(offset == status.frames, 'native PCM not covered by blocks')
        assert(bit.band(status.failures, 32) ~= 0 or not less(status.end_cycle, status.begin_cycle),
            'native end clock regression without failure')
        result.complete = status.failures == 0
    end)
    if not ok then result.export_error = tostring(err) end
    m.report('audio.json', result)
    return result
end

return M
