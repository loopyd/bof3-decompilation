-- Host-edited closed-lid callback seeds. Lid-open time is not serialized.
local m,snapshot,ffi=require 'support',require 'snapshot',require 'ffi'
m.begin('branch',function()
    PCSX.pauseEmulator();local branch=m.argument('branch','standby')
    local drives={standby=0,stopped=4,unknown=255,initial=1,reading=1,rotating=1,rescan=2,prepare=3,closed=1}
    assert(drives[branch],'unknown lid fixture branch')
    local state=snapshot.live();local r,c=state.registers,state.cdrom
    local cycle=tonumber((tostring(r.cycle):gsub('^#','')))
    assert(cycle and cycle>=0 and cycle<9007199254740000,'fixture cycle exceeds exact range')
    r.pc,r.cp0[13],r.next_is_delay_slot=0x80010000,0,false
    for _,name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        r[name]=r[name] or {};r[name].active,r[name].pc_active=false,false
    end
    r.interrupt=8192;r.interrupt_targets[14]='#'..string.format('%.0f',cycle+64)
    if branch=='reading' then
        r.interrupt=r.interrupt+8;r.interrupt_targets[4]='#'..string.format('%.0f',cycle+100000)
    end
    c.ctrl,c.stat,c.reg2,c.irq,c.cmd=64,0,31,0,0
    c.stat_p=({rotating=0xf2,rescan=0xf0,prepare=0x90,closed=0xf0})[branch] or 0xe2
    c.seeked,c.play,c.reading,c.muted=1,true,branch=='reading' and 1 or 0,true
    c.mode,c.drive_state,c.fast_forward,c.fast_backward=128,drives[branch],3,4
    c.result,c.result_c,c.result_p,c.result_ready=string.rep('Z',16),4,3,0
    c.param_c,c.param=3,'\171\205\239\18\52\86\120\144'
    c.set_sector_play,c.prev,c.cur_track='\0\3\0\0','\0\2\4\0',1
    c.set_loc_pending,c.location_changed,c.suceeded=0,false,false
    local hardware=state.memory.hardware
    state.memory.hardware=hardware:sub(1,0x1070)..string.rep('\0',8)..hardware:sub(0x1079)
    local program=ffi.new('uint32_t[7]',{0x240b0020,0x256bffff,0x1560fffe,0,0,0x08004005,0})
    state.memory.ram=state.memory.ram:sub(1,0x10000)..ffi.string(program,28)..state.memory.ram:sub(0x10000+29)
    m.write('state.pbuf',require('pb').encode('SaveState',state))
    m.report('fixture.json',{schema='psx.lid-fixture/v1',branch=branch,entry=r.pc,stop=0x80010014,
        provenance='host-edited controller/scheduler state; native closed-lid callback after restore'})
    m.finish()
end)
