import copy
import hashlib
import json
import re
from pathlib import Path
from harness.build.compilation import configured_command
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings
from harness.build.dependencies import IncludeSnapshot, collect_forced_paths
from harness.common.inputs import InputBatch
from harness.domain.functions import parse_function_records
from harness.domain.manifests import _parse_manifest
from harness.emi.catalog_verify import resolve_shipped_entry
from harness.match.members import capture_members

root = Path.cwd()
sha = lambda content: hashlib.sha256(content).hexdigest()
oldpath = Path('out/source-refinement/payload-c15-plan.json')
runnerpath = Path('out/source-refinement/run-payload-c15.py')
old = json.loads(oldpath.read_bytes())
runner = runnerpath.read_text()
assert sha(runner.encode()) == '2dd220db8363585cb43a960f81ada8fdbf4758d9bfae72f9803ad1f6cba96077'
targets = ['emi/plchar/plp467/00', 'emi/plchar/plp478/00', 'emi/plchar/plp567/00', 'emi/plchar/plp678/00', 'emi/scenario/sce10eff/00']
taskpath = old['task_binding']['path']
matches = [s for s in Path(taskpath).read_text().splitlines() if s.startswith('- [ ] 6.22 ')]
assert len(matches) == 1
task = matches[0]
assert sha(task.encode()) == 'cd78737fdfffc264c021edcba1089a248ed9c311de951b5bbf13ca51c68483c4'
runner = runner.replace('C15', 'C16').replace('c15', 'c16').replace(repr(old['task_binding']['text']), repr(task)).replace(json.dumps(old['targets']), json.dumps(targets))
plan = copy.deepcopy(old)
plan.update(batch='C16', task='6.22', targets=targets, targets_detail=[], partials=[], placed_data=[], replicas=[], claims=0, metadata_functions=0, metadata_free_psyq_sources=0, reviewed_original_function_bytes=0, original_payload_bytes=0, literal_include_states=0, current_input_states={}, evidence_output='out/source-refinement/payload-c16', fresh_output_paths=['out/payloads/c16-' + t.replace('/', '-') for t in targets], expected_compiled_bytes=None, native_measured=False, runner_sha256=sha(runner.encode()), source_pattern={'plan': str(oldpath), 'plan_sha256': sha(oldpath.read_bytes()), 'runner': str(runnerpath), 'runner_sha256': sha(runnerpath.read_bytes())}, task_binding={'path': taskpath, 'text': task, 'text_sha256': sha(task.encode()), 'launch_check': old['task_binding']['launch_check']})
recon = Path(plan['historical_reconciliation']['path'])
group = next(g for g in json.loads(recon.read_bytes())['remaining_historical_groups'] if g['id'] == 'C16')
assert group['targets'] == targets
plan['historical_reconciliation'] = {'path': str(recon), 'sha256': sha(recon.read_bytes()), 'frozen_group': group}
names = {n for n in old['current_input_states'] if not n.startswith(('src/', 'include/', 'toolchains/psyq/4.7/include/', 'out/binaries/', 'out/extracted/', 'config/targets/emi/'))}
includes = set()
flags, compilers = load_compiler_configuration(root)
for target in targets:
    manifest_name = f'config/targets/{target}/target.toml'
    m = _parse_manifest(root / manifest_name, (root / manifest_name).read_bytes())
    entry = resolve_shipped_entry(root, m.disc_id)
    direct = set(m.sources + m.support_sources + m.headers) | {manifest_name, m.splat, m.binary, f'config/targets/{target}/symbols.txt'} | {Path(entry[k]).relative_to(root).as_posix() for k in ('archive_path', 'manifest_path', 'payload_path')}
    assert (root / m.binary).read_bytes() == Path(entry['payload_path']).read_bytes()
    assert m.load_address == entry['load_address'] and not m.replicas
    detail = {'target': target, 'claims': [], 'members': [], 'shipped_entry': entry, 'reviewed_original_function_bytes': 0}
    for source in m.sources + m.support_sources:
        text = (root / source).read_text()
        records = parse_function_records(text)
        args, _ = configured_command(root, root / source, root / 'build' / Path(source).with_suffix('.o'), allow_grouped=True)
        settings = resolve_compiler_settings(source, text, flags, compilers)
        detail['claims'].append({'source': source, 'metadata_members': len(records), 'profile': {'compiler_id': settings.compiler_id, 'flags': settings.flags}, 'configured_arguments': args, 'zero_function_credit': not records})
        snapshot = IncludeSnapshot(root, root / source, forced=tuple(root / n for n in sorted(collect_forced_paths(root, args, root))))
        try:
            inputs = snapshot.describe_inputs()
            snapshot.validate()
            includes.update(inputs)
            names.update(inputs)
        finally:
            snapshot.close()
        if not records:
            plan['metadata_free_psyq_sources'] += int(source == m.psyq_source)
            continue
        captured = capture_members(root, source)
        names.update(captured['inputs'])
        for f in captured['functions']:
            size = len(f['original'])
            detail['members'].append({**{k: f[k] for k in ('selector', 'symbol', 'file_start')}, 'source': source, 'file_end': f['file_start'] + size, 'original_sha256': sha(f['original'])})
            detail['reviewed_original_function_bytes'] += size
            record = next(r for r in records if r.address == f['address'])
            if re.search(r'@status\s+partial\b', record.metadata):
                plan['partials'].append({'source': source, 'selector': f['selector'], 'original_bytes': size, 'match': re.search(r'@match\s+(\S+)', record.metadata).group(1), 'metadata_sha256': sha(record.metadata.encode()), 'preserve': 'Source body, ABI, progress and residual remain unchanged; differing original ranges must remain retained.'})
        for section, data in captured['data_sections'].items():
            plan['placed_data'].append({'target': target, 'source': source, 'section': section, 'address': data['address'], 'file_start': data['file_start'], 'file_end': data['file_start'] + len(data['original']), 'size': len(data['original']), 'original_sha256': sha(data['original']), 'owners': data['owners']})
    detail['direct_input_sha256'] = {n: sha((root / n).read_bytes()) for n in sorted(direct)}
    detail['direct_fingerprint'] = sha(json.dumps(detail['direct_input_sha256'], sort_keys=True, separators=(',', ':')).encode())
    names.update(direct)
    plan['targets_detail'].append(detail)
    plan['claims'] += len(detail['claims'])
    plan['metadata_functions'] += len(detail['members'])
    plan['reviewed_original_function_bytes'] += detail['reviewed_original_function_bytes']
    plan['original_payload_bytes'] += entry['size']
