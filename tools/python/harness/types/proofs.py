"""Pinned private application proof validation for shared type promotion."""

from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any, Callable

from harness.domain.includes import local_include_files
from harness.domain.receipts import sha256_file
from harness.io import unique_object


def private_proofs(
    root: Path,
    values: object,
    manifests: dict[str, Any],
    *,
    normalize_target: Callable[[object, dict[str, Any]], str],
    verify_reviewed_application: Callable[[Path, object, str], dict[str, Any]]
    | None = None,
) -> list[dict[str, Any]]:
    if verify_reviewed_application is None:
        from harness.types.application import verify_reviewed_application

    if not isinstance(values, list) or len(values) < 2:
        raise ValueError("shared promotion requires two private transaction proofs")
    proofs = []
    for value in values:
        if not isinstance(value, dict) or set(value) != {
            "path",
            "target",
            "expected_envelope_digest",
        }:
            raise ValueError("private transaction proof pin is invalid")
        name = value["path"]
        expected_digest = value["expected_envelope_digest"]
        path = root / name if isinstance(name, str) else root
        resolved = path.resolve()
        review_root = (root / "out/reviews").resolve()
        if (
            not isinstance(name, str)
            or not name
            or Path(name).is_absolute()
            or not resolved.is_relative_to(review_root)
            or resolved.relative_to(root.resolve()).as_posix() != name
            or path.is_symlink()
            or not path.is_file()
        ):
            raise ValueError("private transaction proof path is invalid")
        target = normalize_target(value["target"], manifests)
        if not isinstance(expected_digest, str) or not re.fullmatch(
            r"v1:[0-9a-f]{64}", expected_digest
        ):
            raise ValueError("private transaction proof digest pin is invalid")
        proof = json.loads(
            path.read_text(encoding="utf-8"), object_pairs_hook=unique_object
        )
        from harness.types.transactions import _manifest
        from harness.common.promotion import private_application

        envelope = proof
        proof, verified = private_application(
            root,
            envelope,
            expected_digest,
            "type",
            verify_reviewed_application,
            manifest_validator=_manifest,
        )
        if verified["target"] != target:
            raise ValueError("private transaction proof target does not match pin")
        if proof["concern"] == "shared":
            raise ValueError("shared promotion must reference private transactions")
        proofs.append(
            {
                "path": name,
                "target": target,
                "expected_envelope_digest": expected_digest,
                "sha256": sha256_file(path),
                "application": proof,
                "reviewed_envelope": envelope,
            }
        )
    if len({item["path"] for item in proofs}) != len(proofs):
        raise ValueError("shared promotion private proof paths must be unique")
    if len({item["target"] for item in proofs}) != len(proofs):
        raise ValueError("shared promotion private proof targets must be unique")
    if len({item["expected_envelope_digest"] for item in proofs}) != len(proofs):
        raise ValueError("shared promotion private proof digests must be unique")
    proofs.sort(key=lambda item: (item["target"], item["path"]))
    contracts = {
        (
            json.dumps(item["application"]["representation"], sort_keys=True),
            json.dumps(item["application"]["semantics"], sort_keys=True),
        )
        for item in proofs
    }
    if len(contracts) != 1:
        raise ValueError("shared promotion private contracts differ")
    if any(
        int(value, 16) >= 0x80000000
        for value in re.findall(r"0x[0-9A-Fa-f]+", " ".join(next(iter(contracts))))
    ):
        raise ValueError("shared promotion contract contains a target-local address")
    return proofs


def proof_dependencies(root: Path, proof: dict[str, Any]) -> set[str]:
    """Return live repo-local dependencies pinned by one private application."""

    manifest = proof["application"].get("manifest", {})
    seeds = [root / name for name in manifest.get("allowed_paths", [])]
    dependencies = {
        path.resolve().relative_to(root.resolve()).as_posix()
        for path in local_include_files(root, seeds)
        if path.is_file() and path.resolve().is_relative_to(root.resolve())
    }
    dependencies.update(
        name
        for name in manifest.get("allowed_paths", [])
        if isinstance(name, str) and (root / name).is_file()
    )
    return dependencies


def validate_shared_header(
    root: Path,
    header: str,
    reviewed: list[dict[str, Any]],
    proofs: list[dict[str, Any]],
) -> None:
    if any(header not in item["candidate"]["locations"] for item in reviewed):
        raise ValueError(
            "shared type header must be a reviewed candidate location for every owner"
        )
    if any(header not in proof_dependencies(root, proof) for proof in proofs):
        raise ValueError(
            "shared type header must be a private proof dependency for every owner"
        )
