import hashlib
import json
import tempfile
from pathlib import Path

from harness.common.lease import acquire_writer
from harness.match.images import capture_source_images

root = Path.cwd().resolve()
output = Path(tempfile.mkdtemp(prefix='images-native-', dir=root/'out/source-refinement'))
cases = [
 ('grouped', 'src/bof3/ui/panelMotionGame00.c', True),
 ('support', 'src/bof3/world/invokeZeroArgWorld00Area00713_801F2C04.c', True),
 ('compiler-override', 'src/bof3/audio/dispatchSoundCue.c', True),
 ('initialized-data', 'src/bof3/equipment/getEquipRecordBase.c', True),
 ('partial', 'src/bof3/ui/func_801ADDD4.c', False),
]
results = []
with acquire_writer(root):
 for name, source, exact in cases:
  result = capture_source_images(root, source, output)
  (output/(name+'.json')).write_text(json.dumps(result.report, indent=2)+'\n')
  assert bool(result.ranges) is exact, (name, result.report.get('status'))
  rows=[]
  original_name=result.report['original_binary']
  original=Path(original_name)
  if not original.is_absolute(): original=root/original
  content=original.read_bytes()
  for index, image in enumerate(result.ranges):
   assert image.content and image.content==content[image.original_file_start:image.original_file_start+len(image.content)]
   (output/(name+'-'+str(index)+'.bin')).write_bytes(image.content)
   rows.append({'address':image.address,'file_start':image.original_file_start,'size':len(image.content),'sha256':hashlib.sha256(image.content).hexdigest(),'symbol':image.symbol,'section':image.section,'owners':image.owners})
  if name=='initialized-data': assert any(row['section']=='.rodata' for row in rows)
  summary={'case':name,'status':result.report['status'],'source':source,'ranges':rows,'configured_profile':result.report.get('configured_profile')}
  results.append(summary)
  (output/'summary.json').write_text(json.dumps(results,indent=2)+'\n')
  print(name,result.report['status'],len(rows),'ranges',sum(row['size'] for row in rows),'bytes',flush=True)
print('evidence',output.relative_to(root),flush=True)
