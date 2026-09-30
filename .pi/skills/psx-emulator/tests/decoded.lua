-- Explicit decoded-buffer branches and rejection checks, independent of DSP.
local ffi,decoded=require 'ffi',require 'decoded'
local function id(n) return ffi.new('uint64_t',n) end
local rows,frame,marks
local function fixture(play,control,address,assertion,clock)
    rows,marks={},{}
    local function context()
        local w={};for n=1,56 do w[n]=n end;w[16]=play
        return {words=w,parameters_hex='123456789abcdef0',response_hex=string.rep('5a',16),subq_hex=string.rep('ab',8)}
    end
    frame={role=12,id=id(3),before_context=context(),after_context=context()}
    local function emit(name,kind,phase,values)
        local d={};for n=0,7 do d[n]=(values or {})[n+1] or 0 end
        local row={kind=kind,device=phase,data=d,cycle=id(9007199254740992)+1}
        marks[name]=row;rows[#rows+1]=row
    end
    emit('environment',34,2,{2,clock or 33868800});emit('before',34,3)
    if control then emit('control',46,8,{control}) end
    if address then emit('address',46,9,{address}) end
    if assertion then emit('assert',12,0,{512});emit('schedule',37,12,{0,196608}) end
    emit('basic',45,3);emit('mode',45,5);emit('after',34,4)
end
local function scan()
    local current;local t=decoded.new({current=function() return current end})
    for _,row in ipairs(rows) do if row==marks.before then current=frame end;t.push(row) end
    return t.finish()
end
local function check(class,play,control,address,assertion,clock)
    fixture(play,control,address,assertion,clock)
    local r=scan();assert(r[class]==1 and #r.callbacks==1);return r
end
check('inactive',0)
for _,value in ipairs({0,32,0xffbf}) do check('disabled',1,value) end
for _,address in ipairs({256,257,65535}) do check('outside',1,64,address) end
for _,address in ipairs({0,128,255}) do check('asserted',1,0xffff,address,true) end
-- Floor before multiplication: this clock still yields 196608, not 196609.
check('asserted',1,64,0,true,33869000)
local function rejects(edit,needle)
    fixture(1,64,255,true);edit();local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
local function remove(name)
    for n,row in ipairs(rows) do if row==marks[name] then table.remove(rows,n);return end end
end
for _,name in ipairs({'control','address','assert','schedule'}) do
    rejects(function() remove(name) end,'expected '..name)
end
rejects(function() marks.control.data[0]=0 end,'unexplained body')
rejects(function() marks.address.data[0]=256 end,'unexplained body')
rejects(function() marks.control.data[0]=65536 end,'register width')
rejects(function() marks.address.data[0]=65536 end,'register width')
rejects(function() marks.assert.data[0]=4 end,'wrong IRQ')
rejects(function() marks.schedule.data[1]=196609 end,'exact delay')
rejects(function() marks.schedule.device=14 end,'unexpected CD decode')
rejects(function() marks.schedule.data[0]=1 end,'unexpected CD decode')
rejects(function() table.insert(rows,3,marks.before) end,'overlapping')
for n=1,56 do rejects(function() frame.after_context.words[n]=frame.after_context.words[n]+1 end,'word '..n) end
for _,name in ipairs({'parameters_hex','response_hex','subq_hex'}) do
    rejects(function() frame.after_context[name]='00' end,name)
end
rejects(function() marks.after.cycle=marks.after.cycle+1 end,'scope/cycle')
rejects(function() remove('after') end,'unfinished')
print('CD decoded-buffer checks: gates, thresholds, exact delay, unchanged contexts and corruptions passed')
