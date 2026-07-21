from __future__ import annotations

from pathlib import Path

import pytest

from harness.canonical import Symbol, load_map, load_target_symbols, sdk_map_path
from harness.commands.symbols import main as symbols_main
from harness.domain import load_target_manifests
from harness.io import repo_layout


def _write_target(
    root: Path, target: str, *, space: str | None = None, load_address: int = 0x801CE000
) -> None:
    manifest = root / "config" / "targets" / target / "target.toml"
    manifest.parent.mkdir(parents=True)
    lines = [
        'schema = "harness.target/v2"',
        f'id = "{target}"',
        'kind = "executable"',
        f'source_dir = "src/{target}"',
        f'binary = "out/binaries/{target}.bin"',
        f'splat = "config/targets/{target}/splat.yaml"',
        f"load_address = 0x{load_address:08X}",
    ]
    if space is not None:
        lines += ["[psyq]", f'space = "{space}"']
    manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _write_sdk(root: Path, space: str, text: str) -> None:
    sdk = root / "config" / "sdk"
    sdk.mkdir(parents=True, exist_ok=True)
    (sdk / f"psyq-{space}.txt").write_text(text, encoding="utf-8")


def test_sdk_map_path_is_space_qualified() -> None:
    assert sdk_map_path(Path("/r"), "slus") == Path("/r/config/sdk/psyq-slus.txt")
    assert sdk_map_path(Path("/r"), "logo") == Path("/r/config/sdk/psyq-logo.txt")


def test_psyq_space_defaults_to_slus(tmp_path: Path) -> None:
    _write_target(tmp_path, "emi/test/00")
    assert load_target_manifests(tmp_path)["emi/test/00"].psyq_space == "slus"


def test_psyq_space_rejects_unknown(tmp_path: Path) -> None:
    _write_target(tmp_path, "exe/logo", space="bogus")
    with pytest.raises(ValueError, match="unsupported psyq space"):
        load_target_manifests(tmp_path)


def test_load_target_symbols_composes_sdk_by_space(tmp_path: Path) -> None:
    _write_target(tmp_path, "exe/logo", space="logo")
    _write_sdk(tmp_path, "logo", "PadInit = 0x801CEE7C;\n")
    _write_sdk(tmp_path, "slus", "PadInit = 0x80174668;\n")

    symbols = load_target_symbols(tmp_path, "exe/logo")

    assert Symbol(0x801CEE7C, "PadInit") in symbols
    assert Symbol(0x80174668, "PadInit") not in symbols


def test_psyq_bindings_generator_writes_sdk_bindings(tmp_path: Path) -> None:
    _write_target(tmp_path, "exe/logo", space="logo")
    _write_sdk(tmp_path, "logo", "PadInit = 0x801CEE7C;\n")

    assert (
        symbols_main(["--root", str(tmp_path), "psyq-bindings", "exe/logo", "--write"])
        == 0
    )

    text = (tmp_path / "src/exe/logo/symbols/psyq.c").read_text(encoding="utf-8")
    assert "WEAK_SYMBOL_AT(PadInit, 0x801CEE7C);" in text


def test_real_sdk_maps_are_space_consistent() -> None:
    root = repo_layout().root
    slus = load_map(sdk_map_path(root, "slus"))
    logo = load_map(sdk_map_path(root, "logo"))
    logo_base = 0x801CE000

    assert slus and logo
    assert all(symbol.address < logo_base for symbol in slus)
    assert all(symbol.address >= logo_base for symbol in logo)


def test_real_targets_compose_their_own_space() -> None:
    root = repo_layout().root

    slus_symbols = load_target_symbols(root, "exe/slus_004_22")
    logo_symbols = load_target_symbols(root, "exe/logo")

    assert Symbol(0x8017E3D4, "rand") in slus_symbols
    assert Symbol(0x801CEE7C, "PadInit") in logo_symbols
    assert Symbol(0x80174668, "PadInit") not in logo_symbols
