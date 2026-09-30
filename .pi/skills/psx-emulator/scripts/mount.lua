-- Associate native absolute paths with literal harness staging identities.
-- Final input stability is established by the passed harness receipt, not Lua.
local m = require 'support'
local M = {}
local function hex(bytes)
    return (bytes:gsub('.',function(c) return string.format('%02x',c:byte()) end))
end

function M.parse(directory, text)
    assert(type(directory)=='string' and #directory>1 and #directory<=4096 and
        directory:sub(1,1)=='/' and directory:sub(-1)~='/' and not directory:find('%z') and
        not directory:find('//',1,true) and not (directory..'/'):find('/./',1,true) and
        not (directory..'/'):find('/../',1,true), 'missing or invalid harness runtime directory')
    assert(type(text)=='string' and #text<=12288 and (text=='' or text:sub(-1)=='\n'),
        'missing or invalid harness media identities')
    local entries,total=m.array(),0
    for line in text:gmatch('([^\n]*)\n') do
        local key,size,hash=line:match('^(disc:track%d%d%.bin)\t([1-9]%d*)\t([0-9a-f]+)$')
        local ordinal=#entries+1
        assert(key and ordinal<=99 and key==string.format('disc:track%02d.bin',ordinal) and
            #size<=10 and #hash==64, 'invalid or unordered harness media identity')
        local bytes=tonumber(size)
        total=total+bytes
        assert(bytes<=1073741824 and total<=2147483648, 'harness media exceeds staging bounds')
        local path=directory..'/disc/'..key:sub(6)
        assert(#path<=4096, 'staged CD path exceeds native descriptor bound')
        entries[ordinal]={key=key,path_hex=hex(path),bytes=size,sha256=hash}
    end
    return entries
end

function M.read()
    return M.parse(os.getenv('PSX_RUNTIME_DIRECTORY'),os.getenv('PSX_RUNTIME_MEDIA'))
end
return M
