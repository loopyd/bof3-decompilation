-- Configure native system registers and verify decoded state and integer boundaries.
local m, s, devices = require 'support', require 'snapshot', require 'devices'
m.begin('scratch', function()
    PCSX.pauseEmulator()
    local state = s.live()
    state.registers.pc, state.registers.cp0[13], state.registers.next_is_delay_slot = 0x80010000, 0, false
    for _, name in ipairs({'delay_slot_info_1', 'delay_slot_info_2'}) do
        state.registers[name] = state.registers[name] or {}
        state.registers[name].active, state.registers[name].pc_active = false, false
    end
    m.write('initial.pbuf', require('pb').encode('SaveState', state))
    local file = Support.File.open('initial.pbuf'); PCSX.loadSaveState(file); file:close()
    local operations = {
        {0x1f801070, 0}, {0x1f801074, 8},
        {0x1f8010a0, 0x123456}, {0x1f8010a4, 0x00030004}, {0x1f8010a8, 0x201},
        {0x1f8010f0, 0xb00}, {0x1f8010f4, 0x848000},
        {0x1f801108, 30000}, {0x1f801104, 0x50}, {0x1f801100, 0x1234},
        {0x1f801118, 500}, {0x1f801114, 0x100},
        {0x1f801128, 10000}, {0x1f801124, 0x200}}
    require('bus').write(32, operations, function(probe)
        local configured = s.live()
        local dma, irq, timers = devices.dma(configured, 2), devices.irq(configured), devices.timers(configured)
        local channel = dma.channels[1]
        assert(#dma.channels == 1 and channel.registers.madr == 0x123456 and channel.address_aligned_24 == 0x123454)
        assert(channel.direction == 'from-ram' and channel.configured_words == 12 and channel.synchronization == 1)
        assert(channel.priority == 3 and channel.enabled and not channel.busy)
        assert(dma.force_irq and dma.master_irq_enabled and dma.master_irq_flag and dma.requested_by_flags)
        assert(irq.lines[4].pending and irq.lines[4].enabled and irq.pending_enabled == 8)
        assert(not irq.cpu_global_enabled)
        assert(timers.timers[1].serialized.mode == 0x450 and timers.timers[1].serialized.target == 30000)
        assert(timers.timers[1].count_before_update >= 0x1234 and timers.timers[1].count_before_update < 30000)
        assert(timers.timers[2].clock_source == 'hblank' and timers.timers[3].serialized.rate == 8)
        m.report('devices.json', {schema = 'psx.skill-devices/v1', dma = dma, irq = irq, timers = timers, probe = probe})
        m.write('state.pbuf', tostring(PCSX.createSaveState()))
        configured.registers.cycle = '#9007199254741116'
        configured.counters.rcnts[1].cycle_start = '#9007199254740993'
        configured.counters.rcnts[1].rate = 1
        assert(devices.timers(configured, 0).timers[1].count_before_update == 123, 'uint64 precision lost')
        configured.registers.cycle = '#4'
        configured.counters.rcnts[1].cycle_start = '#18446744073709551610'
        assert(devices.timers(configured, 0).timers[1].count_before_update == 10, 'uint64 wrap differs')
        configured.counters.rcnts[1].rate = 0
        assert(not pcall(devices.timers, configured, 0), 'zero timer divisor accepted')
        configured.memory.hardware = 'short'
        assert(not pcall(devices.dma, configured), 'truncated hardware bytes accepted')
        m.report('checks.json', {schema = 'psx.skill-checks/v1', passed = 7,
            checks = {'DMA register fields', 'DMA force interrupt', 'IRQ masks and CPU eligibility',
                'native timer configuration', 'uint64 timer precision', 'uint64 wrap', 'malformed state bounds'}})
        m.finish()
    end)
end)
