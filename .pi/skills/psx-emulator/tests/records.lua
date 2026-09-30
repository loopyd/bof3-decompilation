-- CD wire export only; synthetic rows intentionally do not form a lifecycle.
local m, events, ffi = require 'support', require 'events', require 'ffi'
local function uint(n) return ffi.new('uint64_t',n) end
local cycle, identity = uint(9007199254740992)+1, uint(9007199254740992)+17
local rows, payload, outputs, reports, decoded, fixture
local function row(kind, device, values, size)
    local data={}; for n=0,7 do data[n]=(values or {})[n+1] or 0 end
    local result={sequence=uint(#rows+1),cycle=cycle,kind=kind,device=device,data=data,
        request=identity,related=identity+1,offset=#payload,length=size or 0}
    rows[#rows+1]=result; payload=payload..string.rep('\0',size or 0)
    return result
end
local function reset()
    rows,payload,outputs,reports,decoded,fixture,m.captures={},'',{},{},{},{},{}
    fixture.context=row(34,0,{2,0,1,0,0,0,56},256)
    row(34,1,{2,0,1,0,0,0,56},256)
    fixture.environment=row(34,2,{2,33868800,14,15,5,6,1,1},183)
    for role=1,4 do for phase=0,1 do row(35,phase,{role,0x1f801800,8,12,0x80010000,phase,1}) end end
    row(36,0,{6,2,0,0,0},8);row(36,1,{0x106,6,2,3,200,1},8);row(36,2,{6,200,1,0x106,400,0,9,0})
    for phase=0,4 do row(37,2,{phase,0,10,0,4,2,0,0},phase==0 and 8 or 0) end
    row(38,0,{2,1,9,0});fixture.response=row(38,1,{2,1,1,0,3},2);row(38,2,{0,0,2,1,1,0xff})
    for phase=0,2 do row(39,phase,{3,31,1,0});row(40,phase,{1,0,0,0,0,0,0,0}) end
    row(41,0,{0,2352},2352);fixture.mutation=row(41,1,{0,8,1,0,150,1},40);row(41,2,{0,2352},2352)
    row(42,0,{0,0x1f801802,0xab,1,1,1,0},8);row(42,1,{1,0x80000400,0xcd,1,2,1,0},8)
    row(43,0,{1,44100,588,1},24);fixture.feed=row(43,1,{6,44100,588,1,588,1,2,1},16)
    for phase=2,5 do row(43,phase,{0x3ff,64,1,2,0,0x040201,0,0},16) end
    row(44,0,{1,0,0,4,0,6,2,2},40);row(44,1,{0,2,1,4,0,3,2,3},40)
    row(45,0,{6,0,0,0,0x80010000,0,1});row(45,1,{6,0,0,0,0x80010000,0,1})
    for phase=2,3 do row(45,phase,{1,3,31,2,2,0,1,12}) end
    for phase=4,5 do fixture.mode=row(45,phase,{64,1,2,0,0,0x80000080,0,0x101}) end
    for phase=3,4 do row(34,phase,{2,0,1,0,0,0,56},256) end
    row(46,1,{4,4});row(46,2,{1,1});row(46,3,{1});row(46,4,{1})
    fixture.subq=row(46,4,{2,0x1234,0x1234},12);row(46,5,{0},3);row(46,6,{3,0})
    row(46,7,{0,1,2,0x200});row(46,8,{0x40});row(46,9,{0x100})
    fixture.availability=row(46,10,{1,1,0});row(46,11,{2,0,0,1})
    fixture.image=row(47,0,{2,716,0,100},2864);row(47,1,{2,716,0,100},2864)
end
PCSX={History={status=function()
    return {version=1,state=2,events=#rows,bytes=#payload,beginCycle=cycle,endCycle=cycle,
        failures=0,firstFailure=0,dropped=0}
end,bytes=function(offset,length) return payload:sub(offset+1,offset+length) end,
record=function(index) return rows[index+1] end}}
m.write=function(name,bytes) outputs[name]=bytes end
m.report=function(name,value) reports[name]=value end
local old_open,old_json=io.open,m.json
m.json=function(value)
    if type(value)=='table' and value.sequence then decoded[#decoded+1]=value end
    return old_json(value)
end
io.open=function(name,mode)
    assert(mode=='wb');outputs[name]=''
    return {write=function(_,value) outputs[name]=outputs[name]..value;return true end,
        close=function() return true end}
end
reset()
local result=events.export()
assert(result.complete and result.events==#rows and result.cd_scope:find('wire export only',1,true),result.export_error)
assert(outputs['payload.bin']==payload and #decoded==#rows)
for _,name in ipairs({'context','port','command','schedule','response','irq','media','buffer','data','audio','stream','transition','predicate','image'}) do
    assert(result.kinds['cd-'..name],name)
end
for _,value in ipairs(decoded) do
    assert(value.cycle=='9007199254740993' and value.request=='9007199254741009' and value.related=='9007199254741010')
end
assert(decoded[3].fields.configured_backend_bytes==5 and decoded[3].fields.scale_count==15)
local feed=decoded[tonumber(fixture.feed.sequence)].fields
assert(feed.reason==6 and feed.output_frames==588 and feed.event_disposition==2 and feed.audio_state==1)
local last=decoded[tonumber(fixture.mode.sequence)].fields
assert(last.attenuation==0x80000080 and last.read_play_flags==0x101 and last.channel==2)
for _,value in ipairs(decoded) do
    if value.kind=='cd-schedule' then
        local f=value.fields
        assert((f.phase==0)==(f.queued_opcode~=nil))
        assert((f.phase==1)==(f.nested_helper~=nil))
        assert((f.phase>=2)==(f.reserved_7~=nil))
    elseif value.kind=='cd-buffer' and value.device~=1 then
        assert(value.fields.reason==nil and value.fields.source_kind==nil)
    end
end
local function rejects(edit, message)
    reset();edit()
    local report=events.export()
    assert(not report.complete and report.export_error:find(message,1,true),report.export_error)
    assert(outputs['payload.bin']==payload, 'failure lost raw payload')
end
rejects(function() fixture.context.data[0]=1 end,'context version')
rejects(function() fixture.context.data[6]=47 end,'context layout')
rejects(function() fixture.environment.data[4]=1025 end,'environment layout')
rejects(function() fixture.environment.length=182 end,'payload length mismatch')
rejects(function() rows[4].device=2 end,'record variant')
rejects(function() rows[4].data[0]=5 end,'operation role')
rejects(function() fixture.response.data[0]=17 end,'response exceeds')
rejects(function() fixture.mutation.data[0]=2350 end,'buffer range exceeds')
rejects(function() fixture.feed.device=6 end,'record variant')
rejects(function() rows[15].data[0]=5 end,'scheduler phase/owner')
rejects(function() rows[16].data[7]=1 end,'scheduler tail')
rejects(function() rows[17].data[6]=1 end,'scheduler tail')
rejects(function() rows[1].kind=48 end,'unknown native history kind')
rejects(function() rows[4].data[7]=1 end,'reserved CD field')
rejects(function() fixture.mutation.data[6]=1 end,'unused CD field')
rejects(function() fixture.response.data[7]=1 end,'unused CD field')
rejects(function() fixture.subq.data[0]=3 end,'SubQ predicate')
rejects(function() fixture.subq.data[1]=65536 end,'SubQ CRC')
rejects(function() fixture.subq.length=11 end,'payload length mismatch')
rejects(function() fixture.availability.device=12 end,'predicate selector')
rejects(function() fixture.availability.data[0]=0 end,'availability predicate')
rejects(function() fixture.image.data[2]=203 end,'image layout')
rejects(function() fixture.image.data[4]=1 end,'unused CD field')
rejects(function() fixture.image.length=2863 end,'image layout')
io.open,m.json=old_open,old_json
print('CD wire export checks: all variants, exact uint64 identities, payload preservation and malformed layouts passed')
