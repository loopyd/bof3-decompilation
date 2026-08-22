from __future__ import annotations

import json
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[3]
AVAILABLE_AGENT_TOOLS = {
    "bash",
    "contact_supervisor",
    "edit",
    "fetch_content",
    "find",
    "get_search_content",
    "grep",
    "intercom",
    "ls",
    "mcp",
    "memory_remember",
    "memory_search",
    "read",
    "session_search",
    "vcc_recall",
    "web_search",
    "write",
}


def _agent_fields(text: str) -> dict[str, str]:
    front = text.split("---", 2)[1]
    return dict(line.split(": ", 1) for line in front.splitlines() if ": " in line)


def test_every_agent_first_command_and_tools_match_runtime_contract() -> None:
    role_aliases = {
        "bof3-cleanup": "cleanup",
        "bof3-reverse": "reverse",
        "bof3-review": "review",
    }
    for definition in sorted((ROOT / ".pi/agents").glob("*.md")):
        text = definition.read_text(encoding="utf-8")
        fields = _agent_fields(text)
        name = fields["name"]
        assert name == definition.stem
        commands = re.findall(r"`(bin/agent-context [^`]+)`", text)
        tools = {value for value in fields.get("tools", "").split(",") if value}
        if name == "classifier":
            assert commands == []
            assert tools == set()
            continue
        assert len(commands) == 1, definition
        assert "First repository command" in text, definition
        assert commands[0].split()[1] == role_aliases.get(name, name), definition
        assert "bash" in tools, definition
        assert tools <= AVAILABLE_AGENT_TOOLS, (
            definition,
            tools - AVAILABLE_AGENT_TOOLS,
        )
        assert "append_ledger" not in tools, definition
        assert "structured_output" not in tools, definition
        assert not any(tool.startswith("mcp:") for tool in tools), definition
        if "mcp" in text:
            assert "mcp" in tools, definition


def test_agent_context_qwen_roles_stay_bounded() -> None:
    command = ROOT / "bin/agent-context"
    for definition in sorted((ROOT / ".pi/agents").glob("*.md")):
        text = definition.read_text(encoding="utf-8")
        fields = _agent_fields(text)
        commands = re.findall(r"`(bin/agent-context [^`]+)`", text)
        assert "skills/bof3-re/scripts/agent-context" not in text, definition
        if fields.get("name") == "classifier":
            assert commands == []
            assert not fields.get("tools")
            continue
        assert len(commands) == 1, definition
        assert "First repository command" in text, definition
        if "qwen" not in fields.get("model", ""):
            continue
        argv = commands[0].split()[1:]
        result = subprocess.run(
            (str(command), *argv),
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=True,
        )
        assert len(result.stdout.encode()) < 14_000, fields["name"]
        assert result.stdout.count("=====") <= 8, fields["name"]
        assert "===== context prefill contract =====" in result.stdout


def test_checkpoint_and_orchestration_security_self_checks() -> None:
    scripts = ROOT / ".pi/skills/bof3-lift-loop/scripts"
    for name in ("test-attempt-checkpoint.py", "test-orchestration.py"):
        subprocess.run(
            (sys.executable, str(scripts / name)),
            cwd=ROOT,
            check=True,
            capture_output=True,
            text=True,
        )


def test_retained_lift_cleanup_uses_canonical_payload_not_generic_repair() -> None:
    template = (
        ROOT / ".pi/skills/bof3-lift-loop/scripts/lift_workflow_template.py"
    ).read_text()
    assert 'const cleanupRequest = ["retained-lift"' in template
    assert "task: cleanupRequest" in template
    assert 'cleanupRequest.startsWith("repair ")' in template
    assert 'task: "Clean the retained "' not in template


def test_agent_and_skill_context_files_stay_compact() -> None:
    files = sorted((ROOT / ".pi/agents").glob("*.md"))
    files += sorted((ROOT / ".pi/skills").glob("*/SKILL.md"))
    files += sorted((ROOT / ".pi/skills/bof3-re/references").glob("*/*.md"))
    total = sum(len(path.read_bytes()) for path in files)
    assert total <= 72_000, f".pi context files re-inflated: {total} bytes"


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
