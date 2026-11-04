import hashlib
import json
import stat
import struct
from pathlib import Path

from harness.match.extraction import read_function_bytes, read_linked_image

root = Path.cwd().resolve()
path = root / 'out/payloads/game00-first-naming-post/report.json'
report = json.loads(path.read_bytes())
assert report['target'] == 'emi/etc/game/00'
sha = lambda data: hashlib.sha256(data).hexdigest()
file = lambda name: root / name
original = file(report['original']).read_bytes()
payload = file(report['output']).read_bytes()
assert payload == original and len(payload) == report['size']
assert sha(payload) == report['sha256'] and report['whole_payload_equal']
assert report['compiled_bytes'] > 0
intervals = sorted(report['compiled'] + report['retained'], key=lambda row: row['start'])
end = 0
for row in intervals:
    assert row['start'] == end and row['end'] > end
    end = row['end']
assert end == len(payload)
for key in ('compiled', 'retained'):
    assert sum(row['end'] - row['start'] for row in report[key]) == report[key + '_bytes']
for name, expected in report['inputs'].items():
    p = file(name)
    assert (sha(p.read_bytes()) if p.exists() else None) == expected, name

comparisons = {row['source']: row for row in report['source_comparisons']}
assert len(comparisons) == len(report['source_comparisons'])
partial = []
for name, row in comparisons.items():
    comparison = row['comparison']
    assert comparison['fresh_compilation']
    if row['compiled_range_count'] == 0:
        assert comparison['status'] == 'different'
        partial.append({'source': name, 'status': comparison['status'], 'original_size': comparison.get('original_size'), 'current_size': comparison.get('current_size'), 'images': comparison.get('function_image')})
    for key in ('native_inputs', 'include_inputs', 'native_outputs', 'outputs'):
        for name, expected in comparison.get(key, {}).items():
            p = file(name)
            assert (sha(p.read_bytes()) if p.exists() else None) == (None if expected is None else expected['sha256']), name
            if expected and 'metadata' in expected:
                current = p.stat(follow_symlinks=False)
                for field, value in expected['metadata'].items():
                    actual = stat.S_IFMT(current.st_mode) if field == 'st_mode' else getattr(current, field)
                    assert actual == value, (name, field)

for row in report['compiled']:
    assert row['start'] == row['normalized_start'] and row['address'] == 0x80195800 + row['start']
    content = payload[row['start']:row['end']]
    assert sha(content) == row['sha256'] and content == original[row['start']:row['end']]
    comparison = comparisons[row['source']]['comparison']
    linked = file(comparison['placement']['path']) if 'placement' in comparison else Path(comparison['current_object']).with_suffix('.linked.o')
    if row['symbol']:
        actual = read_function_bytes(linked.read_bytes(), function_name=row['symbol'], address=row['address']).content
    else:
        image = read_linked_image(linked.read_bytes(), limit=64 * 1024 * 1024)
        sections = [part for part in image.sections if part.name == row['section'] and part.address == row['address']]
        assert len(sections) == 1
        actual = sections[0].content
    assert actual == content, row

archive = report['archive']
assert archive['index'] == 0 and archive['whole_archive_equal'] and archive['entry_equal']
archive_bytes = file(archive['output']).read_bytes()
assert archive_bytes == file(archive['original']).read_bytes() and sha(archive_bytes) == archive['sha256']
assert (path.parent / 'entry.bin').read_bytes() == payload
assert struct.unpack_from('<I', archive_bytes)[0] == 2
entry_size, entry_load, _, entry_type, _ = struct.unpack_from('<IIIHH', archive_bytes, 0x10)
assert entry_size == len(payload) and entry_load == 0x80195800 and entry_type == 0
assert archive_bytes[0x800:0x800 + entry_size] == payload
current_tool = file(archive['tool']).stat(follow_symlinks=False)
for field, value in archive['tool_state']['metadata'].items():
    actual = stat.S_IFMT(current_tool.st_mode) if field == 'st_mode' else getattr(current_tool, field)
    assert actual == value, (archive['tool'], field)
assert sha(file(archive['tool']).read_bytes()) == archive['tool_sha256']
groups = [row for row in report['source_comparisons'] if row['comparison'].get('member_count', 1) > 1]
assert sorted(row['comparison']['member_count'] for row in groups) == [3, 3, 25]
assert all(row['comparison']['all_native_bytes_match'] for row in groups)
result = {'target': report['target'], 'report': str(path.relative_to(root)), 'size': len(payload), 'compiled_bytes': report['compiled_bytes'], 'retained_bytes': report['retained_bytes'], 'compiled_ranges': len(report['compiled']), 'source_comparisons': len(comparisons), 'group_members': [row['comparison']['member_count'] for row in groups], 'retained_partial_sources': partial, 'inputs_verified': len(report['inputs']), 'archive_bytes': len(archive_bytes), 'entry': archive['index'], 'whole_payload_equal': True, 'whole_archive_equal': True, 'entry_equal': True, 'emitted_elf_slices_equal': True}

