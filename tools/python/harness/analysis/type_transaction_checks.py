"""Required command checks for reviewed type and macro transactions."""

from __future__ import annotations

import hashlib
import json
import os
import re
import subprocess
from pathlib import Path
from typing import Any, Callable

from .type_candidate_review import digest
from ..domain.ids import normalize_target_id

_HEX8_RE = re.compile(r"^0[xX][0-9a-fA-F]{8}$")
_SELECTOR_RE = re.compile(r"^(?P<target>.+)@(?P<address>0x[0-9a-fA-F]{8})$")

_COMMON_KEYS = {
    "schema",
    "status",
    "exact_match",
    "byte_match",
    "source",
    "function",
    "address",
    "original_size",
    "current_size",
    "size_delta",
    "original_binary",
    "current_object",
    "outputs",
}
_ASM_KEYS = _COMMON_KEYS | {"instruction_count", "first_mismatch"}
_SCHEMAS = {
    "bin/asm-diff": ("harness.asm-diff-one/v2", _ASM_KEYS),
    "bin/byte-match": ("harness.byte-match-one/v1", _COMMON_KEYS),
}


def _check(
    tool: str,
    target: str,
    selector: str | None = None,
    function: str | None = None,
    source: str | None = None,
) -> dict[str, Any]:
    argv = [f"bin/{tool}", selector or target]
    if tool == "asm-diff":
        argv += ["--json", "--detail", "full"]
    elif tool == "byte-match":
        argv += ["--json"]
    return {
        "command": " ".join(argv),
        "argv": argv,
        "target": target,
        "selector": selector,
        "function": function,
        "source": source,
    }


def _integer(value: object, *, minimum: int = 0) -> bool:
    return isinstance(value, int) and not isinstance(value, bool) and value >= minimum


def _hex8(value: object) -> int | None:
    """Parse an 0x-prefixed 8-digit hex address string as an integer."""

    if not isinstance(value, str) or _HEX8_RE.match(value) is None:
        return None
    return int(value, 16)


def _traverses_symlink(path: Path, root: Path) -> bool:
    """True when any repo-relative component of ``path`` is a symlink."""

    try:
        parts = path.relative_to(root).parts
    except ValueError:
        return False
    current = root
    for part in parts:
        current = current / part
        if current.is_symlink():
            return True
    return False


def _has_symlink_component(path: Path) -> bool:
    """True when any component of the absolute ``path`` is a symlink."""

    current = path
    while current != current.parent:
        if current.is_symlink():
            return True
        current = current.parent
    return False


def _canonical_source(source: object, expected: object, root: Path) -> str | None:
    """Repo-relative canonical form of a tool-emitted source identity.

    Returns the canonical repo-relative source only when the payload path is
    itself a canonical in-repo path: an absolute path must lexically equal
    ``root/expected`` with no symlink component anywhere; a relative path
    must stay lexically under ``root``, traverse no symlink, and resolve to
    exactly ``root/expected``.  Paths outside the repo, symlinked paths, and
    non-string values return None.
    """

    if not isinstance(source, str) or not source or not isinstance(expected, str):
        return None
    try:
        root_resolved = root.resolve()
    except OSError:
        return None
    path = Path(source).expanduser()
    if path.is_absolute():
        if path != root / expected or _has_symlink_component(path):
            return None
        return expected
    lexical = Path(os.path.normpath(root / path))
    try:
        lexical.relative_to(root)
    except ValueError:
        return None
    if _traverses_symlink(lexical, root):
        return None
    try:
        relative = lexical.resolve().relative_to(root_resolved)
    except (OSError, ValueError):
        return None
    canonical = relative.as_posix()
    return canonical if canonical == expected else None


def _canonical_identity(value: object) -> object:
    """Normalize target spelling and hex case in a selector/target identity."""

    if not isinstance(value, str):
        return value
    match = _SELECTOR_RE.match(value)
    if match is None:
        try:
            return normalize_target_id(value).value
        except ValueError:
            return value
    try:
        target = normalize_target_id(match.group("target")).value
    except ValueError:
        target = match.group("target")
    return f"{target}@0x{int(match.group('address'), 16):08X}"


