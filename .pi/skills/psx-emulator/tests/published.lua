-- Guest publishes/acknowledges GetStat before a seeded PLAY autopause callback.
local m,snapshot,ffi=require 'support',require 'snapshot',require 'ffi'
m.begin('',function()
    PCSX.pauseEmulator();local state=snapshot.live();local r,c=state.registers,state.cdrom
    local cycle=tonumber((tostring(r.cycle):gsub('^#','')))
    assert(cycle and cycle<9007199253740000,'fixture cycle exceeds exact range')
    r.pc,r.cp0[13],r.next_is_delay_slot=0x80010000,0,false
    for _,name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        r[name]=r[name] or {};r[name].active,r[name].pc_active=false,false
    end
    r.interrupt=16384;r.interrupt_targets[15]='#'..string.format('%.0f',cycle+100000)
    c.ctrl,c.stat,c.stat_p,c.reg2,c.irq,c.seeked=0,0,0xe2,31,0,1
    c.play,c.reading,c.muted,c.mode,c.track_changed=true,0,true,6,true
    c.set_sector_play,c.set_sector_end='\0\3\0\0','\0\9\0\0'
    c.set_loc_pending,c.location_changed,c.cur_track=0,false,1
    -- Restore performs readTrack(++prev): this pregap position generates the
    -- track-change flag before Find_CurTrack restores the playing track.
    c.prev='\0\2\73\0'
    c.result,c.result_c,c.result_p,c.result_ready=string.rep('Z',16),4,3,0
    local hardware=state.memory.hardware
    state.memory.hardware=hardware:sub(1,0x1070)..string.rep('\0',8)..hardware:sub(0x1079)
    local words={0x3c081f80,0x35081800,0x24090001,0xa1090001,0xa1090000,
        0x910a0003,0,0x314a001f,0x1140fffc,0,0x2409001f,0xa1090003,0xa1000000,
        0x240b7fff,0x256bffff,0x1560fffe,0,0,0x08004012,0}
    local bytes=ffi.string(ffi.new('uint32_t[?]',#words,words),#words*4)
    state.memory.ram=state.memory.ram:sub(1,0x10000)..bytes..state.memory.ram:sub(0x10001+#bytes)
    m.write('state.pbuf',require('pb').encode('SaveState',state))
    m.report('fixture.json',{schema='psx.publication-fixture/v1',entry=r.pc,stop=0x80010048,
        provenance='host-edited PLAY preconditions; guest GetStat,IRQ polling and acknowledgment precede native autopause'})
    m.finish()
end)
