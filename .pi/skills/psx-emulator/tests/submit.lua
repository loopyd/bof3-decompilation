-- Generate guest stores from explicit host-edited controller preconditions.
local m,snapshot,ffi=require 'support',require 'snapshot',require 'ffi'
m.begin('scenario,mode,value',function()
    PCSX.pauseEmulator()
    local scenario=m.argument('scenario','all');assert(scenario=='all' or scenario=='mode','unknown submission fixture')
    local state=snapshot.live();local r,c=state.registers,state.cdrom
    local cycle=tonumber((tostring(r.cycle):gsub('^#','')))
    assert(cycle and cycle<9007199253740000,'fixture cycle exceeds exact range')
    r.pc,r.cp0[13],r.next_is_delay_slot=0x80010000,0,false
    for _,name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        r[name]=r[name] or {};r[name].active,r[name].pc_active=false,false
    end
    r.interrupt=4+8+16384
    for _,slot in ipairs({3,4,15}) do r.interrupt_targets[slot]='#'..string.format('%.0f',cycle+1000000) end
    c.ctrl,c.stat,c.stat_p,c.reg2,c.cmd,c.irq=64,5,0xf2,31,99,265
    c.irq_repeated,c.e_cycle,c.seeked=255,777,1
    c.mode=m.integer('mode',0,0,255);c.reading,c.play,c.fast_forward,c.fast_backward=2,true,3,4
    c.param_c,c.param=0,'\0\2\0\170\187\204\221\238'
    c.result,c.result_c,c.result_p,c.result_ready=string.rep('Z',16),8,7,1
    c.set_sector_play,c.set_sector,c.set_loc_pending='\0\2\0\0','\17\34\51\0',0
    c.attenuator_left_to_left_t,c.attenuator_left_to_right_t=0x44,0x33
    c.attenuator_right_to_left_t,c.attenuator_right_to_right_t=0x22,0x11
    local hardware=state.memory.hardware
    state.memory.hardware=hardware:sub(1,0x1070)..string.rep('\0',8)..hardware:sub(0x1079)
    local program={0x3c081f80,0x35081800};local writes={}
    local function store(port,value)
        program[#program+1]=0x24090000+value;program[#program+1]=0xa1090000+port
        writes[#writes+1]={port=port,value=value}
    end
    local function bank(n) store(0,n) end
    local function clear() bank(1);store(3,64);bank(0) end
    local function command(op,params,empty)
        if params then clear();for _,value in ipairs(params) do store(2,value) end end
        if empty then clear() end
        store(1,op)
    end
    if scenario=='mode' then command(14,{m.integer('value',64,0,255)},true)
    else
        bank(1);store(1,6);bank(2);store(1,9);bank(3);store(1,254);bank(0)
        command(9) -- Delayed repeat preserves 265/777, then stops both active streams.
        command(4);command(0);command(6);command(27);command(10);command(28);command(255)
        for _,values in ipairs({{0,2,0x16},{0,2,0x17},{0,1,0x74},{0x99,0x59,0x74},
                {0x9a,2,0},{0xa0,2,0},{0,0x60,0},{0,0x5a,0},{0,2,0x75},{0,2,0x0a}}) do
            command(2,values,true)
        end
        for _,value in ipairs({64,64,65,64,0,192}) do command(14,{value},true) end
    end
    local stop=0x80010000+#program*4
    program[#program+1]=0x08000000+math.floor(stop/4)%0x4000000;program[#program+1]=0
    local bytes=ffi.string(ffi.new('uint32_t[?]',#program,program),#program*4)
    state.memory.ram=state.memory.ram:sub(1,0x10000)..bytes..state.memory.ram:sub(0x10001+#bytes)
    m.write('state.pbuf',require('pb').encode('SaveState',state))
    m.report('fixture.json',{schema='psx.submission-fixture/v1',scenario=scenario,entry=r.pc,stop=stop,
        writes=m.array(writes),provenance='host-edited initial state; subsequent bank,parameter,command stores execute on guest CPU'})
    m.finish()
end)
