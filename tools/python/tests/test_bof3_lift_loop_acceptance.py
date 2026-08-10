from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[3]


def test_agent_context_qwen_roles_stay_bounded() -> None:
    script = ROOT / ".pi/skills/bof3-re/scripts/agent-context.py"
    for definition in sorted((ROOT / ".pi/agents").glob("*.md")):
        front = definition.read_text(encoding="utf-8").split("---")[1]
        fields = dict(
            line.split(": ", 1) for line in front.splitlines() if ": " in line
        )
        if "qwen" not in fields.get("model", ""):
            continue
        name = fields["name"]
        result = subprocess.run(
            (sys.executable, str(script), name),
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=True,
        )
        assert len(result.stdout.encode()) < 13_000, name
        assert result.stdout.count("=====") <= 4, name


def test_agent_and_skill_context_files_stay_compact() -> None:
    files = sorted((ROOT / ".pi/agents").glob("*.md"))
    files += sorted((ROOT / ".pi/skills").glob("*/SKILL.md"))
    files += sorted((ROOT / ".pi/skills/bof3-re/references").glob("*/*.md"))
    total = sum(len(path.read_bytes()) for path in files)
    assert total <= 69_000, f".pi context files re-inflated: {total} bytes"


def test_function_brief_data_table_probe() -> None:
    script = ROOT / ".pi/skills/bof3-re/scripts/function-brief.py"

    def probe(selector: str) -> dict:
        result = subprocess.run(
            (sys.executable, str(script), selector),
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=True,
        )
        return json.loads(result.stdout)["data_table_probe"]

    table = probe("emi/scenario/scena16/00@0x801F8538")
    assert table["likely_data_table"] is True
    assert table["warning"]

    code = probe("emi/scenario/scena00/00@0x801FC7D0")
    assert code["likely_data_table"] is False
    assert code["warning"] is None
