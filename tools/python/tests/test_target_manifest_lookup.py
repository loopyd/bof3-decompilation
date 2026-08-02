from harness.domain import TargetManifest, load_target_manifests, lookup_target_manifest


def _write_manifest(root: Path, target_id: str, kind: str = "emi") -> None:
    config = root / "config" / "targets" / target_id / "target.toml"
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
    assert (
        lookup_target_manifest(tmp_path, "emi/battle/battle/15") == manifest
    )


def test_lookup_returns_existing_frozen_manifest_type(tmp_path: Path) -> None:
    _write_manifest(tmp_path, "emi/etc/game/00")

    manifests = load_target_manifests(tmp_path)

    # The lookup returns the typed manifest itself, never a wrapper bundle.
    assert lookup_target_manifest(tmp_path, "EMI/ETC/GAME/00") == manifests[
        "emi/etc/game/00"
    ]


def test_lookup_valid_unknown_target_returns_none(tmp_path: Path) -> None:
    _write_manifest(tmp_path, "emi/etc/game/00")

    assert lookup_target_manifest(tmp_path, "exe/not_a_real_target") is None
    assert lookup_target_manifest(tmp_path, "BIN/NOPE/NOPE.EMI#0") is None


def test_lookup_malformed_selector_keeps_value_error(tmp_path: Path) -> None:
    with pytest.raises(ValueError, match="must not be empty"):
        lookup_target_manifest(tmp_path, "")
    with pytest.raises(ValueError, match="archive slot"):
        lookup_target_manifest(tmp_path, "BIN/BATTLE/BATTLE.EMI")
