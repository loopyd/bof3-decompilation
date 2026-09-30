-- Check proposed GPU History field export; this fixture is not native capture evidence.
local m, events, ffi = require 'support', require 'events', require 'ffi'
local uint = function(value) return ffi.new('uint64_t',value) end
local cycle = uint(9007199254740992)+1
local rows, outputs, reports = {}, {}, {}
local function record(kind,device,values,request,related)
    local data={}; for i=0,7 do data[i]=(values or {})[i+1] or 0 end
    rows[#rows+1]={sequence=uint(#rows+1),cycle=cycle,kind=kind,device=device,data=data,
        request=uint(request or 0),related=uint(related or 0),offset=0,length=0}
end
local function reset()
    rows,outputs,reports,m.captures={},{},{},{}
    record(30,0,{0,16,1024,0x1ffffc,0,1})
    record(31,0,{0,0,0,0,1,2,0})
    record(32,2,{0,0x1f801810,1,0x020000ff,0,16,0x80010000,1},0,1)
    record(33,2,{0,0x1f801810,1,0x020000ff,2,16,0x80010000,0},0,1)
    record(32,2,{5,0x1f801810,1,0,0,16,0x80010004,1},0,2)
    record(33,2,{5,0x1f801810,1,0x12345678,0,12,0x80010004,1},0,2)
    record(30,1,{0,12,1024,0x1ffffc,7,1})
    record(31,1,{0x123,0x456,0x789,0,1,2,0})
end
PCSX={History={status=function()
    return {version=1,state=2,events=#rows,bytes=0,beginCycle=cycle,endCycle=cycle,
        failures=0,firstFailure=0,dropped=0}
end,bytes=function() return '' end,record=function(index) return rows[index+1] end}}
m.write=function(name,bytes) outputs[name]=bytes end
m.report=function(name,value) reports[name]=value end
local old_open=io.open
io.open=function(name,mode)
    assert(mode=='wb'); outputs[name]=''
    return {write=function(_,value) outputs[name]=outputs[name]..value; return true end,
        close=function() return true end}
end
reset()
local result=events.export()
assert(result.complete and result.events==8 and result.kinds['gpu-context']==2)
local text=outputs['events.ndjson']
assert(text:find('9007199254740993',1,true) and text:find('"known_environment":0',1,true))
assert(text:find('"source":"cpu-read-gp0"',1,true) and text:find('read result only at gpu-result',1,true))
assert(text:find('"buffer":"2"',1,true) and text:find('"value":305419896',1,true))
assert(not text:find('"value_scope":true',1,true))
reset(); rows[1].data[4]=8
result=events.export(); assert(not result.complete and result.export_error:find('GPU context flags'))
reset(); rows[3].data[0]=10
result=events.export(); assert(not result.complete and result.export_error:find('GPU input source'))
reset(); rows[3].related=uint(0)
result=events.export(); assert(not result.complete and result.export_error:find('buffer identity'))
reset(); rows[3].device=3
result=events.export(); assert(not result.complete and result.export_error:find('input boundary'))
io.open=old_open
print('GPU history export checks passed: named boundaries, uint64 identities, availability flags and invalid records')
