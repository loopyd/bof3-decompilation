-- Export frozen native history without converting uint64 identities or cycles to doubles.
local m, bit = require 'support', require 'bit'
local M = {}
local names = {'register-attempt', 'register-result', 'dma-start', 'dma-data',
    'dma-header', 'dma-complete', 'dma-irq', 'dma-deferred', 'dma-initial',
    'schedule', 'dispatch', 'irq-assert', 'irq-clear', 'irq-accept',
    'timer-before', 'timer-after', 'timer-read', 'timer-write', 'timer-gate',
    'discontinuity', 'dma-end', 'timer-rate', 'timer-initial', 'schedule-initial',
    'irq-initial', 'dma-device', 'dma-buffer', 'dma-cancel', 'dma-peek',
    'gpu-context', 'gpu-environment', 'gpu-input', 'gpu-result',
    'cd-context', 'cd-port', 'cd-command', 'cd-schedule', 'cd-response', 'cd-irq',
    'cd-media', 'cd-buffer', 'cd-data', 'cd-audio', 'cd-stream', 'cd-transition', 'cd-predicate', 'cd-image'}
local gpu_sources = {'cpu-gp0', 'cpu-gp1', 'direct-dma', 'chain-dma', 'read-dma',
    'cpu-read-gp0', 'host-gp0', 'host-gp1', 'host-read-gp0', 'host-replay'}
local failures = {'events', 'bytes', 'thread', 'unsupported', 'association',
    'discontinuity', 'bounds', 'clock-regression'}

function M.exact(value)
    local text = tostring(value):gsub('ULL$', '')
    assert(text:match('^%d+$'), 'invalid unsigned history integer')
    return text
end

