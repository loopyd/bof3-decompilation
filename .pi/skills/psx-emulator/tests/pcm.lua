-- Standalone LuaJIT contract checks; these do not establish native capture fidelity.
local m, pcm, wave = require 'support', require 'pcm', require 'wave'
local original_open = io.open
local files, reports, fault, status, blocks, events, bytes
local function copy(value)
    if type(value) ~= 'table' then return value end
    local result = {}; for k, v in pairs(value) do result[k] = copy(v) end; return result
end
local sample = string.char(0,0,0,0, 0,0,0,128, 0,0,224,63, 0,0,192,191)
local clock = '9007199254740993'
local function reset()
    files, reports, fault, bytes, m.captures = {}, {}, nil, sample, {}
    local settings = {configured_backend = '', configured_device = '', active_driver = 'fixture'}
    for _, k in ipairs({'volume','interpolation','reverb','mute','mono','streaming','null_sync',
        'irq_wait','decode_irq','scaler','forced_irq','voice_mute','voice_solo','xa'}) do settings[k] = 0 end
    status = {version=1, state=2, rate=44100, channels=2, format='f32le',
        stage='post-mixer-mute-mono/pre-SDL-conversion', frames=2, blocks=1, events=2,
        frame_limit=2646000, block_limit=65536, event_limit=65536, failures=0, first_failure=0,
        dropped_events=0, begin_cycle=clock, end_cycle='9007199254740999',
        dropped_frames='0', streaming_dropped='0', initial_voice_queue=4, initial_disc_queue=0,
        final_voice_queue=6, final_disc_queue=0, settings=settings}
    blocks = {{index=0, observed_cpu_cycle=clock, offset=0, frames=2, voice_frames=2, disc_frames=1, flags=4}}
    events = {{id=1, kind=1, cycle=clock, observed_frame='0', parent='0', address=0xbf801c00, value=65535, width=16},
        {id=2, kind=2, cycle=clock, observed_frame='2', parent='1', address=0x1f801c00, value=65535, width=16}}
end
m.write = function(name, value)
    assert(not files[name], 'duplicate test output')
    if name == fault then error('injected publication failure') end
    files[name] = value; m.captures[#m.captures+1] = {path=name, bytes=#value}
end
m.report = function(name, value) reports[name] = value end
io.open = function(name, mode)
    assert(mode == 'wb' and not files[name]); files[name] = ''
    return {write=function(_, value)
        if name == fault then return nil, 'injected journal failure' end
        files[name] = files[name] .. value; return true
    end, close=function() return true end}
end
PCSX = {Audio={status=function() return copy(status) end,
    samples=function(offset, frames) return bytes:sub(offset*8+1, (offset+frames)*8) end,
    block=function(index) return copy(blocks[index+1]) end,
    event=function(index) return copy(events[index+1]) end}}
reset()
local result = pcm.export()
assert(result.complete and result.pcm_bytes == 16 and result.zero_fill.disc_frames == 1)
assert(files['audio.f32'] == sample and files['audio.wav']:sub(59) == sample)
assert(#files['audio.wav'] == 74 and files['audio.wav']:sub(1,4) == 'RIFF')
assert(files['spu.ndjson']:find(clock, 1, true) and reports['audio.json'].native.begin_cycle == clock)
local wav = files['audio.wav']
assert(not pcall(wave.encode, 'bad') and not pcall(wave.encode, 3))
reset(); status.failures, status.first_failure, blocks[1].flags = 256, 256, 0
result = pcm.export(); assert(not result.complete and not result.export_error and result.failures[1] == 'output')
reset(); status.failures, status.first_failure, status.end_cycle = 32, 32, '0'
result = pcm.export(); assert(not result.complete and not result.export_error and #files['audio.f32'] == 16)
reset(); events[2].parent = '2'
result = pcm.export(); assert(not result.complete and result.events.error:find('parent') and result.events.rows == 2)
reset(); blocks[1].offset = 1
result = pcm.export(); assert(not result.complete and result.blocks.error and result.events.complete)
reset(); events[2].cycle = '9007199254740992'
result = pcm.export(); assert(not result.complete and result.events.error:find('regression'))
reset(); status.begin_cycle = 9007199254740992
result = pcm.export(); assert(not result.complete and result.export_error:find('decimal string'))
reset(); bytes = ''
result = pcm.export(); assert(not result.complete and result.export_error:find('short native PCM'))
reset(); status.dropped_frames = '1'
result = pcm.export(); assert(not result.complete and result.export_error:find('loss without failure'))
reset(); status.version = 2
result = pcm.export(); assert(not result.complete and result.export_error:find('ABI'))
reset(); fault = 'audio.wav'
result = pcm.export(); assert(not result.complete and result.export_error and files['audio.f32'] == sample)
reset(); fault = 'blocks.ndjson'
result = pcm.export(); assert(not result.complete and result.blocks.error and result.events.complete)
reset(); status.frames, status.blocks, status.events, bytes, events = 0, 0, 0, '', {}
result = pcm.export(); assert(result.complete and #files['audio.wav'] == 58 and files['spu.ndjson'] == '')
reset(); status.frames, status.blocks, status.events, bytes, events, blocks = 65538, 33, 0, sample:rep(32769), {}, {}
for i = 0, 32 do
    local n = math.min(2048, status.frames - i*2048)
    blocks[i+1] = {index=i, observed_cpu_cycle=clock, offset=i*2048, frames=n, voice_frames=n, disc_frames=0, flags=4}
end
result = pcm.export(); assert(result.complete and files['audio.f32'] == bytes)
io.open = original_open
if arg[1] then local file=assert(io.open(arg[1], 'wb')); assert(file:write(wav)); assert(file:close()) end
print('PCM contract checks passed: exact float bytes, uint64 order, bounds, loss, journals and publication failures')
