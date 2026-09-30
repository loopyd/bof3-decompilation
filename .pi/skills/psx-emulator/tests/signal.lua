-- Check exported synthetic disc/mixed signals independently of the native recorder.
local ffi = require 'ffi'
local directory, scenario = assert(arg[1], 'capture directory required'), assert(arg[2], 'scenario required')
assert(scenario == 'cdda' or scenario == 'xa' or scenario == 'mix', 'expected cdda, xa or mix')
local function read(name)
    local file = assert(io.open(directory .. '/' .. name, 'rb'))
    local bytes = assert(file:read('*a')); assert(file:close()); return bytes
end
local raw, wav = read('audio.f32'), read('audio.wav')
assert(#raw > 16000 and #raw % 8 == 0 and #raw <= 88200 * 8, 'PCM size outside fixture bounds')
local function u32(bytes, offset)
    local a,b,c,d = bytes:byte(offset, offset+3)
    assert(d, 'short RIFF integer'); return a+b*256+c*65536+d*16777216
end
assert(wav:sub(1,4) == 'RIFF' and wav:sub(9,12) == 'WAVE' and u32(wav,5) == #wav-8)
local cursor, data, fact = 13, nil, nil
while cursor <= #wav do
    local name, length = wav:sub(cursor,cursor+3), u32(wav,cursor+4)
    local payload = wav:sub(cursor+8,cursor+7+length)
    assert(#payload == length, 'truncated RIFF chunk')
    if name == 'fmt ' then
        assert(payload:sub(1,4) == string.char(3,0,2,0) and u32(payload,5) == 44100)
    elseif name == 'data' then assert(not data); data = payload
    elseif name == 'fact' then fact = u32(payload,1) end
    cursor = cursor+8+length+length%2
end
assert(data == raw and fact == #raw/8, 'WAV payload/count differs from raw PCM')
local samples = ffi.new('float[?]', #raw/4); ffi.copy(samples,raw,#raw)
local silence, steady, varied, transitions = 0,0,0,0
local left, right = 8192/32767, -4096/32767
for frame = 0,#raw/8-1 do
    local l,r = tonumber(samples[frame*2]),tonumber(samples[frame*2+1])
    assert(l == l and r == r and math.abs(l) < 2 and math.abs(r) < 2, 'invalid fixture sample')
    if l == 0 and r == 0 then silence = silence+1
    elseif math.abs(l-left) < 0.001 and math.abs(r-right) < 0.001 then steady = steady+1
    elseif scenario == 'mix' and math.abs(l-left) < 0.15 and math.abs(r-right) < 0.08 then
        varied = varied+1
    else transitions = transitions+1 end
end
assert(steady > 1000, 'disc steady channel levels missing')
if scenario ~= 'mix' then assert(transitions <= 8, 'unexpected disc signal outside startup transient')
else assert(varied > 4000 and transitions < 2000, 'SPU contribution missing or signal exceeds expected mix') end
print(string.format('%s frames=%d silence=%d steady=%d mixed=%d transitions=%d; WAV exact',
    scenario,#raw/8,silence,steady,varied,transitions))
