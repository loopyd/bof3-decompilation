-- Native displayed pixels with explicit storage metadata; no VRAM inference.
local M = {}
function M.capture()
    local screen = PCSX.GPU.takeScreenShot()
    local bytes, bpp = tostring(screen.data), tonumber(screen.bpp)
    assert(bpp == 0 or bpp == 1, 'unsupported screenshot pixel format')
    local width, height = tonumber(screen.width), tonumber(screen.height)
    assert(width <= 1024 and height <= 512, 'native display dimensions exceed bounds')
    assert(#bytes == width * height * (bpp == 0 and 2 or 3), 'unexpected screenshot storage')
    return bytes, {width = width, height = height, format = bpp == 0 and 'psx-bgr555-le' or 'rgb24',
        bytes = #bytes, available = #bytes > 0}
end

function M.compare(before, after, left, right, options)
    local differences, equal = require('support').array(), true
    for _, key in ipairs({'width', 'height', 'format', 'bytes', 'available'}) do
        if left[key] ~= right[key] then
            equal = false
            differences[#differences + 1] = {field = key, before = left[key], after = right[key]}
        end
    end
    local pixels = require('difference').compare(before, after, 'bytes', false, 'display.pixels', options)
    pixels.scope, pixels.presence = 'native display byte storage', 'captured bytes'
    return {schema = 'psx.runtime-display-comparison/v1', equal = equal and pixels.equal,
        metadata = differences, before = left, after = right, pixels = pixels,
        image_comparison_available = left.available and right.available}
end
return M
