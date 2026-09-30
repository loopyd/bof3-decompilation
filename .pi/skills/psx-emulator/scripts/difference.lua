-- Exact bounded schema comparisons; no equality claim after omitted work.
local pb, m = require 'pb', require 'support'
local M = {}
function M.compare(before, after, kind, repeated, path, options)
    options = options or {}
    local report = {schema = 'psx.runtime-comparison/v2', path = path,
        scope = 'selected schema subtree', presence = 'encoded fields',
        equal = true, compared_nodes = 0, compared_bytes = 0,
        changed_bytes = 0, differences = m.array()}
    local limit, depth = options.limit or 1024, options.depth or 16
    local nodes, bytes = options.nodes or 100000, options.bytes or 67108864
    local function add(entry)
        assert(#report.differences < limit, 'comparison difference limit exceeded; narrow the path or increase limit')
        report.equal = false
        report.differences[#report.differences + 1] = entry
    end
    local function summary(value, fieldType)
        if value == nil then return {present = false} end
        if fieldType == 'bytes' then return {present = true, bytes = #value} end
        if type(value) == 'table' then return {present = true, type = fieldType} end
        return {present = true, value = value}
    end
    local function preview(value, first, last)
        return value:sub(first, math.min(last, first + 31)):gsub('.', function(c)
            return string.format('%02x', c:byte())
        end)
    end
    local function compare(a, b, fieldType, array, fieldPath, remaining)
        report.compared_nodes = report.compared_nodes + 1
        assert(report.compared_nodes <= nodes, 'comparison node limit exceeded')
        if a == nil or b == nil then
            if a ~= b then add({path = fieldPath, kind = 'presence',
                before = summary(a, fieldType), after = summary(b, fieldType)}) end
            return
        end
        if not array and fieldType == 'bytes' then
            local count = math.max(#a, #b)
            report.compared_bytes = report.compared_bytes + count
            assert(report.compared_bytes <= bytes, 'comparison byte limit exceeded')
            local first
            local function flush(last)
                if not first then return end
                add({path = fieldPath, kind = 'bytes', offset = first - 1,
                    length = last - first + 1, before_bytes = #a, after_bytes = #b,
                    before_hex = preview(a, first, last), after_hex = preview(b, first, last),
                    preview_truncated = last - first + 1 > 32})
                first = nil
            end
            for start = 1, count, 4096 do
                local finish = math.min(start + 4095, count)
                if a:sub(start, finish) == b:sub(start, finish) then flush(start - 1)
                else
                    for i = start, finish do
                        if a:byte(i) ~= b:byte(i) then
                            first = first or i
                            report.changed_bytes = report.changed_bytes + 1
                        else flush(i - 1) end
                    end
                end
            end
            flush(count)
        elseif type(a) ~= 'table' and type(b) ~= 'table' then
            if a ~= b then add({path = fieldPath, kind = 'value',
                before = summary(a, fieldType), after = summary(b, fieldType)}) end
        else
            assert(type(a) == 'table' and type(b) == 'table', 'incompatible decoded field: ' .. fieldPath)
            assert(remaining > 0, 'comparison depth exceeded at ' .. fieldPath .. '; narrow the path or increase depth')
            if array then
                for i = 1, math.max(#a, #b) do
                    compare(a[i], b[i], fieldType, false, fieldPath .. '[' .. i .. ']', remaining - 1)
                end
            else
                local fields = {}
                for name, number, childType, _, label in pb.fields(fieldType) do
                    fields[#fields + 1] = {name = name, number = number, type = childType,
                        repeated = label == 'repeated' or label == 'packed'}
                end
                table.sort(fields, function(x, y) return x.number < y.number end)
                for _, field in ipairs(fields) do
                    local childPath = fieldPath == '' and field.name or fieldPath .. '.' .. field.name
                    compare(a[field.name], b[field.name], field.type, field.repeated, childPath, remaining - 1)
                end
            end
        end
    end
    compare(before, after, kind, repeated, path, depth)
    report.before_present, report.after_present = before ~= nil, after ~= nil
    report.difference_count = #report.differences
    report.complete = true
    return report
end
return M
