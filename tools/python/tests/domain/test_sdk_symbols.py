from __future__ import annotations

from pathlib import Path

import pytest

from harness.domain.sdk_map import (
    SdkSymbol,
    load_sdk_map,
    project_symbols,
    sdk_map_path,
)
from harness.domain.symbols import (
    MapSymbol,
    load_target_symbols,
)
from harness.commands.symbols import main as symbols_main
from harness.domain.manifests import load_target_manifests
from harness.io import repo_layout


def _write_target(
    root: Path,
    target: str,
    *,
    space: str | None = None,
    load_address: int = 0x801CE000,
    support_sources: tuple[str, ...] = (),
    psyq_source: str | None = None,
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
    if support_sources:
        lines.append(
            "support_sources = ["
            + ", ".join(f'"{source}"' for source in support_sources)
            + "]"
        )
    if psyq_source is not None:
        lines.append(f'psyq_source = "{psyq_source}"')
    if space is not None:
        lines += ["[psyq]", f'space = "{space}"']
    manifest.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _write_sdk(root: Path, *lines: str) -> None:
    sdk = root / "config" / "sdk"
    sdk.mkdir(parents=True, exist_ok=True)
    (sdk / "psyq.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")


def test_sdk_map_is_one_combined_file() -> None:
    assert sdk_map_path(Path("/r")) == Path("/r/config/sdk/psyq.txt")


def test_psyq_space_defaults_to_slus(tmp_path: Path) -> None:
    _write_target(tmp_path, "emi/test/00")
    assert load_target_manifests(tmp_path)["emi/test/00"].psyq_space == "slus"


def test_psyq_space_rejects_unknown(tmp_path: Path) -> None:
    _write_target(tmp_path, "exe/logo", space="bogus")
    with pytest.raises(ValueError, match="unsupported psyq space"):
        load_target_manifests(tmp_path)


def test_load_target_symbols_composes_sdk_for_the_image(tmp_path: Path) -> None:
    _write_target(tmp_path, "exe/logo", space="logo")
    _write_sdk(
        tmp_path,
        "PadInit = exe/logo@0x801CEE7C, exe/slus_004_22@0x80174668",
    )

    symbols = load_target_symbols(tmp_path, "exe/logo")

    assert MapSymbol(0x801CEE7C, "PadInit") in symbols
    assert MapSymbol(0x80174668, "PadInit") not in symbols


def test_project_symbols_prefers_a_target_local_copy() -> None:
    symbols = [
        SdkSymbol("rand", "exe/slus_004_22", 0x8017E3D4),
        SdkSymbol("rand", "emi/bmagic/magic216/00", 0x801EF03C),
    ]

    projected = project_symbols(symbols, target="emi/bmagic/magic216/00", space="slus")
    resident = project_symbols(symbols, target="emi/battle/battle/15", space="slus")

    assert MapSymbol(0x801EF03C, "rand") in projected
    assert MapSymbol(0x8017E3D4, "rand") not in projected
    assert MapSymbol(0x8017E3D4, "rand") in resident


def test_psyq_bindings_generator_writes_sdk_bindings(tmp_path: Path) -> None:
    _write_target(
        tmp_path,
        "exe/logo",
        space="logo",
        support_sources=("src/bof3/support/logo_psyq.c",),
        psyq_source="src/bof3/support/logo_psyq.c",
    )
    _write_sdk(tmp_path, "PadInit = exe/logo@0x801CEE7C")
    support = tmp_path / "src/bof3/support/logo_psyq.c"
    support.parent.mkdir(parents=True)
    support.write_text("", encoding="utf-8")

    assert (
        symbols_main(["--root", str(tmp_path), "psyq-bindings", "exe/logo", "--write"])
        == 0
    )

    text = support.read_text(encoding="utf-8")
    assert "WEAK_SYMBOL_AT(PadInit, 0x801CEE7C);" in text


def test_psyq_bindings_without_psyq_source_errors(tmp_path: Path) -> None:
    """A target must declare psyq_source; the output path is never guessed."""
    _write_target(tmp_path, "emi/test/00")
    support = tmp_path / "src/bof3/support/test_psyq.c"
    support.parent.mkdir(parents=True)
    support.write_text("", encoding="utf-8")
    manifest = tmp_path / "config/targets/emi/test/00/target.toml"
    manifest.write_text(
        manifest.read_text(encoding="utf-8")
        + 'support_sources = ["src/bof3/support/test_psyq.c"]\n',
        encoding="utf-8",
    )
    _write_sdk(tmp_path, "PadInit = exe/slus_004_22@0x80174668")

    assert (
        symbols_main(
            ["--root", str(tmp_path), "psyq-bindings", "emi/test/00", "--write"]
        )
        == 2
    )


def test_psyq_bindings_generator_writes_claimed_support_path(tmp_path: Path) -> None:
    """psyq_source names the exact generated output; no stem guessing."""
    _write_target(tmp_path, "emi/test/00")
    support = tmp_path / "src/bof3/support/test_psyq.c"
    support.parent.mkdir(parents=True)
    support.write_text("", encoding="utf-8")
    manifest = tmp_path / "config/targets/emi/test/00/target.toml"
    manifest.write_text(
        manifest.read_text(encoding="utf-8")
        + 'support_sources = ["src/bof3/support/test_psyq.c"]\n'
        + 'psyq_source = "src/bof3/support/test_psyq.c"\n',
        encoding="utf-8",
    )
    _write_sdk(tmp_path, "PadInit = exe/slus_004_22@0x80174668")

    assert (
        symbols_main(
            ["--root", str(tmp_path), "psyq-bindings", "emi/test/00", "--write"]
        )
        == 0
    )
    assert "WEAK_SYMBOL_AT(PadInit, 0x80174668);" in support.read_text(encoding="utf-8")


def _migrated_binding_target(root: Path, binding_text: str) -> None:
    """One migrated target claiming a relocated hand-maintained binding file."""
    manifest = root / "config/targets/emi/test/00/target.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        'schema = "harness.target/v2"\n'
        'id = "emi/test/00"\n'
        'kind = "emi"\n'
        'source_dir = "src/emi/test/00"\n'
        'binary = "out/binaries/emi/test/00.bin"\n'
        'splat = "config/targets/emi/test/00/splat.yaml"\n'
        "load_address = 0x801EEC00\n"
        'sources = ["src/bof3/ui/selectUiMode14.c"]\n'
        'support_sources = ["src/bof3/support/test_symbols.c"]\n',
        encoding="utf-8",
    )
    source = root / "src/bof3/ui/selectUiMode14.c"
    source.parent.mkdir(parents=True)
    source.write_text("/* @source 0x801F0EC8 @behavior x */\n", encoding="utf-8")
    binding = root / "src/bof3/support/test_symbols.c"
    binding.parent.mkdir(parents=True)
    binding.write_text(binding_text, encoding="utf-8")
    (root / "config/targets/emi/test/00/symbols.txt").write_text(
        "foo = 0x801448EB;\nfunc_801F0EC8 = 0x801F0EC8;\n",
        encoding="utf-8",
    )
    sdk = root / "config/sdk"
    sdk.mkdir(parents=True)
    (sdk / "psyq.txt").write_text(
        "PadInit = exe/slus_004_22@0x80174668\n", encoding="utf-8"
    )


