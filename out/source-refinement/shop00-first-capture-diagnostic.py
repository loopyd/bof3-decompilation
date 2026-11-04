import json, sys, traceback
from pathlib import Path
from harness.common.lease import acquire_writer
from harness.match.images import capture_source_images
root=Path.cwd()
out=root/"out/source-refinement/shop00-first-capture-diagnostic"
out.mkdir()
def trace(frame,event,arg):
    if event=="exception" and frame.f_code.co_filename.endswith(("build/programs.py","common/observation.py")):
        kind,error,tb=arg
        if not issubclass(kind,(StopIteration,GeneratorExit)):
            with (out/"first-exceptions.log").open("a") as f:
                f.write(f"{frame.f_code.co_filename}:{frame.f_lineno}: {kind.__name__}: {error}\n")
    return trace
sys.settrace(trace)
try:
    with acquire_writer(root):
        result=capture_source_images(root,"src/bof3/ui/advancePanelField6To16.c",out)
        (out/"report.json").write_text(json.dumps(result.report,indent=2)+"\n")
finally:
    sys.settrace(None)
