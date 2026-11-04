"""Apply the reviewed WORKAREA_RESET reuse and comment correction."""
import hashlib
import json
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
root=Path.cwd();base=root/'out/source-refinement';draft=base/'workarea-reset-ready-draft'
old_plan=(draft/'plan.json').read_bytes()
assert hashlib.sha256(old_plan).hexdigest()=='fe071edae00e0cdf46fd439d0ec68e6fcc3fbe2523fce4c99ee1669a66320e0f'
plan=json.loads(old_plan);header='include/game/workarea.h'
outputs={name:(draft/'files'/name).read_bytes() for name in plan['proposed_output_sha256']}
for name,content in outputs.items(): assert hashlib.sha256(content).hexdigest()==plan['proposed_output_sha256'][name],name
outputs[header]=(base/'workarea-header-comment-proposed.h').read_bytes()
assert hashlib.sha256(outputs[header]).hexdigest()=='b11751bb45690c1aebf5b6496860be606e29d1717d644d4f5830f2b4150d2efc'
assert len(outputs)==32
out=base/'workarea-reset-application';assert not out.exists()
def read(batch,name): return batch.read(root/name,max_bytes=64*1024*1024,include_metadata=True,single_link=True)
def save(path,value): path.write_text(json.dumps(value,indent=2,sort_keys=True)+'\n')
with acquire_writer(root):
 with InputBatch(root) as batch:
  before={name:read(batch,name)[0] for name in plan['current_input_states']}
  assert before==plan['current_input_states']
  index_state,index=read(batch,'.git/index')
  pre={name:read(batch,name) for name in outputs}
 out.mkdir();(out/'index.pre.bin').write_bytes(index)
 for name,(state,content) in pre.items():
  assert state is not None and content is not None,name
  path=out/'pre'/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(content)
 with InputBatch(root) as stable:
  for name,expected in before.items():
   if name not in outputs: assert read(stable,name)[0]==expected,name
  assert read(stable,'.git/index')==(index_state,index)
  for name,content in outputs.items():
   assert (root/name).read_bytes()==pre[name][1],name
   with (root/name).open('r+b') as stream: stream.write(content);stream.truncate()
  for name,expected in before.items():
   if name not in outputs: assert read(stable,name)[0]==expected,name
  assert read(stable,'.git/index')==(index_state,index)
 with InputBatch(root) as batch:
  after={name:read(batch,name)[0] for name in before}
  for name,content in outputs.items():
   state,actual=read(batch,name);assert actual==content,name
   assert state['metadata']['st_ino']==pre[name][0]['metadata']['st_ino'],name
  assert read(batch,'.git/index')==(index_state,index)
 amendment={'status':'applied; POST and final checks pending','parent_plan_sha256':hashlib.sha256(old_plan).hexdigest(),'header_comment_sha256':hashlib.sha256(outputs[header]).hexdigest(),'owned_outputs':{name:hashlib.sha256(content).hexdigest() for name,content in outputs.items()},'before_states':{name:pre[name][0] for name in outputs},'after_states':{name:after[name] for name in outputs},'index_before':index_state,'index_after':index_state,'scope':'Original31 reviewed outputs plus one reviewed comment-only shared-header correction; macro body unchanged.'}
 save(out/'report.json',amendment)
 plan['current_input_states']=after
 plan['proposed_output_sha256']=amendment['owned_outputs']
 plan['pre_header_sha256']=plan.pop('unchanged_header_sha256')
 plan['header_comment_amendment']='out/source-refinement/workarea-header-comment-proposed.h'
 plan['application_report']='out/source-refinement/workarea-reset-application/report.json'
 save(base/'workarea-reset-post-plan.json',plan)
 print('Applied',len(outputs),'owned outputs; index preserved')