def test_symbols_check_validates_relocated_binding_file(tmp_path: Path) -> None:
    """symbols check scans claimed support binding .c files outside
    ``source_dir`` and accepts bindings the composed map owns."""
    _migrated_binding_target(tmp_path, "WEAK_SYMBOL_AT(foo, 0x801448eb);\n")
    assert symbols_main(["--root", str(tmp_path), "check", "emi/test/00"]) == 0


def test_symbols_check_flags_bad_relocated_binding(tmp_path: Path) -> None:
    """A WEAK_SYMBOL_AT address no composed map owns is drift, even from a
    relocated claimed support file."""
    _migrated_binding_target(
        tmp_path,
        "WEAK_SYMBOL_AT(foo, 0x801448eb);\nWEAK_SYMBOL_AT(ghost, 0xDEADBEEF);\n",
    )
    assert symbols_main(["--root", str(tmp_path), "check", "emi/test/00"]) == 2


def test_psyq_source_must_be_explicitly_claimed(tmp_path: Path) -> None:
    _write_target(tmp_path, "emi/test/00")
    source = tmp_path / "src/bof3/ui/lift.c"
    helper = tmp_path / "src/bof3/support/helper.c"
    source.parent.mkdir(parents=True)
    helper.parent.mkdir(parents=True)
    source.write_text("/* @source 0x801EEC00 @behavior test */\n", encoding="utf-8")
    helper.write_text("/* test */\n", encoding="utf-8")
    manifest = tmp_path / "config/targets/emi/test/00/target.toml"
    manifest.write_text(
        manifest.read_text(encoding="utf-8")
        + 'sources = ["src/bof3/ui/lift.c"]\n'
        + 'support_sources = ["src/bof3/support/helper.c"]\n'
        + 'psyq_source = "src/emi/test/00/symbols/psyq.c"\n',
        encoding="utf-8",
    )

    with pytest.raises(ValueError, match="psyq_source must be explicitly claimed"):
        load_target_manifests(tmp_path)


def test_psyq_source_must_be_canonical_path(tmp_path: Path) -> None:
    _write_target(tmp_path, "emi/test/00")
    manifest = tmp_path / "config/targets/emi/test/00/target.toml"
    manifest.write_text(
        manifest.read_text(encoding="utf-8") + 'psyq_source = "/abs/psyq.c"\n',
        encoding="utf-8",
    )
    with pytest.raises(ValueError, match="invalid psyq_source path"):
        load_target_manifests(tmp_path)


def test_real_sdk_map_lists_each_image() -> None:
    root = repo_layout().root
    symbols = load_sdk_map(sdk_map_path(root))
    logo_base = 0x801CE000

    slus = [symbol for symbol in symbols if symbol.target == "exe/slus_004_22"]
    logo = [symbol for symbol in symbols if symbol.target == "exe/logo"]

    assert slus and logo
    assert all(symbol.address < logo_base for symbol in slus)
    assert all(symbol.address >= logo_base for symbol in logo)


def test_real_targets_compose_their_own_space() -> None:
    root = repo_layout().root

    slus_symbols = load_target_symbols(root, "exe/slus_004_22")
    logo_symbols = load_target_symbols(root, "exe/logo")

    assert MapSymbol(0x8017E3D4, "rand") in slus_symbols
    assert MapSymbol(0x801CEE7C, "PadInit") in logo_symbols
    assert MapSymbol(0x80174668, "PadInit") not in logo_symbols
