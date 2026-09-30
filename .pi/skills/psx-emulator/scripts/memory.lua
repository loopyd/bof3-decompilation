-- Inspect or patch physical RAM/scratchpad without triggering mapped device reads.
local ffi = require 'ffi'
local m = require 'support'
m.begin('target,region,offset,length,hex,savestate', function()
    local region = m.argument('region', 'ram')
    local regions = {ram = {2097152, PCSX.getMemPtr}, scratch = {1024, PCSX.getScratchPtr},
        rom = {524288, PCSX.getRomPtr}}
    local selected = assert(regions[region], 'region must be ram, scratch, or rom')
    local offset = m.integer('offset', 0, 0, selected[1] - 1)
    local length = m.integer('length', 256, 1, selected[1] - offset)
    local hex, patch = m.argument('hex'), nil
    if hex then
        assert(region ~= 'rom', 'ROM patches are unsupported')
        assert(#hex > 0 and #hex % 2 == 0 and hex:match('^%x+$'), 'hex requires byte pairs')
        patch = hex:gsub('..', function(byte) return string.char(tonumber(byte, 16)) end)
        assert(#patch == length, 'hex byte count must equal length')
    end
    local save = m.integer('savestate', 0, 0, 1) == 1
    m.atTarget(function()
        PCSX.pauseEmulator()
        local pointer = ffi.cast('uint8_t*', selected[2]()) + offset
        m.write('before.bin', ffi.string(pointer, length))
        if patch then
            m.requireIdleCapture()
            ffi.copy(pointer, patch, length)
            PCSX.invalidateCache()
            m.write('after.bin', ffi.string(pointer, length))
        end
        m.report('memory.json', {schema = 'psx.runtime-memory/v1', region = region,
            offset = offset, bytes = length, patched = patch ~= nil, registers = m.registers()})
        if save then m.write('state.pbuf', tostring(PCSX.createSaveState())) end
        m.finish()
    end)
end)
