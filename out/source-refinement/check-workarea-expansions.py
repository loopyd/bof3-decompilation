"""Check all configured WORKAREA_RESET emissions against reviewed function bodies."""
from contextlib import closing
from dataclasses import asdict
import json
from pathlib import Path
import re
from harness.build.dependencies import IncludeSnapshot
from harness.build.expansion import expand_function_members
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.domain.functions import parse_function_records
root=Path.cwd();base=root/'out/source-refinement'
plan_name='out/source-refinement/workarea-reset-post-plan.json'
plan=json.loads((root/plan_name).read_text());members=plan['selected']+plan['existing_consumers']
out=base/'workarea-reset-expansions.json';assert not out.exists()
rows=[]
def tokens(text): return re.findall(r'\w+|[^\w\s]',text)
with acquire_writer(root),InputBatch(root) as batch:
 states={}
 def bind(name):
  state,content=batch.read(root/name,max_bytes=64*1024*1024,include_metadata=True,single_link=True)
  assert name not in states or state==states[name],name
  states[name]=state;return content
 for name,expected in plan['current_input_states'].items(): bind(name);assert states[name]==expected,name
 bind(plan_name)
 sources=[root/m['source'] for m in members]
 with closing(IncludeSnapshot(root,sources[0],forced=tuple(sources[1:]))) as includes:
  for name,expected in includes.describe_inputs().items(): bind(name);assert states[name]==expected,name
  for member in members:
   source=member['source'];text=bind(source).decode();records=parse_function_records(text)
   assert len(records)==1 and records[0].kind=='template' and records[0].metadata==member['record']['metadata'],source
   expanded=expand_function_members(root,root/source)
   address=member['record']['address'];assert set(expanded)=={address}
   actual=expanded[address]
   expected=f"void {member['symbol']}(void) {{ func_80196070(); }}"
   assert actual.symbol==member['symbol'] and actual.source==source and actual.source_line==records[0].line
   assert tokens(actual.implementation)==tokens(expected),source
   assert tokens(actual.invocation)==tokens(f"WORKAREA_RESET({member['symbol']})")
   rows.append({'selector':member['selector'],**asdict(actual),'tokens_equal_reviewed_body':True})
   print(member['selector'],flush=True)
   includes.validate()
   for name in list(states): bind(name)
assert len(rows)==26
out.write_text(json.dumps({'status':'configured emissions verified','members':rows,'current_input_states':states,'guards_and_writer_closed':True},indent=2,sort_keys=True)+'\n')
