-- Explicit lid state-machine vectors; native host-time openness is not simulated.
package.loaded.image={track=function() return {type=1} end,td=function() return 1792 end}
local ffi,lid=require 'ffi',require 'lid'
local function id(n) return ffi.new('uint64_t',n) end
local rows,frame,marks,geometry
local function context(words)
    return {words=words,parameters_hex='abcdef1234567890',response_hex=string.rep('5a',16),subq_hex=string.rep('ab',8)}
end
local function emit(name,kind,phase,d,current,request)
    local values={};for n=0,7 do values[n]=(d or {})[n+1] or 0 end
    local row={kind=kind,device=phase,data=values,frame=current,request=request or id(1),cycle=id(9007199254740992)+1}
    marks[name]=row;rows[#rows+1]=row;return row
end
local function fixture(options)
    options=options or {};rows,marks={},{};geometry={header={main=options.mounted and 1 or 0}}
    local b,c={},{};for n=1,56 do b[n]=n end
    b[3],b[15],b[16],b[27],b[31]=0xe2,0,1,512,0
    for n,v in pairs(options.before or {}) do b[n]=v end
    for n=1,56 do c[n]=b[n] end
    for n,v in pairs(options.after or {}) do c[n]=v end
    frame={role=9,id=id(1),before_context=context(b),after_context=context(c)}
    emit('env',34,2,{2,options.clock or 33868800});emit('before',34,3,{},frame)
    for _,step in ipairs(options.steps or {}) do
        local name=step[1];local defs={status={46,7},stop={44,step.phase or 0},cancel={37,3},schedule={37,13}}
        local def=assert(defs[name]);emit(name,def[1],def[2],step[2],frame)
    end
    emit('basic',45,3,{},frame);emit('mode',45,5,{},frame);emit('after',34,4,{},frame)
end
local function scan()
    local current;local tracker=lid.new({current=function() return current end},function() return geometry end)
    for _,row in ipairs(rows) do current=row.frame;tracker.push(row) end
    return tracker.finish()
end
local status={'status',{0,2,128,512}}
local function check(options,class)
    fixture(options);local result=scan();assert(result.callbacks[1].classification==class);return result
end
local standby={after={[3]=0xa2},steps={status}}
check(standby,'standby')
for _,drive in ipairs({4,255}) do check({before={[31]=drive},after=standby.after,steps={status}},'standby') end
check({before={[16]=0},after=standby.after,steps={{'status',{0,1,0,512}}}},'standby')
local opened={after={[3]=0x22,[16]=0,[31]=1,[32]=0,[33]=0},
    steps={{'status',{1,2,144,512}},{'stop',{0,3,1},phase=1},{'schedule',{0,2048}}}}
check(opened,'opened')
check({before={[16]=0},after={[3]=0xa2,[31]=1},
    steps={{'status',{1,1,16,512}},{'stop',{0,3,0},phase=1},{'schedule',{0,2048}}}},'opened')
local opening={before={[31]=1,[15]=1},after={[3]=0x92,[15]=0},
    steps={status,{'stop',{0,3,1}},{'cancel',{2}},{'schedule',{0,13547520}}}}
check(opening,'opening')
check({before={[31]=1},after={[3]=0x92},steps={status,{'stop',{0,3,0}},{'schedule',{0,13547520}}}},'opening')
local spindown={before={[31]=1,[3]=0xf2},after={[3]=0xf0},steps={status,{'schedule',{0,1354752}}}}
check(spindown,'spindown')
check({before={[31]=1,[3]=0xf0},steps={{'status',{1,2,144,512}},{'schedule',{0,1354752}}}},'waiting')
local closed={before={[31]=1,[3]=0xf0},after={[31]=2},steps={status,{'schedule',{0,47416320}}}}
assert(check(closed,'closed').callbacks[1].check_deferred)
check({before={[31]=2,[3]=0xf0},after={[31]=3,[3]=0xf2},steps={{'schedule',{0,67737600}}}},'rescan')
check({before={[31]=3,[3]=0x90},after={[31]=0,[3]=0xd0},steps={{'schedule',{0,11741184}}}},'prepare')
check({before={[31]=2,[3]=0xf0},after={[31]=3,[3]=0xf2},clock=33868874,steps={{'schedule',{0,67737600}}}},'rescan')
local function rejects(options,edit,needle)
    fixture(options);edit();local ok,err=pcall(scan)
    assert(not ok and tostring(err):find(needle,1,true),'expected '..needle..'; got '..tostring(err))
end
local function remove(name)
    for n,row in ipairs(rows) do if row==marks[name] then table.remove(rows,n);return end end
end
rejects(closed,function() geometry.header.main=1 end,'mounted-media')
rejects(opened,function() remove('stop') end,'expected stop')
rejects(opening,function() remove('cancel') end,'expected cancel')
rejects(spindown,function() remove('status') end,'expected status')
rejects(opening,function() remove('schedule') end,'expected schedule')
rejects(opened,function() marks.stop.data[1]=0 end,'stop mismatch')
rejects(opened,function() marks.stop.device=0 end,'stop mismatch')
rejects(opening,function() marks.schedule.data[1]=451584 end,'exact delay')
for n=0,3 do rejects(standby,function() marks.status.data[n]=marks.status.data[n]+1 end,'predicate mismatch') end
rejects(standby,function() marks.after.cycle=marks.after.cycle+1 end,'cycle changed')
rejects(standby,function() marks.basic.kind=18 end,'unexpected CD lid')
rejects(standby,function() remove('after') end,'unfinished')
for n=1,56 do rejects(standby,function() frame.after_context.words[n]=frame.after_context.words[n]+1 end,'word '..n) end
for _,name in ipairs({'parameters_hex','response_hex','subq_hex'}) do
    rejects(standby,function() frame.after_context[name]='00' end,name)
end

-- Direct lid interrupt refreshes endpoint/stops CDDA, then delegates exactly one
-- nested lid-seek. This is component coverage until a guarded native API exists.
local function nested()
    fixture(standby);local child=frame;child.id,child.parent=id(2),id(1)
    local before,between,after={},{},{}
    for n=1,56 do before[n]=child.before_context.words[n];between[n]=before[n];after[n]=child.after_context.words[n] end
    between[3],between[16],between[29],between[32],between[33]=0x62,0,1792,0,0
    for n=1,56 do after[n]=between[n] end;after[3]=0x22
    child.before_context,child.after_context=context(between),context(after)
    frame={role=8,id=id(1),before_context=context(before),after_context=context(after)}
    rows,marks={},{}
    emit('env',34,2,{2,33868800});emit('before',34,3,{},frame)
    emit('stop',44,1,{0,3,1},frame)
    emit('enter',45,0,{9},child,id(2));emit('dispatch',37,13,{1},child,id(2))
    emit('childbasic',45,2,{},child,id(2));emit('childmode',45,4,{},child,id(2))
    emit('childbefore',34,3,{},child,id(2));emit('status',46,7,{0,1,0,512},child,id(2))
    emit('childafter',34,4,{},child,id(2));emit('exit',45,1,{9},frame,id(2))
    emit('after',34,4,{},frame)
    return child
end
nested();local result=scan();assert(result.interrupts==1 and result.seeks==1 and #result.callbacks==2)
for _,case in ipairs({{'childbefore',function() marks.childbefore.cycle=marks.childbefore.cycle+1 end,'overlapping'},
    {'endpoint',function() marks.childbefore.frame.before_context.words[29]=512 end,'word 29'},
    {'parent',function() frame.after_context=context({}) end,'word 1'}}) do
    nested();case[2]();local ok,err=pcall(scan);assert(not ok and tostring(err):find(case[3],1,true),tostring(err))
end
print('CD lid checks: drive priorities, stops, predicates, exact schedules, nesting and corruptions passed')
