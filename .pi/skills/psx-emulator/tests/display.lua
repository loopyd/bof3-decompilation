-- Establish visible native pixels, then compare an actual GPU colour change.
local m, ffi = require 'support', require 'ffi'
local display, bus = require 'display', require 'bus'
m.begin('scratch', function()
    PCSX.pauseEmulator()
    local r = PCSX.getRegisters()
    r.pc, r.CP0.r[12] = 0x80010000, 0
    local ram = ffi.cast('uint8_t*', PCSX.getMemPtr())
    local code = ffi.cast('uint32_t*', ram + 0x10000)
    code[0], code[1] = 0x08004000, 0
    PCSX.invalidateCache()
    local function park(name)
        local saved = require('snapshot').live()
        saved.registers.next_is_delay_slot = false
        saved.registers.delay_slot_info_1 = saved.registers.delay_slot_info_1 or {}
        saved.registers.delay_slot_info_2 = saved.registers.delay_slot_info_2 or {}
        saved.registers.delay_slot_info_1.active, saved.registers.delay_slot_info_1.pc_active = false, false
        saved.registers.delay_slot_info_2.active, saved.registers.delay_slot_info_2.pc_active = false, false
        saved.registers.pc = 0x80010000
        m.write(name, require('pb').encode('SaveState', saved))
        local file = Support.File.open(name)
        PCSX.loadSaveState(file); file:close()
    end
    park('initial.pbuf')
    local operations = {
        {0x1f801814, 0}, {0x1f801814, 0x08000001},
        {0x1f801814, 0x05000000}, {0x1f801814, 0x06c60260},
        {0x1f801814, 0x07040010}, {0x1f801814, 0x03000000},
        {0x1f801810, 0x020000ff}, {0x1f801810, 0}, {0x1f801810, 0x00f00140}}
    bus.write(32, operations, function()
        local count = 0
        m.event('GPU::Vsync', function()
            count = count + 1
            if count ~= 2 then return end
            PCSX.pauseEmulator()
            PCSX.nextTick(m.guard(function()
                local red, info = display.capture()
                assert(info.available and info.width == 320 and info.height == 240, 'visible 320x240 display required')
                assert(info.format == 'psx-bgr555-le' and red == string.rep('\31\0', 320 * 240), 'red native display differs')
                m.write('red.bin', red)
                m.write('state.pbuf', tostring(PCSX.createSaveState()))
                -- Vsync can interrupt a delay slot; use the parked probe's original context.
                park('probe.pbuf')
                bus.write(32, {{0x1f801810, 0x0200ff00}, {0x1f801810, 0}, {0x1f801810, 0x00f00140}}, function()
                    local green, other = display.capture()
                    assert(green == string.rep('\224\3', 320 * 240), 'green native display differs')
                    local comparison = display.compare(red, green, info, other)
                    assert(not comparison.equal and comparison.image_comparison_available)
                    assert(comparison.pixels.changed_bytes == 320 * 240 * 2)
                    m.write('green.bin', green)
                    m.report('checks.json', {schema = 'psx.skill-checks/v1', passed = 3,
                        checks = {'native red display', 'native green display', 'exact pixel difference'}, comparison = comparison})
                    m.finish()
                end)
            end))
        end)
        PCSX.resumeEmulator()
    end)
end)
