"""Pinned private application proof validation for shared type promotion."""

from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any, Callable

from ..domain.receipts import sha256_file
from ..domain.sources import local_include_files


def private_proofs(
    root: Path,
    values: object,
    manifests: dict[str, Any],
    *,
    normalize_target: Callable[[object, dict[str, Any]], str],
    verify_application: Callable[[Path, object, str], dict[str, Any]],
) -> list[dict[str, Any]]:
    if not isinstance(values, list) or len(values) < 2:
        raise ValueError("shared promotion requires two private transaction proofs")
    proofs = []
    for value in values:
        if not isinstance(value, dict) or set(value) != {
            "path",
            "target",
            "expected_application_digest",
        }:
            raise ValueError("private transaction proof pin is invalid")
        name = value["path"]
        expected_digest = value["expected_application_digest"]
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
        proof = json.loads(path.read_text(encoding="utf-8"))
        verified = verify_application(root, proof, expected_digest)
        if verified["target"] != target:
            raise ValueError("private transaction proof target does not match pin")
        if proof["concern"] == "shared":
            raise ValueError("shared promotion must reference private transactions")
        proofs.append(
            {
                "path": name,
                "target": target,
                "expected_application_digest": expected_digest,
                "sha256": sha256_file(path),
                "application": proof,
            }
        )
    if len({item["path"] for item in proofs}) != len(proofs):
        raise ValueError("shared promotion private proof paths must be unique")
    if len({item["target"] for item in proofs}) != len(proofs):
        raise ValueError("shared promotion private proof targets must be unique")
    if len({item["expected_application_digest"] for item in proofs}) != len(proofs):
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
