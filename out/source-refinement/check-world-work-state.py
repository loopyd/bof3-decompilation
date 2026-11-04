"""Run the fixed world work-state batch's current checks and normal builds."""
import json
from pathlib import Path
import sys

root = Path.cwd().resolve()
sys.path.insert(0, str(root / 'tools/python'))
from harness.common.process import run_bounded

plan = json.loads((root / 'out/source-refinement/world-work-state-draft/application-plan.json').read_text())
phase = sys.argv[1]
assert phase in ('refresh', 'checks')
out = root / (sys.argv[2] if len(sys.argv) > 2 else f'out/source-refinement/world-work-state-{phase}')
out.mkdir(exist_ok=False)
results = []
def run(label, arguments, allow_foreign=False):
    print(' '.join(arguments), flush=True)
    with (out / f'{label}.stdout').open('wb') as stdout, (out / f'{label}.stderr').open('wb') as stderr:
        def retain(stream, chunk):
            (stdout if stream == 'stdout' else stderr).write(chunk)
        result = run_bounded(root, arguments, output_limit=32 * 1024 * 1024, on_output=retain)
    row = {'label': label, 'argv': arguments, 'exit_code': result['exit_code'], 'failure': result['failure']}
    results.append(row)
    (out / 'summary.json').write_text(json.dumps(results, indent=2) + '\n')
    assert not result['failure'], row
    if allow_foreign:
        report = json.loads(result['stdout'])
        row['passed'] = report['passed']
        row['incomplete'] = report['incomplete']
        row['findings'] = report['findings']
        assert not any(record['findings'] for record in report['records']), row
        assert all(finding['code'] == 'shared_consumers' for finding in report['findings']), row
        assert result['exit_code'] in (0, 1), row
        (out / 'summary.json').write_text(json.dumps(results, indent=2) + '\n')
    else:
        assert result['exit_code'] == 0, row

for batch in plan['batches']:
    target = batch['target']
    label = target.replace('/', '_')
    if phase == 'refresh':
        run(label, ['bin/harness', 'analysis', 'rz-project', 'analyze', target])
    else:
        selectors = [member['selector'] for member in batch['members']]
        for concern in ('macros', 'combiner'):
            run(label + '-' + concern, ['bin/harness', concern, 'check', target, *selectors, '--json'], allow_foreign=concern == 'macros')
        run(label + '-symbols', ['bin/harness', 'source', 'symbols', 'check', target])
        run(label + '-splat', ['bin/harness', 'source', 'splat', target])
        run(label + '-build', ['bin/harness', 'build', target])
if phase == 'refresh':
    run('index', ['bin/harness', 'analysis', 'index'])
else:
    run('source-validation', ['bin/harness', 'source', 'validate', '--out', str(out / 'source-validation.json')])
print(f'{phase} complete: {len(results)} commands', flush=True)
