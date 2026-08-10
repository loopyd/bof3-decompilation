from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]


def test_cleanup_agent_and_skill_context_stay_compact() -> None:
    files = [ROOT / ".pi/agents/bof3-cleanup.md"]
    files += sorted((ROOT / ".pi/skills/bof3-re/references/CLEANUP").glob("*.md"))
    total = sum(len(path.read_bytes()) for path in files)
    assert total <= 21_000, f"cleanup agent context re-inflated: {total} bytes"