plan = json.loads((root / 'out/source-refinement/game00-nine-boundary-plan.json').read_bytes())
reviewed_ranges = []
for planned in plan['rows']:
    complete = planned['proposed_c_range']
    suffix = planned['retained_suffix']
    rows = [row for row in report['compiled'] if row['source'] == planned['source']]
    assert len(rows) == 1, planned['source']
    emitted = rows[0]
    assert (emitted['start'], emitted['end']) == (complete['file_start'], complete['file_end'])
    assert emitted['address'] == complete['start']
    assert emitted['sha256'] == complete['sha256']
    assert comparisons[planned['source']]['comparison']['status'] == 'exact_match'
    start, end = suffix['file_start'], suffix['file_end']
    assert payload[start:end] == original[start:end]
    assert sha(payload[start:end]) == suffix['sha256']
    assert not any(row['start'] < end and start < row['end'] for row in report['compiled'])
    coverage = sum(max(0, min(end, row['end']) - max(start, row['start'])) for row in report['retained'])
    assert coverage == suffix['size'] == end - start
    reviewed_ranges.append({'source': planned['source'], 'compiled_size': complete['size'], 'compiled_sha256': emitted['sha256'], 'retained_suffix_size': suffix['size'], 'retained_suffix_sha256': suffix['sha256']})
assert len(reviewed_ranges) == 9
assert sum(row['compiled_size'] for row in reviewed_ranges) == 664
assert sum(row['retained_suffix_size'] for row in reviewed_ranges) == 12116
result['reviewed_boundaries'] = reviewed_ranges
result['reviewed_complete_c_bytes'] = 664
result['reviewed_retained_suffix_bytes'] = 12116


image_dir = root / 'out/source-refinement/game00-first-naming-post-images'
image_dir.mkdir(exist_ok=False)
image_pairs = []
for source, source_row in comparisons.items():
    comparison = source_row['comparison']
    grouped = comparison.get('member_count', 1) > 1
    linked = file(comparison['placement']['path']) if grouped else Path(comparison['current_object']).with_suffix('.linked.o')
    linked_bytes = linked.read_bytes()
    members = comparison['members'] if grouped else [{'address': int(comparison['address'], 0), 'symbol': comparison['function'], 'original_size': comparison['original_size'], 'current_size': comparison['current_size'], 'original_sha256': comparison['function_image']['original_sha256'], 'current_sha256': comparison['function_image']['current_sha256']}]
    for member in members:
        address = member['address']
        offset = address - plan['load_address']
        before = original[offset:offset + member['original_size']]
        current = read_function_bytes(linked_bytes, function_name=member['symbol'], address=address).content
        assert len(current) == member['current_size']
        assert sha(before) == member['original_sha256']
        assert sha(current) == member['current_sha256']
        paths = {}
        for kind, content in [('original', before), ('current', current)]:
            destination = image_dir / f"{address:08X}.{kind}.bin"
            destination.write_bytes(content)
            assert destination.read_bytes() == content
            paths[kind] = str(destination.relative_to(root))
        first = next((index for index in range(min(len(before), len(current))) if before[index] != current[index]), None)
        if first is None and len(before) != len(current):
            first = min(len(before), len(current))
        image_pairs.append({'selector': f"emi/etc/game/00@0x{address:08X}", 'source': source, 'symbol': member['symbol'], 'original_size': len(before), 'current_size': len(current), 'original_sha256': sha(before), 'current_sha256': sha(current), 'byte_match': before == current, 'first_byte_mismatch_offset': first, 'images': paths})
assert len(image_pairs) == 247
(image_dir / 'members.json').write_text(json.dumps({'payload_report': str(path.relative_to(root)), 'members': image_pairs}, indent=2) + '\n')
result['saved_image_pairs'] = len(image_pairs)
result['saved_images'] = str(image_dir.relative_to(root))


previous = json.loads((root / 'out/source-refinement/game00-reviewed-native-images/members.json').read_bytes())['members']
prior_by_id = {row['selector']: row for row in previous}
assert len(prior_by_id) == len(image_pairs) == 247
rename_plan = json.loads((root / 'out/source-refinement/first-naming-ready.json').read_bytes())['plan']
renames = {row['selector']: row for row in rename_plan}
for member in image_pairs:
    prior = prior_by_id[member['selector']]
    for field in ('original_size', 'current_size', 'original_sha256', 'current_sha256', 'byte_match', 'first_byte_mismatch_offset'):
        assert member[field] == prior[field], (member['selector'], field)
    for kind in ('original', 'current'):
        assert file(member['images'][kind]).read_bytes() == file(prior['images'][kind]).read_bytes()
    rename = renames.get(member['selector'])
    assert member['source'] == (rename['destination'] if rename else prior['source'])
    assert member['symbol'] == (rename['new_name'] if rename else prior['symbol'])
assert report['compiled_bytes'] == 25484 and report['retained_bytes'] == 204236
result['all_247_complete_native_images_unchanged'] = True
result['six_names_and_paths_migrated_by_identity'] = True

(root / 'out/source-refinement/payload-game00-first-naming-post-verification.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps({key: value for key, value in result.items() if key != 'retained_partial_sources'}, indent=2))
