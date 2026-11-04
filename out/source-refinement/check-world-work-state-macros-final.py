"""Retain nine current shared-macro checks and their conservative findings."""
import json
from pathlib import Path
import sys
root = Path.cwd().resolve()
sys.path.insert(0, str(root / 'tools/python'))
from harness.common.process import run_bounded
plan = json.loads((root / 'out/source-refinement/world-work-state-draft/application-plan.json').read_text())
out = root / 'out/source-refinement/world-work-state-macros-final'
out.mkdir(exist_ok=False)
results = []
targets = {b['target'] for b in plan['batches']}
for batch in plan['batches']:
    target = batch['target']
    label = target.replace('/', '_')
    command = ['bin/harness', 'macros', 'check', target, *[m['selector'] for m in batch['members']], '--json']
    print(' '.join(command), flush=True)
    with (out / (label + '.stdout')).open('wb') as stdout, (out / (label + '.stderr')).open('wb') as stderr:
        def retain(stream, chunk):
            (stdout if stream == 'stdout' else stderr).write(chunk)
        execution = run_bounded(root, command, output_limit=32*1024*1024, on_output=retain)
    assert not execution['failure'], execution['failure']
    report = json.loads(execution['stdout'])
    row = {'target': target, 'argv': command, 'exit_code': execution['exit_code'], 'passed': report['passed'], 'incomplete': report['incomplete'], 'findings': report['findings']}
    results.append(row)
    (out / 'summary.json').write_text(json.dumps(results, indent=2)+'\n')
    assert execution['exit_code']==1 and report['passed'] is False and report['incomplete'] is True
    assert len(report['definitions'])==2
    assert {d['name'] for d in report['definitions']}=={m['macro'] for m in batch['members']}
    assert len(report['findings'])==1 and report['findings'][0]['code']=='shared_consumers'
    assert set(report['findings'][0]['targets'])==targets-{target}
    assert len(report['records'])==2 and all(not r['findings'] and r['measurement']['state']=='live' and r['measurement']['status']=='exact' and r['measurement']['original_size']==r['measurement']['current_size'] for r in report['records'])
    print(f'{target}: both live exact; retained eight-foreign-target finding', flush=True)
print('Nine actual failing-global reports retained; explicit combined eighteen-member closure requires independent review.', flush=True)