-- Wire fields only. Whole-journal controller/stream/media/Audio relationships
-- belong to the CD capture consumer; a raw export cannot establish them.
local function cd_fields(record, data)
    local kind, phase, size = tonumber(record.kind), tonumber(record.device), tonumber(record.length)
    local labels, expected, result = nil, 0, {}
    local function variant(maximum)
        assert(phase >= 0 and phase <= maximum, 'unknown CD record variant')
    end
    if kind == 34 then
        variant(4)
        assert(data[1] == 2, 'unknown CD context version')
        if phase == 2 then
            labels = {'version','clock_hz','setting_count','scale_count','configured_backend_bytes',
                'configured_device_bytes','initialized','stream_present'}
            assert(data[3] == 14 and data[4] == 15 and data[5] <= 1024 and data[6] <= 1024 and
                data[7] <= 1 and data[8] <= 1, 'invalid CD environment layout')
            expected = 172 + data[5] + data[6]
        else
            labels = {'version','audio_joined','history_epoch_low','history_epoch_high',
                'audio_epoch_low','audio_epoch_high','word_count','reserved'}
            assert(data[2] <= 1 and data[7] == 56 and data[8] == 0, 'invalid CD context layout')
            expected = 256
        end
    elseif kind == 35 or (kind == 45 and phase <= 1) then
        variant(1)
        labels = {'role','address','width','value','pc','returned','depth','reserved'}
        assert(data[1] >= (kind == 35 and 1 or 5) and data[1] <= (kind == 35 and 4 or 12),
            'unknown CD operation role')
    elseif kind == 36 then
        variant(2)
        if phase == 0 then
            labels = {'command','parameter_count','old_queued_opcode','irq_status','old_response_ready'}
        elseif phase == 1 then
            labels = {'queued_opcode','latest_command','parameter_count','irq_status','saved_delay','repeated'}
        else
            labels = {'old_opcode','old_delay','old_repeated','submitted_opcode','submitted_delay',
                'repeated','old_owner_low','old_owner_high'}
        end
        expected = phase < 2 and 8 or 0
    elseif kind == 37 then
        variant(14)
        assert(data[1] <= 4 and data[6] <= 4, 'unknown CD scheduler phase/owner')
        labels = {'phase',data[1] == 0 and 'requested_delay' or (data[1] == 1 and 'role' or 'reserved'),
            'target_low','target_high','pending_mask','owner_kind',
            data[1] == 0 and 'old_owner_kind' or (data[1] == 1 and 'nested_helper' or 'reserved_7'),
            data[1] == 0 and 'queued_opcode' or 'reserved_8'}
        assert((data[1] == 0 and data[7] <= 4) or (data[1] == 1 and data[7] <= 1 and data[8] == 0) or
            (data[1] >= 2 and data[2] == 0 and data[7] == 0 and data[8] == 0), 'invalid CD scheduler tail')
        expected = data[1] == 0 and 8 or 0
    elseif kind == 38 then
        variant(2)
        if phase == 0 then labels = {'size','revision','displaced_low','displaced_high'}
        elseif phase == 1 then
            labels = {'size','revision','ready','cursor','irq_status'}
            assert(data[1] <= 16, 'CD response exceeds native buffer'); expected = data[1]
        else labels = {'cursor','wrapped_index','count','ready','within_count','returned_byte'} end
    elseif kind == 39 then
        variant(2)
        labels = ({[0]={'status','mask','asserted','response_building'},
            {'status','old_mask','written_mask'}, {'old_status','written_bits','status','mask'}})[phase]
    elseif kind == 40 then
        variant(2)
        labels = ({[0]={'lookup_kind','packed_msf','absolute_frame','prior_backing_low','prior_backing_high'},
            {'lookup_kind','native_success','disposition'},
            {'reason','count_low','count_high','track','file','backend_sector_bits','flags','base_bytes'}})[phase]
    elseif kind == 41 then
        variant(2)
        labels = phase == 1 and {'offset','byte_count','reason','packed_msf','absolute_frame','source_kind'} or
            {'offset','byte_count'}
        assert(data[1] <= 2352 and data[2] <= 2352-data[1], 'CD buffer range exceeds native storage')
        expected = data[2] + (phase == 1 and 32 or 0)
    elseif kind == 42 then
        variant(1)
        labels = {'original_index','address','returned_byte','old_read_ready','index','read_ready','wrapped','reserved'}
        expected = 8
    elseif kind == 43 then
        variant(5)
        if phase == 0 then labels = {'cdda','source_rate','source_frames','stereo'}; expected = 24
        elseif phase == 1 then
            labels = {'reason','source_rate','source_frames','stereo','output_frames','audio_state','event_disposition','cdda'}
            expected = 16
        else
            labels = {'gate_flags','mode','file','channel','first_sector_bits','subheader','decode_result_bits','cdda'}
            expected = 16
        end
    elseif kind == 44 then
        variant(1)
        labels = {'start','reason','previous_active','pending_mask','packed_msf','latest_command','status','slot_owner_kind'}
        expected = 40
    elseif kind == 45 then
        variant(5)
        if phase <= 3 then
            labels = {'control','irq_status','irq_mask','parameter_count','response_count','response_cursor','ready','transfer_index'}
        else labels = {'mode','file','channel','first_sector_bits','muted','attenuation','pending_attenuation','read_play_flags'} end
    elseif kind == 46 then
        assert(phase>=1 and phase<=11, 'unknown CD predicate selector')
        if phase==1 then labels={'istat','imask'}
        elseif phase==2 then
            labels={'site','present'}
            assert(data[1]>=1 and data[1]<=2 and data[2]<=1, 'invalid CD buffer predicate')
        elseif phase==3 then
            labels={'present'};assert(data[1]<=1, 'invalid CD subchannel predicate')
        elseif phase==4 then
            assert(data[1]==1 or data[1]==2, 'invalid CD SubQ predicate')
            if data[1]==1 then labels={'source'}
            else
                labels={'source','calculated_crc','stored_crc'};expected=12
                assert(data[2]<=65535 and data[3]<=65535, 'invalid CD SubQ CRC')
            end
        elseif phase==5 then
            labels={'patched'};expected=3;assert(data[1]<=1, 'invalid CD SBI predicate')
        elseif phase==6 then
            labels={'site','missing'}
            assert(data[1]>=1 and data[1]<=3 and data[2]<=1, 'invalid CD missing predicate')
        elseif phase==7 then
            labels={'lid_open','type','status','packed_msf'}
            assert(data[1]<=1 and data[4]<16777216, 'invalid CD status predicate')
        elseif phase==8 or phase==9 then
            labels={'value'};assert(data[1]<=65535, 'invalid CD SPU predicate')
        elseif phase==10 then
            labels={'handle_present','failed_evaluated','failed'}
            assert(data[1]<=1 and data[2]==data[1] and data[3]<=1 and (data[1]==1 or data[3]==0),
                'invalid CD availability predicate')
        else
            labels={'site','handle_present','mixed','missing'}
            assert(data[1]>=1 and data[1]<=2 and data[2]<=1 and data[3]<=1 and data[4]<=1,
                'invalid CD subchannel state')
        end
    elseif kind == 47 then
        variant(1)
        labels={'version','word_count','file_count','slot_count'}
        assert(data[1]==2 and data[2]==716 and data[3]<=202 and data[4]==100 and
            size>=2864+data[3]*32 and size<=836720, 'invalid CD image layout')
        expected=size
    end
    assert(labels and size == expected, 'CD record payload length mismatch')
    for i, label in ipairs(labels) do
        if label:match('^reserved') then assert(data[i]==0,'nonzero reserved CD field') end
        result[label] = data[i]
    end
    for i=#labels+1,8 do assert(data[i]==0,'nonzero unused CD field') end
    return result
