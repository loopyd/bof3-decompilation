"""Run current concern checks and builds for the finite WORKAREA_RESET application."""
import json
import os
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded
root=Path.cwd();base=root/'out/source-refinement';out=base/'workarea-reset-final-checks-serial'
assert not out.exists();out.mkdir()
plan=json.loads((base/'workarea-reset-post-plan.json').read_text());members=plan['selected']+plan['existing_consumers']
targets=sorted({m['target'] for m in members});assert len(targets)==16
environment=dict(os.environ);prefix=Path('/home/koija/.codex/tmp/arg0-linux')
environment['PATH']=os.pathsep.join(entry for entry in environment['PATH'].split(os.pathsep) if not Path(entry).is_relative_to(prefix))
environment['CMAKE_BUILD_PARALLEL_LEVEL']='1'
summary={'predecessor_summary_sha256':'5b7829024317e15fcae873b9229b8517077d4f6ef71cfd0b7a69616649647e6e','predecessor':'out/source-refinement/workarea-reset-final-checks/summary.json','scheduling_change':'CMAKE_BUILD_PARALLEL_LEVEL=1; guards, compiler and flags unchanged','status':'running','commands':[],'macro_checks_passed':False,'shared_consumer_findings':[]}
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
  predecessor=json.loads((root/summary['predecessor']).read_text())
  assert predecessor['status']=='failed' and predecessor['commands'][-1]['label']=='emi-world03-area134-13-build'
  run('emi-world03-area134-13-build',['bin/harness','build','emi/world03/area134/13']);guard()
  remaining=targets[targets.index('emi/world03/area134/13')+1:]
  assert len(remaining)==5
  for target in remaining:
   label=target.replace('/','-');selected=[m for m in members if m['target']==target]
   selectors={m['selector'] for m in selected}
   r=run(label+'-macros',['bin/harness','macros','check',target,*sorted(selectors),'--json'],(1,))
   report=json.loads(r['stdout']);assert {row['selector'] for row in report['records']}==selectors
   for row in report['records']:
    native=row['measurement'];assert not row['findings'] and native['state']=='live' and native['status']=='exact' and native['original_size']==native['current_size']==32 and native['match_percent']==100
   findings=report['findings'];assert not report['passed'] and report['incomplete'] and len(findings)==1
   finding=findings[0];assert finding['code']=='shared_consumers' and finding['status']=='unavailable' and set(finding['targets'])==set(targets)-{target},target
   assert len(report['definitions'])==1 and report['definitions'][0]['name']=='WORKAREA_RESET' and report['definitions'][0]['source_path']=='include/game/workarea.h' and report['definitions'][0]['owner_target']=='__shared__'
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
 summary.update(status='local native checks and builds complete; independent acceptance pending',all_26_live_exact=True,normal_builds_combined=16,predecessor_checks_preserved=True,shared_consumer_requests_covered_by_explicit_target_checks=True,continuous_inputs_closed=True)
 save()
except BaseException as error:
 summary.update(status='failed',error=f'{type(error).__name__}: {error}');save();raise
