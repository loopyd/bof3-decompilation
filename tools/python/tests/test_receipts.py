from __future__ import annotations

import hashlib
import json
from pathlib import Path

import pytest

from harness.domain.receipts import (
    CANDIDATE_KINDS,
    CANDIDATE_SCHEMA,
    TRANSACTION_SCHEMA,
    canonical_json,
    validate_candidate,
    validate_transaction,
)


def _candidate(root: Path, kind: str = "rename") -> dict[str, object]:
    owner = root / "src/test.c"
    owner.parent.mkdir(parents=True, exist_ok=True)
    owner.write_text("int test;\n", encoding="utf-8")
    manifest = root / "config/targets/exe/test/target.toml"
    manifest.parent.mkdir(parents=True, exist_ok=True)
    manifest.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src'\nbinary='out/test.bin'\n"
        "splat='config/targets/exe/test/splat.yaml'\nload_address=0x80100000\n"
        "sources=['src/test.c']\n",
        encoding="utf-8",
    )
    digest = hashlib.sha256(owner.read_bytes()).hexdigest()
    return {
        "schema": CANDIDATE_SCHEMA,
        "id": f"{kind}:exe/test@80100000",
        "kind": kind,
        "status": "blocked",
        "target": "exe/test",
        "address": 0x80100000,
        "end": 0x80100004,
        "owners": ["src/test.c"],
        "locations": ["src/test.c"],
        "fingerprints": {"src/test.c": digest},
        "provenance": ["original_bytes", "reviewed_splat"],
        "authority": "original",
        "observations": [{"id": "bytes", "text": "verified original bytes"}],
        "missing_facts": ["semantic role"],
        "receipts": [],
    }


def test_all_candidate_kinds_validate_and_serialize_deterministically(
    tmp_path: Path,
) -> None:
    assert len(CANDIDATE_KINDS) == 8
    for kind in CANDIDATE_KINDS:
        candidate = _candidate(tmp_path, kind)
        assert validate_candidate(candidate, tmp_path)["kind"] == kind
        assert json.loads(canonical_json(candidate, tmp_path))["kind"] == kind
        assert canonical_json(candidate, tmp_path) == canonical_json(
            dict(reversed(list(candidate.items()))), tmp_path
        )


def test_candidate_rejects_archive_identity_and_invalid_facts(tmp_path: Path) -> None:
    candidate = _candidate(tmp_path)
    candidate["target"] = "BIN/BATTLE/TEST.EMI#1"
    with pytest.raises(ValueError, match="canonical"):
        validate_candidate(candidate, tmp_path)
    candidate = _candidate(tmp_path)
    candidate["target"] = "exe/missing"
    candidate["id"] = "rename:exe/missing@80100000"
    with pytest.raises(ValueError, match="unknown candidate target"):
        validate_candidate(candidate, tmp_path)
    candidate = _candidate(tmp_path)
    candidate["kind"] = "class"
    with pytest.raises(ValueError, match="unknown candidate kind"):
        validate_candidate(candidate, tmp_path)
    candidate = _candidate(tmp_path)
    candidate["fingerprints"] = {"src/test.c": "bad"}
    with pytest.raises(ValueError, match="SHA-256|stale"):
        validate_candidate(candidate, tmp_path)
    candidate = _candidate(tmp_path)
    candidate["end"] = candidate["address"]
    with pytest.raises(ValueError, match="range"):
        validate_candidate(candidate, tmp_path)


@pytest.mark.parametrize(
    ("field", "value", "message"),
    [
        ("owners", ["invented/nope.c"], "unowned"),
        ("locations", ["invented/nope.h"], "unowned"),
        ("provenance", ["made_up"], "invented provenance"),
        ("authority", "decompiler_guess", "authority"),
        ("observations", [], "observations"),
    ],
)
def test_candidate_rejects_fabricated_or_incomplete_evidence(
    tmp_path: Path, field: str, value: object, message: str
) -> None:
    candidate = _candidate(tmp_path)
    candidate[field] = value
    with pytest.raises(ValueError, match=message):
        validate_candidate(candidate, tmp_path)


def test_candidate_rejects_existing_but_unclaimed_owner(tmp_path: Path) -> None:
    candidate = _candidate(tmp_path)
    unrelated = tmp_path / "docs/unrelated.md"
    unrelated.parent.mkdir(parents=True)
    unrelated.write_text("unrelated\n", encoding="utf-8")
    digest = hashlib.sha256(unrelated.read_bytes()).hexdigest()
    candidate["owners"] = ["docs/unrelated.md"]
    candidate["locations"] = ["docs/unrelated.md"]
    candidate["fingerprints"] = {"docs/unrelated.md": digest}
    with pytest.raises(ValueError, match="unowned"):
        validate_candidate(candidate, tmp_path)


def test_candidate_accepts_manifest_owned_local_include(tmp_path: Path) -> None:
    candidate = _candidate(tmp_path)
    header = tmp_path / "include/shared.h"
    header.parent.mkdir(parents=True)
    header.write_text("typedef int Shared;\n", encoding="utf-8")
    owner = tmp_path / "src/test.c"
    owner.write_text('#include "shared.h"\nint test;\n', encoding="utf-8")
    candidate["owners"] = ["src/test.c"]
    candidate["locations"] = ["include/shared.h"]
    candidate["fingerprints"] = {
        "src/test.c": hashlib.sha256(owner.read_bytes()).hexdigest(),
        "include/shared.h": hashlib.sha256(header.read_bytes()).hexdigest(),
    }
    assert validate_candidate(candidate, tmp_path)["locations"] == ["include/shared.h"]


def test_candidate_rejects_stale_live_fingerprint_and_arbitrary_id(
    tmp_path: Path,
) -> None:
    candidate = _candidate(tmp_path)
    candidate["fingerprints"] = {"src/test.c": "ab" * 32}
    with pytest.raises(ValueError, match="stale fingerprint"):
        validate_candidate(candidate, tmp_path)
    candidate = _candidate(tmp_path)
    candidate["id"] = "arbitrary"
    with pytest.raises(ValueError, match="canonical identity"):
        validate_candidate(candidate, tmp_path)


def test_transaction_is_concern_isolated_and_deterministic(tmp_path: Path) -> None:
    first = _candidate(tmp_path, "macro")
    second = _candidate(tmp_path, "macro")
    second["id"] = "macro:exe/test@80100004"
    second["address"] = 0x80100004
    second["end"] = 0x80100008
    transaction = {
        "schema": TRANSACTION_SCHEMA,
        "concern": "macro",
        "candidates": [second, first],
    }
    normalized = validate_transaction(transaction, tmp_path)
    assert [item["id"] for item in normalized["candidates"]] == [
        first["id"],
        second["id"],
    ]
    assert canonical_json(transaction, tmp_path, transaction=True) == canonical_json(
        {**transaction, "candidates": [first, second]},
        tmp_path,
        transaction=True,
    )
    first["kind"] = "typedef"
    with pytest.raises(ValueError, match="mix|canonical identity"):
        validate_transaction(transaction, tmp_path)


def test_transaction_rejects_casefold_or_canonical_identity_collision(
    tmp_path: Path,
) -> None:
    first = _candidate(tmp_path, "macro")
    second = dict(first)
    transaction = {
        "schema": TRANSACTION_SCHEMA,
        "concern": "macro",
        "candidates": [first, second],
    }
    with pytest.raises(ValueError, match="identities"):
        validate_transaction(transaction, tmp_path)
