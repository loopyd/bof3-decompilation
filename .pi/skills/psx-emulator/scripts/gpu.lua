-- Inspect explicitly supplied GPU streams or an immutable ordering-table RAM image.
local m, packets = require 'support', require 'packets'
if m.argument('action','decode')=='capture' then require('video').run(); return end
m.begin('action,identity,port,address,words,packets', function()
    PCSX.pauseEmulator()
    local action=m.argument('action','decode')
    assert(action=='decode' or action=='chain','GPU action must be decode or chain')
    local identity=assert(m.argument('identity'),'identity is required')
    assert(#identity>0 and #identity<=160,'identity must be 1..160 bytes')
    local limits={words=m.integer('words',65536,1,1048576),packets=m.integer('packets',4096,1,65536)}
    local function read(name,maximum)
        local file=assert(io.open(m.input(name,true),'rb'))
        local data=file:read(maximum+1) or ''; assert(file:close())
        assert(#data<=maximum,name..' input exceeds byte limit')
        return data
    end
    local bytes, chain, port
    if action=='chain' then
        assert(not m.argument('port'),'port applies only to decode')
        assert(not m.input('words',false),'chain requires ram input, not words')
        chain=packets.chain(read('ram',2097152),m.address('address'),limits)
        bytes,port=chain.bytes,'gp0'; chain.bytes=nil
    else
        assert(not m.argument('address'),'address applies only to chain')
        assert(not m.input('ram',false),'decode requires words input, not ram')
        bytes,port=read('words',limits.words*4),m.argument('port','gp0')
        assert(port=='gp0' or port=='gp1','port must be gp0 or gp1')
    end
    -- Preserve exact input even when decoding fails; never publish partial success.
    m.write('gpu.bin',bytes)
    local decoded=packets[port](bytes,limits)
    m.report('gpu.json',{schema='psx.runtime-gpu-packets/v1',action=action,identity=identity,
        identity_authority='caller label; input and tool hashes in harness receipt',
        port=port,words=#bytes/4,packets=m.array(decoded),chain=chain,limits=limits,
        source='caller-supplied immutable bytes; not observed command execution',
        boundary='complete GP0 packets or independent GP1 words; no inferred prior parser state',
        profile='pinned Redux parser fields; no rasterization, execution timing or hardware equivalence'})
    m.finish()
end)
