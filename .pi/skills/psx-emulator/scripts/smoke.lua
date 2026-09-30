-- Startup and Lua command smoke test; no guest-state acceptance.
local file = assert(io.open('completion.json', 'wb'))
assert(file:write('{"schema":"psx.runtime-completion/v1","captures":[]}'))
assert(file:close())
PCSX.quit(0)
