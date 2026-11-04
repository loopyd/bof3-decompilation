"""Capture the four unchanged grouped work-state siblings before or after folder01."""
import argparse
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
from harness.build.compilation import configured_command
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes, read_linked_image
from harness.match.images import capture_source_images
from harness.match.members import capture_members

root=Path.cwd();base=root/'out/source-refinement'
parser=argparse.ArgumentParser();parser.add_argument('--stage',choices=('pre','post'),required=True);parser.add_argument('--plan-sha256',required=True);parser.add_argument('--pre-summary-sha256');args=parser.parse_args()
plan_name='out/source-refinement/'+('world-folder-01-ready-draft/plan.json' if args.stage=='pre' else 'world-folder-01-post-plan.json')
out=base/('world-folder-01-siblings-'+args.stage);assert not out.exists();out.mkdir()
sha=lambda value:hashlib.sha256(value).hexdigest()
summary={'status':'running','stage':args.stage,'sources':[],'limits':'Four unchanged sibling native images only. No source edits or independent acceptance.'}
def save(): (out/'summary.json').write_text(json.dumps(summary,indent=2,sort_keys=True)+'\n')
save()
try:
 with acquire_writer(root),InputBatch(root) as batch:
  states={}
  def bind(name):
   state,content=batch.read(root/name,max_bytes=64*1024*1024,include_metadata=True,single_link=True)
   assert name not in states or state==states[name],name
   states[name]=state;return content
  def guard():
   for name in list(states):bind(name)
  data=bind(plan_name);assert sha(data)==args.plan_sha256;plan=json.loads(data)
  for name,expected in plan['current_input_states'].items():bind(name);assert states[name]==expected,name
  bind('.git/index');bind(Path(__file__).resolve().relative_to(root).as_posix())
  groups=plan['untouched_authored_siblings'];assert len(groups)==2 and sum(len(g['members']) for g in groups)==4 and sum(m['original_size'] for g in groups for m in g['members'])==184
  if args.stage=='post':
   previous=bind('out/source-refinement/world-folder-01-siblings-pre/summary.json');assert sha(previous)==args.pre_summary_sha256
   previous=json.loads(previous);assert previous['status']=='complete' and previous['guards_and_writer_closed']
  flags,compilers=load_compiler_configuration(root)
  (out/'starting-input-states.json').write_text(json.dumps(states,indent=2,sort_keys=True)+'\n')
  for group in groups:
   source=group['source'];text=bind(source).decode();assert sha(text.encode())==group['source_sha256']
   assert [asdict(r) for r in parse_function_records(text)]==[m['record'] for m in group['members']]
   assert asdict(resolve_compiler_settings(source,text,flags,compilers))==group['effective_settings']
   command,_=configured_command(root,root/source,Path(group['configured_arguments'][-1]),allow_grouped=True);assert command==group['configured_arguments']
   captured=capture_members(root,source);assert captured['target']==group['target'] and len(captured['functions'])==2 and not captured['data_sections'] and not any(captured['section_placements'].values())
   planned={m['selector']:m for m in group['members']};assert {m['selector'] for m in captured['functions']}==set(planned)
   for m in captured['functions']:
    p=planned[m['selector']];assert m['symbol']==p['symbol'] and m['original']==bind(p['original_path']) and len(m['original'])==p['original_size'] and sha(m['original'])==p['original_sha256']
   destination=out/Path(source).stem;destination.mkdir();guard()
   comparison=capture_source_images(root,source,destination);report=comparison.report
   (destination/'comparison.json').write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
   assert report['status']=='exact' and report['all_native_bytes_match'] and report['fresh_compilation'] and report['member_count']==2 and not report['data_sections']
   assert len(comparison.ranges)==2 and {r.owners[0] for r in comparison.ranges}==set(planned)
   linked=[name for name in report['outputs'] if name.endswith('.elf')];assert len(linked)==1
   elf=bind(linked[0]);image=read_linked_image(elf,limit=64*1024*1024);assert not image.relocations and {s.name for s in image.symbols if s.info&15==2 and s.section}=={m['symbol'] for m in group['members']}
   members=[]
   for current in comparison.ranges:
    p=planned[current.owners[0]];original=bind(p['original_path']);actual=read_function_bytes(elf,function_name=p['symbol'],address=p['record']['address']).content
    assert current.content==actual==original and current.source==source and current.address==p['record']['address'] and current.original_file_start==p['file_start']
    name=f"{p['record']['address']:08X}.bin";(destination/name).write_bytes(actual)
    if args.stage=='post':assert actual==bind('out/source-refinement/world-folder-01-siblings-pre/'+Path(source).stem+'/'+name)
    members.append({'selector':p['selector'],'symbol':p['symbol'],'size':len(actual),'sha256':sha(actual),'raw_word_count':len(actual)//4})
   for name,expected in report['outputs'].items():bind(name);assert states[name]==expected,name
   guard();summary['sources'].append({'source':source,'target':group['target'],'members':members,'comparison':str((destination/'comparison.json').relative_to(root)),'linked_elf':linked[0]});save();print(source,flush=True)
  guard();(out/'terminal-input-states.json').write_text(json.dumps(states,indent=2,sort_keys=True)+'\n')
 summary.update(status='complete',member_count=4,complete_original_bytes=184,guards_and_writer_closed=True);save()
except BaseException as error:
 summary.update(status='failed',error=f'{type(error).__name__}: {error}')
 if isinstance(error,ProcessCleanupError):summary['cleanup']='unconfirmed; stop native work'
 save();raise
