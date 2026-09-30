"""Argument and local-variable naming discovery and reviewed transactions."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

import pytest
from harness.naming import source_identifiers as si


SOURCE = """\
/* @behavior Copies a value into the work cell.
 * @source 0x80100000
 */
void func_80100000(u8 value, u8 *cell) {
  u8 local_value;

  local_value = value;
  *cell = local_value;
}

/* @behavior Copies the shared counter.
 * @source 0x80100010
 */
void func_80100010(void) {
  u8 value;

  value = shared_count;
}
"""


def _repo(tmp_path: Path) -> Path:
    (tmp_path / "config/targets/exe/test").mkdir(parents=True)
    (tmp_path / "src/test").mkdir(parents=True)
    (tmp_path / "out").mkdir(parents=True)
    (tmp_path / "out/test.bin").write_bytes(b"\0" * 0x20)
    (tmp_path / "config/targets/exe/test/target.toml").write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=['src/test/func_80100000.c']\nsupport_sources=[]\n"
        "psyq_source=''\n"
    )
    (tmp_path / "config/targets/exe/test/symbols.txt").write_text(
        "func_80100000 = 0x80100000;\nfunc_80100010 = 0x80100010;\n"
    )
    (tmp_path / "config/targets/exe/test/splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [0x10, c, func_80100010]\n"
    )
    (tmp_path / "src/test/func_80100000.c").write_text(SOURCE)
    return tmp_path


def _source(root: Path) -> Path:
    return root / "src/test/func_80100000.c"


def _names(root: Path) -> set[str]:
    return {row["id"] for row in si.collect_source_identifiers(root, "exe/test")}


def test_discovery_enumerates_arguments_and_locals_without_mutation(
    tmp_path: Path,
) -> None:
    root = _repo(tmp_path)
    before = _source(root).read_bytes()
    ids = _names(root)
    assert "exe/test@80100000@argument:value" in ids
    assert "exe/test@80100000@argument:cell" in ids
    assert "exe/test@80100000@local:local_value" in ids
    assert "exe/test@80100010@local:value" in ids
    assert _source(root).read_bytes() == before


def test_describe_requires_exact_fingerprint(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    row = si.describe_source_identifier(
        root, "exe/test", "exe/test@80100000@argument:value"
    )
    assert row["kind"] == "argument"
    assert row["uses"] == 2
    assert (
        si.describe_source_identifier(
            root,
            "exe/test",
            "exe/test@80100000@argument:value",
            expected_fingerprint=row["fingerprint"],
        )
        == row
    )
    with pytest.raises(ValueError):
        si.describe_source_identifier(
            root,
            "exe/test",
            "exe/test@80100000@argument:value",
            expected_fingerprint="v1:deadbeef",
        )


def test_prepare_rejects_scope_escape(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    with pytest.raises(ValueError, match="scope escape"):
        si.prepare_source_transaction(
            root,
            "exe/test",
            "exe/test@80100000",
            kind="argument",
            old_name="value",
            new_name="input_value",
            output=Path("out/tx.json"),
        )


def test_prepare_rejects_existing_new_name(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    with pytest.raises(ValueError, match="already occurs"):
        si.prepare_source_transaction(
            root,
            "exe/test",
            "exe/test@80100000",
            kind="local",
            old_name="local_value",
            new_name="value",
            output=Path("out/tx.json"),
        )


def test_prepare_rejects_keyword_and_same_name(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    with pytest.raises(ValueError):
        si.prepare_source_transaction(
            root,
            "exe/test",
            "exe/test@80100000",
            kind="local",
            old_name="local_value",
            new_name="while",
            output=Path("out/tx.json"),
        )
    with pytest.raises(ValueError):
        si.prepare_source_transaction(
            root,
            "exe/test",
            "exe/test@80100000",
            kind="local",
            old_name="local_value",
            new_name="local_value",
            output=Path("out/tx.json"),
        )


def test_apply_verify_and_rollback_change_only_the_identifier(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    original = _source(root).read_text(encoding="utf-8")
    receipt = Path("out/tx.json")
    si.prepare_source_transaction(
        root,
        "exe/test",
        "exe/test@80100000",
        kind="local",
        old_name="local_value",
        new_name="staged_value",
        output=receipt,
    )
    prepared = json.loads((root / receipt).read_text(encoding="utf-8"))
    assert prepared["applied"] is False
    si.apply_source_transaction(root, receipt)
    applied = _source(root).read_text(encoding="utf-8")
    assert applied != original
    assert "local_value" not in applied
    assert "staged_value" in applied
    assert applied.replace("staged_value", "local_value") == original
    verified = si.verify_source_transaction(root, receipt, native=False)
    assert verified["verified"] is True
    assert verified["new_name"] == "staged_value"
    si.rollback_source_transaction(root, receipt)
    assert _source(root).read_text(encoding="utf-8") == original


def test_apply_refuses_source_changed_after_prepare(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    receipt = Path("out/tx.json")
    si.prepare_source_transaction(
        root,
        "exe/test",
        "exe/test@80100000",
        kind="local",
        old_name="local_value",
        new_name="staged_value",
        output=receipt,
    )
    _source(root).write_text(SOURCE + "\n/* drift */\n", encoding="utf-8")
    with pytest.raises(ValueError, match="changed after prepare"):
        si.apply_source_transaction(root, receipt)


def test_verify_fails_closed_when_native_gate_fails(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _repo(tmp_path)
    receipt = Path("out/tx.json")
    si.prepare_source_transaction(
        root,
        "exe/test",
        "exe/test@80100000",
        kind="local",
        old_name="local_value",
        new_name="staged_value",
        output=receipt,
    )
    si.apply_source_transaction(root, receipt)

    def _fail(root: Path, selector: str):
        raise ValueError("byte identity failed")

    monkeypatch.setattr(si, "native_byte_identity", _fail)
    with pytest.raises(ValueError, match="byte identity failed"):
        si.verify_source_transaction(root, receipt, native=True)


def test_cli_exposes_source_identifier_commands() -> None:
    from harness.naming import cli

    choices = cli.build_parser()._subparsers._group_actions[0].choices
    assert {
        "source-identifiers",
        "describe-source-identifier",
        "prepare-source-transaction",
        "apply-source-transaction",
        "verify-source-transaction",
        "rollback-source-transaction",
    } <= set(choices)


def test_receipt_records_bounded_scope(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    receipt = Path("out/tx.json")
    payload = si.prepare_source_transaction(
        root,
        "exe/test",
        "exe/test@80100000",
        kind="argument",
        old_name="cell",
        new_name="destination",
        output=receipt,
    )
    assert payload["schema"] == si.RECEIPT_SCHEMA
    assert payload["source"] == "src/test/func_80100000.c"
    assert (
        payload["pre_sha256"] == hashlib.sha256(_source(root).read_bytes()).hexdigest()
    )
    assert (root / payload["backup"]).is_file()
