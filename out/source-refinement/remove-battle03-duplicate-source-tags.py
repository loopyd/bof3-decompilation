"""Remove two repeated identical Splat tags, preserving the reviewed bindings."""
import hashlib
import json
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
root=Path.cwd()
base=root/'out/source-refinement'
plan=json.loads((base/'battle03-final-admission-post-plan.json').read_text())
name='config/targets/emi/battle/battle/03/splat.yaml'
out=base/'battle03-source-tag-cleanup'
assert not out.exists()
def read(batch,name):
    return batch.read(root/name,max_bytes=64*1024*1024,include_metadata=True,single_link=True)
with acquire_writer(root), InputBatch(root) as batch:
    for path, expected in plan['current_input_states'].items():
        assert read(batch,path)[0]==expected,path
    before, content=read(batch,name)
    index_state,index_bytes=read(batch,'.git/index')
    edited=content
    for symbol in ['transformFirstPointPairByMode','submitCurrentWorkEffectVariant']:
        line=f"    - '@source: src/bof3/battle/{symbol}.c'\n".encode()
        assert edited.count(line+line)==1
        edited=edited.replace(line+line,line,1)
    out.mkdir()
    (out/'splat.pre.yaml').write_bytes(content)
    (out/'index.pre.bin').write_bytes(index_bytes)
    with (root/name).open('r+b') as stream:
        stream.write(edited)
        stream.truncate()
    for path,expected in plan['current_input_states'].items():
        if path != name:
            assert read(batch,path)[0]==expected,path
    assert read(batch,'.git/index')==(index_state,index_bytes)
with InputBatch(root) as batch:
    after,actual=read(batch,name)
    assert actual==edited
    assert after['metadata']['st_ino']==before['metadata']['st_ino']
plan['current_input_states'][name]=after
plan['source_tag_cleanup']='out/source-refinement/battle03-source-tag-cleanup/report.json'
(base/'battle03-final-admission-clean-plan.json').write_text(json.dumps(plan,indent=2,sort_keys=True)+'\n')
(out/'report.json').write_text(json.dumps({'status':'two identical duplicate tags removed','before':before,'after':after,'index_preserved':True,'semantic_bindings_unchanged':True},indent=2,sort_keys=True)+'\n')
print('Splat SHA256',after['sha256'])
