"""Check the final shared-root layout against the previously reviewed expansions."""
import dataclasses
import hashlib
import json
from pathlib import Path
import sys
root=Path.cwd().resolve()
sys.path.insert(0,str(root/'tools/python'))
from harness.build.expansion import expand_function_members
from harness.build.compiler import DEFAULT_COMPILER_ID,build_compiler_arguments,load_compiler_configuration,resolve_compiler_settings
from harness.domain.functions import parse_function_records
plan=json.loads((root/'out/source-refinement/world-work-state-draft/application-plan.json').read_text())
previous=json.loads((root/'out/source-refinement/world-work-state-draft/expansion-shared.json').read_text())
expected={row['selector']:row for row in previous['members']}
old='include/bof3/world/work_state.h'; new='include/world/work_state.h'
assert not (root/old).exists()
assert (root/new).read_bytes()==(root/'out/source-refinement/world-work-state-draft/proposed'/old).read_bytes()
flags,compilers=load_compiler_configuration(root)
rows=[]
for batch in plan['batches']:
 source=root/batch['destination']; text=source.read_text()
 assert text==(root/'out/source-refinement/world-work-state-draft/proposed'/batch['destination']).read_text().replace('"bof3/world/work_state.h"','"world/work_state.h"')
 records=parse_function_records(text)
 settings=resolve_compiler_settings(batch['destination'],text,flags,compilers)
 assert (settings.compiler_id or DEFAULT_COMPILER_ID)==batch['effective_profile']['compiler']
 assert build_compiler_arguments(root,settings.flags)[1:]==batch['effective_profile']['flags']
 expanded=expand_function_members(root,source)
 for member in batch['members']:
  row=expected[member['selector']]; address=int(member['selector'].split('@')[1],16)
  current=dataclasses.asdict(expanded[address])
  assert current['symbol']==row['symbol'] and current['implementation']==row['implementation'] and current['invocation']==row['invocation'] and current['source_line']==row['source_line']
  record=next(r for r in records if r.address==address)
  assert hashlib.sha256(record.metadata.encode()).hexdigest()==member['metadata_sha256']
  rows.append({'selector':member['selector'],**current,'effective_profile':batch['effective_profile']})
assert len(rows)==18
chars=sum(len((root/b['destination']).read_text()) for b in plan['batches'])+len((root/new).read_text())
assert chars==5697,chars
report={'shared_header':new,'retired_header_absent':True,'members':rows,'configured_expansions_unchanged':True,'metadata_and_profiles_unchanged':True,'source_header_chars':chars,'net_authored_file_reduction':8}
out=root/'out/source-refinement/world-work-state-layout-expansions.json'
with out.open('x') as handle:handle.write(json.dumps(report,indent=2)+'\n')
print('Final shared-root18 expansions and5697 authored characters verified.')
