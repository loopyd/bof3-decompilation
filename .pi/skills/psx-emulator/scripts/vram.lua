-- Export native VRAM rectangles/textures or upload a bounded 16-bit rectangle via GP0.
local m, s = require 'support', require 'snapshot'
m.begin('action,target,x,y,width,height,bpp,clut_x,clut_y,savestate,scratch', function()
    local action = m.argument('action', 'export')
    assert(action == 'export' or action == 'upload', 'action must be export or upload')
    local x, y = m.integer('x', 0, 0, 1023), m.integer('y', 0, 0, 511)
    local bpp = m.integer('bpp', 16, 4, 16)
    assert(bpp == 4 or bpp == 8 or bpp == 16, 'bpp must be 4, 8, or 16')
    local width = m.integer('width', (1024 - x) * 16 / bpp, 1, (1024 - x) * 16 / bpp)
    local height = m.integer('height', 512 - y, 1, 512 - y)
    local rowBytes = math.ceil(width * bpp / 8)
    local cx, cy
    if bpp ~= 16 then
        cx = m.integer('clut_x', nil, 0, 1024 - 2 ^ bpp)
        cy = m.integer('clut_y', nil, 0, 511)
    end
    local save = m.integer('savestate', 0, 0, 1) == 1
    local function extract(state, source, probe)
        local bytes = assert(state.gpu.vram)
        assert(#bytes == 1048576, 'unexpected VRAM size')
        local rows, rgb, masks = {}, {}, {}
        local function word(offset) return bytes:byte(offset + 1) + bytes:byte(offset + 2) * 256 end
        local function expand(v) return math.floor(v * 255 / 31 + 0.5) end
        for row = 0, height - 1 do
            local start = (y + row) * 2048 + x * 2
            rows[#rows + 1] = bytes:sub(start + 1, start + rowBytes)
            for col = 0, width - 1 do
                local pixel
                if bpp == 16 then pixel = word(start + col * 2)
                else
                    local packed = bytes:byte(start + math.floor(col * bpp / 8) + 1)
                    local index = math.floor(packed / 2 ^ ((col * bpp) % 8)) % 2 ^ bpp
                    pixel = word(cy * 2048 + (cx + index) * 2)
                end
                rgb[#rgb + 1] = string.char(expand(pixel % 32), expand(math.floor(pixel / 32) % 32),
                    expand(math.floor(pixel / 1024) % 32))
                masks[#masks + 1] = string.char(math.floor(pixel / 32768))
            end
        end
        m.write('vram.bin', table.concat(rows))
        m.write('vram.ppm', 'P6\n' .. width .. ' ' .. height .. '\n255\n' .. table.concat(rgb))
        m.write('mask.bin', table.concat(masks))
        m.report('vram.json', {schema = 'psx.runtime-vram/v1', action = action, source = source,
            x_words = x, y = y, width_pixels = width, height = height, bpp = bpp,
            row_bytes = rowBytes, clut_x = cx, clut_y = cy, gpu_status = state.gpu.status,
            fifo_reset = action == 'upload', transparency_composited = false, probe = probe})
        if save then
            assert(source == 'live', 'savestate requires a live boundary')
            m.write('state.pbuf', tostring(PCSX.createSaveState()))
        end
        m.finish()
    end
    if action == 'export' then s.observe(extract); return end
    assert(bpp == 16, 'upload accepts 16-bit pixels only')
    local file = assert(io.open(m.input('pixels', true), 'rb'))
    local pixels = assert(file:read(rowBytes * height + 1)); file:close()
    assert(#pixels == rowBytes * height, 'pixels input must exactly fit rectangle')
    m.atTarget(function()
        local operations = {{0x1f801814, 0x01000000}, {0x1f801810, 0xa0000000},
            {0x1f801810, y * 65536 + x}, {0x1f801810, height * 65536 + width}}
        pixels = pixels .. string.rep('\0', (4 - #pixels % 4) % 4)
        for i = 1, #pixels, 4 do
            operations[#operations + 1] = {0x1f801810, pixels:byte(i) + pixels:byte(i + 1) * 256 +
                pixels:byte(i + 2) * 65536 + pixels:byte(i + 3) * 16777216}
        end
        require('bus').write(32, operations, function(probe) extract(s.live(), 'live', probe) end)
    end)
end)
