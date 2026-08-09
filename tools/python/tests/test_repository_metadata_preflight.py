"""Repository-wide metadata preflight for the strict source registry.

Proves, against the real tree:

- every reviewed C lift carries function-level ``@source`` and ``@behavior``;
- every reviewed C lift's ``@source`` address equals its reviewed Splat ``c``
  boundary address;
- no target has duplicate address claims;
- ``func_<ADDR>`` filenames provide no fallback identity: a tagged lift may
  use any filename, and a tagless file is never treated as a lift unless
  Splat expects it (then it is a deterministic error, not a guess).

One pre-existing Splat drift is documented as an exact exception
(``KNOWN_SPLAT_DRIFT``); the test fails closed if the observed drift set ever
differs from it.
"""

from __future__ import annotations

from pathlib import Path

import pytest

from harness.domain.manifests import load_target_manifests
from harness.domain.sources import (
    LiftMetadataError,
    SourceAddressCollision,
    collect_source_addresses,
    expected_lift_sources,
    source_address,
)
from harness.domain.tags import parse_source_tag
from harness.layout import parse_splat_layout

_REPO_ROOT = Path(__file__).resolve().parents[3]

KNOWN_SPLAT_DRIFT: set[tuple[str, str, int, int]] = set()


def test_repository_metadata_preflight() -> None:
    root = _REPO_ROOT
    manifests = load_target_manifests(root)
    assert manifests, "no targets loaded"

    observed_drift: set[tuple[str, str, int, int]] = set()
    accepted: set[Path] = set()
    for target, manifest in sorted(manifests.items()):
        splat = root / manifest.splat
        if not splat.is_file():
            pytest.fail(f"{target}: missing splat layout {splat}")
        layout = parse_splat_layout(splat, manifest.load_address)
        source_dir = root / manifest.source_dir
        if not source_dir.is_dir() and not manifest.has_explicit_sources:
            pytest.fail(f"{target}: missing source dir {source_dir}")
        expected = expected_lift_sources(layout, source_dir)
        try:
            if manifest.has_explicit_sources:
                from harness.domain.claims import collect_manifest_source_addresses

                rows = collect_manifest_source_addresses(
                    root, manifest, expected_lifts=expected
                )
            else:
                rows = collect_source_addresses(source_dir, expected_lifts=expected)
        except SourceAddressCollision as exc:
            pytest.fail(f"{target}: duplicate address claim: {exc}")
        except LiftMetadataError as exc:
            if exc.reason == "address_mismatch":
                expected_address = expected.get(exc.source_path.stem)
                observed_drift.add(
                    (
                        target,
                        exc.source_path.stem,
                        expected_address if expected_address is not None else 0,
                        source_address(exc.source_path),
                    )
                )
                continue
            pytest.fail(f"{target}: {exc}")
        accepted.update(path for path, _ in rows)

    assert observed_drift == KNOWN_SPLAT_DRIFT, (
        "observed Splat drift differs from KNOWN_SPLAT_DRIFT; update the "
        "constant or fix the Splat boundary first. "
        f"observed={sorted(observed_drift)} expected={sorted(KNOWN_SPLAT_DRIFT)}"
    )

    # Address filenames provide no fallback: every tagless func_<ADDR>.c in
    # the tree is an ignored helper/support TU, never an accepted lift.
    tagless_func_stems = {
        path
        for path in root.glob("src/**/*.c")
        if path.name.startswith("func_")
        and parse_source_tag(path.read_text(encoding="utf-8")) is None
    }
    assert not (tagless_func_stems & accepted), (
        "a func_<ADDR> filename was accepted without @source metadata: "
        f"{sorted(p.relative_to(root) for p in tagless_func_stems & accepted)}"
    )

    # Authored SLUS helper/support TUs stay non-lifts (no numeric @source,
    # no Splat c boundary expecting them).
    slus_helpers = {
        "buildEmiEntryLbas.c",
        "emiSlotToLba.c",
        "nextEmiPayloadOffset.c",
        "slot_table_data.c",
        "slot_table_find.c",
        "slot_table_logo_str.c",
    }
    accepted_names = {path.name for path in accepted}
    assert not (slus_helpers & accepted_names), (
        "authored SLUS helper classified as a lift: "
        f"{sorted(slus_helpers & accepted_names)}"
    )

    # Semantic filenames resolve as lifts; raw func_<ADDR> filenames resolve
    # only through their @source tag (both repaired above).
    assert (
        source_address(root / "src/bof3/world/func_801DFFEC.c")
        == 0x801DFFEC
    )
    assert (
        source_address(root / "src/bof3/world/func_801F36A0.c")
        == 0x801F36A0
    )
    assert any(path.name == "seedMenuScratch.c" for path in accepted), (
        "a reviewed semantic-named lift did not resolve"
    )
