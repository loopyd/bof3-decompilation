"""Read-only argument and local-variable naming discovery."""

from __future__ import annotations

from pathlib import Path

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


def test_discovery_reports_identifier_uses(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    rows = si.collect_source_identifiers(root, "exe/test")
    row = next(row for row in rows if row["id"] == "exe/test@80100000@argument:value")
    assert row["kind"] == "argument"
    assert row["uses"] == 2


def test_cli_exposes_only_list_and_check() -> None:
    from harness.naming import cli

    choices = cli.build_parser()._subparsers._group_actions[0].choices
    assert set(choices) == {"list", "check"}
