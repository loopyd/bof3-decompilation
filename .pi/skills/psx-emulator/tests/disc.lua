-- Generate local synthetic data/CDDA media for native capture checks; no game data.
local directory = assert(arg[1], 'output directory required')
local function write(name, data)
    local path = directory .. '/' .. name
    local existing = io.open(path, 'rb')
    if existing then existing:close(); error('fixture already exists: ' .. path) end
    local file = assert(io.open(path, 'wb'))
    assert(file:write(data)); assert(file:close())
end
write('data.bin', string.rep('\0', 2352 * 75))
-- Constant signed16 stereo: left 8192, right -4096. One sector is 588 frames.
write('audio.bin', string.rep(string.char(0, 32, 0, 240), 588 * 300))
write('media.cue', 'FILE "data.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n' ..
    'FILE "audio.bin" BINARY\n  TRACK 02 AUDIO\n    INDEX 01 00:00:00\n')
local sectors = {}
local function bcd(n) return math.floor(n/10)*16+n%10 end
for n = 0,299 do
    local frame = n+150
    local subheader = string.char(1, n%4 == 0 and 0 or 1, 0x64, 1)
    local group = string.rep('\0',16) .. string.rep(string.char(0xf2),112)
    sectors[#sectors+1] = '\0' .. string.rep(string.char(255),10) .. '\0' ..
        string.char(bcd(math.floor(frame/4500)),bcd(math.floor(frame/75)%60),bcd(frame%75),2) ..
        subheader .. subheader .. string.rep(group,18) .. string.rep('\0',24)
    assert(#sectors[#sectors] == 2352)
end
-- Filter-0 XA has the same constant levels at 37800 Hz; channel 0 every fourth sector.
-- EDC is zero: this is a decoder fixture, not a mastered-disc conformance image.
write('xa.bin', table.concat(sectors))
write('xa.cue', 'FILE "xa.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
local data = {}
for n = 0,299 do
    local header = sectors[n+1]:sub(1,16)
    local subheader = string.char(1,0,8,0)
    local payload = {}
    for byte = 0,2047 do payload[#payload+1] = string.char((n*17+byte)%251) end
    data[#data+1] = header .. subheader .. subheader .. table.concat(payload) .. string.rep('\0',280)
    assert(#data[#data] == 2352)
end
write('sector.bin',table.concat(data))
write('sector.cue','FILE "sector.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
print('Synthetic decoder discs: CDDA, multiplexed XA and distinct data-sector payloads; zero EDC/ECC')
