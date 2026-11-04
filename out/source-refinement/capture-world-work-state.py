"""Capture the fixed eighteen-member world work-state batch."""
import dataclasses
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path.cwd().resolve()
sys.path.insert(0, str(ROOT / 'tools/python'))
from harness.common.lease import acquire_writer
from harness.match.images import capture_source_images
from harness.match.members import capture_members

plan = json.loads((ROOT / 'out/source-refinement/world-work-state-draft/application-plan.json').read_text())
stage = sys.argv[1]
assert stage in ('pre', 'private', 'post')
out = ROOT / (sys.argv[2] if len(sys.argv) > 2 else f'out/source-refinement/world-work-state-{stage}')
out.mkdir(exist_ok=False)
sha = lambda content: hashlib.sha256(content).hexdigest()
def save(path, data):
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + '\n')
summary = {'stage': stage, 'status': 'running', 'sources': []}
try:
    with acquire_writer(ROOT):
        if stage == 'pre':
            for name, digest in plan['current_input_sha256'].items():
                assert sha((ROOT / name).read_bytes()) == digest, name
            for name in plan['absent_destinations']:
                assert not os.path.lexists(ROOT / name), name
            owned = list(dict.fromkeys(name for batch in plan['batches'] for name in batch['sources_removed'] + [batch['manifest'], batch['splat']]))
            states = {}
            for name in owned:
                path = ROOT / name
                st = path.lstat()
                assert path.is_file() and not path.is_symlink() and st.st_nlink == 1, name
                dest = out / 'original' / name
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, dest)
                states[name] = {'dev': st.st_dev, 'ino': st.st_ino, 'nlink': st.st_nlink, 'uid': st.st_uid, 'mode': st.st_mode, 'sha256': sha(path.read_bytes())}
            save(out / 'states.json', {'present': states, 'absent': plan['absent_destinations']})
            shutil.copy2(ROOT / '.git/index', out / 'index')
            (out / 'starting-status').write_bytes(subprocess.check_output(['git', 'status', '--porcelain=v1', '-z']))
        batches = plan['batches'][:2] if stage == 'private' else plan['batches']
        for batch in batches:
            if stage == 'pre':
                for concern in ('macros', 'combiner'):
                    command = ['bin/harness', concern, 'list', batch['target'], '--json']
                    result = subprocess.run(command, capture_output=True)
                    dest = out / (batch['target'].replace('/', '_') + '-' + concern + '.json')
                    dest.write_bytes(result.stdout)
                    dest.with_suffix('.stderr').write_bytes(result.stderr)
                    assert result.returncode == 0, (command, result.returncode)
            sources = batch['sources_removed'] if stage == 'pre' else [batch['destination']]
            expected = {int(m['selector'].split('@')[1], 16): m for m in batch['members']}
            for source in sources:
                dest = out / source.replace('/', '_')
                dest.mkdir()
                captured = capture_members(ROOT, source)
                comparison = capture_source_images(ROOT, source, dest)
                save(dest / 'comparison.json', comparison.report)
                members = []
                for member in captured['functions']:
                    address = member['address']
                    planned = expected[address]
                    ranges = [r for r in comparison.ranges if r.address == address and r.symbol == member['symbol']]
                    assert len(ranges) == 1, (source, address, comparison.report)
                    current = ranges[0].content
                    original = member['original']
                    assert current == original and len(original) == planned['original_range']['size'], (source, address)
                    assert sha(original) == planned['original_range']['original_sha256'], (source, address)
                    (dest / f'{address:08X}-original.bin').write_bytes(original)
                    (dest / f'{address:08X}-current.bin').write_bytes(current)
                    members.append({'selector': planned['selector'], 'symbol': member['symbol'], 'size': len(original), 'sha256': sha(original)})
                summary['sources'].append({'source': source, 'target': batch['target'], 'members': members, 'comparison': str((dest / 'comparison.json').relative_to(ROOT))})
                save(out / 'summary.json', summary)
                print(f'{stage}: {source}: {len(members)} complete exact members', flush=True)
        summary['status'] = 'complete'
        summary['member_count'] = sum(len(row['members']) for row in summary['sources'])
        summary['original_bytes'] = sum(m['size'] for row in summary['sources'] for m in row['members'])
        assert summary['member_count'] == (4 if stage == 'private' else 18)
        save(out / 'summary.json', summary)
except BaseException as error:
    summary['status'] = 'failed'
    summary['error'] = f'{type(error).__name__}: {error}'
    save(out / 'summary.json', summary)
    raise
