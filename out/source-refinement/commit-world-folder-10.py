"""Commit only the independently accepted World10 source moves and claims."""
import hashlib
import json
import os
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded

root = Path.cwd()
base = root / 'out/source-refinement'
sha = lambda body: hashlib.sha256(body).hexdigest()
plan = json.loads((base / 'world-folder-10-post-plan.json').read_bytes())
application = json.loads((base / 'world-folder-10-application/report.json').read_bytes())
review = json.loads((base / 'world-folder-10-final-independent-review.json').read_bytes())
assert review['accepted'] is True
owned = sorted(application['after_states'])
assert len(owned) == 33 and owned == sorted(plan['owned_paths']['final'])
expected_parent = 'c8f7b015a4457911fc98fadca849906717eceb2d'
expected_index = 'cccee64b7e2ca45f48e23cded378b37fee0892507478c4012f04fedbf5704851'
env = dict(os.environ)
env['GIT_OPTIONAL_LOCKS'] = '0'
result = {'status': 'started', 'owned_paths': owned, 'parent': expected_parent, 'commands': []}
def save():
    (base / 'world-folder-10-commit.json').write_text(json.dumps(result, indent=2, sort_keys=True) + '\n')
def git(*args):
    output = run_bounded(root, ['git', '--no-optional-locks', *args], env=env, output_limit=1024 * 1024)
    result['commands'].append({'argv': output['argv'], 'exit_code': output['exit_code'], 'failure': output['failure']})
    save()
    assert output['exit_code'] == 0 and output['failure'] is None, output
    return output['stdout']
def endpoint():
    with InputBatch(root) as observation:
        state, _ = observation.read(root / 'src/bof3/battle/func_801DAAE4.c', include_metadata=True, single_link=True)
        return state
assert git('rev-parse', 'HEAD').strip() == expected_parent
assert not git('diff', '--cached', '--name-only', '-z')
# Close the index observation before the authorized Git mutations.
with InputBatch(root) as observation:
    index_state, index_bytes = observation.read(root / '.git/index', include_metadata=True, single_link=True)
    assert sha(index_bytes) == expected_index and index_state == application['index_after']
result['index_before'] = index_state
baseline = json.loads((base / 'world-folder-10-post-foreign-baseline.json').read_bytes())
with InputBatch(root) as inputs:
    states = {}
    def bind(name, expected):
        state, _ = inputs.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
        assert state == expected, name
        states[name] = state
    for name, state in plan['current_input_states'].items():
        assert name != '.git/index'
        bind(name, state)
    for name, state in application['after_states'].items():
        bind(name, state)
    for name, state in baseline['foreign_tracked_states'].items():
        if name != 'src/bof3/battle/func_801DAAE4.c':
            bind(name, state)
    fields = ('st_dev', 'st_gid', 'st_ino', 'st_mode', 'st_nlink', 'st_uid')
    def directories():
        return {name: {key: getattr((root / name).stat(follow_symlinks=False), key) for key in fields} for name in baseline['foreign_directory_identities']}
    assert directories() == baseline['foreign_directory_identities']
    result['foreign_battle_before'] = endpoint()
    git('add', '--', *owned)
    staged = git('diff', '--cached', '--name-only', '--no-renames', '-z').strip('\x00').split('\x00')
    assert sorted(staged) == owned
    git('diff', '--cached', '--check', '--', *owned)
    for name, state in application['after_states'].items():
        if state is not None:
            assert sha(git('show', ':' + name).encode()) == state['sha256'], name
    git('commit', '-m', 'refactor(world): move area176 through area187 sources into world')
    commit = git('rev-parse', 'HEAD').strip()
    result['commit'] = commit
    assert git('rev-parse', 'HEAD^').strip() == expected_parent
    actual = git('diff-tree', '--root', '--no-commit-id', '--name-only', '--no-renames', '-r', '-z', commit).strip('\x00').split('\x00')
    assert sorted(actual) == owned
    for name, state in application['after_states'].items():
        if state is not None:
            assert sha(git('show', commit + ':' + name).encode()) == state['sha256'], name
    assert not git('diff', '--cached', '--name-only', '-z')
    for name, state in states.items():
        bind(name, state)
    assert directories() == baseline['foreign_directory_identities']
    result['foreign_battle_after'] = endpoint()
result.update(status='committed and verified', staged_empty=True, foreign_battle_boundary='Endpoint observations only; no approval of unrelated foreign source.', other_foreign_and_owned_states_preserved=True, main_push=False)
save()
print(json.dumps({key: result[key] for key in ('status', 'commit', 'parent', 'main_push')}, sort_keys=True))
