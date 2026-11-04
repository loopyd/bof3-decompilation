"""Apply the reviewed COMMU00 metadata attachment and reviewed C boundary."""
import hashlib
import json
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
root=Path.cwd();base=root/'out/source-refinement';draft=base/'metadata-commu00-amended-draft'
old_plan=(draft/'plan.json').read_bytes()
assert hashlib.sha256(old_plan).hexdigest()=='4f40cbb46953a2b725b4414cd0978caae0b49a1bf5be7966d8d3fcec05eabb5d'
plan=json.loads(old_plan)
outputs={name:(draft/'files'/name).read_bytes() for name in plan['proposed_output_sha256']}
for name,content in outputs.items(): assert hashlib.sha256(content).hexdigest()==plan['proposed_output_sha256'][name],name
assert len(outputs)==2
pre_summary=base/'commu00-metadata-pre/summary.json'
assert hashlib.sha256(pre_summary.read_bytes()).hexdigest()=='dcb8fa63c375399c538e30a18879ccc3d93025de1019c3d2192118dbf239cdf4'
assert json.loads(pre_summary.read_text())['guards_and_writer_closed']
out=base/'commu00-metadata-application';assert not out.exists()
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
 amendment={'status':'applied; POST and final checks pending','parent_plan_sha256':hashlib.sha256(old_plan).hexdigest(),'owned_outputs':{name:hashlib.sha256(content).hexdigest() for name,content in outputs.items()},'before_states':{name:pre[name][0] for name in outputs},'after_states':{name:after[name] for name in outputs},'index_before':index_state,'index_after':index_state,'scope':'Two reviewed outputs: metadata comment attachment and C/source attribution at offset8904. All implementation tokens and boundaries preserved.'}
 save(out/'report.json',amendment)
 plan['current_input_states']=after
 plan['proposed_output_sha256']=amendment['owned_outputs']
 plan['application_report']='out/source-refinement/commu00-metadata-application/report.json'
 save(base/'commu00-metadata-post-plan.json',plan)
 print('Applied',len(outputs),'owned outputs; index preserved')
