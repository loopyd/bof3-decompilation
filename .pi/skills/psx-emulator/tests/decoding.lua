-- Host-edited decoded-buffer seeds; native guest scheduling performs the callback.
local m,snapshot,ffi=require 'support',require 'snapshot',require 'ffi'
m.begin('branch',function()
    PCSX.pauseEmulator();local branch=m.argument('branch','inactive')
    assert(({inactive=true,disabled=true,outside=true,edge=true,asserted=true,latched=true})[branch],
        'unknown decoded-buffer fixture branch')
    local state=snapshot.live();local r,c,spu=state.registers,state.cdrom,state.spu
    local cycle=tonumber((tostring(r.cycle):gsub('^#','')))
    assert(cycle and cycle>=0 and cycle<9007199254740000,'fixture cycle exceeds exact range')
    r.pc,r.cp0[13],r.next_is_delay_slot=0x80010000,0,false
    for _,name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        r[name]=r[name] or {};r[name].active,r[name].pc_active=false,false
    end
    r.interrupt=4096;r.interrupt_targets[13]='#'..string.format('%.0f',cycle+64)
    c.ctrl,c.stat,c.stat_p,c.reg2,c.irq,c.cmd=64,0,0xe2,31,0,0
    c.seeked,c.play,c.reading,c.muted=1,branch~='inactive',0,true
    c.mode,c.fast_forward,c.fast_backward=128,3,4
    c.result,c.result_c,c.result_p,c.result_ready=string.rep('Z',16),4,3,0
    c.param_c,c.param=3,'\171\205\239\18\52\86\120\144'
    c.set_sector_play,c.prev,c.cur_track='\0\3\0\0','\0\2\4\0',1
    c.set_loc_pending,c.location_changed,c.suceeded=0,false,false
    spu.ctrl=branch=='disabled' and 0 or 64
    spu.irq=({outside=256,edge=255})[branch] or 0
    -- Restore repairs control from the port mirror and ORs 0x4000.
    local ports=assert(spu.ports);assert(#ports==512,'unexpected SPU ports')
    spu.ports=ports:sub(1,0x1aa)..string.char(spu.ctrl,0)..ports:sub(0x1ad)
    local hardware=state.memory.hardware
    local latch=branch=='latched' and '\4\2\0\0' or string.rep('\0',4)
    state.memory.hardware=hardware:sub(1,0x1070)..latch..string.rep('\0',4)..hardware:sub(0x1079)
    local program=ffi.new('uint32_t[7]',{0x240b0020,0x256bffff,0x1560fffe,0,0,0x08004005,0})
    state.memory.ram=state.memory.ram:sub(1,0x10000)..ffi.string(program,28)..state.memory.ram:sub(0x10000+29)
    m.write('state.pbuf',require('pb').encode('SaveState',state))
    m.report('fixture.json',{schema='psx.decode-fixture/v1',branch=branch,entry=r.pc,stop=0x80010014,
        control=spu.ctrl+0x4000,address=spu.irq,
        provenance='host-edited CD/SPU/scheduler state; native callback after restore'})
    m.finish()
end)