end

local function fields(record, data)
    local k, result = tonumber(record.kind), {}
    local function assign(labels)
        for i, label in ipairs(labels) do result[label] = data[i] end
    end
    if k >= 34 and k <= 47 then return cd_fields(record, data)
    elseif k == 1 or k == 2 then
        assign({'address', 'requested_value', 'width', 'aligned_mirror', 'read'})
    elseif k == 3 then
        assign({'original_madr', 'normalized_madr', 'bcr', 'chcr', 'previous_chcr', 'dpcr'})
        result.previous_request = M.exact(record.related)
    elseif k == 4 or k == 29 then
        assign({'ram_address', 'device_address', 'byte_count', 'direction'})
        if record.device == 3 then result.mapped_offset = data[5] end
        result.direction = assert(({[0]='device-to-ram', [1]='ram-to-device'})[data[4]],
            'unknown DMA direction')
        result.device_address_scope = record.device == 3 and 'CD transfer-buffer index' or
            (record.device == 4 and 'SPU byte address' or 'unspecified')
        if record.device == 0 then result.input_request = M.exact(record.request)
        elseif record.device == 1 then result.input_request = M.exact(record.related) end
    elseif k == 5 then assign({'address', 'header', 'words'})
    elseif k == 6 then
        assign({'chcr_before', 'chcr_after'})
        result.touched_channel_request = M.exact(record.related)
    elseif k == 7 then
        assign({'stage', 'dicr'})
        result.touched_channel_request = M.exact(record.related)
    elseif k == 8 or k == 9 then assign({'madr', 'bcr', 'chcr'})
    elseif k == 10 then
        assign({'delay', 'prior_pending_mask', 'old_target_low', 'old_target_high',
            'old_request_low', 'old_request_high'})
        result.target_cycle = M.exact(record.related)
    elseif k == 11 then result.target_cycle = M.exact(record.related)
    elseif k == 12 or k == 13 then assign({'mask', 'istat_before', 'istat_after'})
    elseif k == 14 then assign({'istat', 'imask', 'cp0_status', 'pc'})
    elseif k == 17 then
        assign({'selector', 'value', 'mode_after', 'guest_address', 'guest_width'})
        if data[1] == 8 then result.mode_after = nil end
    elseif k == 15 or k == 16 or k == 18 or k == 19 or k == 22 or k == 23 then
        assign({'mode', 'target', 'rate', 'irq', 'counter_state', 'irq_state', 'gate_started', 'reason'})
        result.cycle_start = M.exact(record.related)
    elseif k == 21 then assign({'terminal_address'})
    elseif k == 24 then
        if record.device == 4294967295 then
            result.pending_mask, result.lowest_target = data[1], M.exact(record.related)
        else result.pending, result.target_cycle = data[1], M.exact(record.related) end
    elseif k == 25 then assign({'istat', 'imask', 'cp0_status', 'cp0_cause'})
    elseif k == 26 then
        if record.device == 0 then
            assign({'reason', 'reg0', 'reg1', 'pending_madr', 'pending_bcr', 'pending_chcr', 'buffer_offset'})
            result.buffer_input_request = M.exact(record.related)
        elseif record.device == 1 then
            assign({'reason', 'source_offset', 'source_end', 'buffer_offset'})
            result.retained_input_request = M.exact(record.related)
        else error('unknown DMA device-state variant') end
    elseif k == 27 then
        assign({'byte_count'})
        result.input_request = M.exact(record.related)
    elseif k == 28 then result.discarded_buffer_request = M.exact(record.related)
    elseif k == 30 or k == 31 then
        assert(record.length == 0 and (record.device == 0 or record.device == 1), 'invalid GPU context')
        result.phase = record.device == 0 and 'begin' or 'end'
        if k == 30 then
            assign({'processor', 'read_fifo_bytes', 'data_return', 'ram_mask', 'known_environment',
                'ready', 'reserved_1', 'reserved_2'})
            assert(data[5] <= 7 and data[6] <= 1 and data[7] == 0 and data[8] == 0,
                'invalid GPU context flags')
        else
            assign({'last_page', 'last_window', 'last_offset', 'window_mirror', 'area_start_mirror',
                'area_end_mirror', 'offset_mirror', 'reserved'})
            assert(data[8] == 0, 'invalid GPU environment flags')
            result.known_scope = 'last_* availability comes from paired gpu-context bitmask; zero is not proof of initialization'
        end
    elseif k == 32 or k == 33 then
        assert(record.device == 2 and record.length == 0 and data[8] <= 1, 'invalid GPU input boundary')
        assign({'source_index', 'origin', 'words', 'value', 'processor', 'read_fifo_bytes', 'pc', 'ready'})
        result.source = assert(gpu_sources[data[1] + 1], 'unknown GPU input source')
        result.buffer = M.exact(record.related)
        assert(result.buffer ~= '0', 'missing GPU buffer identity')
        result.value_scope = (data[1] == 5 or data[1] == 8)
            and 'read result only at gpu-result; input value is a placeholder'
            or 'write operand for CPU/host writes; DMA bytes reside in enclosed dma-data'
    end
    return result
