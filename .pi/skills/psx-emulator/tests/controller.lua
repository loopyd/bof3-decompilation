-- Execute bounded guest CD commands and verify terminal responses before history integration.
local m, snapshot, ffi = require 'support', require 'snapshot', require 'ffi'
m.begin('', function()
    PCSX.pauseEmulator()
    local state = snapshot.live()
    state.registers.pc, state.registers.cp0[13], state.registers.next_is_delay_slot = 0x80010000, 0, false
    for _, name in ipairs({'delay_slot_info_1','delay_slot_info_2'}) do
        state.registers[name] = state.registers[name] or {}
        state.registers[name].active, state.registers[name].pc_active = false, false
    end
    m.write('initial.pbuf', require('pb').encode('SaveState',state))
    local file = Support.File.open('initial.pbuf'); PCSX.loadSaveState(file); file:close()
    local words, labels, fixups, expected = {}, {}, {}, {}
    local function emit(value) words[#words+1] = value end
    local function constant(reg,value)
        emit(0x3c000000+reg*65536+math.floor(value/65536))
        emit(0x34000000+reg*0x210000+value%65536)
    end
    local function store(address,value,width)
        constant(8,address);constant(9,value);emit(width==8 and 0xa1090000 or 0xad090000)
    end
    local function label(name) assert(not labels[name]);labels[name]=#words end
    local function branch(reg,name)
        fixups[#fixups+1]={index=#words+1,name=name,reg=reg};emit(0);emit(0)
    end
    local function jump(name)
        fixups[#fixups+1]={index=#words+1,name=name};emit(0);emit(0)
    end
    local function acknowledge()
        store(0x1f801800,1,8);store(0x1f801803,0x5f,8);store(0x1f801800,0,8)
    end
    local function response(command,irq,count)
        local index=#expected+1
        expected[index]={command=command,irq=irq,bytes=count}
        store(0x1f801800,1,8);constant(8,0x1f801803);constant(11,2000000)
        label('poll'..index)
        -- Bound observation volume: wait in guest RAM between status polls.
        -- No host callback consumes or acknowledges the controller response.
        constant(12,4096);label('wait'..index);emit(0x258cffff);branch(12,'wait'..index)
        emit(0x910a0000);emit(0);emit(0x314a0007)
        branch(10,'got'..index);emit(0x256bffff);branch(11,'poll'..index);jump('timeout')
        label('got'..index);constant(8,0x80038000+(index-1)*24);emit(0xa10a0000)
        for byte=1,count do
            constant(8,0x1f801801);emit(0x910a0000);emit(0)
            constant(8,0x80038000+(index-1)*24+byte);emit(0xa10a0000)
        end
        acknowledge()
    end
    local function command(op,params,phases)
        store(0x1f801800,0,8)
        for _,value in ipairs(params) do store(0x1f801802,value,8) end
        store(0x1f801801,op,8)
        for _,phase in ipairs(phases) do response(op,phase[1],phase[2]) end
    end
    store(0xfffe0130,0x1e988);store(0x1f801070,0);store(0x1f801074,0)
    store(0x1f8010f0,0x08888888);acknowledge()
    command(1,{},{{3,1}})
    command(0x0d,{1,0},{{3,1}})
    command(0x0f,{},{{3,5}})
    command(2,{0,2,0},{{3,1}})
    command(0x15,{},{{3,1},{2,1}})
    command(6,{},{{3,1},{1,1}})
    store(0x1f801800,0,8);store(0x1f801803,0x80,8)
    for byte=0,3 do
        constant(8,0x1f801802);emit(0x910a0000);emit(0)
        constant(8,0x80039000+byte);emit(0xa10a0000)
    end
    store(0x1f8010b0,0x39004);store(0x1f8010b4,1);store(0x1f8010b8,0x11000000)
    command(9,{},{{3,1},{2,1}})
    command(8,{},{{3,1},{2,1}})
    command(0xff,{},{{5,2}})
    label('complete');jump('complete');label('timeout');jump('timeout')
    for _,fix in ipairs(fixups) do
        local target=assert(labels[fix.name])
        if fix.reg then
            local displacement=target-fix.index
            assert(displacement>=-32768 and displacement<=32767)
            words[fix.index]=0x14000000+fix.reg*0x200000+displacement%65536
        else words[fix.index]=0x08000000+(0x10000+target*4)/4 end
    end
    local ram=ffi.cast('uint8_t*',PCSX.getMemPtr())
    local program=ffi.new('uint32_t[?]',#words)
    for i,value in ipairs(words) do program[i-1]=value end
    ffi.copy(ram+0x10000,program,#words*4);ffi.fill(ram+0x38000,4096,0xcd)
    ffi.fill(ram+0x39000,8,0xcd);PCSX.invalidateCache()
    m.write('program.bin',ffi.string(program,#words*4))
    m.report('program.json',{entry=0x80010000,stop=0x80010000+labels.complete*4,
        timeout=0x80010000+labels.timeout*4,poll_wait_iterations=4096})
    m.write('seed.pbuf',tostring(PCSX.createSaveState()))
    local timer=luv.new_timer();m.retained[#m.retained+1]=timer
    timer:start(15000,0,m.guard(function() error('CD controller fixture wall timeout') end))
    m.breakpoint(0x80010000+labels.timeout*4,'Exec',4,function() error('guest CD response timeout') end)
    m.breakpoint(0x80010000+labels.complete*4,'Exec',4,function()
        PCSX.pauseEmulator();timer:stop();timer:close()
        m.write('responses.bin',ffi.string(ram+0x38000,#expected*24))
        m.write('transfer.bin',ffi.string(ram+0x39000,8))
        m.write('state.pbuf',tostring(PCSX.createSaveState()))
        for i,row in ipairs(expected) do
            row.observed_irq=tonumber(ram[0x38000+(i-1)*24])
            assert(row.observed_irq==row.irq,'wrong IRQ at response '..i)
        end
        assert(ram[0x38000+2*24+4]==1 and ram[0x38000+2*24+5]==0,'filter response differs')
        assert(ffi.string(ram+0x39000,8)==string.char(0,1,2,3,4,5,6,7),'PIO/DMA sector offsets differ')
        assert(ram[0x38000+(#expected-1)*24+2]==0x40,'invalid command error differs')
        m.report('checks.json',{schema='psx.skill-checks/v1',passed=4,responses=expected,
            checks={'multi-stage command IRQs','filter response','PIO/DMA bytes','invalid-command error'},
            scope='guest diagnostic with staged code/RAM; no native CD history acceptance'})
        m.finish();return false
    end)
    PCSX.resumeEmulator()
end)
