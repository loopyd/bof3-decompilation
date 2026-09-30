-- A bounded data-only schedule: source order defines events sharing a Vsync.
local M = {}
function M.read(path)
    local file = assert(io.open(path, 'rb'))
    local text = assert(file:read(65537)); file:close()
    assert(#text <= 65536 and not text:find('\0', 1, true), 'schedule exceeds 64 KiB or contains NUL')
    local events, samples, previous, stopped = {}, {}, 0, false
    local lineNumber = 0
    for line in (text .. '\n'):gmatch('(.-)\n') do
        line = line:gsub('\r$', '')
        lineNumber = lineNumber + 1
        if lineNumber == 1 then assert(line == 'psx.schedule/v1', 'unsupported schedule header')
        elseif not line:match('^%s*$') then
            assert(not stopped and #events < 512, 'events after stop or schedule exceeds 512 events')
            local words = {}
            for word in line:gmatch('%S+') do words[#words + 1] = word end
            local frame = tonumber(words[1])
            assert(words[1]:match('^%d+$') and frame >= previous and frame <= 36000,
                'schedule frames must be monotonic integers in 0..36000')
            local event = {frame = frame, action = words[2]}
            if event.action == 'sample' or event.action == 'screen' then
                assert(#samples < 4, 'at most four checkpoints')
                if event.action == 'sample' then
                    assert(#words == 3, 'sample requires one schema path')
                    event.path = words[3] == '.' and '' or words[3]
                else assert(#words == 2, 'screen takes only a frame') end
                event.index = #samples + 1
                samples[event.index] = event
            elseif event.action == 'buttons' then
                local slot = tonumber(words[3])
                assert(#words == 4 and (slot == 1 or slot == 2), 'buttons requires slot 1|2 and names or -')
                event.slot, event.buttons, event.names = slot, {}, words[4]
                if words[4] ~= '-' then
                    local names, seen = {}, {}
                    for name in words[4]:gmatch('[^,]+') do
                        local button = assert(PCSX.CONSTS.PAD.BUTTON[name], 'unknown button: ' .. name)
                        assert(not seen[name], 'duplicate button: ' .. name)
                        names[#names + 1], event.buttons[#event.buttons + 1], seen[name] = name, button, true
                    end
                    assert(table.concat(names, ',') == words[4], 'invalid comma-separated buttons')
                end
            elseif event.action == 'stop' then
                assert(frame >= 1, 'stop requires a frame in 1..36000')
                if #words > 2 then event.condition = require('condition').parse(words) end
                stopped = true
            else error('unknown schedule action: ' .. tostring(event.action)) end
            previous, events[#events + 1] = frame, event
        end
    end
    assert(stopped and #samples > 0, 'schedule requires samples and a final stop')
    return {events = events, samples = samples, frames = previous}
end

function M.compatible(a, b)
    assert(a.frames == b.frames and #a.samples == #b.samples, 'replay schedules have different checkpoint boundaries')
    for i, sample in ipairs(a.samples) do
        assert(sample.frame == b.samples[i].frame and sample.path == b.samples[i].path
            and sample.action == b.samples[i].action,
            'replay schedules have different checkpoint boundaries')
    end
end
return M
