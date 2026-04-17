from __future__ import annotations

import difflib
import hashlib
from pathlib import Path
from typing import Any

from ..jsonio import read_json, write_json
from .workspace import load_workspace


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_text_if_possible(path: Path) -> str | None:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return None


def diff_excerpt(expected: Path, actual: Path) -> str | None:
    expected_text = read_text_if_possible(expected)
    actual_text = read_text_if_possible(actual)
    if expected_text is None or actual_text is None:
        return None
    diff_lines = list(
        difflib.unified_diff(
            expected_text.splitlines(),
            actual_text.splitlines(),
            fromfile=str(expected),
            tofile=str(actual),
            lineterm="",
        )
    )
    if not diff_lines:
        return ""
    return "\n".join(diff_lines[:200])


def load_build_status(path: Path) -> dict[str, Any] | None:
    if not path.is_file():
        return None
    payload = read_json(path)
    return payload if isinstance(payload, dict) else None


def build_diff_payload(
    workspace_payload: dict[str, Any],
    *,
    expected_artifact: Path | None,
    actual_artifact: Path | None,
) -> dict[str, Any]:
    outputs = workspace_payload["outputs"]
    build_status = load_build_status(Path(str(outputs["build_status"])))
    build_succeeded = (
        None if build_status is None else bool(build_status.get("succeeded"))
    )

    expected_exists = bool(expected_artifact and expected_artifact.is_file())
    actual_exists = bool(actual_artifact and actual_artifact.is_file())

    if build_succeeded is False:
        status = "blocked_build_failed"
        next_steps = ["fix the recorded build failure before diffing"]
    elif not expected_exists:
        status = "missing_expected"
        next_steps = ["set --expected or record an expected artifact in the workspace"]
    elif not actual_exists:
        status = "missing_actual"
        next_steps = ["set --actual or record an actual artifact in the workspace"]
    else:
        exact_match = sha256(expected_artifact) == sha256(actual_artifact)
        status = "exact_match" if exact_match else "different"
        next_steps = (
            ["artifacts match exactly"]
            if exact_match
            else [
                "inspect the recorded diff excerpt and iterate on the workspace build"
            ]
        )

    payload = {
        "schema": "rebof3-simple.match-diff/v1",
        "workspace_dir": workspace_payload["workspace"]["workspace_dir"],
        "program_path": workspace_payload["function"]["program_path"],
        "entry_hex": workspace_payload["function"]["entry_hex"],
        "expected_artifact": None
        if expected_artifact is None
        else str(expected_artifact),
        "actual_artifact": None if actual_artifact is None else str(actual_artifact),
        "expected_exists": expected_exists,
        "actual_exists": actual_exists,
        "build_status_present": build_status is not None,
        "build_status": build_status,
        "status": status,
        "next_steps": next_steps,
        "exact_match": status == "exact_match",
        "expected_sha256": None if not expected_exists else sha256(expected_artifact),
        "actual_sha256": None if not actual_exists else sha256(actual_artifact),
        "expected_size": None
        if not expected_exists
        else expected_artifact.stat().st_size,
        "actual_size": None if not actual_exists else actual_artifact.stat().st_size,
        "diff_excerpt": None
        if not expected_exists or not actual_exists
        else diff_excerpt(expected_artifact, actual_artifact),
    }
    return payload


def render_diff_markdown(payload: dict[str, Any]) -> str:
    lines = [
        "# Match Diff",
        "",
        f"- Program: `{payload['program_path']}`",
        f"- Entry: `{payload['entry_hex']}`",
        f"- Status: `{payload['status']}`",
        f"- Expected exists: {payload['expected_exists']}",
        f"- Actual exists: {payload['actual_exists']}",
        "",
        "## Next Steps",
        "",
    ]
    for step in payload["next_steps"]:
        lines.append(f"- {step}")
    diff_excerpt_text = payload.get("diff_excerpt")
    if diff_excerpt_text not in {None, ""}:
        lines.extend(["", "## Diff Excerpt", "", "```diff", diff_excerpt_text, "```"])
    return "\n".join(lines) + "\n"


def resolve_diff_inputs(
    workspace_payload: dict[str, Any],
    *,
    expected_artifact: Path | None,
    actual_artifact: Path | None,
) -> tuple[Path | None, Path | None]:
    configured_inputs = workspace_payload["inputs"]
    effective_expected = expected_artifact
    if effective_expected is None and configured_inputs.get("expected_artifact"):
        effective_expected = Path(str(configured_inputs["expected_artifact"]))
    effective_actual = actual_artifact
    if effective_actual is None and configured_inputs.get("actual_artifact"):
        effective_actual = Path(str(configured_inputs["actual_artifact"]))
    if effective_expected is not None:
        effective_expected = effective_expected.expanduser().resolve()
    if effective_actual is not None:
        effective_actual = effective_actual.expanduser().resolve()
    return effective_expected, effective_actual


def run_match_diff(
    workspace_path: Path,
    *,
    expected_artifact: Path | None,
    actual_artifact: Path | None,
) -> tuple[dict[str, Any], Path, Path]:
    workspace_payload = load_workspace(workspace_path)
    effective_expected, effective_actual = resolve_diff_inputs(
        workspace_payload,
        expected_artifact=expected_artifact,
        actual_artifact=actual_artifact,
    )
    payload = build_diff_payload(
        workspace_payload,
        expected_artifact=effective_expected,
        actual_artifact=effective_actual,
    )
    outputs = workspace_payload["outputs"]
    diff_json_path = Path(str(outputs["diff_json"]))
    diff_markdown_path = Path(str(outputs["diff_markdown"]))
    write_json(diff_json_path, payload)
    diff_markdown_path.write_text(render_diff_markdown(payload), encoding="utf-8")
    return payload, diff_json_path, diff_markdown_path
