"""Bounded OpenCode execution for one reverse-engineering mission."""

from __future__ import annotations

import json
import os
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from harness.domain.registry import ResolvedTarget, resolve_target
from harness.reverse import MissionState, save_mission


RESULT_SCHEMA = "bof3.reverse-result/v1"
MAX_OUTPUT_BYTES = 2 * 1024 * 1024


@dataclass(frozen=True)
class RunnerResult:
    """Normalized result from a single OpenCode mission."""

    exit_code: int
    payload: dict[str, Any]
    artifact_dir: Path


def _relative(root: Path, path: Path) -> str:
    return path.relative_to(root).as_posix()


def _write_text(path: Path, value: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(value, encoding="utf-8")
    os.replace(temporary, path)


def _write_json(path: Path, value: dict[str, Any]) -> None:
    _write_text(path, json.dumps(value, indent=2, sort_keys=True) + "\n")


def _bounded_text(value: str | bytes | None) -> str:
    if isinstance(value, bytes):
        value = value.decode("utf-8", errors="replace")
    return (value or "")[:MAX_OUTPUT_BYTES]


def build_prompt(root: Path, mission: MissionState, target: ResolvedTarget) -> str:
    """Build a prompt solely from validated target and mission facts."""

    if mission.address is None:
        raise ValueError("OpenCode missions require a function address")
    source = target.source_dir / f"func_{mission.address:08x}.c"
    snapshot = root / "out" / "reverse" / target.id.value / "snapshot.json"
    replay = target.reviewed_replay_path
    snapshot_text = _relative(root, snapshot) if snapshot.is_file() else "absent"
    replay_text = _relative(root, replay) if replay.is_file() else "absent"

    return f"""Execute one bounded BOF3 reverse-engineering mission.

Mission:
- ID: {mission.mission_id}
- Target: {target.id.value}
- Function: 0x{mission.address:08x}
- Goal: {mission.goal}
- Strategy: {mission.strategy}
- Time limit: {mission.budget_time_seconds} seconds

Verified target facts:
- Load address: 0x{target.load_address:08x}
- Source directory: {_relative(root, target.source_dir)}
- Expected source: {_relative(root, source)}
- Input image: {_relative(root, target.binary_path)}
- Splat layout: {_relative(root, target.splat_path)}
- Reviewed replay: {replay_text}
- Analyzer snapshot: {snapshot_text}

Required workflow:
1. Load $decomp-loop and its required references before changing C.
2. Load $psx-rizin only when analyzer evidence is needed. Load $bof3-specs only
   when payload, EMI, or cross-binary interpretation is required.
3. Verify the function boundary, code/data ownership, and load mapping from
   original bytes and canonical Splat assembly. Those outrank analyzer output.
4. Work only on this target/function and strictly required adjacent declarations
   or bindings. Use factual readable C89; never use handwritten assembly.
5. Use bin/harness diff and bin/asmdiff for comparison. Use bin/permute only
   after the source compiles and its boundary/control flow are credible.
6. Do not run bin/harness reverse, delegate work, commit, push, promote targets,
   install dependencies, or modify unrelated files.

At completion, emit exactly one JSON object with this schema and no Markdown:
{{
  "schema": "{RESULT_SCHEMA}",
  "mission_id": "{mission.mission_id}",
  "target": "{target.id.value}",
  "address": {mission.address},
  "status": "complete|progress|blocked|failed",
  "summary": "short factual summary",
  "changed_paths": ["repository-relative paths"],
  "checks": [{{"command": "...", "status": "pass|fail|skipped"}}],
  "exact_match": false,
  "instruction_match_percent": 0.0,
  "byte_match_percent": 0.0,
  "blockers": []
}}
"""


def _extract_result(output: str) -> dict[str, Any] | None:
    decoder = json.JSONDecoder()
    best: dict[str, Any] | None = None

    def inspect(value: Any) -> None:
        nonlocal best
        if isinstance(value, dict):
            if value.get("schema") == RESULT_SCHEMA:
                best = value
            for child in value.values():
                inspect(child)
        elif isinstance(value, list):
            for child in value:
                inspect(child)
        elif isinstance(value, str):
            for start, char in enumerate(value):
                if char != "{":
                    continue
                try:
                    nested, _ = decoder.raw_decode(value[start:])
                except json.JSONDecodeError:
                    continue
                inspect(nested)

    for index, char in enumerate(output):
        if char != "{":
            continue
        try:
            value, _ = decoder.raw_decode(output[index:])
        except json.JSONDecodeError:
            continue
        inspect(value)
    return best


def _validate_result(root: Path, mission: MissionState, result: dict[str, Any]) -> None:
    if result.get("mission_id") != mission.mission_id:
        raise ValueError("OpenCode result mission_id does not match the launched mission")
    if result.get("target") != mission.target_id or result.get("address") != mission.address:
        raise ValueError("OpenCode result target does not match the launched mission")
    if result.get("status") not in {"complete", "progress", "blocked", "failed"}:
        raise ValueError("OpenCode result has an invalid status")
    for key in ("instruction_match_percent", "byte_match_percent"):
        value = result.get(key)
        if not isinstance(value, (int, float)) or not 0 <= value <= 100:
            raise ValueError(f"OpenCode result has invalid {key}")
    if result.get("exact_match") and (
        result["instruction_match_percent"] != 100
        or result["byte_match_percent"] != 100
    ):
        raise ValueError("OpenCode result claims an incomplete exact match")
    for changed in result.get("changed_paths", []):
        path = Path(changed)
        if path.is_absolute() or ".." in path.parts:
            raise ValueError(f"OpenCode result has unsafe changed path: {changed}")
        if path.parts and path.parts[0] in {"inputs", "build", "toolchains"}:
            raise ValueError(f"OpenCode result changed a protected path: {changed}")
        if not (root / path).resolve().is_relative_to(root):
            raise ValueError(f"OpenCode result has external changed path: {changed}")


def _opencode_command(root: Path, mission: MissionState, prompt: str) -> list[str]:
    executable = os.environ.get("HARNESS_OPENCODE_BIN", "opencode")
    if not Path(executable).is_absolute() and shutil.which(executable) is None:
        raise FileNotFoundError(f"OpenCode executable not found: {executable}")
    agent = os.environ.get("HARNESS_OPENCODE_AGENT", "bof3-reverse")
    command = [
        executable,
        "run",
        "--format",
        "json",
        "--dir",
        str(root),
        "--agent",
        agent,
        "--title",
        mission.mission_id,
    ]
    model = os.environ.get("HARNESS_OPENCODE_MODEL")
    if model:
        command.extend(("--model", model))
    command.append(prompt)
    return command


def run_opencode_mission(root: Path, mission: MissionState) -> RunnerResult:
    """Run one mission and retain its prompt, raw output, and normalized result."""

    if os.environ.get("HARNESS_REVERSE_ACTIVE"):
        raise RuntimeError("refusing recursive harness reverse mission")
    if mission.address is None:
        raise ValueError("OpenCode missions require a function address")
    if mission.budget_functions != 1:
        raise ValueError("OpenCode missions support exactly one function")
    if mission.budget_time_seconds < 1:
        raise ValueError("OpenCode mission time budget must be positive")

    target = resolve_target(root, mission.target_id)
    if not target.source_dir.is_dir():
        raise FileNotFoundError(f"target source directory missing: {target.source_dir}")
    if not target.splat_path.is_file():
        raise FileNotFoundError(f"target Splat configuration missing: {target.splat_path}")

    artifact_dir = root / "out" / "reverse" / mission.target_id / "functions" / f"func_{mission.address:08x}"
    artifact_dir.mkdir(parents=True, exist_ok=True)
    save_mission(root, mission)
    prompt = build_prompt(root, mission, target)
    _write_text(artifact_dir / "prompt.txt", prompt)

    environment = os.environ.copy()
    environment["HARNESS_REVERSE_ACTIVE"] = "1"
    try:
        completed = subprocess.run(
            _opencode_command(root, mission, prompt),
            cwd=root,
            env=environment,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=mission.budget_time_seconds,
            start_new_session=True,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        output = _bounded_text(exc.stdout)
        error = _bounded_text(exc.stderr)
        _write_text(artifact_dir / "opencode.events.jsonl", output)
        _write_text(artifact_dir / "opencode.stderr.log", error)
        payload = {"status": "failed", "error": "OpenCode mission timed out"}
        _write_json(artifact_dir / "result.json", payload)
        return RunnerResult(2, payload, artifact_dir)

    output = _bounded_text(completed.stdout)
    error = _bounded_text(completed.stderr)
    _write_text(artifact_dir / "opencode.events.jsonl", output)
    _write_text(artifact_dir / "opencode.stderr.log", error)
    result = _extract_result(output)
    if completed.returncode != 0 or result is None:
        payload = {
            "status": "failed",
            "error": "OpenCode did not return a valid mission result",
            "returncode": completed.returncode,
        }
        _write_json(artifact_dir / "result.json", payload)
        return RunnerResult(2, payload, artifact_dir)
    try:
        _validate_result(root, mission, result)
    except ValueError as exc:
        payload = {"status": "failed", "error": str(exc)}
        _write_json(artifact_dir / "result.json", payload)
        return RunnerResult(2, payload, artifact_dir)
    _write_json(artifact_dir / "result.json", result)
    return RunnerResult(0 if result["status"] == "complete" else 1, result, artifact_dir)
