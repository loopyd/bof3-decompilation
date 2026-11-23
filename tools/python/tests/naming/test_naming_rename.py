"""Rename transaction: the dry-run plan and its refusals."""

from __future__ import annotations

import textwrap
from pathlib import Path

from harness.naming import rename


OLD = "raiseFlag4AndInvokeHelperX_80100000"


def _target(
    root: Path, target: str, sources: list[str], rows: tuple[str, ...] = (OLD, "otherExisting")
) -> None:
    directory = root / "config" / "targets" / target
    directory.mkdir(parents=True)
    claims = "".join(f'    "{source}",\n' for source in sources)
    (directory / "target.toml").write_text(
        textwrap.dedent(
            f"""\
            schema = "harness.target/v2"
            id = "{target}"
            disc_id = "SLUS_004.22"
            kind = "executable"
            source_dir = "src/bof3/battle"
            binary = "out/binaries/{target}.bin"
            splat = "config/targets/{target}/splat.yaml"
            load_address = 0x80010000
            sources = [
            {claims}]
            """
        ),
        encoding="utf-8",
    )
    (directory / "symbols.txt").write_text(
        "".join(f"{name} = 0x{0x80100000 + 4 * index:08X};\n" for index, name in enumerate(rows)),
        encoding="utf-8",
    )
    (directory / "splat.yaml").write_text(
        f"- [0x80100000, c, {OLD}]\n    - '@source: src/bof3/battle/{OLD}.c'\n",
        encoding="utf-8",
    )


def _repo(tmp_path: Path) -> Path:
    root = tmp_path
    _target(root, "exe/game", [f"src/bof3/battle/{OLD}.c"])
    source = root / "src" / "bof3" / "battle" / f"{OLD}.c"
    source.parent.mkdir(parents=True)
    source.write_text(f"void {OLD}(void) {{}}\n", encoding="utf-8")
    return root


def _plan(root: Path, new: str) -> dict:
    return rename.plan_report(root, "exe/game", OLD, new)


def test_dry_run_names_every_surface_and_writes_nothing(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    before = (root / "src" / "bof3" / "battle" / f"{OLD}.c").read_text()
    payload = _plan(root, "stateRaiseFlag4")
    assert payload["passed"] is True, payload["refusals"]
    assert {row["file"] for row in payload["edits"]} == {
        f"src/bof3/battle/{OLD}.c",
        "config/targets/exe/game/splat.yaml",
        "config/targets/exe/game/symbols.txt",
        "config/targets/exe/game/target.toml",
    }
    assert payload["renames"] == [
        {
            "from": f"src/bof3/battle/{OLD}.c",
            "to": "src/bof3/battle/stateRaiseFlag4.c",
        }
    ]
    assert (root / "src" / "bof3" / "battle" / f"{OLD}.c").read_text() == before


def test_schema_violation_refuses_the_name(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    assert [row["code"] for row in _plan(root, "BadName")["refusals"]] == ["schema"]
    assert [row["code"] for row in _plan(root, "aBcDeFgHi")["refusals"]] == ["schema"]


def test_existing_map_row_collides(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    assert [row["code"] for row in _plan(root, "otherExisting")["refusals"]] == [
        "collision"
    ]


def test_unknown_symbol_is_refused(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    payload = rename.plan_report(root, "exe/game", "absentRow", "stateRaiseFlag4")
    assert [row["code"] for row in payload["refusals"]] == ["symbol"]


def test_equal_spellings_are_refused(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    assert [row["code"] for row in _plan(root, OLD)["refusals"]] == ["no-op"]


def test_shared_map_claim_refuses_the_rename(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    shared = root / "config" / "targets" / "shared"
    shared.mkdir(parents=True)
    (shared / "symbols.txt").write_text(f"{OLD} = 0x80100000;\n", encoding="utf-8")
    assert [row["code"] for row in _plan(root, "stateRaiseFlag4")["refusals"]] == [
        "shared"
    ]


def test_foreign_consumer_refuses_the_rename(tmp_path: Path) -> None:
    root = _repo(tmp_path)
    _target(root, "exe/other", ["src/bof3/battle/other.c"], rows=("otherTargetRow",))
    other = root / "src" / "bof3" / "battle" / "other.c"
    other.write_text(
        f"extern void {OLD}(void);\nvoid otherConsumer(void) {{ {OLD}(); }}\n",
        encoding="utf-8",
    )
    codes = [row["code"] for row in _plan(root, "stateRaiseFlag4")["refusals"]]
    assert codes and set(codes) == {"fan-out"}
