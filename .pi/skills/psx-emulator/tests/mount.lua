-- Literal harness media transport; no file reads or native guest execution.
local mount, m = require 'mount', require 'support'
local hash=string.rep('a',64)
local function row(n,size,digest)
    return string.format('disc:track%02d.bin\t%s\t%s\n',n,size or '2352',digest or hash)
end
local function fails(directory,text,needle)
    local ok,err=pcall(mount.parse,directory,text)
    assert(not ok and tostring(err):find(needle,1,true),tostring(err))
end
assert(#mount.parse('/capture','')==0)
local entries=mount.parse('/capture',row(1)..row(2))
assert(#entries==2 and entries[1].key=='disc:track01.bin' and entries[1].bytes=='2352')
assert(entries[1].sha256==hash and entries[1].path_hex==
    '2f636170747572652f646973632f747261636b30312e62696e')
assert(entries[2].path_hex~=entries[1].path_hex, 'aliases must keep distinct staged paths')
local arbitrary=mount.parse('/capt ure\n\255',row(1))
assert(arbitrary[1].path_hex:find('0aff',1,true) and not m.json(arbitrary):find('\255',1,true))
fails(nil,'','runtime directory')
for _,directory in ipairs({'','relative','/','/capture/','/capture/..','/capture/.',
    '/capture//x','/x/../y','/x/./y','/x\0y', '/'..string.rep('x',4096)}) do
    fails(directory,'','runtime directory')
end
fails('/capture',nil,'media identities')
fails('/capture',row(1):sub(1,-2),'media identities')
fails('/capture',string.rep('x',12289),'media identities')
for _,text in ipairs({'\n',row(2),row(1)..row(1),row(0),row(1)..'\n',
    row(1,'0'),row(1,'01'),row(1,'-1'),row(1,'1.0'),row(1,'1e3'),
    row(1,'2352',hash:upper()),row(1,'2352',hash..'a'),
    row(1):gsub('\t',' ',1),row(1):gsub('\n','\r\n'),row(1)..'trailing\n'}) do
    fails('/capture',text,'media identity')
end
fails('/capture',row(1,'1073741825'),'staging bounds')
assert(#mount.parse('/capture',row(1,'1073741824')..row(2,'1073741824'))==2)
fails('/capture',row(1,'1073741824')..row(2,'1073741824')..row(3,'1'),'staging bounds')
local rows={};for n=1,99 do rows[n]=row(n,'1') end
assert(#mount.parse('/capture',table.concat(rows))==99)
fails('/capture',table.concat(rows)..row(100,'1'),'media identity')
fails('/'..string.rep('x',4080),row(1),'descriptor bound')
print('CD mount checks: exact staged paths, raw bytes, ordered aliases, missing data and bounded literal identities passed')
