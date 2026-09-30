-- Shared bounded mission transport; Lua runs as trusted host code.
-- Native FFI calls can synchronously reenter Lua through emulator events.
-- Keep the host VM out of the trace compiler, including preexisting traces.
local jit = require 'jit'
jit.off()
jit.flush()
local ffi = require 'ffi'
local M = {captures = {}, retained = {}, done = false}
local array = {}
function M.requireIdleCapture()
    assert(PCSX.History and PCSX.History.status().state ~= 1,
        'host edits are forbidden during native history capture')
    -- Builds without Audio cannot have an active audio recorder. Capture itself
    -- requires the binding; this check supplies no alternative implementation.
    assert(not PCSX.Audio or PCSX.Audio.status().state ~= 1,
        'host edits are forbidden during native audio capture')
end
function M.array(value) return setmetatable(value or {}, array) end

function M.argument(name, default)
    return os.getenv('PSX_RUNTIME_ARG_' .. name:upper()) or default
end

function M.integer(name, default, minimum, maximum)
    local value = tonumber(M.argument(name, default and tostring(default)))
    assert(value and value % 1 == 0 and value >= minimum and value <= maximum,
        name .. ' must be an integer in ' .. minimum .. '..' .. maximum)
    return value
end

function M.address(name, default)
    local value = M.integer(name, default, 0, 4294967295)
    assert(value % 4 == 0, name .. ' must be word aligned')
    return value
end

function M.input(name, required)
    local path = os.getenv('PSX_RUNTIME_INPUT_' .. name:upper())
    assert(path or not required, 'missing --input ' .. name .. '=PATH')
    return path
end

function M.json(value)
    local kind = type(value)
    if kind == 'string' then
        return '"' .. value:gsub('[%z\1-\31\\"]', function(c)
            return string.format('\\u%04x', c:byte())
        end) .. '"'
    elseif kind == 'number' then
        assert(value == value and math.abs(value) ~= math.huge, 'nonfinite JSON number')
        return string.format('%.17g', value)
    elseif kind == 'boolean' then return tostring(value)
    elseif kind == 'nil' then return 'null'
    end
    assert(kind == 'table', 'unsupported JSON value: ' .. kind)
    local items = {}
    if #value > 0 or getmetatable(value) == array then
        for i = 1, #value do items[i] = M.json(value[i]) end
        return '[' .. table.concat(items, ',') .. ']'
    end
    local keys = {}
    for key in pairs(value) do keys[#keys + 1] = key end
    table.sort(keys)
    for _, key in ipairs(keys) do items[#items + 1] = M.json(key) .. ':' .. M.json(value[key]) end
    return '{' .. table.concat(items, ',') .. '}'
end

function M.write(name, bytes)
    assert(name:match('^[%w][%w_.-]*$') and #bytes <= 67108864, 'invalid capture name/size')
    for _, capture in ipairs(M.captures) do assert(capture.path ~= name, 'duplicate capture') end
    local file = assert(io.open(name, 'wb'))
    assert(file:write(bytes)); assert(file:close())
    M.captures[#M.captures + 1] = {path = name, bytes = #bytes}
end

function M.report(name, value) M.write(name, M.json(value)) end

function M.finish()
    assert(not M.done, 'mission already finished')
    M.done = true
    PCSX.pauseEmulator()
    local captures = #M.captures == 0 and '[]' or M.json(M.captures)
    local file = assert(io.open('completion.json', 'wb'))
    assert(file:write('{"schema":"psx.runtime-completion/v1","captures":' .. captures .. '}'))
    assert(file:close())
    PCSX.quit(0)
end

function M.guard(callback)
    return function(...)
        if M.done then return false end
        local ok, result = pcall(callback, ...)
        if not ok then
            M.done = true
            -- Refuse an enabled-JIT entry without reentering native code in it.
            jit.off()
            jit.flush()
            PCSX.pauseEmulator()
            io.stderr:write('mission failed: ' .. tostring(result) .. '\n')
            PCSX.quit(1)
            return false
        end
        return result
    end
end

function M.begin(allowed, callback)
    M.guard(function()
        assert(not jit.status(), 'maintained missions require host Lua JIT disabled')
        for name in (os.getenv('PSX_RUNTIME_ARGUMENT_NAMES') or ''):gmatch('[^,]+') do
            assert((',' .. allowed:upper() .. ','):find(',' .. name .. ',', 1, true),
                'unknown mission argument: ' .. name)
        end
        callback()
    end)()
end

function M.breakpoint(address, kind, width, callback)
    local bp = PCSX.addBreakpoint(address, kind, width, 'skill mission', M.guard(callback))
    M.retained[#M.retained + 1] = bp
    return bp
end

function M.event(name, callback)
    local event = PCSX.Events.createEventListener(name, M.guard(callback))
    M.retained[#M.retained + 1] = event
    return event
end

function M.registers()
    local regs, values = PCSX.getRegisters(), {}
    for i = 0, 31 do values[i + 1] = tonumber(regs.GPR.r[i]) end
    return {schema = 'psx.runtime-registers/v1', pc = tonumber(regs.pc),
        registers = values, cycles = tonumber(PCSX.getCPUCycles())}
end

function M.capture(save)
    M.write('ram.bin', ffi.string(PCSX.getMemPtr(), 2097152))
    M.report('registers.json', M.registers())
    if save then M.write('state.pbuf', tostring(PCSX.createSaveState())) end
end

function M.restore()
    local path = M.input('state', false)
    if not path then return false end
    M.requireIdleCapture()
    local snapshot = require 'snapshot'
    snapshot.validate(snapshot.read(path))
    local file = Support.File.open(path)
    PCSX.loadSaveState(file)
    file:close()
    PCSX.pauseEmulator()
    return true
end

function M.atTarget(callback)
    local raw = M.argument('target', os.getenv('PSX_RUNTIME_ENTRY'))
    local restored = M.restore()
    if raw then
        local target = M.address('target', tonumber(raw))
        if restored and tonumber(PCSX.getRegisters().pc) == target then callback(); return end
        M.breakpoint(target, 'Exec', 4, function() PCSX.pauseEmulator(); callback(); return false end)
        PCSX.resumeEmulator()
    else
        assert(restored, 'supply target, executable, or a saved state')
        callback()
    end
end

return M
