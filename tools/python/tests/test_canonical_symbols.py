import subprocess
import pytest

from harness.commands.symbols import main as symbols_main
from harness.match._asm_link import (
    _target_map_bindings,
    link_object_at_address,
    resolve_symbol_address,
)
def _write_check_target(root: Path, target: str, *, kind: str = "executable") -> None:
    manifest = root / "config" / "targets" / target / "target.toml"
    manifest.parent.mkdir(parents=True)
    lines = [
        'schema = "harness.target/v2"',
        f'id = "{target}"',
        f'kind = "{kind}"',
        f'source_dir = "src/{target}"',
        f'binary = "out/binaries/{target}.bin"',
        f'splat = "config/targets/{target}/splat.yaml"',
        "load_address = 0x801CE000",
    ]
    manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")


def test_symbols_check_target_scope(tmp_path: Path, capsys) -> None:
    """`symbols check [TARGET]` selects one target; no-operand checks all."""
    # --- shared and SDK maps (empty but exist) ---
    (tmp_path / "config" / "targets" / "shared").mkdir(parents=True)
    (tmp_path / "config" / "targets" / "shared" / "symbols.txt").write_text(
        "", encoding="utf-8"
    )
    (tmp_path / "config" / "sdk").mkdir(parents=True)
    (tmp_path / "config" / "sdk" / "psyq-slus.txt").write_text("", encoding="utf-8")

    # --- Target 1: exe/keep (clean) ---
    _write_check_target(tmp_path, "exe/keep")
    map1 = tmp_path / "config" / "targets" / "exe" / "keep" / "symbols.txt"
    map1.write_text("func_80100000 = 0x80100000;\n", encoding="utf-8")
    src1 = tmp_path / "src" / "exe" / "keep"
    src1.mkdir(parents=True)
    (src1 / "func_80100000.c").write_text("// stub\n", encoding="utf-8")

    # --- Target 2: emi/battle/keep/15 (source/map drift) ---
    _write_check_target(tmp_path, "emi/battle/keep/15", kind="emi")
    map2 = (
        tmp_path
        / "config"
        / "targets"
        / "emi"
        / "battle"
        / "keep"
        / "15"
        / "symbols.txt"
    )
    map2.write_text("func_80200004 = 0x80200004;\n", encoding="utf-8")
    src2 = tmp_path / "src" / "emi" / "battle" / "keep" / "15"
    src2.mkdir(parents=True)
    # func_80200000.c exists but map only has func_80200004 → drift
    (src2 / "func_80200000.c").write_text("// stub\n", encoding="utf-8")

    # --- 1: selected clean target ---
    code1 = symbols_main(["--root", str(tmp_path), "check", "exe/keep"])
    captured1 = capsys.readouterr()
    assert code1 == 0, f"expected 0 for clean target, got {code1}: {captured1.err}"
    assert "func_80200000" not in captured1.out + captured1.err, (
        "selected check must not mention unrelated target"
    )

    # --- 2: shipped spelling resolves to EMI target ---
    code2 = symbols_main(["--root", str(tmp_path), "check", "battle/keep.emi#15"])
    captured2 = capsys.readouterr()
    assert code2 == 2, f"expected 2 for drifted EMI, got {code2}: {captured2.err}"
    assert "source/map drift" in captured2.err, (
        f"expected source/map drift in output: {captured2.err}"
    )

    # --- 3: unknown target retains the standard error/exit contract ---
    code3 = symbols_main(["--root", str(tmp_path), "check", "exe/no_such_target"])
    captured3 = capsys.readouterr()
    assert code3 == 2
    assert "unknown target: exe/no_such_target" in captured3.err

    # --- 4: no operand checks all targets ---
    code3 = symbols_main(["--root", str(tmp_path), "check"])
    captured3 = capsys.readouterr()
    assert code3 == 2, f"expected 2 for all-target check, got {code3}: {captured3.err}"
    assert "exe/keep" not in captured3.err, "exe/keep should have no errors"
    assert "emi/battle/keep/15" in captured3.err, (
        "full check should name the failing target"
    )




def test_link_uses_supplied_bindings_without_map_reload(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(
        "harness.match._asm_link._target_map_bindings",
        lambda *_: (_ for _ in ()).throw(AssertionError("fallback map loaded")),
    )
    commands: list[list[str]] = []
    monkeypatch.setattr(
        subprocess,
        "run",
        lambda command, **_: (
            commands.append(command) or subprocess.CompletedProcess(command, 0, "", "")
        ),
    )
    object_path = tmp_path / "test.o"
    object_path.touch()
    link_object_at_address(
        object_path=object_path,
        address=0x801CE000,
        undefined_symbols=["PadRead"],
        layout=SimpleNamespace(psn00b_toolchain_root=tmp_path, root=tmp_path),
    )
    assert f"--defsym=PadRead={0x801CE760}" in commands[0]
