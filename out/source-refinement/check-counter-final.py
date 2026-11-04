"""Run current concern checks and builds for the finite COUNTER_ADVANCE/COUNTER_RETREAT application."""
import json
import os
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded
root=Path.cwd();base=root/'out/source-refinement';out=base/'counter-advance-final-checks'
assert not out.exists();out.mkdir()
plan=json.loads((base/'counter-advance-post-plan.json').read_text());members=plan['selected']+plan['existing_consumers']
targets=sorted({m['target'] for m in members});assert len(targets)==5
environment=dict(os.environ);prefix=Path('/home/koija/.codex/tmp/arg0-linux')
environment['CMAKE_BUILD_PARALLEL_LEVEL']='1'
environment['PATH']=os.pathsep.join(entry for entry in environment['PATH'].split(os.pathsep) if not Path(entry).is_relative_to(prefix))
summary={'status':'running','commands':[],'macro_checks_passed':False,'shared_consumer_findings':[]}
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
  run('expansions',[str(root/'.venv/bin/python'),'out/source-refinement/check-counter-expansions.py','--pre-expansion-sha256','333262f232ad547fafa85ad6a6bf7a661967d631efd7232f2cda491a93a44f6f','--post-plan-sha256','53bcb717e68e1c5111c989e069da729ed505ea5afbd06712d0f8816ca4fe192f','--application-sha256','35e2d300f8c9ea231ef1bc368dbb82be5870aff1f54b9ffea71d2a5760c92e71']);guard()
  for target in targets:
   label=target.replace('/','-');selected=[m for m in members if m['target']==target]
   selectors={m['selector'] for m in selected}
   r=run(label+'-macros',['bin/harness','macros','check',target,*sorted(selectors),'--json'],(1,))
   report=json.loads(r['stdout']);assert {row['selector'] for row in report['records']}==selectors
   for row in report['records']:
    native=row['measurement'];assert not row['findings'] and native['state']=='live' and native['status']=='exact' and native['original_size']==native['current_size']==40 and native['match_percent']==100
   expected_definitions={m['macro_name'] for m in selected}
   assert {d['name'] for d in report['definitions']}==expected_definitions
   for definition in report['definitions']:
    assert definition['source_path']=='include/game/counter_step.h' and definition['owner_target']=='__shared__'
   expected_foreign={m['target'] for m in members if m['macro_name'] in expected_definitions}-{target}
   findings=report['findings'];assert not report['passed'] and report['incomplete'] and len(findings)==1
   finding=findings[0];assert finding['code']=='shared_consumers' and finding['status']=='unavailable' and set(finding['targets'])==expected_foreign,target
   summary['shared_consumer_findings'].append({'target':target,**finding});save()
   run(label+'-symbols',['bin/harness','source','symbols','check',target])
   run(label+'-splat',['bin/harness','source','splat',target])
   run(label+'-build',['bin/harness','build',target]);guard()
  run('source-validation',['bin/harness','source','validate','--out',str(out.relative_to(root)/'source-validation.json')]);guard()
  validation=json.loads((out/'source-validation.json').read_text());assert validation['coverage_error'] is None
  by_target={r['target']:r for r in validation['targets']}
  for target in targets: assert by_target[target]['coverage_error'] is None and by_target[target]['lifts']['invalid']==0,target
  run('pytest',[str(root/'.venv/bin/python'),'-m','pytest','-q','tools/python/tests/macros','tools/python/tests/analysis/test_reverse_index.py','tools/python/tests/analysis/test_index_command.py','tools/python/tests/build/test_build.py','tools/python/tests/build/test_splat.py']);guard()
  run('ruff',['ruff','check','tools/python/harness/macros/index.py']);guard()
 summary.update(status='local native checks and builds complete; independent acceptance pending',normal_builds=5,all_7_live_exact=True,shared_consumer_requests_covered_by_explicit_target_checks=True,continuous_inputs_closed=True)
 save()
except BaseException as error:
 summary.update(status='failed',error=f'{type(error).__name__}: {error}');save();raise
