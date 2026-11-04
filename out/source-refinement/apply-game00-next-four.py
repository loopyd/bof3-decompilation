"""Apply the reviewed Game00 next-four naming batch and verify its pinned outputs."""
import hashlib
import json
from pathlib import Path

from harness.common.lease import acquire_writer
from harness.common.process import run_bounded

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "out/source-refinement"
PLAN = BASE / "next-four-game00-naming-ready-draft/plan.json"
PATCH = BASE / "next-four-game00-naming-ready-draft/application.patch"
OUT = BASE / "game00-next-four-application.json"
PLAN_SHA = "99b6fc91f89d584c970be5437fe494796bf5ec27d87107f514abbefa57101ff7"
PATCH_SHA = "e635cce0bc5c44ab4b16583c2f18db77f2104764133dbfcd7e59e00f9bd72bcd"


def sha(body):
    return hashlib.sha256(body).hexdigest()


def main():
    assert Path.cwd() == ROOT
    assert not OUT.exists(), "batch already recorded"
    plan_body = PLAN.read_bytes()
    assert sha(plan_body) == PLAN_SHA
    patch_body = PATCH.read_bytes()
    assert sha(patch_body) == PATCH_SHA
    plan = json.loads(plan_body)
    pre = plan["owned_input_sha256"]
    post = plan["proposed_output_sha256"]
    assert set(pre) == set(post)
    observed_pre = {name: sha((ROOT / name).read_bytes()) for name in pre}
    assert observed_pre == pre, "owned PRE changed"
    record = {"task": "3.36", "plan_sha256": PLAN_SHA, "patch_sha256": PATCH_SHA, "pre": observed_pre}
    with acquire_writer(ROOT):
        result = run_bounded(ROOT, ["git", "apply", "--verbose", str(PATCH.relative_to(ROOT))], output_limit=1 << 20)
        record["apply_process"] = result
        assert result["failure"] is None and result["exit_code"] == 0, "patch application failed"
        observed_post = {name: sha((ROOT / name).read_bytes()) for name in post}
        record["post"] = observed_post
        record["post_matches_plan"] = observed_post == post
        assert observed_post == post, "applied outputs disagree with the reviewed plan"
        record["status"] = "applied; fresh POST capture and acceptance pending"
        OUT.write_text(json.dumps(record, indent=2, sort_keys=True, default=str) + "\n")


if __name__ == "__main__":
    main()
