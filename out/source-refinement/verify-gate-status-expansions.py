"""Check configured private/shared gate macro expansions against the twelve bodies."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
from harness.build.expansion import expand_function_members
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.domain.functions import parse_function_records

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('stage',choices=('private','shared'))
stage=parser.parse_args().stage
root=Path.cwd().resolve()
directory=root/'out/source-refinement/gate-status-ready-draft'
plan_bytes=(directory/'plan.json').read_bytes()
sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha(plan_bytes)=='75c6b8e2a788ee8909dd1d09556363c2a6dcc05f04901b6471397862d32644d0'
plan=json.loads(plan_bytes)
expected={m['selector']:m for m in plan['members']}
pins=dict(plan['current_input_sha256'])
removed=plan['private_removed_sources'] if stage=='private' else plan['removed_sources']
for name in removed:del pins[name]
pins.update(plan['private_output_sha256'] if stage=='private' else plan['proposed_output_sha256'])
absent=(set(plan['absent_paths'])-pins.keys())|set(removed)
output=root/f'out/source-refinement/gate-status-expansion-{stage}.json'
assert not output.exists()
tokens=lambda s:re.findall(r'\w+|[^\w\s]',s)
rows=[]
with acquire_writer(root):
 with InputBatch(root) as batch:
  states={}
  for name,digest in pins.items():
   state,_=batch.read(root/name,max_bytes=64*1024*1024,include_metadata=True,single_link=True)
   assert state and state['sha256']==digest,name
   states[name]=state
  for name in absent:assert not os.path.lexists(root/name),name
  for group in plan['batches']:
   if stage=='private' and not group['private_first']:continue
   source=group['destination'];records=parse_function_records((root/source).read_text());by_address={r.address:r for r in records}
   expanded=expand_function_members(root,root/source)
   wanted={int(expected[s]['native_range']['address_start'],16):expected[s] for s in group['members']}
   assert len(expanded)==len(wanted) and set(expanded)==set(wanted) and set(by_address)==set(wanted),source
   for address,emitted in expanded.items():
    member=wanted[address];record=by_address[address]
    assert emitted.address==address and emitted.symbol==member['symbol'] and emitted.source==source and emitted.source_line==record.line
    assert tokens(emitted.invocation)==tokens(member['draft_invocation'])
    assert tokens(emitted.implementation)==tokens(member['implementation']),member['selector']
    assert record.metadata==member['metadata']
    rows.append({'selector':member['selector'],'source':source,'symbol':emitted.symbol,'source_line':emitted.source_line,'invocation':emitted.invocation,'implementation':emitted.implementation,'implementation_sha256':sha(emitted.implementation.encode()),'body_tokens_equal_original':True})
  for name,expected_state in states.items():
   current,_=batch.read(root/name,max_bytes=64*1024*1024,include_metadata=True,single_link=True);assert current==expected_state,name
  for name in absent:assert not os.path.lexists(root/name),name
assert len(rows)==(11 if stage=='private' else 12)
report={'stage':stage,'passed':True,'plan_sha256':sha(plan_bytes),'members':rows,'input_states':states,'absent':sorted(absent),'limit':'Configured per-invocation expansion attribution and original-body token equivalence. Native ELF/receipt evidence remains separate. No full historical environment claim.'}
with output.open('x') as file:file.write(json.dumps(report,indent=2)+'\n')
print(json.dumps({'stage':stage,'passed':True,'members':len(rows),'report':str(output.relative_to(root))}))