with InputBatch(root) as batch:
    for name in sorted(names):
        plan['current_input_states'][name] = batch.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)[0]
plan['literal_include_states'] = len(includes)
assert (plan['claims'], plan['metadata_functions'], plan['reviewed_original_function_bytes'], plan['original_payload_bytes']) == (35, 29, 7032, 38000)
assert plan['metadata_free_psyq_sources'] == 5
assert len(plan['partials']) == len(plan['placed_data']) == 1
assert plan['partials'][0]['selector'] == 'emi/scenario/sce10eff/00@0x801D165C'
assert plan['partials'][0]['match'] == '85.46'
assert plan['placed_data'][0]['original_sha256'] == '9d3138caacf73d6008a9c8e010aa04c96306b6ad474e77cc49c03bd3fa54d3fc'
assert len(plan['current_input_states']) == 265 and plan['literal_include_states'] == 164
assert all(c['profile'] == {'compiler_id': None, 'flags': None} for detail in plan['targets_detail'] for c in detail['claims'])
assert not Path(plan['evidence_output']).exists()
assert all(not Path(p).exists() for p in plan['fresh_output_paths'])
encoded = json.dumps(plan, indent=2, sort_keys=True) + '\n'
assert sha(encoded.encode()) == '15f44fcddc297fe87d09b75f948eff131d0651713b2fa0e49dce254661ead363'
assert sha(runner.encode()) == '46bfe18200151f6ef288be7f4f79f8efc829c4d8c4649ee7754183b8e0ede1a7'
compile(runner, 'run-payload-c16.py', 'exec')
for path, content in [('out/source-refinement/payload-c16-plan.json', encoded), ('out/source-refinement/run-payload-c16.py', runner)]:
    with Path(path).open('x') as output:
        output.write(content)
print('Saved static C16 plan', sha(encoded.encode()), 'runner', sha(runner.encode()), 'native_measured', plan['native_measured'])
