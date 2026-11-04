"""Apply the reviewed E5CA4 source and prepare fresh bounded PRE plans."""
import hashlib
import json
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer

root = Path.cwd()
base = root / 'out/source-refinement'
source = 'src/bof3/battle/func_801E5CA4.c'
draft = json.loads((base / 'battle03-e5ca4-repair-draft/report.json').read_text())
proposal = (base / 'battle03-e5ca4-final-proposed.c').read_bytes()
assert hashlib.sha256(proposal).hexdigest() == '5e55866a4743dfbb1e2ce31e138d98ad2b20716f1ae7a819c726ed8106229a63'
output = base / 'battle03-e5ca4-application'
assert not output.exists()

def read(batch, name):
    return batch.read(root / name, max_bytes=64*1024*1024, include_metadata=True, single_link=True)

def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')

with acquire_writer(root):
    with InputBatch(root) as before:
        for name, expected in draft['current_input_states'].items():
            assert read(before, name)[0] == expected, name
        pre_state, old = read(before, source)
        index_state, index_bytes = read(before, '.git/index')
    output.mkdir()
    (output / 'source.pre.c').write_bytes(old)
    (output / 'index.pre.bin').write_bytes(index_bytes)
    with InputBatch(root) as stable:
        for name, expected in draft['current_input_states'].items():
            if name != source:
                assert read(stable, name)[0] == expected, name
        assert read(stable, '.git/index') == (index_state, index_bytes)
        assert (root / source).read_bytes() == old
        with (root / source).open('r+b') as stream:
            stream.write(proposal)
            stream.truncate()
        for name, expected in draft['current_input_states'].items():
            if name != source:
                assert read(stable, name)[0] == expected, name
        assert read(stable, '.git/index') == (index_state, index_bytes)
    with InputBatch(root) as after:
        post_state, content = read(after, source)
        assert content == proposal and post_state['metadata']['st_ino'] == pre_state['metadata']['st_ino']
        post_index = read(after, '.git/index')[0]
        assert post_index == index_state
    save(output / 'report.json', {'status': 'source applied; live byte/data gate pending', 'source': source, 'pre_state': pre_state, 'post_state': post_state, 'index_before': index_state, 'index_after': post_index, 'draft_report_sha256': hashlib.sha256((base / 'battle03-e5ca4-repair-draft/report.json').read_bytes()).hexdigest(), 'old_failures_preserved': True, 'previous_batch_preserved': True})
    for label in ['battle03-admission-pre-03', 'battle03-admission-pre-04', 'battle03-admission-pre-05', 'battle03-bin-pre-01', 'battle03-bin-pre-02']:
        old_plan = base / (label + '-plan.json')
        plan = json.loads(old_plan.read_text())
        with InputBatch(root) as current:
            states = {name: read(current, name)[0] for name in plan['current_input_states']}
            for name, expected in plan['current_input_states'].items():
                assert states[name] == (post_state if name == source else expected), name
        plan['current_input_states'] = states
        plan['source_repair_application'] = str((output / 'report.json').relative_to(root))
        plan['scope'] += ' Fresh complete capture after separately reviewed E5CA4 source repair; historical plans and failures remain unchanged.'
        new_label = label + '-repaired'
        new_plan = base / (new_label + '-plan.json')
        assert not new_plan.exists()
        save(new_plan, plan)
        script = (base / ('capture-' + label + '.py')).read_text()
        script = script.replace(hashlib.sha256(old_plan.read_bytes()).hexdigest(), hashlib.sha256(new_plan.read_bytes()).hexdigest()).replace(label, new_label)
        (base / ('capture-' + new_label + '.py')).write_text(script)
        print(new_label, hashlib.sha256(new_plan.read_bytes()).hexdigest(), flush=True)