def _same_identity(value: object, expected: object) -> bool:
    return _canonical_identity(value) == _canonical_identity(expected)


def _validate_common(
    payload: dict[str, Any], check: dict[str, Any], root: Path
) -> None:
    selector = check.get("selector")
    expected_address = _hex8(selector.rsplit("@", 1)[1]) if selector else None
    original = payload.get("original_size")
    current = payload.get("current_size")
    outputs = payload.get("outputs")
    size_delta = payload.get("size_delta")
    tool = check["argv"][0]
    output_keys = (
        {
            "directory",
            "summary",
            "diff",
            "original",
            "current",
            "compiler",
            "original_bytes",
            "build_log",
        }
        if tool == "bin/asm-diff"
        else set()
    )
    if (
        payload.get("status") != "different"
        or payload.get("exact_match") is not False
        or payload.get("byte_match") is not False
        or payload.get("function") != check.get("function")
        or _canonical_source(payload.get("source"), check.get("source"), root) is None
        or _hex8(payload.get("address")) != expected_address
        or not _integer(original)
        or not _integer(current)
        or not isinstance(original, int)
        or not isinstance(current, int)
        or not isinstance(size_delta, int)
        or isinstance(size_delta, bool)
        or size_delta != current - original
        or not isinstance(payload.get("original_binary"), str)
        or not payload["original_binary"]
        or not isinstance(payload.get("current_object"), str)
        or not payload["current_object"]
        or not isinstance(outputs, dict)
        or set(outputs)
        not in {frozenset(output_keys), frozenset(output_keys | {"linked"})}
        or any(not isinstance(value, str) or not value for value in outputs.values())
    ):
        raise ValueError("partial transaction evidence has invalid values or identity")


def _validate_asm(payload: dict[str, Any]) -> None:
    counts = payload.get("instruction_count")
    mismatch = payload.get("first_mismatch")
    if not isinstance(counts, dict) or set(counts) != {
        "original",
        "current",
        "matching",
        "match_percent",
    }:
        raise ValueError("partial asm-diff evidence has invalid instruction counts")
    values = tuple(counts.get(key) for key in ("original", "current", "matching"))
    percent = counts.get("match_percent")
    if not all(_integer(value) for value in values):
        raise ValueError("partial asm-diff evidence has invalid instruction ranges")
    original = counts["original"]
    current = counts["current"]
    matching = counts["matching"]
    assert isinstance(original, int)
    assert isinstance(current, int)
    assert isinstance(matching, int)
    if (
        matching > min(original, current)
        or not isinstance(percent, (int, float))
        or isinstance(percent, bool)
        or percent != round((matching / max(original, current, 1)) * 100, 2)
    ):
        raise ValueError("partial asm-diff evidence has invalid instruction ranges")
    mismatch_keys = {
        "original_index",
        "current_index",
        "original_offset",
        "current_offset",
        "original",
        "current",
    }
    if not isinstance(mismatch, dict) or set(mismatch) != mismatch_keys:
        raise ValueError("partial asm-diff evidence has invalid first mismatch")
    if mismatch["original_index"] is None and mismatch["current_index"] is None:
        raise ValueError("partial asm-diff mismatch has no differing instruction")
    for side, count in (("original", original), ("current", current)):
        index = mismatch[f"{side}_index"]
        offset = mismatch[f"{side}_offset"]
        instruction = mismatch[side]
        if index is None:
            if offset is not None or instruction is not None:
                raise ValueError("partial asm-diff mismatch fields disagree")
        elif (
            not _integer(index)
            or index >= count
            or offset != index * 4
            or not isinstance(instruction, str)
        ):
            raise ValueError("partial asm-diff mismatch fields are out of range")


