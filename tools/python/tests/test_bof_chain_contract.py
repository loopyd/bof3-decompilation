import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CHAIN = ROOT / ".pi" / "chains" / "bof.chain.json"


def _load_chain() -> dict:
    return json.loads(CHAIN.read_text(encoding="utf-8"))


def test_chain_json_parses() -> None:
    chain = _load_chain()
    assert isinstance(chain["chain"], list)
    assert chain["chain"]


def test_planner_has_exactly_one_repo_recon_handoff() -> None:
    chain = _load_chain()
    recon = next(s for s in chain["chain"] if s.get("phase") == "Recon")
    recon_agents = [s["agent"] for s in recon["parallel"]]
    assert "scout" not in recon_agents
    planner = next(s for s in chain["chain"] if s.get("agent") == "planner")
    assert "{outputs.scoutResult}" not in planner["task"]
    assert "{outputs.contextResult}" in planner["task"]


def test_worker_task_routes_lifts_to_lift_loop() -> None:
    chain = _load_chain()
    impl = next(s for s in chain["chain"] if s.get("phase") == "Implementation")
    task = impl["parallel"]["task"]
    assert "TARGET@0xADDRESS" not in task or "out of scope" in task
    assert "/skill:bof3-re" not in task
    assert "/skill:bof3-lift-loop" in task
