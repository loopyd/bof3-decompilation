import json, os, sys, time
from pathlib import Path
root=Path.cwd()
out=root/"out/source-refinement/shop00-payload-diagnostic"
out.mkdir()
os.environ["PATH"]=os.pathsep.join(p for p in os.environ["PATH"].split(os.pathsep) if "arg0-linux" not in p)
import harness.build.payload as owner
from harness.cli import main
original=owner.capture_source_images
current=None
counter=0
def capture(*args,**kwargs):
    global current,counter
    current=args[1]
    print("CAPTURE "+current,flush=True)
    result=original(*args,**kwargs)
    (out/f"{counter:03d}.json").write_text(json.dumps(result.report,indent=2)+"\n")
    counter+=1
    return result
owner.capture_source_images=capture
def trace(frame,event,arg):
    if event=="call":
        return trace if frame.f_code.co_filename.endswith(("build/programs.py","common/observation.py")) else None
    if event=="exception":
        kind,error,tb=arg
        if not issubclass(kind,(StopIteration,GeneratorExit)):
            with (out/"first-exceptions.log").open("a") as f:
                f.write(f"{current}: {frame.f_code.co_filename}:{frame.f_lineno}: {kind.__name__}: {error}\n")
    return trace
sys.settrace(trace)
start=time.monotonic()
try:
    status=main(["build","payload","emi/etc/shop/00","--output-dir","out/payloads/shop00-boundary-diagnostic","--json"])
finally:
    sys.settrace(None)
    (out/"terminal.json").write_text(json.dumps({"elapsed_seconds":time.monotonic()-start,"completed_sources":counter,"last_source":current})+"\n")
raise SystemExit(status)
