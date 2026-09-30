-- Seed isolated native READ callbacks through the emulator's saved-state schema.
-- These host-edited states exercise branches, not reachable game initialization.
local m, snapshot, ffi = require 'support', require 'snapshot', require 'ffi'
m.begin('branch',function()
    PCSX.pauseEmulator()
    local branch=m.argument('branch','inactive')
    assert(branch=='inactive' or branch=='busy' or branch=='retry' or branch=='error' or
        branch=='speed' or branch=='location','unknown READ fixture branch')
    local state=snapshot.live()
    local regs,cd=state.registers,state.cdrom
    regs.pc,regs.cp0[13],regs.next_is_delay_slot=0x80010000,0,false
    for _,name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        regs[name]=regs[name] or {};regs[name].active,regs[name].pc_active=false,false
    end
    local cycle=tonumber((tostring(regs.cycle):gsub('^#','')))
    assert(cycle and cycle>=0 and cycle<9007199254740000,'fixture cycle exceeds exact range')
    regs.interrupt=8
    regs.interrupt_targets[4]='#'..string.format('%.0f',cycle+64)
    cd.reading,cd.stat,cd.irq=branch=='inactive' and 0 or 1,branch=='busy' and 3 or 0,0
    cd.mode,cd.location_changed=branch=='speed' and 128 or 0,branch=='location'
    cd.read_scheduled,cd.read,cd.play=0,0,false
    cd.set_sector_play,cd.prev,cd.cur_track='\0\2\0\0',string.rep('\0',4),1
    cd.reg2,cd.suceeded=31,false
    -- IRQ retry reads the saved hardware latches without enabling CPU exceptions.
    local hardware=state.memory.hardware
    local function word(offset,value)
        hardware=hardware:sub(1,offset)..string.char(value,0,0,0)..hardware:sub(offset+5)
    end
    word(0x1070,branch=='retry' and 4 or 0);word(0x1074,branch=='retry' and 4 or 0)
    state.memory.hardware=hardware
    -- A short guest-only delay reaches the first callback before the stop PC.
    local program=ffi.new('uint32_t[7]',{0x240b0020,0x256bffff,0x1560fffe,0,0,0x08004005,0})
    state.memory.ram=state.memory.ram:sub(1,0x10000)..ffi.string(program,28)..state.memory.ram:sub(0x10000+29)
    m.write('state.pbuf',require('pb').encode('SaveState',state))
    m.report('fixture.json',{schema='psx.read-fixture/v1',branch=branch,entry=0x80010000,
        stop=0x80010014,scheduled_cycle=string.format('%.0f',cycle+64),
        provenance='host-edited controller/scheduler/hardware state; native guest callback follows restore'})
    m.finish()
end)
