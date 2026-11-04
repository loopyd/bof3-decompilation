"""Run the fixed Area131 header/move batch's existing final checks once."""
import hashlib
import json
from contextlib import redirect_stdout
from pathlib import Path
from harness.commands import compile_commands
from harness.common.lease import acquire_writer
from harness.common.process import run_bounded

ROOT=Path.cwd().resolve()
OUT=ROOT/'out/source-refinement/area131-header-final-checks'
TARGET='emi/world03/area131/13'
PLAN=json.loads((ROOT/'out/source-refinement/area131-header-ready-draft/plan.json').read_text())
SELECTORS=[m['selector'] for m in PLAN['members']]
sha=lambda b:hashlib.sha256(b).hexdigest()

def verify():
    for name,digest in PLAN['projected_post_pins']['sha256'].items():
        if sha((ROOT/name).read_bytes())!=digest:
            raise ValueError(f'Changed approved input: {name}')
    for member in PLAN['members']:
        if (ROOT/member['old_source']).exists():
            raise ValueError(f"Retired source remains: {member['old_source']}")

def save(name,value):
    (OUT/name).write_text(json.dumps(value,indent=2)+'\n')

verify()
OUT.mkdir(exist_ok=False)
state={'status':'running','commands':[]}
save('summary.json',state)

def run(label,argv):
    verify()
    print(' '.join(argv),flush=True)
    with (OUT/(label+'.stdout')).open('wb') as stdout,(OUT/(label+'.stderr')).open('wb') as stderr:
        def retain(stream,chunk):
            (stdout if stream=='stdout' else stderr).write(chunk)
        result=run_bounded(ROOT,argv,output_limit=32*1024*1024,on_output=retain)
    save(label+'.process.json',result)
    state['commands'].append({'label':label,'argv':argv,'exit_code':result['exit_code'],'failure':result['failure']})
    save('summary.json',state)
    if result['failure'] or result['exit_code']!=0:
        raise ValueError(f'{label} failed; retained output and process evidence')
    verify()
    return result['stdout']

try:
    for concern,argv in (
        ('metadata',['source','metadata','check',TARGET,*SELECTORS,'--json']),
        ('combiner',['combiner','check',TARGET,*SELECTORS,'--json']),
        ('symbols',['source','symbols','check',TARGET]),
        ('splat',['source','splat',TARGET]),
        ('normal-build',['build',TARGET]),
        ('source-validation',['source','validate','--out',str(OUT/'source-validation.json')]),
        ('target-analysis',['analysis','rz-project','analyze',TARGET]),
        ('global-index',['analysis','index']),
    ):
        output=run(concern,['bin/harness',*argv])
        if concern in ('metadata','combiner'):
            report=json.loads(output)
            if not report['passed'] or report['incomplete'] or report['findings'] or any(r['findings'] for r in report['records']):
                raise ValueError(f'{concern} report is not complete/passed')
    with acquire_writer(ROOT):
        with (OUT/'compile-commands.stdout').open('w') as stdout,redirect_stdout(stdout):
            code=compile_commands.main([])
        if code!=0:
            raise ValueError('Compilation database generation failed')
        verify()
    ready=json.loads(run('readiness',['bin/harness','analysis','readiness',TARGET,'--detail','full']))
    if not ready['ready'] or not ready['index_ready'] or ready['stale_facts']['count']:
        raise ValueError('Final readiness is stale')
    names=['build/cmake/build.ninja','build/cmake/bof3-inventory.json','compile_commands.json']
    texts={n:(ROOT/n).read_text() for n in names}
    inventory=json.loads(texts[names[1]])
    db=json.loads(texts[names[2]])
    files=[r['file'] for r in db]
    for member in PLAN['members']:
        new,old=member['new_source'],member['old_source']
        if any(new not in text or old in text for text in texts.values()):
            raise ValueError(f'Generated discovery differs for {new}')
        if new not in inventory['sources'] or new not in inventory['profiles'] or files.count(str(ROOT/new))!=1:
            raise ValueError(f'Generated source/profile membership differs for {new}')
    header=PLAN['new_header']['path']
    if header not in texts[names[0]]:
        raise ValueError('New header absent from normal graph')
    verify()
    save('discovery.json',{'new_sources':[m['new_source'] for m in PLAN['members']],'old_sources_absent':[m['old_source'] for m in PLAN['members']],'new_header':header,'new_header_in_normal_graph':True,'compilation_database_entries':len(db),'generated_sha256':{n:sha((ROOT/n).read_bytes()) for n in names},'post_input_pins_current':len(PLAN['projected_post_pins']['sha256']),'pruned_folders':[]})
    state.update(status='complete',full_campaign_complete=False)
    save('summary.json',state)
except BaseException as error:
    state.update(status='failed',error=f'{type(error).__name__}: {error}')
    save('summary.json',state)
    raise
