"""Finish SHOP delivery after preserving original comparison errors."""
import hashlib,json,os,time
from pathlib import Path
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded,ProcessCleanupError
root=Path.cwd();base=root/"out/source-refinement";out=base/"shop00-boundary-fixed-final";out.mkdir()
sha=lambda b:hashlib.sha256(b).hexdigest()
old=json.loads((base/"shop00-boundary-final-followup/starting-input-states.json").read_bytes())
fix="tools/python/harness/match/images.py"
foreign="src/bof3/battle/func_801DAAE4.c"
expected={fix:"d4cabbc9eb8dfe2bf5fbc2e07cb72aed913dd59d5c8830248f01ae6332792d23",foreign:"4d5f5fdf082ebf2416f7696356d4800d7d8cb0afa10b9e2fb1c9c8d1cd8b8d58"}
summary={"status":"running","changed_condition":"Reviewed error-preservation fix only. Original failure not reproduced by diagnostic; no restore. Foreign battle file changed externally and remains excluded from this change.","limits":"Hybrid original-based SHOP payload only; partials remain original; no runtime/global compiler-input-closure claim."}
def save(n,d): (out/n).write_text(json.dumps(d,indent=2,sort_keys=True)+"\n")
save("summary.json",summary)
start=time.monotonic()
try:
 with InputBatch(root) as batch:
  current={}
  for name,state in old.items():
   observed,body=batch.read(root/name,max_bytes=64<<20,include_metadata=True,single_link=True)
   if name in expected: assert observed["sha256"]==expected[name],name
   else: assert observed==state,name
   current[name]=observed
  save("starting-input-states.json",current)
  save("explicit-input-deltas.json",{n:{"before":old[n],"current":current[n],"ownership":"owned harness fix" if n==fix else "foreign; preserve, no acceptance claim"} for n in expected})
  argv=["/usr/bin/timeout","--signal=INT","--kill-after=15s","900s","bin/harness","build","payload","emi/etc/shop/00","--output-dir","out/payloads/shop00-boundary-fixed","--json"]
  env=dict(os.environ);env["PATH"]=os.pathsep.join(p for p in env["PATH"].split(os.pathsep) if not Path(p).is_relative_to(Path("/home/koija/.codex/tmp/arg0-linux")))
  env.update(PYTHONDONTWRITEBYTECODE="1",GIT_OPTIONAL_LOCKS="0",PYTHONPATH=str(root/"tools/python"))
  save("attempt.json",{"argv":argv,"bound":"900s INT +15s","start_epoch":time.time(),"primary_index_sha256":current[".git/index"]["sha256"]})
  result=run_bounded(root,argv,env=env,output_limit=64<<20)
  save("process.json",result)
  assert result["exit_code"]==0 and not result["failure"],result.get("stderr")
  report=json.loads(result["stdout"])
  reportpath=root/"out/payloads/shop00-boundary-fixed/report.json"
  assert json.loads(reportpath.read_bytes())==report and report["target"]=="emi/etc/shop/00"
  assert report["whole_payload_equal"] and report["size"]==87660
  payload=(root/report["output"]).read_bytes();original=(root/report["original"]).read_bytes()
  assert payload==original and sha(payload)==report["sha256"]
  archive=report["archive"]
  assert archive["whole_archive_equal"] and archive["entry_equal"]
  assert (root/archive["output"]).read_bytes()==(root/archive["original"]).read_bytes()
  assert (root/"out/payloads/shop00-boundary-fixed/entry.bin").read_bytes()==payload
  selected=[r for r in report["compiled"] if "emi/etc/shop/00@0x801D3654" in r["owners"]]
  assert len(selected)==1 and selected[0]["start"]==10836 and selected[0]["end"]==10888
  for name,state in current.items():
   observed,_=batch.read(root/name,max_bytes=64<<20,include_metadata=True,single_link=True)
   assert observed==state,name
  save("terminal-input-states.json",current)
  summary.update(status="payload complete; independent acceptance pending",report_sha256=sha(reportpath.read_bytes()),compiled_bytes=report["compiled_bytes"],retained_bytes=report["retained_bytes"])
except BaseException as error:
 summary.update(status="failed; keep applied batch",error=f"{type(error).__name__}: {error}")
 if isinstance(error,ProcessCleanupError): summary["cleanup"]="unconfirmed; stop"
 raise
finally:
 summary["elapsed_seconds"]=time.monotonic()-start
 save("summary.json",summary)