end

-- Shared wire decoding for raw export and device correlation. This validates
-- the row layout only; callers still own ordering, payload bounds and ancestry.
function M.describe(record)
    local kind = assert(names[tonumber(record.kind)], 'unknown native history kind')
    local data = m.array()
    for i = 0, 7 do data[i + 1] = tonumber(record.data[i]) end
    local decoded = fields(record, data)
    if record.kind == 4 then
        assert(record.length == data[3], 'DMA payload length differs from native byte count')
    end
    return kind, data, decoded
end

function M.export()
    local h = assert(PCSX.History, 'native history binding required')
    local status = h.status()
    assert(status.version == 1 and status.state == 2, 'unsupported or unfrozen native history')
    local count, size = tonumber(status.events), tonumber(status.bytes)
    assert(count <= 65536 and size <= 8388608, 'native history exceeds ABI limits')
    m.write('payload.bin', h.bytes(0, size))
    local file = assert(io.open('events.ndjson', 'wb'))
    local total, offset, previous = 0, 0, status.beginCycle
    local kinds, channels = {}, {}
    local ok, err = pcall(function()
        for index = 0, count - 1 do
            local record = h.record(index)
            local kind, data, decoded = M.describe(record)
            assert(record.sequence == index + 1, 'noncontiguous native event sequence')
            assert(record.cycle >= previous and record.cycle <= status.endCycle,
                'native event outside monotonic capture interval')
            previous = record.cycle
            assert(record.offset == offset and offset + record.length <= size,
                'native event payload bounds are inconsistent')
            local value = {sequence = M.exact(record.sequence), cycle = M.exact(record.cycle),
                request = M.exact(record.request), related = M.exact(record.related), kind = kind,
                device = tonumber(record.device), raw = data, fields = decoded,
                payload = {offset = offset, bytes = tonumber(record.length)}}
            local line = m.json(value) .. '\n'
            assert(total + #line <= 67108864, 'event export exceeds 64 MiB')
            assert(file:write(line)); total = total + #line
            offset = offset + tonumber(record.length)
            kinds[kind] = (kinds[kind] or 0) + 1
            if record.kind == 4 then
                assert(record.length == data[3], 'DMA byte count differs from retained payload')
                local channel = tostring(tonumber(record.device))
                channels[channel] = (channels[channel] or 0) + tonumber(record.length)
            end
        end
        assert(offset == size, 'unreferenced native payload bytes')
    end)
    assert(file:close())
    m.captures[#m.captures + 1] = {path = 'events.ndjson', bytes = total}
    local faults = m.array()
    for i, name in ipairs(failures) do
        if bit.band(status.failures, 2 ^ (i - 1)) ~= 0 then faults[#faults + 1] = name end
    end
    if bit.band(status.failures, 0xffffff00) ~= 0 then faults[#faults + 1] = 'unknown-native-failure' end
    local result = {schema = 'psx.native-history/v1', native_version = 1,
        begin_cycle = M.exact(status.beginCycle), end_cycle = M.exact(status.endCycle),
        events = count, payload_bytes = size, exported_bytes = total, kinds = kinds,
        transferred_bytes = channels, failures = faults, failure_bits = tonumber(status.failures),
        first_failure = tonumber(status.firstFailure), dropped_attempts = tonumber(status.dropped),
        complete = ok and status.failures == 0, export_error = not ok and tostring(err) or nil,
        ordering = 'native CPU-thread hook order; bulk records do not imply byte-cycle timing',
        request_zero = 'absent or pre-capture owner; inspect initial state and pending events',
        payload_scope = 'device context and copied buffers; only dma-data counts as transferred bytes',
        cd_scope = kinds['cd-context'] and 'wire export only; CD lifecycle, ownership and Audio joins require the CD consumer' or nil,
        irq_scope = 'CPU-visible latch/delivery; SPU worker requests may coalesce',
        host_edits = 'audited mission required; arbitrary Lua pointer writes are not intercepted'}
    m.report('events.json', result)
    return result
end
return M
