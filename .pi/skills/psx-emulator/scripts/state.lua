-- Inspect a supplied raw save state without loading it into the running machine.
local m = require 'support'
m.begin('action,path,depth,offset,length,limit', function()
    PCSX.pauseEmulator()
    local snapshot = require 'snapshot'
    local action = m.argument('action', 'query')
    if action == 'schema' then m.write('state.proto', snapshot.schema); m.finish(); return end
    assert(action == 'query' or action == 'export' or action == 'compare', 'unknown state action')
    local path = m.argument('path', '')
    local comparison = action == 'compare'
    local value, kind, repeated = snapshot.select(snapshot.read(m.input('state', true), comparison), path, comparison)
    if action == 'export' then
        assert(kind == 'bytes' and not repeated, 'export requires a bytes field')
        local offset = m.integer('offset', 0, 0, #value)
        local length = m.integer('length', #value - offset, 0, #value - offset)
        m.write('region.bin', value:sub(offset + 1, offset + length))
        m.report('region.json', {schema = 'psx.runtime-region/v1', path = path,
            offset = offset, bytes = length, source_bytes = #value})
    elseif action == 'compare' then
        assert(not m.argument('offset') and not m.argument('length'), 'compare selects whole schema fields; offset/length are export-only')
        local other = snapshot.select(snapshot.read(m.input('other', true), true), path, true)
        local report = require('difference').compare(value, other, kind, repeated, path,
            {depth = m.integer('depth', 16, 0, 64), limit = m.integer('limit', 1024, 1, 65536)})
        report.schema_compatibility = 'all encoded fields validated against running schema; version 4'
        report.identities = 'input and emulator hashes in receipt.json; exact schema in state.proto'
        m.write('state.proto', snapshot.schema)
        m.report('comparison.json', report)
    else
        m.report('query.json', {schema = 'psx.runtime-query/v1', path = path,
            value = snapshot.describe(value, kind, repeated, m.integer('depth', 1, 0, 8))})
    end
    m.finish()
end)
