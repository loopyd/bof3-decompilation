-- Synthetic controller journal checks, independent of native execution and of
-- the still-required scheduler/media/stream/Audio consumer.
local ffi, controller, m, bit = require 'ffi', require 'controller', require 'support', require 'bit'
local function uint(n) return ffi.new('uint64_t',n) end
local base=uint(9007199254740992)+1
local function bytes(values)
    local result={}
    for _,n in ipairs(values) do
        result[#result+1]=string.char(n%256,math.floor(n/256)%256,math.floor(n/65536)%256,math.floor(n/16777216)%256)
    end
    return table.concat(result)
end
local rows, stack, serial, responseSerial, latest, queued, published, opcode, delay, repeated
local state, mode, parameters, response, builder, marks, dmaChcr
local function emit(kind,phase,values,request,related,payload)
    local data={};for n=0,7 do data[n]=(values or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=data,request=uint(request or 0),related=uint(related or 0),
        cycle=base+#rows,payload=payload or ''}
    if (kind==45 and (phase==4 or phase==5)) or (kind==34 and phase>=3) then
        row.cycle=rows[#rows].cycle
    end
    rows[#rows+1]=row;return row
end
local function context(phase)
    local w={};for n=1,56 do w[n]=0 end;w[52]=254
    for n,slot in ipairs({1,2,4,9,10,11,12,13}) do w[slot]=state[n] end
    for n,slot in ipairs({18,19,20,21,23,34,35}) do w[slot]=mode[n] end
    w[14]=mode[8]%256;w[15]=math.floor(mode[8]/256)%256;w[16]=math.floor(mode[8]/65536)%256
    w[37],w[39],w[45]=latest,queued,published
    w[54]=dmaChcr
    local frame=phase>=3 and stack[#stack]
    return emit(34,phase,{2,0,7,2097152,0,0,56},frame and frame.id or 0,
        frame and stack[1].id or 0,bytes(w)..parameters..response..string.rep('\0',8))
end
local slots={[5]=2,[6]=3,[7]=14,[9]=13,[11]=10,[12]=12}
local function scope(role,address,value,body,owner,ownerKind)
    serial=serial+1
    local parent=stack[#stack];local frame={id=serial,role=role}
    stack[#stack+1]=frame
    local kind=role<=4 and 35 or 45
    local width=role<=4 and 8 or 0;local pc=0x80010000
    frame.enter=emit(kind,0,{role,address or 0,width,value or 0,pc,0,#stack},frame.id,parent and parent.id or 0)
    if role>=3 then
        if slots[role] then
            emit(37,slots[role],{1,role,0,0,0,ownerKind or 0,parent and 1 or 0},owner or 0,frame.id)
        end
        emit(45,2,state,frame.id,stack[1].id);emit(45,4,mode,frame.id,stack[1].id);context(3)
    end
    local returned=body and body(frame) or 0
    if role>=3 then
        emit(45,3,state,frame.id,stack[1].id);emit(45,5,mode,frame.id,stack[1].id);context(4)
    end
    frame.exit=emit(kind,1,{role,address or 0,width,(role==1 or role==3) and returned or (value or 0),
        pc,(role==1 or role==3) and 1 or 0,#stack},frame.id,parent and parent.id or 0)
    table.remove(stack);return returned,frame
end
local function port(write,address,value,body)
    return scope(write and 2 or 1,address+0xa0000000,value,function()
        return scope(write and 4 or 3,address,write and value%256 or value,body)
    end)
end
local function parameter(value)
    port(true,0x1f801802,value,function()
        local n=state[4];parameters=parameters:sub(1,n)..string.char(value)..parameters:sub(n+2);state[4]=n+1
    end)
end
local function queue(command,cycles)
    local repeatCommand=opcode~=0 and (command==opcode or command+256==opcode)
    local before=queued
    if not repeatCommand then queued=stack[1].role==5 and stack[1].owner or latest end
    marks.queue=emit(36,2,{opcode,delay,repeated,command,cycles,repeatCommand and 1 or 0,before,0},queued,stack[#stack].id)
    if repeatCommand then repeated=1 else opcode,delay=command,cycles end
end
local function submit(command)
    port(true,0x1f801801,command,function(frame)
        latest=frame.id
        marks.submission=emit(36,0,{command,state[4],opcode,state[2],state[7]},latest,stack[1].id,parameters)
        state[7]=0;state[1]=state[1]+(state[1]<128 and 128 or 0)
        queue(command,0x800)
    end)
end
local function resize(size)
    if builder then builder.revision=builder.revision+1
    else responseSerial=responseSerial+1;builder={id=responseSerial,revision=1,displaced=published};published=0 end
    local displaced=builder.revision==1 and builder.displaced or 0
    marks.resize=emit(38,0,{size,builder.revision,displaced,0},builder.id,stack[1].id)
    builder.size=size;state[5],state[6],state[7]=size,0,1
end
local function assertion()
    local id=builder and builder.id or published
    marks.irq=emit(39,0,{state[2],state[3],bit.band(state[2],state[3])~=0 and 1 or 0,builder and 1 or 0},id,stack[#stack].id)
end
local function publish(payload)
    response=payload..response:sub(#payload+1);published=builder.id
    marks.publication=emit(38,1,{builder.size,builder.revision,1,0,state[2]},published,stack[1].id,payload);builder=nil
end
local function callback(payload,revise)
    local owner=queued
    scope(5,nil,nil,function(frame)
        frame.owner=owner
        marks.execution=emit(36,1,{opcode,1,state[4],state[2],delay,repeated},owner,frame.id,parameters)
        opcode=0
        if revise then resize(1);resize(2) end
        resize(#payload);state[2]=3;assertion();state[4]=0;state[1]=0
        publish(payload)
    end,owner,owner==0 and 0 or 1)
end
local function readResponse()
    return port(false,0x1f801801,0,function(frame)
        local index=state[6]%16;local within=index<state[5]
        local value=within and response:byte(index+1) or 0
        marks.read=emit(38,2,{state[6],index,state[5],state[7],within and 1 or 0,value},published,frame.id)
        state[6]=(state[6]+1)%256;if state[6]==state[5] then state[7]=0 end
        return value
    end)
end
local function reset(cursor,ready)
    rows,stack,serial,responseSerial,latest,queued,published,opcode,delay,repeated={},{},0,0,0,0,0,0,0,0
    marks={};builder=nil;dmaChcr=0
    state={0,0,31,0,3,cursor or 0,ready or 0,0};mode={0,1,1,0,0,0x80000080,0,0}
    parameters=string.rep('\0',8);response=string.char(0x11,0x22,0x33)..string.rep('\0',13)
    emit(34,2,{2,33868800,14,15,0,0,1,1},0,0,string.rep('\0',172))
    marks.initial=context(0)
end
local function finish() marks.final=context(1) end
local function scan(withTransfers)
    local size=0
    for n,row in ipairs(rows) do row.sequence=uint(n);row.offset=size;row.length=#row.payload;size=size+row.length end
    local status={version=1,state=2,events=#rows,bytes=size,failures=0,dropped=0,beginCycle=base,endCycle=base+65536}
    local tracker=controller.new(status)
    local transfers=withTransfers and require('transfers').new(tracker)
    for _,row in ipairs(rows) do
        tracker.push(row,row.payload)
        if transfers then transfers.push(row,row.payload) end
    end
    local result=tracker.finish()
    if transfers then result.transfers=transfers.finish() end
    return result
end
reset(17,0);assert(readResponse()==0x22);finish()
local result=scan()
assert(#result.accesses==2 and result.accesses[1].value==0x22 and #result.responses==0)
assert(result.accesses[1].before[7]==0 and result.accesses[1].after[6]==18)
assert(result.contexts[1].cycle=='9007199254740994')
assert(type(m.json(result))=='string', 'controller output must be serializable')

reset();port(true,0x1f801800,0x12340006,function() state[1]=2 end);finish();result=scan()
assert(result.accesses[1].value==6 and result.accesses[2].value==0x12340006,
    'SB keeps the raw CPU operand and truncates only at the handler boundary')

local function fixture()
    reset();parameter(0x12);submit(1);submit(1) -- Native repeat retains the first queued owner.
    parameter(0x34) -- Callback sees the changed shared parameter bytes.
    callback(string.char(0x20,0x40,0x60),true)
    assert(readResponse()==0x20);finish()
end
fixture();result=scan()
assert(#result.commands==2 and #result.commands[1].executions==1 and #result.commands[2].executions==0)
assert(result.commands[1].parameters_hex:sub(3,4)=='00' and result.commands[1].executions[1].parameters_hex:sub(3,4)=='34')
assert(result.responses[1].revision==3 and result.responses[1].data_hex=='204060' and result.responses[1].byte_count==3)
assert(result.irqs[1].raw[4]==1, 'IRQ must retain unfinished builder association')
assert(result.accesses[#result.accesses].value==0x20)
assert(type(m.json(result))=='string')

reset();callback(string.char(0x44),false);finish();result=scan()
assert(#result.prehistory_executions==1 and #result.commands==0)

reset();submit(1);callback(string.char(0x55),false);submit(1);callback(string.char(0x66),false);finish();result=scan()
assert(result.responses[2].displaced=='1' and result.responses[2].id=='2')

reset()
scope(8,nil,nil,function()
    state[2]=1
    scope(9,nil,nil,function() resize(1);state[2]=5;assertion() end)
    publish(string.char(0x80))
end)
finish();result=scan()
assert(#result.accesses==2 and result.responses[1].operation=='1')
assert(result.responses[1].data_hex=='80')
assert(not m.json(result):find('[\128-\255]'), 'binary response escaped the explicit hex encoding')

reset();parameter(0xff);submit(1);callback(string.char(0xff),false);finish();result=scan()
assert(result.commands[1].parameters_hex:sub(1,2)=='ff' and result.contexts[2].response_hex:sub(1,2)=='ff')
assert(result.environment.payload.offset==0 and result.environment.payload.bytes==172)
assert(not m.json(result):find('[\128-\255]'), 'binary parameters must produce ASCII JSON')

reset();submit(1);callback(string.char(0x44),false)
port(true,0x1f801800,1,function() state[1]=1 end)
port(true,0x1f801802,0,function(frame)
    emit(39,1,{state[2],state[3],0},0,frame.id);state[3]=0;assertion()
end)
port(true,0x1f801803,3,function(frame)
    emit(39,2,{state[2],3,0,state[3]},0,frame.id);state[2]=0
end)
finish();result=scan()
assert(#result.irqs==4 and result.irqs[3].raw[3]==0 and result.irqs[4].raw[3]==0,
    'mask writes and acknowledgment preserve their nonasserting outcomes')

-- DMA continues after a buffer wrap clears read-ready. Each consumption must
-- join the prior controller cursor/readiness and the final enclosing snapshot.
local function dmaFixture()
    reset();mode[1],mode[8],state[8]=0x20,1,2339
    rows={rows[1]};marks.initial=context(0)
    emit(3,3,{0x400,0x400,1,0x11000000,0,0x8000},7)
    scope(10,nil,nil,function(frame)
        marks.dma={}
        for n=0,3 do
            local old=state[8];local ready=mode[8]%256
            state[8]=n;mode[8]=0
            marks.dma[#marks.dma+1]=emit(42,1,{old,0x400+n,0x55,ready,n,0,n==0 and 1 or 0},
                0,frame.id,bytes({7,0}))
        end
    end)
    finish()
end
dmaFixture();result=scan()
assert(result.accesses[1].before[8]==2339 and result.accesses[1].after[8]==3 and
    result.accesses[1].after_mode[8]==0)
marks.dma[2].data[3]=1
local ok,err=pcall(scan)
assert(not ok and tostring(err):find('consumption differs from controller cursor/readiness',1,true))

for _,size in ipairs({0,16,32,48}) do
    for _,ready in ipairs({0,1}) do
        reset();mode[1],mode[8],state[8]=size,ready,99
        rows={rows[1]};context(0)
        port(true,0x1f801803,128,function()
            if ready==0 then mode[8]=1;state[8]=(size==0 or size==16) and 12 or 0 end
        end)
        local saved=state[8]
        port(true,0x1f801803,0,function() end) -- Clearing the request bit does not disable data.
        finish();result=scan();assert(result.accesses[1].after[8]==saved)
    end
end
reset();mode[1],mode[8],state[8]=0,0,55;rows={rows[1]};context(0)
port(true,0x1f801803,128,function() mode[8]=1;state[8]=0 end);finish()
ok,err=pcall(scan);assert(not ok and tostring(err):find('transfer-enable cursor/readiness',1,true))

-- A prior cursor can exceed a newly selected size: subtract once, not modulo,
-- and clear ready only at zero. Corrupting the mode must reject a false wrap.
reset();mode[1],mode[8],state[8]=0,1,2339;rows={rows[1]};context(0)
port(false,0x1f801802,0,function(frame)
    emit(42,0,{2339,0x1f801802,0x55,1,280,1,1},0,frame.id,bytes({0,0}));state[8]=280;return 0x55
end);finish();scan()
dmaFixture();marks.dma[1].data[4]=280;marks.dma[1].data[5]=1
ok,err=pcall(scan);assert(not ok and tostring(err):find('native mode wrap/readiness',1,true))

local function transferFixture(ready,size,bcr,chcr,count,dicr)
    reset();mode[1],mode[8],state[8]=size,ready,0
    rows={rows[1]};context(0)
    dmaChcr=chcr
    emit(3,3,{0x80000403,0x400,bcr,chcr,0,0x8000},7)
    local function complete()
        scope(11,nil,nil,function()
            marks.complete=emit(6,3,{chcr,chcr-0x01000000},7,7)
            dmaChcr=chcr-0x01000000
            emit(7,3,{0,dicr},7,7)
            local after=dicr
            if bit.band(dicr,0x880000)==0x880000 then
                after=bit.bor(dicr,0x88000000)%4294967296
                if bit.band(dicr,0x80000000)==0 then emit(12,0,{8,0,8}) end
            end
            marks.completed=emit(7,3,{1,after},7,7)
        end,7,4)
    end
    scope(10,nil,nil,function(frame)
        if ready==0 then complete();return end
        for n=0,count-1 do
            local cursor,prior=state[8],mode[8]
            state[8]=state[8]+1
            local limit=(size==16 or size==32) and 2340 or 2060
            local wrapped=state[8]>=limit
            if wrapped then state[8]=state[8]-limit end
            if state[8]==0 then mode[8]=0 end
            emit(4,3,{0x400+n,cursor,1,0,0x400+n},7,0,string.char(0x55))
            marks.byte=emit(42,1,{cursor,0x400+n,0x55,prior,state[8],mode[8],wrapped and 1 or 0},
                0,frame.id,bytes({7,0}))
        end
        local delay=chcr==0x11400100 and math.floor(count/16) or count/4
        emit(10,10,{delay},7,base)
        marks.schedule=emit(37,10,{0,delay,0,0,1024,4},7,frame.id,bytes({0,0}))
    end)
    if ready~=0 then complete() end
    finish()
end
for _,case in ipairs({{0,0,1,0x11000000,0,0},{0,0,1,0x11000000,0,0x880000},
    {0,0,1,0x11000000,0,0x88880000},{1,0,0x10001,0x11000000,4,0},
    {1,0,0,0x11000000,2048,0},{1,16,0,0x11000000,2328,0},
    {1,32,0,0x11400100,2340,0},{1,48,0,0x11400100,2048,0}}) do
    transferFixture(unpack(case));result=scan(true)
    assert(#result.transfers.requests==1 and result.transfers.requests[1].bytes==case[5])
    assert(#result.transfers.completions==1)
end
local function rejectsTransfer(edit,message,ready)
    transferFixture(ready or 1,0,1,0x11000000,(ready==0) and 0 or 4,0)
    edit();local accepted,why=pcall(scan,true)
    assert(not accepted and tostring(why):find(message,1,true),tostring(why))
end
rejectsTransfer(function() marks.schedule.data[1]=2 end,'exact post-transfer schedule')
rejectsTransfer(function() marks.byte.data[1]=0x499 end,'byte count/address/order mismatch')
rejectsTransfer(function()
    for n=#rows,1,-1 do
        if rows[n].kind==10 or (rows[n].kind==37 and rows[n].data[0]==0) then table.remove(rows,n) end
    end
end,'complete transfer/schedule')
rejectsTransfer(function() marks.completed.data[1]=1 end,'IRQ result mismatch',0)
rejectsTransfer(function()
    for n=#rows,1,-1 do
        if rows[n].kind==6 or rows[n].kind==7 then table.remove(rows,n) end
    end
end,'differs from busy eligibility')
rejectsTransfer(function()
    local after=false
    for _,row in ipairs(rows) do
        if row==marks.complete then after=true end
        if after and row.kind==34 and row.device==4 then
            row.payload=row.payload:sub(1,212)..bytes({0x11000000})..row.payload:sub(217);break
        end
    end
end,'final CHCR differs from busy clear')
rejectsTransfer(function()
    for _,row in ipairs(rows) do
        if row.kind==34 and row.device==3 then
            row.payload=row.payload:sub(1,212)..bytes({0})..row.payload:sub(217)
        end
    end
end,'completion busy transition mismatch')
rejectsTransfer(function() marks.complete.related=uint(8) end,'completion busy transition mismatch',0)
rejectsTransfer(function()
    for n=#rows,1,-1 do if rows[n].kind==3 then table.remove(rows,n) end end
end,'root operation lacks adjacent start',0)
rejectsTransfer(function()
    for _,row in ipairs(rows) do if row.kind==6 or row.kind==7 then row.related=uint(999) end end
end,'completion busy transition mismatch')
rejectsTransfer(function()
    for n,row in ipairs(rows) do
        if row==marks.completed then
            local originals={rows[n-2],rows[n-1],row}
            for offset,original in ipairs(originals) do
                local copy={};for key,value in pairs(original) do copy[key]=value end
                copy.cycle=row.cycle;copy.related=uint(0);table.insert(rows,n+offset,copy)
            end
            break
        end
    end
end,'completion busy transition mismatch')

reset();dmaChcr=0x10000000;rows={rows[1]};context(0)
scope(11,nil,nil,nil,0,0);finish();result=scan(true)
assert(#result.transfers.completions==0 and #result.transfers.callbacks==1 and
    not result.transfers.callbacks[1].busy and result.transfers.callbacks[1].chcr_after==0x10000000,
    'idle callback must preserve CHCR and emit no completion')

local function rejects(edit,message)
    fixture();edit()
    local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(message,1,true),tostring(err))
end
rejects(function() marks.publication.request=uint(99) end,'publication mismatch')
rejects(function()
    for n,row in ipairs(rows) do if row==marks.publication then table.remove(rows,n);break end end
end,'callback command/response identity mismatch')
rejects(function() marks.resize.data[1]=8 end,'revision mismatch')
rejects(function() marks.read.data[5]=0xaa end,'FIFO byte mismatch')
rejects(function()
    for n,row in ipairs(rows) do if row==marks.read then table.remove(rows,n);break end end
end,'lacks its FIFO/data observation')
rejects(function()
    for n,row in ipairs(rows) do
        if row==marks.read then
            local copy={};for k,v in pairs(row) do copy[k]=v end
            table.insert(rows,n+1,copy);break
        end
    end
end,'FIFO identity/cursor mismatch')
rejects(function() marks.execution.payload=string.rep('\0',8) end,'callback owner/parameters mismatch')
rejects(function() marks.queue.request=uint(99) end,'queued command owner mismatch')
rejects(function() marks.irq.request=uint(99) end,'IRQ response/assertion mismatch')
rejects(function()
    local p=marks.final.payload;marks.final.payload=p:sub(1,176)..bytes({99})..p:sub(181)
end,'final command/response identity mismatch')
rejects(function() marks.final.payload=marks.final.payload:sub(1,224)..string.rep('\0',8)..marks.final.payload:sub(233) end,
    'final parameter bytes mismatch')
rejects(function() rows[3].related=uint(1) end,'parent/depth mismatch')
rejects(function() rows[4].data[1]=0x1f801803 end,'handler/CPU access mismatch')
rejects(function() rows[5].device=3 end,'after-state is unpaired')
rejects(function() rows[6].device=5 end,'state pair was interrupted')
rejects(function() rows[3].request=uint(4) end,'identity is not contiguous')
rejects(function() marks.final.data[2]=9 end,'epochs/profile changed')
local function callbackContext(phase,afterPublication)
    local found=not afterPublication
    for _,row in ipairs(rows) do
        if row==marks.publication then found=true end
        if found and row.kind==34 and row.device==phase then return row end
    end
    error('missing fixture callback context')
end
rejects(function() callbackContext(3).data[0]=1 end,'context version')
rejects(function() callbackContext(3).cycle=callbackContext(3).cycle+1 end,'state pair was interrupted')
rejects(function()
    local row=callbackContext(3);row.payload=row.payload:sub(1,144)..bytes({99})..row.payload:sub(149)
end,'callback command/response identity mismatch')
rejects(function()
    local row=callbackContext(4,true);row.payload=row.payload:sub(1,232)..'x'..row.payload:sub(234)
end,'callback response publication bytes mismatch')
rejects(function()
    local row=callbackContext(3,true);row.payload=row.payload:sub(1,247)..'x'..row.payload:sub(249)
end,'callback response continuity mismatch')
rejects(function()
    local row=callbackContext(4);row.payload=row.payload:sub(1,247)..'x'..row.payload:sub(249)
end,'callback changed response bytes')
-- Native callbacks may revise a long response down to a short publication;
-- the final tail is still observed and must survive subsequent boundaries.
fixture()
local active=false
for _,row in ipairs(rows) do
    if row==marks.publication then active=true end
    if active and row.kind==34 then
        row.payload=row.payload:sub(1,247)..'x'..row.payload:sub(249)
    end
end
assert(scan().contexts[2].response_hex:sub(-2)=='78')
print('CD controller checks: responses, transfer enable/wrap, DMA sizing/scheduling/completion, ancestry and corruption passed')
