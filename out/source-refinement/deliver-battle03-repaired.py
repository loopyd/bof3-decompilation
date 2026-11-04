"""Refresh changed Battle03 discovery, then run its supported payload delivery."""
import json
import os
from pathlib import Path
from harness.common.process import run_bounded
root=Path.cwd()
base=root/'out/source-refinement'
out=base/'battle03-repaired-delivery'
payload=root/'out/payloads/battle03-repaired-complete'
assert not out.exists() and not payload.exists()
environment=dict(os.environ)
prefix=Path('/home/koija/.codex/tmp/arg0-linux')
environment['PATH']=os.pathsep.join(entry for entry in environment['PATH'].split(os.pathsep) if not Path(entry).is_relative_to(prefix))
out.mkdir()
commands=[
 ('analysis',['bin/harness','analysis','rz-project','analyze','emi/battle/battle/03']),
 ('index',['bin/harness','analysis','index']),
 ('source-validate',['bin/harness','source','validate','--out',str(out.relative_to(root)/'source-validation.json')]),
 ('payload',['bin/harness','build','payload','emi/battle/battle/03','--output-dir',str(payload.relative_to(root)),'--json']),
]
summary={'status':'running','commands':[]}
def save():
 (out/'summary.json').write_text(json.dumps(summary,indent=2,sort_keys=True)+'\n')
save()
try:
 for label,argv in commands:
  print(label,flush=True)
  with (out/(label+'.stdout')).open('wb') as stdout,(out/(label+'.stderr')).open('wb') as stderr:
   result=run_bounded(root,argv,env=environment,output_limit=64*1024*1024,on_output=lambda stream,chunk:(stdout if stream=='stdout' else stderr).write(chunk))
  (out/(label+'.process.json')).write_text(json.dumps(result,indent=2,sort_keys=True)+'\n')
  summary['commands'].append({'label':label,'argv':argv,'exit_code':result['exit_code'],'failure':result['failure']})
  save()
  if result['failure'] or result['exit_code']:
   raise RuntimeError(f'{label} failed; preserve batch and outputs')
 summary['status']='public delivery complete; independent acceptance pending'
 save()
except BaseException as error:
 summary.update(status='failed',error=f'{type(error).__name__}: {error}')
 save()
 raise
