"""Unit tests for the analysis replay, snapshot, and graph modules."""

from __future__ import annotations

from pathlib import Path


from harness.analysis.replay import (
    ReplayInputs,
    render_generated_replay,
)
from harness.analysis.snapshot import (
    SNAPSHOT_SCHEMA,
    SnapshotFunction,
    build_snapshot,
)
from harness.analysis.graph import GRAPH_SCHEMA, build_graph
from harness.analysis.rizin import AnalyzerDump, RawFunction
from harness.domain.registry import ResolvedTarget
from harness.domain.ids import TargetId
from harness.layout import parse_splat_layout
from harness.source_inventory import build_source_inventory


def _manifest(slug: str, load_address: int, root: Path | None = None) -> ResolvedTarget:
    base = root or Path("")
    return ResolvedTarget(
        id=TargetId(slug, slug.upper()),
        manifest_path=base / f"config/targets/{slug}.toml",
        disc_id=slug.upper(),
        kind="emi",
        source_dir=base / f"src/emi/{slug}",
        binary_path=base / f"out/binaries/emi/{slug}.bin",
        splat_path=base / f"config/splat/emi/{slug}.yaml",
        reviewed_replay_path=base / f"config/analysis/{slug}/reviewed.r2",
        load_address=load_address,
        profile="native/capcom97",
    )


def _write_target_inputs(
    root: Path,
    resolved: ResolvedTarget,
    *,
    func_addresses: list[int],
) -> Path:
    payload = b"\x00" * 0x400
    binary = root / resolved.binary_path
    binary.parent.mkdir(parents=True, exist_ok=True)
    binary.write_bytes(payload)
    source_dir = root / resolved.source_dir
    source_dir.mkdir(parents=True, exist_ok=True)
    symbols_path = source_dir / "symbols.c"
    lines = ["/* Canonical binding entry point. */"]
    for address in func_addresses:
        name = f"func_{address:08x}"
        lines.append(f"WEAK_SYMBOL_AT({name}, 0x{address:08x})")
    symbols_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    splat_path = root / resolved.splat_path
    splat_path.parent.mkdir(parents=True, exist_ok=True)
    splat_path.write_text(
        "segments:\n  - [0x0, bin]\n  - [0x400]\n", encoding="utf-8"
    )
    return binary


def test_replay_inputs_composite_hash_changes() -> None:
    a = ReplayInputs(
        target_id="test",
        manifest_sha256="aaa",
        binary_sha256="bbb",
        splat_sha256="ccc",
        source_inventory_sha256="ddd",
        generated_replay_sha256=None,
        reviewed_replay_sha256=None,
        shared_types_sha256=None,
        shared_hwregs_sha256=None,
    )
    b = ReplayInputs(
        target_id="test",
        manifest_sha256="aaa",
        binary_sha256="bbb",
        splat_sha256="ccc",
        source_inventory_sha256="changed",
        generated_replay_sha256=None,
        reviewed_replay_sha256=None,
        shared_types_sha256=None,
        shared_hwregs_sha256=None,
    )
    assert a.composite_hash() != b.composite_hash()


def test_render_generated_replay_classifies_semantic_names_as_data(
    tmp_path: Path,
) -> None:
    resolved = _manifest("emi/etc/game/01", 0x801D0C00, root=tmp_path)
    source_dir = tmp_path / resolved.source_dir
    source_dir.mkdir(parents=True)
    (source_dir / "symbols.c").write_text(
        "\n".join(
            [
                "WEAK_SYMBOL_AT(func_801d0c00, 0x801d0c00)",
                "WEAK_SYMBOL_AT(func_801d0c80, 0x801d0c80)",
                "WEAK_SYMBOL_AT(GAME_FRONT_STATE, 0x80143c10)",
                "WEAK_SYMBOL_AT(DAT_80143b40, 0x80143b40)",
            ]
        )
        + "\n",
        encoding="utf-8",
    )
    internal = source_dir / "internal.h"
    internal.write_text(
        "extern vu16 GAME_FRONT_STATE;\n"
        "extern vu16 DAT_80143b40;\n",
        encoding="utf-8",
    )
    splat_path = tmp_path / resolved.splat_path
    splat_path.parent.mkdir(parents=True, exist_ok=True)
    splat_path.write_text(
        "segments:\n  - [0x0, c, func_801d0c00]\n  - [0x400]\n",
        encoding="utf-8",
    )

    layout = parse_splat_layout(splat_path, 0x801D0C00)
    inventory = build_source_inventory(source_dir, resolved.id.value)
    text = render_generated_replay(
        layout=layout,
        inventory=inventory,
        splat_path=splat_path,
        source_dir=source_dir,
    )

    assert "afn func_801d0c00 @ 0x801d0c00" in text
    assert "afn func_801d0c80 @ 0x801d0c80" in text
    assert "f data.GAME_FRONT_STATE 4 @ 0x80143c10" in text
    assert "f data.DAT_80143b40 4 @ 0x80143b40" in text
    assert "f data.func_801d0c00" not in text


