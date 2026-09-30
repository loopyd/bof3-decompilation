-- Local CDDA peak vectors; the last frame distinguishes a full-sector scan.
local directory=assert(arg[1],'output directory required')
local function write(name,bytes)
    local path=directory..'/'..name;local old=io.open(path,'rb')
    if old then old:close();error('fixture already exists: '..path) end
    local f=assert(io.open(path,'wb'));assert(f:write(bytes));assert(f:close())
end
write('data.bin',string.rep('\0',2352*75))
local sector=string.rep('\0\128\0\64',587)..'\0\128\208\138'
write('audio.bin',string.rep(sector,300))
write('media.cue','FILE "data.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n'..
    'FILE "audio.bin" BINARY\n  TRACK 02 AUDIO\n    INDEX 01 00:00:00\n')
print('CDDA peak media: left -32768; right 16384 with final-frame -30000')
