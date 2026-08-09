"""Focused tests for explicit target-manifest source claims (migration Phase 1).

Covers: out-of-root claimed sources resolving by ``(target, @source address)``
and exact manifest-claimed path, duplicate path claims across targets being
rejected at load, duplicate ``(target, address)`` claims being rejected in the
registry, equal addresses in different targets staying independent, and the
build command consuming claims (not the legacy ``source_dir`` inventory) when
a target is migrated.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
from unittest.mock import patch

import pytest


from harness.analyzer import EngineIdentity, build_snapshot
from harness.commands import build as build_cmd
from harness.domain.claims import (
    collect_manifest_source_addresses,
    manifest_header_paths,
    manifest_source_paths,
    resolve_manifest_source_for_address,
)
from harness.domain.manifests import load_target_manifests
from harness.domain.sources import (
    SourceAddressCollision,
    lift_metadata,
    owning_manifest,
)
from harness.io import repo_layout

_REPO_ROOT = Path(__file__).resolve().parents[3]

_SLUS_TOML = (
    'schema = "harness.target/v2"\n'
    'id = "exe/slus_004_22"\n'
    'kind = "executable"\n'
    'source_dir = "src/exe/slus_004_22"\n'
    'binary = "out/binaries/exe/slus_004_22.bin"\n'
    'splat = "config/targets/exe/slus_004_22/splat.yaml"\n'
    "load_address = 0x80096800\n"
)


def _target(
    root: Path,
    target_id: str,
    *,
    source_dir: str | None = None,
    claims: str = "",
) -> None:
    target = root / "config" / "targets" / target_id / "target.toml"
    target.parent.mkdir(parents=True, exist_ok=True)
    text = _SLUS_TOML if target_id == "exe/slus_004_22" else (
        'schema = "harness.target/v2"\n'
        f'id = "{target_id}"\n'
        'kind = "emi"\n'
        f'source_dir = "{source_dir or f"src/{target_id}"}"\n'
        f'binary = "out/binaries/{target_id}.bin"\n'
        f'splat = "config/targets/{target_id}/splat.yaml"\n'
        "load_address = 0x80100000\n"
    )
    target.write_text(text + claims, encoding="utf-8")
    binary = root / f"out/binaries/{target_id}.bin"
    binary.parent.mkdir(parents=True, exist_ok=True)
    binary.write_bytes(b"\0" * 32)


def _write_lift(root: Path, relative: str, address: int, behavior: str = "x") -> Path:
    source = root / relative
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(
        f"/* @source 0x{address:08X} @behavior {behavior} */\n", encoding="utf-8"
    )
    return source


# -- claim enumeration: legacy fallback vs explicit claims ---------------------


def test_manifest_source_paths_legacy_inventory_when_unclaimed(
    tmp_path: Path,
) -> None:
    _target(tmp_path, "exe/slus_004_22")
    legacy = tmp_path / "src/exe/slus_004_22"
    (legacy / "initState.c").parent.mkdir(parents=True)
    (legacy / "initState.c").write_text("void initState(void) {}\n")
    (legacy / "symbols" / "psyq.c").parent.mkdir(parents=True)
    (legacy / "symbols" / "psyq.c").write_text("// generated\n")

    manifest = load_target_manifests(tmp_path)["exe/slus_004_22"]
    paths = manifest_source_paths(tmp_path, manifest)
    assert paths == sorted(
        [legacy / "initState.c", legacy / "symbols" / "psyq.c"]
    )
    assert not manifest.has_explicit_sources


def test_manifest_source_paths_spans_semantic_dirs_with_claims(
    tmp_path: Path,
) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims=(
            'sources = ["src/bof3/io/initEmiLoader.c", "src/bof3/io/emiLoaderSlotLba.c"]\n'
            'support_sources = ["src/exe/slus_004_22/symbols.c"]\n'
            'headers = ["src/exe/slus_004_22/internal.h", "src/bof3/io/emi_loader_internal.h"]\n'
        ),
    )
    for relative in (
        "src/bof3/io/initEmiLoader.c",
        "src/bof3/io/emiLoaderSlotLba.c",
        "src/exe/slus_004_22/symbols.c",
    ):
        _write_lift(tmp_path, relative, 0x80100000)
    for relative in (
        "src/exe/slus_004_22/internal.h",
        "src/bof3/io/emi_loader_internal.h",
    ):
        header = tmp_path / relative
        header.parent.mkdir(parents=True, exist_ok=True)
        header.write_text("#ifndef GUARD\n#define GUARD\n#endif\n")

    manifest = load_target_manifests(tmp_path)["exe/slus_004_22"]
    assert manifest.has_explicit_sources
    assert manifest_source_paths(tmp_path, manifest) == sorted(
        [
            tmp_path / "src/bof3/io/initEmiLoader.c",
            tmp_path / "src/bof3/io/emiLoaderSlotLba.c",
            tmp_path / "src/exe/slus_004_22/symbols.c",
        ]
    )
    assert manifest_header_paths(tmp_path, manifest) == sorted(
        [
            tmp_path / "src/exe/slus_004_22/internal.h",
            tmp_path / "src/bof3/io/emi_loader_internal.h",
        ]
    )


# -- out-of-root claimed source resolution -------------------------------------


def test_resolve_manifest_source_for_address_out_of_root(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["src/bof3/io/initEmiLoader.c"]\n',
    )
    claimed = _write_lift(tmp_path, "src/bof3/io/initEmiLoader.c", 0x80161F58)

    manifest = load_target_manifests(tmp_path)["exe/slus_004_22"]
    rows = collect_manifest_source_addresses(tmp_path, manifest)
    assert rows == [(claimed, 0x80161F58)]
    assert (
        resolve_manifest_source_for_address(tmp_path, manifest, 0x80161F58)
        == claimed
    )
    assert resolve_manifest_source_for_address(tmp_path, manifest, 0x80100000) is None


def test_owning_manifest_claim_wins_over_ancestry(tmp_path: Path) -> None:
    # The claimed lift lives inside another manifest's source_dir: the claim,
    # not path containment, must decide ownership.
    _target(tmp_path, "exe/slus_004_22")
    _target(
        tmp_path,
        "emi/etc/game/00",
        source_dir="src/bof3/io",
        claims='sources = ["src/bof3/io/gameEntry.c"]\n',
    )
    claimed = _write_lift(tmp_path, "src/bof3/io/gameEntry.c", 0x80100010)

    owner = owning_manifest(tmp_path, claimed)
    assert owner is not None
    assert owner.id.value == "emi/etc/game/00"
    assert lift_metadata(claimed)[0] == 0x80100010


# -- duplicate claims and address collisions -----------------------------------


def test_duplicate_path_claim_across_targets_rejected(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["src/bof3/io/shared.c"]\n',
    )
    _target(
        tmp_path,
        "emi/etc/game/00",
        claims='sources = ["src/bof3/io/shared.c"]\n',
    )
    with pytest.raises(ValueError, match="claimed source path .* owned by both"):
        load_target_manifests(tmp_path)


def test_invalid_path_claim_rejected(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["../escape.c"]\n',
    )
    with pytest.raises(ValueError, match="invalid sources path"):
        load_target_manifests(tmp_path)


@pytest.mark.parametrize(
    "bad",
    [
        '"src//bof3/io/x.c"',
        '"src/bof3/./io/x.c"',
        '"src/bof3/io/x.c/"',
        '"/abs/path.c"',
        "'src/bof3/io\\\\x.c'",
        '"src/bof3/io/x.c;evil.c"',
        '"src/bof3/io/x.c|y"',
    ],
)
def test_non_canonical_or_corrupting_claim_path_rejected(
    tmp_path: Path, bad: str
) -> None:
    """Alias spellings and CMake-handoff-corrupting characters are rejected."""

    _target(
        tmp_path,
        "exe/slus_004_22",
        claims=f"sources = [{bad}]\n",
    )
    with pytest.raises(ValueError, match="invalid sources path|non-canonical sources path"):
        load_target_manifests(tmp_path)


def test_header_only_claims_keep_legacy_source_inventory(tmp_path: Path) -> None:
    """Headers alone must not activate explicit source mode or drop sources."""

    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='headers = ["src/exe/slus_004_22/internal.h"]\n',
    )
    legacy = tmp_path / "src/exe/slus_004_22"
    (legacy / "initState.c").parent.mkdir(parents=True)
    (legacy / "initState.c").write_text("void initState(void) {}\n")
    (legacy / "internal.h").write_text("#ifndef GUARD\n#define GUARD\n#endif\n")

    manifest = load_target_manifests(tmp_path)["exe/slus_004_22"]
    assert not manifest.has_explicit_sources
    assert manifest_source_paths(tmp_path, manifest) == [
        legacy / "initState.c"
    ]
    assert manifest_header_paths(tmp_path, manifest) == [legacy / "internal.h"]


def test_claimed_source_missing_file_rejected(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["src/bof3/io/missing.c"]\n',
    )
    with pytest.raises(ValueError, match="claimed sources file missing for exe/slus_004_22"):
        load_target_manifests(tmp_path)


def test_claimed_path_wrong_kind_rejected(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='headers = ["src/bof3/io/not_a_header.c"]\n',
    )
    (tmp_path / "src/bof3/io/not_a_header.c").parent.mkdir(parents=True)
    (tmp_path / "src/bof3/io/not_a_header.c").write_text("int x;\n")
    with pytest.raises(ValueError, match="claimed headers path has unexpected kind"):
        load_target_manifests(tmp_path)


def test_duplicate_address_within_target_rejected(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims=(
            'sources = ["src/bof3/io/first.c", "src/bof3/io/second.c"]\n'
        ),
    )
    _write_lift(tmp_path, "src/bof3/io/first.c", 0x80161F58)
    _write_lift(tmp_path, "src/bof3/io/second.c", 0x80161F58)

    manifest = load_target_manifests(tmp_path)["exe/slus_004_22"]
    with pytest.raises(SourceAddressCollision, match="0x80161F58"):
        collect_manifest_source_addresses(tmp_path, manifest)


def test_same_address_across_targets_resolves_independently(tmp_path: Path) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["src/bof3/io/slusEntry.c"]\n',
    )
    _target(
        tmp_path,
        "emi/etc/game/00",
        claims='sources = ["src/bof3/io/emiEntry.c"]\n',
    )
    slus = _write_lift(tmp_path, "src/bof3/io/slusEntry.c", 0x80161F58)
    emi = _write_lift(tmp_path, "src/bof3/io/emiEntry.c", 0x80161F58)

    manifests = load_target_manifests(tmp_path)
    assert (
        resolve_manifest_source_for_address(
            tmp_path, manifests["exe/slus_004_22"], 0x80161F58
        )
        == slus
    )
    assert (
        resolve_manifest_source_for_address(
            tmp_path, manifests["emi/etc/game/00"], 0x80161F58
        )
        == emi
    )


# -- analyzer snapshot resolves claimed out-of-root sources --------------------


def test_analyzer_snapshot_resolves_claimed_out_of_root_source(
    tmp_path: Path,
) -> None:
    binary = tmp_path / "target.bin"
    binary.write_bytes(b"\0" * 64)
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": 0x80100000, "size": 16, "name": "func_80100000"}]
    claimed = _write_lift(tmp_path, "src/bof3/io/initState.c", 0x80100000)

    with patch("harness.analyzer._run_analysis", return_value=(functions, [])):
        snapshot = build_snapshot(
            engine,
            binary,
            0x80100000,
            "test",
            source_paths=[claimed],
        )

    assert snapshot.functions[0].is_lifted
    assert snapshot.functions[0].source == str(claimed)


def test_analyzer_snapshot_ignores_unclaimed_support_path(tmp_path: Path) -> None:
    binary = tmp_path / "target.bin"
    binary.write_bytes(b"\0" * 64)
    engine = EngineIdentity("rizin", tmp_path / "rizin", "test", {})
    functions = [{"offset": 0x80100000, "size": 16, "name": "func_80100000"}]
    support = tmp_path / "src/bof3/io/symbols.c"
    support.parent.mkdir(parents=True, exist_ok=True)
    support.write_text("WEAK_SYMBOL_AT(x, 0x80100000);\n")

    with patch("harness.analyzer._run_analysis", return_value=(functions, [])):
        snapshot = build_snapshot(
            engine,
            binary,
            0x80100000,
            "test",
            source_paths=[support],
        )

    assert not snapshot.functions[0].is_lifted
    assert snapshot.functions[0].source is None


# -- status fingerprint carries claim/source/support identity ----------------


def test_target_fingerprint_includes_claim_identity(tmp_path: Path) -> None:
    """Adding or removing a source claim changes the whole-target fingerprint."""

    from harness.match.status_cache import target_fingerprint

    _target(tmp_path, "exe/slus_004_22")
    legacy = tmp_path / "src/exe/slus_004_22"
    legacy.mkdir(parents=True)
    (legacy / "boot.c").write_text("void boot(void) {}\n")
    unclaimed = target_fingerprint(
        tmp_path, load_target_manifests(tmp_path)["exe/slus_004_22"]
    )

    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["src/bof3/io/boot.c"]\n',
    )
    _write_lift(tmp_path, "src/bof3/io/boot.c", 0x80161F58)
    claimed = target_fingerprint(
        tmp_path, load_target_manifests(tmp_path)["exe/slus_004_22"]
    )
    assert unclaimed != claimed

    _target(tmp_path, "exe/slus_004_22")
    dropped = target_fingerprint(
        tmp_path, load_target_manifests(tmp_path)["exe/slus_004_22"]
    )
    assert claimed != dropped


def test_target_fingerprint_includes_header_only_claims(tmp_path: Path) -> None:
    """Header-only claims change the fingerprint without enabling source mode."""

    from harness.match.status_cache import target_fingerprint

    _target(tmp_path, "exe/slus_004_22")
    legacy = tmp_path / "src/exe/slus_004_22"
    legacy.mkdir(parents=True)
    (legacy / "boot.c").write_text("void boot(void) {}\n")
    baseline = target_fingerprint(
        tmp_path, load_target_manifests(tmp_path)["exe/slus_004_22"]
    )

    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='headers = ["src/bof3/io/extra.h"]\n',
    )
    header = tmp_path / "src/bof3/io/extra.h"
    header.parent.mkdir(parents=True)
    header.write_text("#ifndef EXTRA\n#define EXTRA\n#endif\n")
    manifest = load_target_manifests(tmp_path)["exe/slus_004_22"]
    assert not manifest.has_explicit_sources
    assert target_fingerprint(tmp_path, manifest) != baseline


# -- CMake configure groups multiple claimed sources under the owner -----------


@pytest.mark.skipif(shutil.which("ninja") is None, reason="ninja required")
def test_cmake_groups_multiple_claimed_sources_under_target_owner(
    tmp_path: Path,
) -> None:
    """A real CMake configure parses multiline manifest_claims output and
    groups every claimed source under the target owner, never the semantic
    folder path."""

    shutil.copy(_REPO_ROOT / "CMakeLists.txt", tmp_path / "CMakeLists.txt")
    (tmp_path / "tools").mkdir()
    os.symlink(_REPO_ROOT / "tools" / "python", tmp_path / "tools" / "python")
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims=(
            'sources = ["src/bof3/io/a.c", "src/bof3/io/b.c"]\n'
            'support_sources = ["src/exe/slus_004_22/symbols.c"]\n'
            'headers = ["src/exe/slus_004_22/internal.h"]\n'
        ),
    )
    _write_lift(tmp_path, "src/bof3/io/a.c", 0x80161F58)
    _write_lift(tmp_path, "src/bof3/io/b.c", 0x80162A6C)
    legacy = tmp_path / "src/exe/slus_004_22"
    legacy.mkdir(parents=True)
    (legacy / "symbols.c").write_text("WEAK_SYMBOL_AT(x, 0x80100000);\n")
    (legacy / "internal.h").write_text(
        "#ifndef GUARD\n#define GUARD\n#endif\n"
    )

    result = subprocess.run(
        ["cmake", "-S", str(tmp_path), "-B", str(tmp_path / "build/cmake"),
         "-G", "Ninja"],
        cwd=tmp_path,
        text=True,
        capture_output=True,
    )
    assert result.returncode == 0, result.stdout + result.stderr

    ninja = (tmp_path / "build/cmake/build.ninja").read_text()
    grouped: dict[str, set[str]] = {}
    for line in ninja.splitlines():
        match = re.match(r"build (target_[0-9a-f]+): phony (.+)", line)
        if match:
            grouped[match.group(1)] = set(match.group(2).split())
    owner_hash = hashlib.sha1(b"src/exe/slus_004_22").hexdigest()[:16]
    deps = grouped[f"target_{owner_hash}"]
    for object_path in (
        "build/src/bof3/io/a.o",
        "build/src/bof3/io/b.o",
        "build/src/exe/slus_004_22/symbols.o",
    ):
        assert any(dep.endswith(object_path) for dep in deps), (
            f"{object_path} missing from {deps}"
        )
    # The semantic folder must not own the claimed lifts.
    assert f"target_{hashlib.sha1(b'src/bof3/io').hexdigest()[:16]}" not in grouped


@pytest.mark.skipif(shutil.which("ninja") is None, reason="ninja required")
def test_cmake_migrated_target_excludes_unclaimed_legacy_files(
    tmp_path: Path,
) -> None:
    """A migrated manifest's build target groups exactly its claimed
    translation units: an unclaimed file under its legacy source_dir must not
    join the target, even though the global `lifts` target still compiles
    it."""

    shutil.copy(_REPO_ROOT / "CMakeLists.txt", tmp_path / "CMakeLists.txt")
    (tmp_path / "tools").mkdir()
    os.symlink(_REPO_ROOT / "tools" / "python", tmp_path / "tools" / "python")
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims=(
            'sources = ["src/bof3/io/a.c"]\n'
            'support_sources = ["src/exe/slus_004_22/symbols.c"]\n'
        ),
    )
    _write_lift(tmp_path, "src/bof3/io/a.c", 0x80161F58)
    legacy = tmp_path / "src/exe/slus_004_22"
    legacy.mkdir(parents=True)
    (legacy / "symbols.c").write_text("WEAK_SYMBOL_AT(x, 0x80100000);\n")
    # Unclaimed legacy lift: compiled by `lifts`, never owned by the target.
    (legacy / "unclaimedLegacy.c").write_text("void unclaimedLegacy(void) {}\n")

    result = subprocess.run(
        ["cmake", "-S", str(tmp_path), "-B", str(tmp_path / "build/cmake"),
         "-G", "Ninja"],
        cwd=tmp_path,
        text=True,
        capture_output=True,
    )
    assert result.returncode == 0, result.stdout + result.stderr

    ninja = (tmp_path / "build/cmake/build.ninja").read_text()
    grouped: dict[str, set[str]] = {}
    for line in ninja.splitlines():
        match = re.match(r"build (target_[0-9a-f]+): phony (.+)", line)
        if match:
            grouped[match.group(1)] = set(match.group(2).split())
    owner_hash = hashlib.sha1(b"src/exe/slus_004_22").hexdigest()[:16]
    deps = grouped[f"target_{owner_hash}"]
    assert any(dep.endswith("build/src/bof3/io/a.o") for dep in deps)
    assert any(
        dep.endswith("build/src/exe/slus_004_22/symbols.o") for dep in deps
    )
    assert not any(dep.endswith("unclaimedLegacy.o") for dep in deps), deps


def test_claimed_path_escaping_repo_via_symlink_rejected(tmp_path: Path) -> None:
    """A claim whose file resolves outside the repository is rejected."""

    outside = Path(tempfile.mkdtemp())
    (outside / "escape.c").write_text("void escape(void) {}\n")
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["src/bof3/io/escape.c"]\n',
    )
    link = tmp_path / "src/bof3/io/escape.c"
    link.parent.mkdir(parents=True)
    link.symlink_to(outside / "escape.c")

    with pytest.raises(ValueError, match="escapes repository"):
        load_target_manifests(tmp_path)


def test_sources_support_sources_overlap_rejected(tmp_path: Path) -> None:
    """One path claimed as both source and support source is rejected."""

    _target(
        tmp_path,
        "exe/slus_004_22",
        claims=(
            'sources = ["src/bof3/io/symbols.c"]\n'
            'support_sources = ["src/bof3/io/symbols.c"]\n'
        ),
    )
    _write_lift(tmp_path, "src/bof3/io/symbols.c", 0x80161F58)

    with pytest.raises(ValueError, match="both sources and support_sources"):
        load_target_manifests(tmp_path)


# -- build command consumes claims, not source_dir inventory -------------------


def test_build_migrated_target_uses_claims_outside_source_dir(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(
        tmp_path,
        "exe/slus_004_22",
        claims='sources = ["src/bof3/io/initEmiLoader.c"]\n',
    )
    _write_lift(tmp_path, "src/bof3/io/initEmiLoader.c", 0x80161F58)
    # The legacy source_dir is empty: only the out-of-root claim owns inputs.
    assert not list((tmp_path / "src/exe/slus_004_22").rglob("*.c"))

    monkeypatch.setattr(build_cmd, "repo_layout", lambda: repo_layout(tmp_path))
    recorded: dict[str, object] = {}

    def fake_cmake_target(directory: str) -> str:
        recorded["dir"] = directory
        return "target_x"

    monkeypatch.setattr(build_cmd, "cmake_target_for_directory", fake_cmake_target)

    def fake_build(root: Path, target: str) -> subprocess.CompletedProcess[str]:
        recorded["built"] = (root, target)
        return subprocess.CompletedProcess([], 0, "", "")

    monkeypatch.setattr(build_cmd, "build", fake_build)
    rc = build_cmd.run(build_cmd.build_parser().parse_args(["exe/slus_004_22"]))
    assert rc == 0
    assert recorded["dir"] == "src/exe/slus_004_22"
    assert recorded["built"] == (tmp_path, "target_x")
