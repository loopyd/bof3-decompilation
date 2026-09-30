from __future__ import annotations

from pathlib import Path
import os

import pytest

from harness.domain.manifests import TargetManifest, load_target_manifests
from harness.domain.registry import lookup_target_manifest


def _write_manifest(root: Path, target_id: str, kind: str = "emi") -> None:
    config = root / "config" / "targets" / target_id / "target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema = 'harness.target/v2'\n"
        f"id = '{target_id}'\n"
        f"kind = '{kind}'\n"
        "source_dir = 'src/'\n"
        "binary = 'out/binaries/missing.bin'\n"
        "splat = 'config/targets/splat.yaml'\n",
        encoding="utf-8",
    )


def test_lookup_resolves_canonical_executable_without_binary(tmp_path: Path) -> None:
    _write_manifest(tmp_path, "exe/slus_004_22", kind="executable")

    manifest = lookup_target_manifest(tmp_path, "exe/slus_004_22")

    assert manifest is not None
    assert isinstance(manifest, TargetManifest)
    assert manifest.id.value == "exe/slus_004_22"
    assert manifest.kind == "executable"
    # No target binary was created: the lookup must not require one.
    assert not (tmp_path / manifest.binary).exists()


def test_lookup_shipped_executable_selector_yields_canonical_identity(
    tmp_path: Path,
) -> None:
    _write_manifest(tmp_path, "exe/slus_004_22", kind="executable")

    manifest = lookup_target_manifest(tmp_path, "SLUS_004.22")

    assert manifest is not None
    assert manifest.id.value == "exe/slus_004_22"


def test_lookup_shipped_emi_selector_yields_canonical_identity(
    tmp_path: Path,
) -> None:
    _write_manifest(tmp_path, "emi/battle/battle/15")

    manifest = lookup_target_manifest(tmp_path, "BIN/BATTLE/BATTLE.EMI#15")

    assert manifest is not None
    assert manifest.id.value == "emi/battle/battle/15"
    assert lookup_target_manifest(tmp_path, "emi/battle/battle/15") == manifest


def test_lookup_returns_existing_frozen_manifest_type(tmp_path: Path) -> None:
    _write_manifest(tmp_path, "emi/etc/game/00")

    manifests = load_target_manifests(tmp_path)

    # The lookup returns the typed manifest itself, never a wrapper bundle.
    assert (
        lookup_target_manifest(tmp_path, "EMI/ETC/GAME/00")
        == manifests["emi/etc/game/00"]
    )


def test_lookup_valid_unknown_target_returns_none(tmp_path: Path) -> None:
    _write_manifest(tmp_path, "emi/etc/game/00")

    assert lookup_target_manifest(tmp_path, "exe/not_a_real_target") is None
    assert lookup_target_manifest(tmp_path, "BIN/NOPE/NOPE.EMI#0") is None


def test_manifest_cache_detects_same_size_edit_with_restored_mtime(
    tmp_path: Path,
) -> None:
    _write_manifest(tmp_path, "emi/etc/game/00")
    path = tmp_path / "config/targets/emi/etc/game/00/target.toml"
    path.write_text(path.read_text() + "disc_id = 'X'\n")
    before = load_target_manifests(tmp_path)
    content = path.read_bytes()
    stat = path.stat()
    path.write_bytes(content.replace(b"disc_id = 'X'", b"disc_id = 'Y'"))
    os.utime(path, ns=(stat.st_atime_ns, stat.st_mtime_ns))

    after = load_target_manifests(tmp_path)

    assert before["emi/etc/game/00"].disc_id == "X"
    assert after["emi/etc/game/00"].disc_id == "Y"


def test_manifest_claim_parents_are_rechecked_and_symlink_escape_rejected(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    target = "emi/etc/game/00"
    _write_manifest(tmp_path, target)
    manifest_path = tmp_path / "config/targets" / target / "target.toml"
    manifest_path.write_text(
        manifest_path.read_text(encoding="utf-8") + "sources = ['src/test/a.c']\n",
        encoding="utf-8",
    )
    source = tmp_path / "src/test/a.c"
    source.parent.mkdir(parents=True)
    source.write_text("/* @source 0x80100000 */\n", encoding="utf-8")
    original = Path.resolve
    calls = 0

    def counted(path: Path, strict: bool = False) -> Path:
        nonlocal calls
        if path == source.parent:
            calls += 1
        return original(path, strict=strict)

    monkeypatch.setattr(Path, "resolve", counted)
    load_target_manifests(tmp_path)
    assert calls == 2

    outside = tmp_path.parent / f"{tmp_path.name}-outside.c"
    outside.write_text("outside\n", encoding="utf-8")
    source.unlink()
    source.symlink_to(outside)
    with pytest.raises(ValueError, match="escapes repository"):
        load_target_manifests(tmp_path)


def test_lookup_malformed_selector_keeps_value_error(tmp_path: Path) -> None:
    with pytest.raises(ValueError, match="must not be empty"):
        lookup_target_manifest(tmp_path, "")
    with pytest.raises(ValueError, match="archive slot"):
        lookup_target_manifest(tmp_path, "BIN/BATTLE/BATTLE.EMI")


def test_manifest_cache_revalidates_claims_and_isolates_nested_maps(
    tmp_path: Path,
) -> None:
    target = "emi/etc/game/00"
    _write_manifest(tmp_path, target)
    manifest_path = tmp_path / "config/targets" / target / "target.toml"
    manifest_path.write_text(
        manifest_path.read_text(encoding="utf-8")
        + "sources = ['src/test/a.c']\n"
        + "[psyq.libraries.lib]\n"
        + "members = ['a']\n",
        encoding="utf-8",
    )
    source = tmp_path / "src/test/a.c"
    source.parent.mkdir(parents=True)
    source.write_text("/* @source 0x80100000 */\n", encoding="utf-8")

    first = load_target_manifests(tmp_path)
    first[target].libraries["poison"] = ("bad",)
    second = load_target_manifests(tmp_path)
    assert "poison" not in second[target].libraries

    source.unlink()
    with pytest.raises(ValueError, match="claimed sources file missing"):
        load_target_manifests(tmp_path)
