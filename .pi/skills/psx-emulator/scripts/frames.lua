-- Advance counted Vsync events, optionally holding controller buttons and capturing display pixels.
local m = require 'support'
m.begin('target,frames,buttons,slot,screenshot,savestate', function()
    local count = m.integer('frames', 1, 1, 36000)
    local slot = m.integer('slot', 1, 1, 2)
    local screenshot = m.integer('screenshot', 0, 0, 1) == 1
    local save = m.integer('savestate', 0, 0, 1) == 1
    local buttons, names = {}, m.argument('buttons', '')
    local rebuilt = {}
    for name in names:gmatch('[^,]+') do
        local button = assert(PCSX.CONSTS.PAD.BUTTON[name], 'unknown button: ' .. name)
        buttons[#buttons + 1], rebuilt[#rebuilt + 1] = button, name
    end
    assert(table.concat(rebuilt, ',') == names, 'buttons must be comma separated names')
    m.atTarget(function()
        local pad, observed = PCSX.SIO0.slots[slot].pads[1], 0
        for _, button in ipairs(buttons) do pad.setOverride(button) end
        local before = m.registers()
        m.event('GPU::Vsync', function()
            observed = observed + 1
            if observed ~= count then return end
            PCSX.pauseEmulator()
            local pressed = {}
            for i, button in ipairs(buttons) do
                pressed[rebuilt[i]] = pad.getButton(button)
                pad.clearOverride(button)
            end
            -- Finish outside event dispatch with emulation paused.
            PCSX.nextTick(m.guard(function()
                m.capture(save)
                if screenshot then
                    local screen = PCSX.GPU.takeScreenShot()
                    local bytes = tostring(screen.data)
                    local bpp = tonumber(screen.bpp)
                    assert(bpp == 0 or bpp == 1, 'unsupported screenshot pixel format')
                    assert(#bytes == screen.width * screen.height * (bpp == 0 and 2 or 3),
                        'unexpected screenshot storage')
                    m.write('screen.bin', bytes)
                    m.report('screen.json', {schema = 'psx.runtime-screen/v1', width = screen.width,
                        height = screen.height, format = bpp == 0 and 'psx-bgr555-le' or 'rgb24',
                        bytes = #bytes, available = #bytes > 0})
                    if #bytes > 0 then
                        local rgb = bytes
                        if bpp == 0 then
                            local pixels = {}
                            for i = 1, #bytes, 2 do
                                local pixel = bytes:byte(i) + bytes:byte(i + 1) * 256
                                local function expand(v) return math.floor(v * 255 / 31 + 0.5) end
                                pixels[#pixels + 1] = string.char(expand(pixel % 32),
                                    expand(math.floor(pixel / 32) % 32), expand(math.floor(pixel / 1024) % 32))
                            end
                            rgb = table.concat(pixels)
                        end
                        m.write('screen.ppm', 'P6\n' .. screen.width .. ' ' .. screen.height .. '\n255\n' .. rgb)
                    end
                end
                m.report('frames.json', {schema = 'psx.runtime-frames/v1', vsyncs = observed,
                    slot = slot, buttons = names, pressed_before_release = pressed,
                    before = before, after = m.registers()})
                m.finish()
            end))
        end)
        PCSX.resumeEmulator()
    end)
end)
