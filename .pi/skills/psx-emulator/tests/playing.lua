-- Host-edited PLAY callback seeds; guest execution supplies the observed work.
local m,snapshot,ffi=require 'support',require 'snapshot',require 'ffi'
m.begin('branch',function()
    PCSX.pauseEmulator();local branch=m.argument('branch','inactive')
    local supported={inactive=true,busy=true,seek=true,queued=true,seekplay=true,endpoint=true,
        autopause=true,report=true,right=true,endauto=true,endreport=true,location=true}
    assert(supported[branch],'unknown PLAY fixture branch')
    local state=snapshot.live();local r,c=state.registers,state.cdrom
    local cycle=tonumber((tostring(r.cycle):gsub('^#','')))
    assert(cycle and cycle<9007199254740000,'fixture cycle exceeds exact range')
    r.pc,r.cp0[13],r.next_is_delay_slot=0x80010000,0,false
    for _,name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        r[name]=r[name] or {};r[name].active,r[name].pc_active=false,false
    end
    r.interrupt=16384;r.interrupt_targets[15]='#'..string.format('%.0f',cycle+64)
    c.ctrl,c.stat,c.stat_p,c.reg2,c.irq,c.cmd=64,branch=='busy' and 3 or 0,0xe2,31,branch=='queued' and 9 or 0,0
    c.seeked=(branch=='busy' or branch=='seek' or branch=='queued' or branch=='seekplay') and 0 or 1
    c.play=branch~='inactive' and branch~='seek' and branch~='queued'
    c.reading,c.muted,c.fast_forward,c.fast_backward=0,branch~='report',3,4
    c.mode=({autopause=6,endauto=6,report=4,right=4,endreport=4})[branch] or 128
    c.result,c.result_c,c.result_p,c.result_ready=string.rep('Z',16),4,3,0
    c.param_c,c.param=3,'\171\205\239\18\52\86\120\144'
    -- Restore replaces the endpoint with disc TD(0), 00:07:00 for peaks media.
    c.set_sector_play=branch:sub(1,3)=='end' and '\0\7\0\0' or '\0\3\0\0'
    c.set_sector_end='\0\7\0\0'
    c.set_sector,c.set_loc_pending='\0\3\1\0',branch=='seekplay' and 1 or 0
    -- Restore also refreshes SubQ with readTrack(++prev), before capture.
    c.prev,c.cur_track,c.suceeded='\0\2\4\0',2,false
    if branch=='right' then c.prev='\0\3\9\0'
    elseif branch=='autopause' then c.prev,c.cur_track='\0\2\73\0',1 end
    c.track_changed,c.location_changed=branch=='autopause',branch=='location'
    c.subq_track,c.subq_index,c.subq_relative=0x12,2,'\0\1\35'
    c.subq_absolute=branch=='right' and '\0\3\16' or '\0\2\5'
    c.attenuator_left_to_left,c.attenuator_right_to_right=64,64
    c.attenuator_left_to_right,c.attenuator_right_to_left=0,0
    local hardware=state.memory.hardware
    state.memory.hardware=hardware:sub(1,0x1070)..string.rep('\0',8)..hardware:sub(0x1079)
    local program=ffi.new('uint32_t[7]',{0x240b0020,0x256bffff,0x1560fffe,0,0,0x08004005,0})
    state.memory.ram=state.memory.ram:sub(1,0x10000)..ffi.string(program,28)..state.memory.ram:sub(0x10000+29)
    m.write('state.pbuf',require('pb').encode('SaveState',state))
    m.report('fixture.json',{schema='psx.play-fixture/v1',branch=branch,entry=r.pc,stop=0x80010014,
        provenance='host-edited initial controller,SubQ,scheduler state; native callback follows restore'})
    m.finish()
end)
