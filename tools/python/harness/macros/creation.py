"""Reviewed ownership admission for one new target-private macro header."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

from harness.common.paths import require_absent, validate_paths
from harness.domain.headers import validate_header_transition
from harness.domain.claims import manifest_source_paths
from harness.domain.includes import local_include_files
from harness.domain.manifests import load_target_manifests
from harness.domain.policy import validate_matching_text

from .facts import parse_macro_definitions

REVIEW_SCHEMA = "bof3.reviewed-macro-opportunity/v2"
_FIELDS = {"target", "header", "header_text", "manifest_before", "manifest_after"}


def validate_contract(value: object) -> dict:
    if not isinstance(value, dict) or set(value) != _FIELDS:
        raise ValueError("invalid reviewed header creation contract")
    if any(not isinstance(item, str) or not item.strip() for item in value.values()):
        raise ValueError("header creation facts must be nonempty text")
    validate_header_transition(
        value["target"],
        value["header"],
        value["manifest_before"],
        value["manifest_after"],
    )
    validate_matching_text(value["header_text"])
    return dict(value)


def manifest_path(value: dict) -> str:
    return f"config/targets/{value['target']}/target.toml"


def validate_admission(root: Path, value: object, manifests: dict) -> dict:
    creation = validate_contract(value)
    target, header = creation["target"], creation["header"]
    if target not in manifests:
        raise ValueError("new header target is not declared")
    validate_paths(root, [header, manifest_path(creation)])
    if not (root / header).parent.is_dir():
        raise ValueError("header creation requires an existing parent directory")
    require_absent(root, header)
    if (root / manifest_path(creation)).read_bytes().decode("utf-8") != creation[
        "manifest_before"
    ]:
        raise ValueError("header manifest PRE changed")
    for manifest in manifests.values():
        if header in manifest.headers:
            raise ValueError("new header has an existing ownership claim")
    return creation


def validate_changes(root: Path, manifest: dict, changes: dict[str, str]) -> None:
    creation = manifest.get("reviewed_opportunity", {}).get("creation")
    if creation is None:
        return
    creation = validate_contract(creation)
    header = creation["header"]
    if changes.get(header) != creation["header_text"]:
        raise ValueError("new header differs from the reviewed proposed contents")
    if changes.get(manifest_path(creation)) != creation["manifest_after"]:
        raise ValueError(
            "new header requires its complete reviewed manifest transition"
        )
    replacements = {root / name: text for name, text in changes.items()}
    allowed_sources = {item["source"] for item in manifest["affected_functions"]}
    if creation["target"] != manifest["target"] or manifest["targets"] != [
        creation["target"]
    ]:
        raise ValueError("private header creation cannot widen target scope")
    definitions = parse_macro_definitions(creation["header_text"], header)
    names = {definition.name for definition in definitions}
    if not names:
        raise ValueError("new macro header has no definitions")
    identifiers = re.compile(
        r"\b(?:" + "|".join(re.escape(name) for name in sorted(names)) + r")\b"
    )
    for directory in (root / "src", root / "include"):
        for path in directory.rglob("*"):
            if path.suffix in {".c", ".h", ".inc"} and path.is_file():
                if identifiers.search(path.read_text(encoding="utf-8")):
                    raise ValueError(
                        f"new macro identifier already occurs in {path.relative_to(root)}"
                    )
    consumers: set[str] = set()
    for source in (root / "src").rglob("*.c"):
        closure = local_include_files(
            root,
            [source],
            include_roots=(
                root / "src",
                root / "include",
                root / "toolchains/psyq/4.7/include",
            ),
            include_angle=True,
            replacements=replacements,
            strict=True,
            require_literal=True,
        )
        if root / header in closure:
            consumers.add(source.relative_to(root).as_posix())
    if consumers != allowed_sources:
        raise ValueError(
            "planned header include consumers differ from the complete affected source scope"
        )
    for target, owner in load_target_manifests(root).items():
        if target == creation["target"]:
            continue
        if consumers.intersection(
            path.relative_to(root).as_posix()
            for path in manifest_source_paths(root, owner)
        ):
            raise ValueError(
                "new private header would reach another target through shared source"
            )


def validate_manifest_state(manifest: dict, creation: dict) -> None:
    header = creation["header"]
    reviewed = manifest["reviewed_opportunity"]
    if (
        manifest["concern"] != "local_template"
        or manifest["target"] != creation["target"]
        or manifest["targets"] != [creation["target"]]
        or reviewed["declared_targets"] != [creation["target"]]
        or reviewed["owners"] != [header]
        or reviewed["owner_fingerprints"] != {header: None}
    ):
        raise ValueError("invalid private header creation scope")
    request = manifest["request"]
    revalidation = "expected_envelope_digest" in request
    if revalidation and (
        set(request)
        != {"expected_envelope_digest", "execution_run_id", "adopted_baseline"}
        or re.fullmatch(r"v1:[0-9a-f]{64}", str(request["expected_envelope_digest"]))
        is None
    ):
        raise ValueError("invalid retained creation revalidation request")
    expected_header = (
        hashlib.sha256(creation["header_text"].encode()).hexdigest()
        if revalidation
        else None
    )
    expected_config = hashlib.sha256(
        creation["manifest_after" if revalidation else "manifest_before"].encode()
    ).hexdigest()
    state = manifest["pre_state"]
    if (
        header not in state
        or state[header] != expected_header
        or state.get(manifest_path(creation)) != expected_config
    ):
        raise ValueError(
            "creation PRE must prove absence; revalidation must adopt the exact reviewed POST"
        )
