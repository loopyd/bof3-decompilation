"""Macro target participation and retained shared proof cardinality."""

from __future__ import annotations

from harness.common.participation import validate_targets
from harness.common.checks import required_checks


def resolve_targets(request: dict, target: str, manifests: dict, normalize) -> set[str]:
    if request["concern"] != "shared_template":
        return {target}
    supplied = request.get("shared_targets")
    if not isinstance(supplied, list):
        raise ValueError("shared template requires at least two declared targets")
    targets = {normalize(item, manifests) for item in supplied}
    if target not in targets or len(targets) < 2 or len(targets) != len(supplied):
        raise ValueError(
            "shared template requires distinct declared targets, at least two"
        )
    return targets


def validate_manifest(manifest: dict) -> None:
    count = validate_targets(manifest)
    proofs = manifest["exact_function_proofs"]
    if manifest["concern"] != "shared_template":
        if count != 1 or proofs:
            raise ValueError("private macro manifest has shared scope")
        return
    if (
        count < 2
        or not isinstance(proofs, list)
        or len(proofs) != count
        or {proof["target"] for proof in proofs} != set(manifest["targets"])
        or any(
            len({proof[key] for proof in proofs}) != count
            for key in ("path", "selector", "expected_envelope_digest")
        )
    ):
        raise ValueError(
            "shared template requires one independent proof per declared target"
        )
    validate_wrappers(proofs, manifest["affected_functions"])
    selectors = {proof["selector"] for proof in proofs}
    wrappers = [
        function
        for function in manifest["affected_functions"]
        if function["selector"] in selectors
    ]
    if any(
        check not in manifest["required_checks"]
        for check in required_checks([], wrappers)
    ):
        raise ValueError("shared template checks omit a proven exact wrapper")


def validate_wrappers(proofs: list[dict], functions: list[dict]) -> None:
    for proof in proofs:
        original = proof["application"]["manifest"]["affected_functions"]
        wrappers = [
            function
            for function in original
            if function["selector"] == proof["selector"]
            and function["target"] == proof["target"]
            and function["status"] == "exact"
        ]
        if len(wrappers) != 1 or not any(
            all(
                function[key] == wrappers[0][key]
                for key in ("selector", "target", "source", "status")
            )
            for function in functions
        ):
            raise ValueError(
                "shared template affected functions omit a proven exact wrapper"
            )