def test_build_snapshot_target_qualified_ids(tmp_path: Path) -> None:
    a = _manifest("emi/etc/game/00", 0x801D0C00, root=tmp_path)
    b = _manifest("emi/etc/game/01", 0x801D0C00, root=tmp_path)
    _write_target_inputs(tmp_path, a, func_addresses=[0x801D0C00])
    _write_target_inputs(tmp_path, b, func_addresses=[0x801D0C00])

    dump = AnalyzerDump(
        functions=(RawFunction(addr=0x801D0C00, size=0x80, name="func_801d0c00"),),
        xrefs=(),
        strings=(),
    )
    inputs = ReplayInputs(
        target_id="test",
        manifest_sha256=None,
        binary_sha256=None,
        splat_sha256=None,
        source_inventory_sha256=None,
        generated_replay_sha256=None,
        reviewed_replay_sha256=None,
        shared_types_sha256=None,
        shared_hwregs_sha256=None,
    )

    layout_a = parse_splat_layout(a.splat_path, 0x801D0C00)
    inventory_a = build_source_inventory(a.source_dir, a.id.value)
    snap_a = build_snapshot(
        resolved=a,
        layout=layout_a,
        inventory=inventory_a,
        dump=dump,
        inputs=inputs,
        root=tmp_path,
    )

    layout_b = parse_splat_layout(b.splat_path, 0x801D0C00)
    inventory_b = build_source_inventory(b.source_dir, b.id.value)
    snap_b = build_snapshot(
        resolved=b,
        layout=layout_b,
        inventory=inventory_b,
        dump=dump,
        inputs=inputs,
        root=tmp_path,
    )

    assert snap_a.functions[0].id != snap_b.functions[0].id
    assert snap_a.functions[0].id == "emi/etc/game/00@801d0c00"
    assert snap_b.functions[0].id == "emi/etc/game/01@801d0c00"


def test_build_graph_target_qualified_duplicates() -> None:
    func_a = SnapshotFunction(
        id="emi/etc/game/00@801d0c00",
        address=0x801D0C00,
        analyzer_size=0x80,
        analyzer_name="func",
        source_name=None,
        semantic_name=None,
        is_reviewed=True,
        is_lifted=False,
        source=None,
        exact_sha256="aaa",
    )
    func_b = SnapshotFunction(
        id="emi/etc/game/01@801d0c00",
        address=0x801D0C00,
        analyzer_size=0x80,
        analyzer_name="func",
        source_name=None,
        semantic_name=None,
        is_reviewed=True,
        is_lifted=False,
        source=None,
        exact_sha256="aaa",
    )

    from harness.analysis.snapshot import TargetSnapshot

    snap_a = TargetSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target="emi/etc/game/00",
        engine={"name": "rizin", "version": "test"},
        inputs={},
        functions=(func_a,),
        calls=(),
        unresolved_calls=(),
    )
    snap_b = TargetSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target="emi/etc/game/01",
        engine={"name": "rizin", "version": "test"},
        inputs={},
        functions=(func_b,),
        calls=(),
        unresolved_calls=(),
    )

    graph = build_graph(
        {"emi/etc/game/00": snap_a, "emi/etc/game/01": snap_b},
    )
    assert len(graph.duplicate_groups) == 1
    group = graph.duplicate_groups[0]
    assert sorted(group) == [
        "emi/etc/game/00@801d0c00",
        "emi/etc/game/01@801d0c00",
    ]


def test_snapshot_schema_version() -> None:
    assert SNAPSHOT_SCHEMA == "bof3.analysis-snapshot/v1"


def test_graph_schema_version() -> None:
    assert GRAPH_SCHEMA == "bof3.analysis-graph/v2"
