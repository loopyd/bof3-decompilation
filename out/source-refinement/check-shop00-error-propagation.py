import json
from pathlib import Path
from harness.common.lease import acquire_writer
from harness.build.programs import ProgramSnapshot
import harness.match.images as images
root=Path.cwd(); out=root/"out/source-refinement/shop00-error-propagation"; out.mkdir()
original_validate=ProgramSnapshot.validate_inputs
original_close=ProgramSnapshot.close
fault=False; repeated=0; closed=[]
sentinel=ValueError("original comparison failure retained")
def validate(self, inputs):
    global repeated
    if fault:
        repeated+=1
        raise ValueError("secondary failed snapshot validation")
    return original_validate(self, inputs)
def close(self):
    original_close(self)
    closed.append(self._closed)
def compare(*args, **kwargs):
    global fault
    fault=True
    raise sentinel
ProgramSnapshot.validate_inputs=validate
ProgramSnapshot.close=close
images._asm_diff_compare=compare
try:
    with acquire_writer(root):
        images.capture_source_images(root,"src/bof3/ui/advancePanelField6To16.c",out)
except ValueError as error:
    assert error is sentinel, repr(error)
else:
    raise AssertionError("comparison failure was accepted")
assert repeated==0 and closed and all(closed),(repeated,closed)
report={"status":"PASS","original_error_preserved":True,"failed_snapshot_reentry_count":repeated,"closed_program_snapshots":len(closed),"scope":"Disposable actual-capture fault injection. No maintained tests or game-source edits."}
(out/"result.json").write_text(json.dumps(report,indent=2)+"\n")
print(json.dumps(report))
