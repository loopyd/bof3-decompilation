-- Host-edited command callback seeds; these do not prove submission reachability.
local m,snapshot,ffi=require 'support',require 'snapshot',require 'ffi'
m.begin('opcode,stat,repeated,delay,target,mode,drive,params,count,reading,play,seek',function()
    PCSX.pauseEmulator()
    local state=snapshot.live();local r,c=state.registers,state.cdrom
    local opcode=m.integer('opcode',1,0,65535)
    local cycle=tonumber((tostring(r.cycle):gsub('^#','')))
    assert(cycle and cycle>=0 and cycle<9007199254740000,'fixture cycle exceeds exact range')
    r.pc,r.cp0[13],r.next_is_delay_slot=0x80010000,0,false
    for _,name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        r[name]=r[name] or {};r[name].active,r[name].pc_active=false,false
    end
    local target=cycle+m.integer('target',64,1,128)
    r.interrupt=4;r.interrupt_targets[3]='#'..string.format('%.0f',target)
    c.ctrl,c.stat_p,c.reg2,c.cmd,c.irq=128,0x12,31,opcode%256,opcode
    c.stat=m.integer('stat',0,0,5);c.irq_repeated=m.integer('repeated',0,0,255)
    c.e_cycle=m.integer('delay',1000,0,4294967295)
    c.mode=m.integer('mode',0,0,255);c.drive_state=m.integer('drive',0,0,4)
    c.reading=m.integer('reading',0,0,255);c.play=m.integer('play',0,0,1)==1
    c.seeked=m.integer('seek',1,0,1)
    local params=m.argument('params','')
    assert(#params<=16 and #params%2==0 and not params:find('[^0-9a-fA-F]'),'invalid fixture params')
    local values={};for pair in params:gmatch('..') do values[#values+1]=string.char(tonumber(pair,16)) end
    c.param_c=m.integer('count',#values,0,8);c.param=table.concat(values)..string.rep('\165',8-#values)
    c.result,c.result_c,c.result_p,c.result_ready=string.rep('Z',16),0,0,0
    c.set_sector_play,c.prev,c.cur_track='\0\2\0\0',string.rep('\0',4),1
    c.set_loc_pending,c.location_changed,c.suceeded,c.read=0,false,false,0
    c.transfer='\1\2\3\4\5\6\7\8'..string.rep('\0',2344)
    c.subq_track,c.subq_index,c.subq_relative,c.subq_absolute=1,1,'\0\0\0','\0\2\0'
    local hardware=state.memory.hardware
    state.memory.hardware=hardware:sub(1,0x1070)..string.rep('\0',8)..hardware:sub(0x1079)
    local program=ffi.new('uint32_t[7]',{0x240b0020,0x256bffff,0x1560fffe,0,0,0x08004005,0})
    state.memory.ram=state.memory.ram:sub(1,0x10000)..ffi.string(program,28)..state.memory.ram:sub(0x10000+29)
    m.write('state.pbuf',require('pb').encode('SaveState',state))
    m.report('fixture.json',{schema='psx.command-fixture/v1',opcode=opcode,entry=0x80010000,stop=0x80010014,
        scheduled_cycle=string.format('%.0f',target),
        provenance='host-edited scheduler/controller/shared bytes; native callback after restore, not submission'})
    m.finish()
end)
