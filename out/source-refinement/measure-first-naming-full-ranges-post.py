"""Measure saved complete reviewed partial ranges using the existing diff helpers."""
import hashlib
import json
from pathlib import Path
import sys

root = Path.cwd().resolve()
sys.path.insert(0, str(root / 'tools/python'))
from harness.common.lease import acquire_writer
from harness.io import repo_layout
from harness.match._asm_disasm import disassemble_original, disassemble_linked, extract_instructions, render_normalized
from harness.match._asm_diff_payload import matching_instruction_count, first_instruction_mismatch

sha = lambda data: hashlib.sha256(data).hexdigest()
pre = root / 'out/source-refinement/first-naming-partials-post'
measured = json.loads((pre / 'summary.json').read_bytes())
game = json.loads((root / 'out/source-refinement/game00-first-naming-post-images/members.json').read_bytes())['members']
area = json.loads((root / 'out/source-refinement/area01613-first-naming-post-images/pairs.json').read_bytes())
images = {row['selector']: {'original': row['images']['original'], 'current': row['images']['current'], 'original_sha256': row['original_sha256'], 'current_sha256': row['current_sha256']} for row in game}
images.update({row['selector']: {'original': row['original']['path'], 'current': row['current']['path'], 'original_sha256': row['original']['sha256'], 'current_sha256': row['current']['sha256']} for row in area})
output = root / 'out/source-refinement/first-naming-full-ranges-post'
objdump = repo_layout().psyqo_toolchain_root / 'bin/mipsel-none-elf-objdump'
tool_hash = sha(objdump.read_bytes())
rows = []
with acquire_writer(root):
    output.mkdir(exist_ok=False)
    for partial in measured['partials']:
        selector = partial['selector']
        address = int(selector.split('@')[1], 16)
        image = images[selector]
        original_path = root / image['original']
        original = original_path.read_bytes()
        current = (root / image['current']).read_bytes()
        assert sha(original) == image['original_sha256']
        assert sha(current) == image['current_sha256'] == partial['function_image']['current_sha256']
        assert current == (root / partial['directory'] / 'current.bin').read_bytes()
        assert len(original) > 0 and len(original) % 4 == len(current) % 4 == 0
        directory = output / selector.replace('/', '_').replace('@', '-')
        directory.mkdir()
        before = disassemble_original(objdump_path=objdump, original_bytes_path=original_path, address=address)
        after = disassemble_linked(objdump_path=objdump, linked_path=root / partial['directory'] / 'current.linked.o', address=address, size=len(current))
        left, right = extract_instructions(before), extract_instructions(after)
        (directory / 'original.objdump').write_text(before)
        (directory / 'current.objdump').write_text(after)
        (directory / 'original.s').write_text(render_normalized(left))
        (directory / 'current.s').write_text(render_normalized(right))
        matching = matching_instruction_count(left, right)
        rows.append({'selector': selector, 'source': partial['source'], 'original_size': len(original), 'current_size': len(current), 'size_delta': len(current)-len(original), 'function_image': {'original_sha256': sha(original), 'current_sha256': sha(current)}, 'instruction_count': {'original': len(left), 'current': len(right), 'matching': matching, 'match_percent': round(100*matching/max(len(left),len(right),1),2)}, 'first_mismatch': first_instruction_mismatch(left,right), 'images': image, 'public_diagnostic_original_size': partial['original_size'], 'public_diagnostic_same_extent': len(original)==partial['original_size'] and sha(original)==partial['function_image']['original_sha256'], 'disassembly_directory': str(directory.relative_to(root))})
    assert len(rows) == 23 and sha(objdump.read_bytes()) == tool_hash
    for name, expected in measured['source_config_sha256'].items():
        path = root / name
        assert (sha(path.read_bytes()) if path.is_file() else None) == expected, name
    result = {'schema': 'disposable.first-naming-full-ranges/v1', 'status': 'complete', 'method': 'Existing disassembly and diff metrics over saved complete freshly compiled native images and complete reviewed original ranges. No range or source edits.', 'objdump': str(objdump), 'objdump_sha256': tool_hash, 'partials': rows}
    previous = json.loads((root / 'out/source-refinement/first-naming-full-ranges-pre/summary.json').read_bytes())
    assert previous['objdump_sha256'] == tool_hash
    previous_by_id = {row['selector']: row for row in previous['partials']}
    for row in rows:
        prior = previous_by_id[row['selector']]
        for field in ('source', 'original_size', 'current_size', 'size_delta', 'function_image', 'instruction_count', 'first_mismatch', 'public_diagnostic_original_size', 'public_diagnostic_same_extent'):
            assert row[field] == prior[field], (row['selector'], field)
    assert len(previous_by_id) == len(rows) == 23
    public_pre = json.loads((root / 'out/source-refinement/first-naming-partials-pre/summary.json').read_bytes())
    public_by_id = {row['selector']: row for row in public_pre['partials']}
    for row in measured['partials']:
        prior = public_by_id[row['selector']]
        for field in ('source', 'exit_code', 'byte_match', 'original_size', 'current_size', 'instruction_count', 'first_mismatch', 'function_image'):
            assert row[field] == prior[field], (row['selector'], field)
    assert len(public_by_id) == len(measured['partials']) == 23
    result['all_23_public_diagnostic_metrics_unchanged'] = True
    result['all_23_complete_images_and_metrics_unchanged'] = True
    result['metric_limits'] = previous['metric_limits']
    (output / 'summary.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({'partials': len(rows), 'same_public_extent': sum(row['public_diagnostic_same_extent'] for row in rows), 'original_bytes': sum(row['original_size'] for row in rows), 'current_bytes': sum(row['current_size'] for row in rows)}))
