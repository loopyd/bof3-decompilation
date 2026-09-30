-- Check native timer gates/overflow/read effects and the CPU IRQ acceptance boundary.
local m, events, bit = require 'support', require 'events', require 'bit'
m.begin('scenario,stop', function()
    local scenario, stop = m.argument('scenario'), m.address('stop')
    local h, timer = PCSX.History, luv.new_timer()
    m.retained[#m.retained + 1] = timer
    timer:start(15000, 0, m.guard(function() error('timing fixture timeout') end))
    local function check()
        timer:stop(); timer:close(); h.stop()
        local report = events.export()
        assert(report.complete)
        local function select(kind, device)
            local rows = {}
            for i = 0, report.events - 1 do
                local r = h.record(i)
                if r.kind == kind and (device == nil or r.device == device) then rows[#rows + 1] = r end
            end
            return rows
        end
        if scenario == 'timers' then
            local horizontal, vertical = select(19, 0), select(19, 1)
            assert(#horizontal > 200 and #vertical >= 1, 'blank gates absent')
            for _, r in ipairs(horizontal) do assert(r.data[7] == 1) end
            for _, r in ipairs(vertical) do assert(r.data[7] == 2) end
            local before, after = select(15, 2), select(16, 2)
            assert(#before > 1 and #before == #after)
            for i, r in ipairs(before) do
                assert(bit.band(after[i].data[0], 0x1000) ~= 0, 'overflow flag absent')
                assert(bit.bxor(bit.band(r.data[0], 0x400), bit.band(after[i].data[0], 0x400)) == 0x400,
                    'timer toggle flag did not change')
                assert(after[i].sequence > r.sequence)
            end
            local reads = select(17, 2)
            assert(#reads == 3 and reads[1].data[0] == 4 and reads[2].data[0] == 4 and reads[3].data[0] == 8)
            assert(bit.band(reads[1].data[1], 0x1000) ~= 0 and bit.band(reads[1].data[2], 0x1800) == 0)
            assert(bit.band(reads[2].data[1], 0x1800) == 0 and reads[3].data[1] == 0xffff)
            assert(reads[1].data[4] == 16 and reads[3].data[4] == 32)
            assert(#select(14) == 0, 'masked IRQ entered CPU')
        elseif scenario == 'gpuirq' then
            local assertions, clears = select(12), select(13)
            assert(#assertions == 0 and select(25)[1].data[0] == 2, 'fixture IRQ initial latch differs')
            local cleared = false
            for _, r in ipairs(clears) do
                if r.data[0] == 2 then
                    assert(r.data[1] == 2 and r.data[2] == 0)
                    cleared = true
                end
            end
            assert(cleared and #select(14) == 0, 'GPU device acknowledgment absent')
        elseif scenario == 'irq' then
            local accepts, assertions = select(14), select(12)
            assert(#accepts == 1 and #assertions >= 1)
            local accept = accepts[1]
            assert(bit.band(accept.data[0], accept.data[1], 16) ~= 0 and
                bit.band(accept.data[2], 0x401) == 0x401)
            assert(accepts[1].sequence > assertions[1].sequence)
            local regs = PCSX.getRegisters()
            assert(regs.pc == 0x80000080 and bit.band(regs.CP0.r[13], 0x7c) == 0 and
                regs.CP0.r[14] == accept.data[3], 'hardware exception context differs')
        else error('unknown timing scenario') end
        m.report('checks.json', {schema = 'psx.skill-checks/v1', scenario = scenario, passed = 3,
            checks = scenario == 'timers' and {'Hblank/Vblank gates', 'repeat overflow/toggle', 'read-clear widths and CPU mask'}
                or (scenario == 'gpuirq' and {'initial GPU latch', 'device acknowledgment', 'CPU masked'}
                    or {'IRQ asserted before CPU acceptance', 'CPU/mask gate values', 'exception vector/Cause/EPC'})})
        m.write('state.pbuf', tostring(PCSX.createSaveState())); m.finish()
    end
    m.breakpoint(stop, 'Exec', 4, function()
        PCSX.pauseEmulator(); timer:stop(); timer:start(1, 0, m.guard(check)); return false
    end)
    m.atTarget(function()
        PCSX.pauseEmulator()
        PCSX.nextTick(m.guard(function()
            if scenario == 'gpuirq' then
                -- Native restore replays GPU acknowledge; establish this fixture latch afterward.
                local ffi = require 'ffi'
                ffi.cast('uint32_t*', ffi.cast('uint8_t*', PCSX.getScratchPtr()) + 0x1070)[0] = 2
            end
            h.begin(16384, 0); PCSX.resumeEmulator()
        end))
    end)
end)
