import hashlib
import json
import stat
from pathlib import Path

from harness.match.extraction import read_function_bytes, read_linked_image

root = Path.cwd().resolve()
path = root / 'out/payloads/game00-grouped-union/report.json'
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
assert sha(file(archive['tool']).read_bytes()) == archive['tool_sha256']
groups = [row for row in report['source_comparisons'] if row['comparison'].get('member_count', 1) > 1]
assert sorted(row['comparison']['member_count'] for row in groups) == [3, 3, 25]
assert all(row['comparison']['all_native_bytes_match'] for row in groups)
result = {'target': report['target'], 'report': str(path.relative_to(root)), 'size': len(payload), 'compiled_bytes': report['compiled_bytes'], 'retained_bytes': report['retained_bytes'], 'compiled_ranges': len(report['compiled']), 'source_comparisons': len(comparisons), 'group_members': [row['comparison']['member_count'] for row in groups], 'retained_partial_sources': partial, 'inputs_verified': len(report['inputs']), 'archive_bytes': len(archive_bytes), 'entry': archive['index'], 'whole_payload_equal': True, 'whole_archive_equal': True, 'entry_equal': True, 'emitted_elf_slices_equal': True}
(root / 'out/source-refinement/payload-game00-grouped-verification.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps({key: value for key, value in result.items() if key != 'retained_partial_sources'}, indent=2))