def _evidence(check: dict[str, Any], output: str, root: Path) -> dict[str, Any]:
    try:
        payload = json.loads(output)
    except (json.JSONDecodeError, TypeError) as error:
        raise ValueError(
            "partial transaction check did not return JSON evidence"
        ) from error
    expected = _SCHEMAS.get(check["argv"][0])
    if not isinstance(payload, dict) or expected is None:
        raise ValueError("partial transaction check did not return JSON evidence")
    schema, keys = expected
    if set(payload) != keys or payload.get("schema") != schema:
        raise ValueError("partial transaction evidence has an invalid tool schema")
    _validate_common(payload, check, root)
    if check["argv"][0] == "bin/asm-diff":
        _validate_asm(payload)
    return {
        "selector": check["selector"],
        "target": check["target"],
        "address": payload["address"],
        "function": payload["function"],
        "raw_sha256": hashlib.sha256(output.encode()).hexdigest(),
    }


def _baseline_facts(
    check: dict[str, Any], exit_code: object, output: str, root: Path
) -> dict[str, Any]:
    if not isinstance(exit_code, int) or isinstance(exit_code, bool) or exit_code != 1:
        raise ValueError("partial transaction check must report mismatch exit 1")
    return {"exit_code": exit_code, **_evidence(check, output, root)}


def capture_partial_baselines(
    root: Path,
    functions: list[dict[str, str]],
    *,
    runner: Callable[..., subprocess.CompletedProcess[str]] = subprocess.run,
) -> dict[str, list[dict[str, Any]]]:
    baselines = {}
    for function in functions:
        if function["status"] != "partial":
            continue
        evidence = []
        for tool in ("asm-diff", "byte-match"):
            check = _check(
                tool,
                function["target"],
                function["selector"],
                function["function"],
                function["source"],
            )
            result = runner(check["argv"], cwd=root, text=True, capture_output=True)
            facts = _baseline_facts(check, result.returncode, result.stdout, root)
            evidence.append({**facts, "digest": digest(facts)})
        baselines[function["selector"]] = evidence
    return baselines


def required_checks(
    targets: list[str],
    functions: list[dict[str, str]],
    partial_baselines: dict[str, list[dict[str, Any]]] | None = None,
) -> list[dict[str, Any]]:
    checks = []
    for target in targets:
        checks += [_check("splat", target), _check("build", target)]
    partial_baselines = partial_baselines or {}
    for function in functions:
        selector = function["selector"]
        function_checks = [
            _check(
                tool,
                function["target"],
                selector,
                function["function"],
                function["source"],
            )
            for tool in ("asm-diff", "byte-match")
        ]
        if function["status"] == "partial":
            baselines = partial_baselines.get(selector)
            if not isinstance(baselines, list) or len(baselines) != 2:
                raise ValueError(f"partial transaction baseline missing: {selector}")
            for check, baseline in zip(function_checks, baselines):
                check["partial_baseline"] = baseline
        checks += function_checks
    return checks


def check_evidence(
    check: dict[str, Any], exit_code: object, output: str, root: Path
) -> dict[str, Any]:
    baseline = check.get("partial_baseline")
    if baseline is None:
        return {"passed": exit_code == 0, "metrics": None, "evidence_digest": None}
    try:
        facts = _baseline_facts(check, exit_code, output, root)
    except ValueError:
        return {"passed": False, "metrics": None, "evidence_digest": None}
    required = {"exit_code", "selector", "target", "address", "function", "raw_sha256"}
    return {
        "passed": set(baseline) == required | {"digest"}
        and facts["exit_code"] == baseline["exit_code"]
        and _same_identity(facts["selector"], baseline["selector"])
        and _same_identity(facts["target"], baseline["target"])
        and _hex8(facts["address"]) == _hex8(baseline["address"])
        and facts["function"] == baseline["function"]
        and facts["raw_sha256"] == baseline["raw_sha256"]
        and baseline.get("digest") == digest(facts),
        "metrics": None,
        "evidence_digest": digest(facts),
    }
