"""Compare current executable resolution before child-only PATH filtering."""
import hashlib
import json
import os
import shutil
from dataclasses import replace
from pathlib import Path
from harness.build.compilation import configured_command
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings, select_configured_compiler
from harness.build.invocation import plan_invocation
from harness.build.programs import describe_inputs, _capture

root = Path.cwd()
base = root / 'out/source-refinement'
plan = json.loads((base / 'battle03-final-admission-post-plan.json').read_text())
prefix = Path('/home/koija/.codex/tmp/arg0-linux')
entries = os.environ['PATH'].split(os.pathsep)
removed = [i for i, entry in enumerate(entries) if Path(entry).is_relative_to(prefix)]
assert removed
filtered = os.pathsep.join(entry for i, entry in enumerate(entries) if i not in removed)
flags, compilers = load_compiler_configuration(root)
records = []
for member in plan['members']:
    source = root / member['source']
    output = Path(member['profile']['ordered_arguments'][-1])
    arguments, environment = configured_command(root, source, output, allow_grouped=True)
    assert arguments == member['profile']['ordered_arguments']
    settings = resolve_compiler_settings(member['source'], source.read_text(), flags, compilers)
    compiler = Path(environment.get('PSX_GCC') or select_configured_compiler(root, settings.compiler_id))
    invocation = plan_invocation(root, arguments[1:], sources=[source], output=output, compiler=compiler, executable=str(compiler), cwd=root, environment=environment)
    changed = {**environment, 'PATH': filtered}
    revised = plan_invocation(root, arguments[1:], sources=[source], output=output, compiler=compiler, executable=str(compiler), cwd=root, environment=changed)
    assert len(invocation.stages) == len(revised.stages)
    for old, new in zip(invocation.stages, revised.stages):
        assert replace(old, environment=new.environment) == new
        a, b = dict(old.environment), dict(new.environment)
        assert {k:v for k,v in a.items() if k != 'PATH'} == {k:v for k,v in b.items() if k != 'PATH'}
    captures = [_capture(root, describe_inputs(item)) for item in (invocation, revised)]
    resolutions = []
    for capture in captures:
        result = []
        for request in capture['requests']:
            files = [(lookup['terminal'], capture['nodes'][lookup['terminal']]) for lookup in request['lookups'] if lookup['status'] == 'file']
            result.append({'stage': request['stage'], 'role':request['role'], 'spelling':request['spelling'], 'files':files})
        resolutions.append(result)
    assert resolutions[0] == resolutions[1], member['symbol']
    assert all(not Path(name).is_relative_to(prefix) for name,node in captures[1]['nodes'].items())
    records.append({'symbol':member['symbol'], 'resolutions':resolutions[1]})
bootstrap = []
for name in ['cmake', 'ninja', 'python3', 'dirname', 'sh', str(root/'bin/harness'), str(root/'bin/python-env'), str(root/'.venv/bin/python')]:
    paths = [name, name] if name == str(root/'bin/python-env') else [shutil.which(name, path=value) for value in (os.environ['PATH'], filtered)]
    assert paths[0] and paths[0] == paths[1], name
    physical = Path(paths[0]).resolve(strict=True)
    bootstrap.append({'name':name, 'physical':str(physical), 'sha256':hashlib.sha256(physical.read_bytes()).hexdigest()})
report = {'status':'equivalent physical program files and stage recipes', 'source_count':len(records), 'removed_path_indices':removed, 'retained_path_entries':len(entries)-len(removed), 'sources':records, 'bootstrap':bootstrap, 'limit':'Read-only current resolution proof. Native child must freshly capture and validate its own program guards.'}
path = base/'battle03-program-equivalence-final.json'
assert not path.exists()
path.write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
print(json.dumps({k:v for k,v in report.items() if k not in ('sources','bootstrap')}))
