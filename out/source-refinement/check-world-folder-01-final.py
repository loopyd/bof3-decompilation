"""Run current concern checks and builds for the world-folder-01 relocation."""
import json
import os
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded
root=Path.cwd();base=root/'out/source-refinement';out=base/'world-folder-01-final-checks'
assert not out.exists();out.mkdir()
plan=json.loads((base/'world-folder-01-post-plan.json').read_text());members=plan['selected']+[dict(m,source=g['source'],target=g['target']) for g in plan['untouched_authored_siblings'] for m in g['members']]
targets=sorted({m['target'] for m in members});assert len(targets)==12 and len(members)==33 and sum(m['original_size'] for m in members)==1132
sizes={m['selector']:m['original_size'] for m in members}
environment=dict(os.environ);prefix=Path('/home/koija/.codex/tmp/arg0-linux')
environment['CMAKE_BUILD_PARALLEL_LEVEL']='1'
environment['PATH']=os.pathsep.join(entry for entry in environment['PATH'].split(os.pathsep) if not Path(entry).is_relative_to(prefix))
summary={'status':'running','commands':[],'metadata_checks_passed':False}
def save(): (out/'summary.json').write_text(json.dumps(summary,indent=2,sort_keys=True)+'\n')
def run(label,argv,allowed=(0,)):
 print(label,flush=True)
 with (out/(label+'.stdout')).open('wb') as stdout,(out/(label+'.stderr')).open('wb') as stderr:
  result=run_bounded(root,argv,env=environment,output_limit=64*1024*1024,on_output=lambda stream,chunk:(stdout if stream=='stdout' else stderr).write(chunk))
 (out/(label+'.process.json')).write_text(json.dumps(result,indent=2,sort_keys=True)+'\n')
 summary['commands'].append({'label':label,'argv':argv,'exit_code':result['exit_code'],'failure':result['failure']});save()
 assert not result['failure'] and result['exit_code'] in allowed,f'{label} failed; retain applied sources and outputs'
 return result
try:
 with InputBatch(root) as inputs:
  def guard():
   for name,expected in plan['current_input_states'].items():
    state,_=inputs.read(root/name,max_bytes=64*1024*1024,include_metadata=True,single_link=True)
    assert state==expected,name
  guard()
  for target in targets:
   label=target.replace('/','-');r=run(label+'-status',['bin/harness','analysis','rz-project','status',target,'--json'],(0,1))
   status=json.loads(r['stdout']);assert status['target']==target
   if not status['fresh']: run(label+'-analysis',['bin/harness','analysis','rz-project','analyze',target])
  run('index',['bin/harness','analysis','index']);guard()
  for target in targets:
   label=target.replace('/','-');selected=[m for m in members if m['target']==target]
   selectors={m['selector'] for m in selected}
   r=run(label+'-combiner',['bin/harness','combiner','check',target,*sorted(selectors),'--json'])
   report=json.loads(r['stdout']);assert report['passed'] and not report['incomplete'] and not report['findings']
   assert {row['selector'] for row in report['records']}==selectors
   for row in report['records']:
    native=row['measurement'];assert not row['findings'] and native['state']=='live' and native['status']=='exact' and native['original_size']==native['current_size']==sizes[row['selector']] and native['match_percent']==100
   run(label+'-symbols',['bin/harness','source','symbols','check',target])
   run(label+'-splat',['bin/harness','source','splat',target])
   run(label+'-build',['bin/harness','build',target]);guard()
  run('source-validation',['bin/harness','source','validate','--out',str(out.relative_to(root)/'source-validation.json')]);guard()
  validation=json.loads((out/'source-validation.json').read_text());assert validation['coverage_error'] is None
  by_target={r['target']:r for r in validation['targets']}
  for target in targets: assert by_target[target]['coverage_error'] is None and by_target[target]['lifts']['invalid']==0,target
  patterns=out/'obsolete-source-paths.txt';patterns.write_text(''.join(m['old_source']+'\n' for m in plan['selected']))
  run('obsolete-references',['rg','-n','-F','-f',str(patterns),'src','include','config','tools','.agents','docs','.pi/workflows','CMakeLists.txt','README.md'],(1,));guard()
  database=json.loads((root/'build/compile_commands.json').read_text())
  compiled={str((root/row['file']).resolve()) if not Path(row['file']).is_absolute() else str(Path(row['file']).resolve()) for row in database}
  for unit in plan['normal_build_units']:assert str(root/unit['source']) in compiled,unit['source']
  for member in plan['selected']:assert str(root/member['old_source']) not in compiled,member['old_source']
  summary['normal_TU_discovery']=len(plan['normal_build_units']);assert summary['normal_TU_discovery']==43
 summary.update(status='local native checks and builds complete; independent acceptance pending',normal_builds=12,complete_1132_bytes_live_exact=True,metadata_checks_passed=True,continuous_inputs_closed=True)
 save()
except BaseException as error:
 summary.update(status='failed',error=f'{type(error).__name__}: {error}');save();raise
