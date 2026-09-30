-- Prepare guest-only device fixture programs before any recording begins.
local m, s, ffi = require 'support', require 'snapshot', require 'ffi'
m.begin('scenario', function()
    PCSX.pauseEmulator()
    local scenario = m.argument('scenario', 'otc')
    assert(scenario == 'otc' or scenario == 'gpu' or scenario == 'spu' or
        scenario == 'unsupported' or scenario == 'loop' or scenario == 'mdec' or
        scenario == 'cd' or scenario == 'gpuread' or scenario == 'timers' or scenario == 'irq' or
        scenario == 'cyclic' or scenario == 'gpuirq' or scenario == 'msan' or
        scenario == 'survivor' or scenario == 'cancel' or scenario == 'sound' or scenario == 'gpuview' or
        scenario == 'gpuports' or scenario == 'gpureset' or scenario == 'cdda' or scenario == 'mix' or scenario == 'xa' or
        scenario == 'cdempty' or scenario == 'cdpending',
        'unknown history fixture')
    local state = s.live()
    state.registers.pc, state.registers.cp0[13], state.registers.next_is_delay_slot = 0x80010000, 0, false
    for _, name in ipairs({'delay_slot_info_1', 'delay_slot_info_2'}) do
        state.registers[name] = state.registers[name] or {}
        state.registers[name].active, state.registers[name].pc_active = false, false
    end
    if scenario == 'cd' or scenario == 'cdpending' then
        local transfer = {}
        for i = 0, 2351 do transfer[#transfer + 1] = string.char(i % 251) end
        state.cdrom.transfer, state.cdrom.transfer_index = table.concat(transfer), 2056
        state.cdrom.read, state.cdrom.mode, state.cdrom.ctrl = 1, 0, 0
    end
    m.write('seed.pbuf', require('pb').encode('SaveState', state))
    local file = Support.File.open('seed.pbuf'); PCSX.loadSaveState(file); file:close()
    local words, checkpoint = {}, nil
    local function emit(value) words[#words + 1] = value end
    local function constant(reg, value)
        emit(0x3c000000 + reg * 65536 + math.floor(value / 65536))
        emit(0x34000000 + reg * 0x210000 + value % 65536)
    end
    local function store(address, value, width)
        constant(8, address); constant(9, value)
        emit(width == 8 and 0xa1090000 or (width == 16 and 0xa5090000 or 0xad090000))
    end
    local function delay(count)
        for _ = 1, count do emit(0x10000001); emit(0) end
    end
    local function interval(count)
        constant(11, count)
        emit(0x256bffff); emit(0x1560fffe); emit(0)
    end
    store(0xfffe0130, 0x1e988)
    if scenario ~= 'gpuirq' then store(0x1f801070, 0) end
    store(0x1f801074, 0)
    store(0x1f8010f0, 0x08888888)
    if scenario == 'cdda' or scenario == 'mix' then
        -- Clear controller IRQ/parameters, then issue Play(track 2) as guest MMIO.
        store(0x1f801800, 1, 8); store(0x1f801803, 0x5f, 8)
        store(0x1f801800, 0, 8); store(0x1f801802, 2, 8); store(0x1f801801, 3, 8)
    end
    if scenario == 'otc' then
        store(0x1f8010f4, 0xc00000)
        store(0x1f801108, 64, 16); store(0x1f801104, 0x58, 16)
        store(0x1f8010e0, 0x3000c); store(0x1f8010e4, 4); store(0x1f8010e8, 0x11000002)
        delay(300)
        constant(8, 0x1f801104); emit(0x950a0000); emit(0)
        store(0x1f801070, 0); store(0x1f801104, 0, 16)
    elseif scenario == 'gpu' or scenario == 'gpuread' or scenario == 'gpuports' or scenario == 'cyclic' then
        store(0x1f801814, 0x04000002)
        store(0x1f8010a0, 0x30000); store(0x1f8010a4, 0x10004)
        store(0x1f8010a8, 0x01000201); delay(100)
        store(0x1f8010a0, 0x30100); store(0x1f8010a8, 0x01000401); delay(100)
        if scenario == 'gpuread' or scenario == 'gpuports' then
            store(0x1f801810, 0xc0000000); store(0x1f801810, 0)
            store(0x1f801810, 0x00010004); store(0x1f801814, 0x04000003)
            store(0x1f8010a0, 0x30200); store(0x1f8010a4, 0x10002)
            store(0x1f8010a8, 0x01000200); delay(100)
        end
        if scenario == 'gpuports' then
            for _, value in ipairs({0xc0000000, 0, 0x00010002}) do store(0x1f801810, value) end
            constant(8, 0x1f801810); emit(0x8d0a0000); emit(0)
            constant(8, 0x80035000); emit(0xad0a0000)
            store(0x1f801814, 0x10000003)
            constant(8, 0x1f801810); emit(0x8d0a0000); emit(0)
            constant(8, 0x80035000); emit(0xad0a0004)
            constant(8, 0x1f801814); emit(0x8d0a0000); emit(0)
            constant(8, 0x80035000); emit(0xad0a0008)
        end
    elseif scenario == 'gpureset' then
        store(0x1f801810, 0x020000ff)
        store(0x1f801814, 0)
    elseif scenario == 'gpuview' then
        -- Establish a visible red display before the capture-start checkpoint.
        for _, value in ipairs({0, 0x08000001, 0x05000000, 0x06c60260, 0x07040010, 0x03000000}) do
            store(0x1f801814, value)
        end
        for _, value in ipairs({0x020000ff, 0, 0x00f00140}) do store(0x1f801810, value) end
        interval(2000000)
        checkpoint = 0x80010000 + #words * 4
        for _, value in ipairs({0xe3000000, 0xe407ffff, 0xe5000000,
            0x0200ff00, 0, 0x00f00140, 0x020000ff}) do store(0x1f801810, value) end
        -- Complete the pending fill through DMA, with GP1 interleaved before it.
        store(0x1f801814, 0x04000002)
        store(0x1f8010a0, 0x30700); store(0x1f8010a4, 0x10002)
        store(0x1f8010a8, 0x01000201); delay(100)
        for _, value in ipairs({0xa0000000, 0, 0x00010002, 0x7c1f7c00,
            0x60ffffff, 0x00200040, 0x00080008}) do store(0x1f801810, value) end
        interval(2000000)
    elseif scenario == 'sound' or scenario == 'mix' then
        store(0x1f801daa, 0xc000, 16)
        store(0x1f801d80, 0x3fff3fff)
        store(0x1f801da6, 0x0200, 16)
        -- Native decoder repeats flags=3 through the explicit repeat register.
        for _, value in ipairs({0x0303, 0x3210, 0x7654, 0x4567, 0x0123, 0xcdef, 0x89ab, 0xba98}) do
            store(0x1f801da8, value, 16)
        end
        store(0xbf801c00, 0x1fff3fff)
        store(0x1f801c04, 0x1000, 16); store(0x1f801c06, 0x0200, 16)
        store(0x1f801c08, 0x000f, 16); store(0x1f801c0a, 0, 16)
        store(0x1f801c0e, 0x0200, 16); store(0x1f801d88, 1, 16)
        interval(800000)
        store(0x1f801c04, 0x2000, 16)
        constant(8, 0x1f801c04); emit(0x950a0000); emit(0)
        constant(8, 0x80035000); emit(0xad0a0000)
        constant(8, 0xbf801c00); emit(0x8d0c0000); emit(0)
        constant(8, 0x80035000); emit(0xad0c0004)
        interval(800000)
        store(0x1f801d8c, 1, 16)
        interval(800000)
    elseif scenario == 'xa' then
        local function acknowledge()
            store(0x1f801800, 1, 8); store(0x1f801803, 0x5f, 8); store(0x1f801800, 0, 8)
        end
        acknowledge()
        store(0x1f801802, 0x40, 8); store(0x1f801801, 0x0e, 8)
        interval(100000); acknowledge()
        for _, value in ipairs({0,2,0}) do store(0x1f801802, value, 8) end
        store(0x1f801801, 2, 8)
        interval(100000); acknowledge()
        store(0x1f801801, 6, 8)
        interval(100000); acknowledge()
        interval(2400000)
    elseif scenario == 'cdda' then
        interval(2400000)
    elseif scenario == 'spu' then
        store(0x1f801da6, 0xffff, 16)
        store(0x1f8010c0, 0x30000); store(0x1f8010c4, 0x10004)
        store(0x1f8010c8, 0x01000201); delay(100)
        store(0x1f801da6, 0xffff, 16)
        store(0x1f8010c0, 0x30200); store(0x1f8010c4, 0x10004)
        store(0x1f8010c8, 0x01000200); delay(100)
    elseif scenario == 'mdec' then
        store(0x1f801824, 0x80000000)
        store(0x1f801090, 0x30400); store(0x1f801094, 0x10020)
        store(0x1f801098, 0x01000200)
        store(0x1f801820, 0x38000006)
        store(0x1f801080, 0x30300); store(0x1f801084, 0x10006)
        store(0x1f801088, 0x01000201)
        checkpoint = 0x80010000 + #words * 4
        -- Clear/restart DMA0 while the decoder and DMA1 callback still own older work.
        store(0x1f801088, 0)
        store(0x1f801820, 0x48000001)
        store(0x1f801080, 0x30600); store(0x1f801084, 0x10020)
        store(0x1f801088, 0x01000201)
        -- Consume the retained partial block, replacing the scheduled DMA1 callback.
        store(0x1f801098, 0)
        store(0x1f801090, 0x30480); store(0x1f801094, 0x10060)
        store(0x1f801098, 0x01000200)
        delay(1000)
    elseif scenario == 'cdempty' then
        store(0x1f8010b0, 0x31000); store(0x1f8010b4, 1); store(0x1f8010b8, 0x11000000)
        store(0x1f8010b8, 0x11400100); delay(100)
    elseif scenario == 'cdpending' then
        -- The first transfer clears ready but leaves completion scheduled.
        -- A later immediate request clears busy before that old callback runs.
        store(0x1f8010b0, 0x31000); store(0x1f8010b4, 1024); store(0x1f8010b8, 0x11000000)
        store(0x1f8010b4, 1); store(0x1f8010b8, 0x11400100); delay(1000)
    elseif scenario == 'cd' then
        store(0x1f8010b0, 0x31000); store(0x1f8010b4, 2)
        store(0x1f8010b8, 0x11000000); delay(100)
        store(0x1f801800, 0, 8); store(0x1f801803, 0x80, 8)
        store(0x1f8010b0, 0x32000); store(0x1f8010b4, 0)
        store(0x1f8010b8, 0x11400100); delay(200)
    elseif scenario == 'timers' then
        store(0x1f801104, 3, 16); store(0x1f801114, 3, 16)
        store(0x1f801128, 0xffff, 16); store(0x1f801124, 0xe0, 16)
        store(0x1f801120, 65530, 16)
        constant(11, 100000)
        emit(0x256bffff); emit(0x1560fffe); emit(0)
        constant(8, 0x1f801124); emit(0x950a0000); emit(0); emit(0x950a0000); emit(0)
        constant(8, 0x1f801128); emit(0x8d0a0000); emit(0)
    elseif scenario == 'irq' then
        store(0x1f801074, 16)
        store(0x1f801108, 64, 16); store(0x1f801104, 0x58, 16)
        constant(9, 0x401); emit(0x40896000)
        delay(100)
    elseif scenario == 'gpuirq' then
        store(0x1f801814, 0x02000000)
    elseif scenario == 'msan' then
        store(0x1f802089, 0, 8)
    elseif scenario == 'survivor' then
        store(0x1f801824, 0x80000000)
        store(0x1f801820, 0x48000001)
        store(0x1f801080, 0x30600); store(0x1f801084, 0x10020)
        store(0x1f801088, 0x01000201)
        store(0x1f801088, 0)
        store(0x1f801820, 0x38000006)
        store(0x1f801080, 0x30300); store(0x1f801084, 0x10006)
        store(0x1f801088, 0x01000201)
        delay(20)
        store(0x1f801090, 0x30400); store(0x1f801094, 0x10080)
        store(0x1f801098, 0x01000200); delay(300)
    elseif scenario == 'cancel' then
        store(0x1f801824, 0x80000000)
        store(0x1f801090, 0x30400); store(0x1f801094, 0x10020)
        store(0x1f801098, 0x01000200)
        store(0x1f801824, 0x80000000)
        store(0x1f801820, 0x38000006)
        store(0x1f801080, 0x30300); store(0x1f801084, 0x10006)
        store(0x1f801088, 0x01000201)
        store(0x1f801098, 0); store(0x1f801094, 0x10020)
        store(0x1f801098, 0x01000200)
        store(0x1f801824, 0x80000000); delay(200)
    elseif scenario == 'unsupported' then
        store(0x1f8010d8, 0x01000201)
    end
    local stop = 0x80010000 + #words * 4
    emit(0x08000000 + (stop % 0x10000000) / 4); emit(0)
    local ram = ffi.cast('uint8_t*', PCSX.getMemPtr())
    local buffer = ffi.new('uint32_t[?]', #words)
    for i, word in ipairs(words) do buffer[i - 1] = word end
    ffi.copy(ram + 0x10000, buffer, #words * 4)
    local payload = ffi.new('uint32_t[4]', {0x020000ff, 0, 0x00100010, 0})
    ffi.copy(ram + 0x30000, payload, 16)
    local list = ffi.new('uint32_t[5]', {0x04ffffff, 0x0200ff00, 0x00200020, 0x00100010, 0})
    if scenario == 'cyclic' then list[0] = 0x04030100 end
    ffi.copy(ram + 0x30100, list, 20)
    local compressed = ffi.new('uint32_t[6]')
    for i = 0, 5 do compressed[i] = 0xfe000000 end
    ffi.copy(ram + 0x30300, compressed, 24)
    ffi.fill(ram + 0x30600, 128, 1)
    local continuation = ffi.new('uint32_t[2]', {0x00100010, 0x00100010})
    ffi.copy(ram + 0x30700, continuation, 8)
    PCSX.invalidateCache()
    m.write('state.pbuf', tostring(PCSX.createSaveState()))
    m.report('fixture.json', {schema = 'psx.history-fixture/v1', scenario = scenario,
        entry = 0x80010000, stop = scenario == 'irq' and 0x80000080 or stop, checkpoint = checkpoint,
        words = #words, expected_otc_bytes = 20})
    m.finish()
end)
