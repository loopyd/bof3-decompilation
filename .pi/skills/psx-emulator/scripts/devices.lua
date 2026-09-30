-- Decode serialized system-device state without reading side-effectful native ports.
local m, bit, ffi = require 'support', require 'bit', require 'ffi'
local M = {}
local function bits(value, start, width) return bit.band(bit.rshift(value, start), 2 ^ width - 1) end
local function flag(value, index) return bits(value, index, 1) == 1 end
local function word(state, offset)
    local bytes = assert(state.memory and state.memory.hardware, 'hardware bytes absent')
    assert(#bytes == 65536, 'hardware region must contain 65536 bytes')
    return bytes:byte(offset + 1) + bytes:byte(offset + 2) * 256 +
        bytes:byte(offset + 3) * 65536 + bytes:byte(offset + 4) * 16777216
end
local function uint64(value)
    local text = tostring(value):gsub('^#', '')
    assert(text:match('^%d+$') and (#text < 20 or (#text == 20 and text <= '18446744073709551615')),
        'invalid serialized uint64')
    local result = ffi.new('uint64_t', 0)
    for digit in text:gmatch('.') do result = result * 10 + tonumber(digit) end
    return result
end

function M.dma(state, selected)
    local dpcr, dicr = word(state, 0x10f0), word(state, 0x10f4)
    local channels, names = m.array(), {'mdec-input', 'mdec-output', 'gpu', 'cdrom', 'spu', 'pio', 'otc'}
    for index = 0, 6 do
        if selected == nil or selected == index then
            local base = 0x1080 + index * 16
            local address, block, control = word(state, base), word(state, base + 4), word(state, base + 8)
            local sync, size, blocks = bits(control, 9, 2), bits(block, 0, 16), bits(block, 16, 16)
            local entry = {channel = index, device = names[index + 1],
                registers = {madr = address, bcr = block, chcr = control},
                address_24 = bits(address, 0, 24), address_aligned_24 = bit.band(address, 0xfffffc),
                direction = flag(control, 0) and 'from-ram' or 'to-ram',
                stride = flag(control, 1) and -4 or 4, synchronization = sync,
                synchronization_name = ({'manual', 'request', 'linked-list', 'reserved'})[sync + 1],
                chopping = flag(control, 8), chopping_dma_power = bits(control, 16, 3),
                chopping_cpu_power = bits(control, 20, 3), busy = flag(control, 24), trigger = flag(control, 28),
                priority = bits(dpcr, index * 4, 3), enabled = flag(dpcr, index * 4 + 3),
                irq_enabled = flag(dicr, 16 + index), irq_flag = flag(dicr, 24 + index),
                block_words_raw = size, blocks_raw = blocks}
            if sync == 0 then entry.configured_words = size == 0 and 65536 or size
            elseif sync == 1 then
                entry.configured_words = size * blocks
                entry.zero_block_field = size == 0 or blocks == 0
            end
            channels[#channels + 1] = entry
        end
    end
    return {dpcr = dpcr, dicr = dicr, channels = channels, force_irq = flag(dicr, 15),
        master_irq_enabled = flag(dicr, 23), master_irq_flag = flag(dicr, 31),
        requested_by_flags = flag(dicr, 15) or (flag(dicr, 23) and
            bit.band(bits(dicr, 16, 7), bits(dicr, 24, 7)) ~= 0),
        boundary = 'serialized register mirrors; configuration is not observed transfer progress'}
end

function M.irq(state)
    local status, mask = bits(word(state, 0x1070), 0, 16), bits(word(state, 0x1074), 0, 16)
    local cp0 = assert(state.registers and state.registers.cp0, 'CP0 bank absent')
    assert(#cp0 == 32, 'CP0 bank must contain 32 values')
    local pending, lines = bit.band(status, mask, 0x7ff), m.array()
    for index, name in ipairs({'vblank', 'gpu', 'cdrom', 'dma', 'timer0', 'timer1', 'timer2',
        'sio0', 'sio1', 'spu', 'lightpen-pio'}) do
        lines[index] = {line = index - 1, source = name, pending = flag(status, index - 1),
            enabled = flag(mask, index - 1), eligible = flag(pending, index - 1)}
    end
    return {status = status, mask = mask, pending_enabled = pending, lines = lines,
        cpu_status = cp0[13], cpu_cause = cp0[14], cpu_global_enabled = flag(cp0[13], 0),
        cpu_hardware_enabled = flag(cp0[13], 10), cpu_hardware_pending = flag(cp0[14], 10),
        cpu_masked_pending = bit.band(cp0[13], cp0[14], 0xff00),
        boundary = 'serialized level/latch state; not an assertion or acknowledgment history'}
end

function M.timers(state, selected)
    local counters = assert(state.counters and state.counters.rcnts, 'counter state absent')
    assert(#counters == 4, 'pinned runtime requires four serialized counters')
    local cycle = uint64(assert(state.registers and state.registers.cycle, 'CPU cycle absent'))
    local timers = m.array()
    for index = 0, 2 do
        if selected == nil or selected == index then
            local r, base = counters[index + 1], 0x1100 + index * 16
            assert(r.rate and r.rate >= 1 and r.rate <= 4294967295, 'invalid counter rate')
            assert(r.mode and r.mode <= 65535 and r.target and r.target <= 65535, 'invalid counter mode/target')
            local count = (cycle - uint64(assert(r.cycle_start))) / ffi.new('uint64_t', r.rate)
            local source = index == 2 and (flag(r.mode, 9) and 'system-div8' or 'system') or
                (flag(r.mode, 8) and (index == 0 and 'dotclock' or 'hblank') or 'system')
            timers[#timers + 1] = {timer = index, serialized = r,
                mirrors = {count = word(state, base), mode = word(state, base + 4), target = word(state, base + 8)},
                count_before_update = tonumber(count % 65536), count_basis = 'native readCounterInternal arithmetic before update/jitter',
                synchronization = flag(r.mode, 0), sync_mode = bits(r.mode, 1, 2),
                reset_at_target = flag(r.mode, 3), irq_at_target = flag(r.mode, 4), irq_at_overflow = flag(r.mode, 5),
                repeat_irq = flag(r.mode, 6), toggle_irq = flag(r.mode, 7), clock_source = source,
                irq_requested = not flag(r.mode, 10), reached_target = flag(r.mode, 11), overflow = flag(r.mode, 12)}
        end
    end
    return {timers = timers, system_counter = counters[4], next_counter = state.counters.psx_next_counter,
        hsync_count = state.counters.hsync_count, cpu_cycles = state.registers.cycle,
        boundary = 'serialized counters; no native update/read side effects or jitter applied'}
end
return M
