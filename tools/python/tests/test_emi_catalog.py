from __future__ import annotations

import hashlib
import json
import struct
from pathlib import Path

import pytest

from harness.commands.companion_check import build_report
from harness.domain import load_target_manifests
from harness.emi.catalog import build_catalog
from harness.emi.catalog_bootstrap import materialize_reviewed_targets
from harness.emi.catalog_verify import verify_declared_companions

CALLER_BASE = 0x801D0C00
CALLSITE = 0x801E0C28
COMPANION_BASE = 0x800F5000
TARGET = 0x800F500C


def _jal(address: int) -> int:
    return 0x0C000000 | ((address >> 2) & 0x03FFFFFF)


def _entry(root: Path, archive: str, slot: int, base: int, payload: bytes) -> None:
    directory = root / "out" / "extracted" / "BIN" / archive
    directory.mkdir(parents=True, exist_ok=True)
    (directory / f"{slot}.bin").write_bytes(payload)
    manifest = directory / "emi.json"
    entries = json.loads(manifest.read_text()) if manifest.exists() else {"entries": []}
    entries["entries"].append(
        {
            "index": slot,
            "name": f"{slot}.bin",
            "ram_ptr": base,
            "size": len(payload),
            "type": 0,
        }
    )
    manifest.write_text(json.dumps(entries), encoding="utf-8")


def _target(
    root: Path, target: str, disc_id: str, base: int, *, companion: bool = False
) -> None:
    path = root / "config" / "targets" / target / "target.toml"
    path.parent.mkdir(parents=True, exist_ok=True)
    text = (
        'schema = "harness.target/v2"\n'
        f'id = "{target}"\n'
        f'disc_id = "{disc_id}"\n'
        'kind = "emi"\n'
        f'source_dir = "src/{target}"\n'
        f'binary = "out/binaries/{target}.bin"\n'
        f'splat = "config/targets/{target}/splat.yaml"\n'
        f"load_address = 0x{base:08X}\n"
    )
    if companion:
        payload = (
            root / "out" / "extracted" / "BIN" / "WORLD00" / "AREA030" / "5.bin"
        ).read_bytes()
        text += (
            "\n[[companion_overlays]]\n"
            'target = "emi/world00/area030/05"\n'
            'disc_id = "BIN/WORLD00/AREA030.EMI#5"\n'
            f'payload_sha256 = "{hashlib.sha256(payload).hexdigest()}"\n'
            f"load_address = 0x{COMPANION_BASE:08X}\n"
            f"size = 0x{len(payload):X}\n"
            'evidence = "reviewed direct call"\n'
            "\n[[companion_overlays.static_calls]]\n"
            f"caller_address = 0x{CALLSITE:08X}\n"
            f"target_address = 0x{TARGET:08X}\n"
        )
    path.write_text(text, encoding="utf-8")


def _fixture_root(tmp_path: Path, *, companion: bool = True) -> Path:
    caller = bytearray(0x1002C)
    struct.pack_into("<I", caller, CALLSITE - CALLER_BASE, _jal(TARGET))
    payload = b"\x17\x00\x00\x00HEADER"
    payload += b"\0" * (0x2250 - len(payload))
    _entry(tmp_path, "WORLD00/AREA030", 4, CALLER_BASE, bytes(caller))
    _entry(tmp_path, "WORLD00/AREA030", 5, COMPANION_BASE, payload)
    _entry(tmp_path, "WORLD02/AREA089", 5, COMPANION_BASE, payload)
    _entry(tmp_path, "WORLD03/AREA129", 5, COMPANION_BASE, payload)
    _target(
        tmp_path, "emi/world00/area030/05", "BIN/WORLD00/AREA030.EMI#5", COMPANION_BASE
    )
    _target(
        tmp_path,
        "emi/world00/area030/04",
        "BIN/WORLD00/AREA030.EMI#4",
        CALLER_BASE,
        companion=companion,
    )
    return tmp_path


def test_companion_relation_requires_declaration(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path, companion=False)
    catalog = build_catalog(root / "out" / "extracted" / "BIN")

    assert len(catalog["build_targets"]) == 2
    assert catalog["companion_relations"] == []


def test_target_scoped_companion_verification_matches_catalog_relation(
    tmp_path: Path,
) -> None:
    root = _fixture_root(tmp_path)
    manifests = load_target_manifests(root)
    catalog = build_catalog(root / "out" / "extracted" / "BIN")

    assert (
        verify_declared_companions(root, manifests["emi/world00/area030/04"])
        == catalog["companion_relations"]
    )


def test_declared_companion_relation_verifies_identity_and_jal(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path)
    catalog = build_catalog(root / "out" / "extracted" / "BIN")

    assert catalog["companion_relations"] == [
        {
            "caller": "emi/world00/area030/04",
            "companion": "emi/world00/area030/05",
            "disc_id": "BIN/WORLD00/AREA030.EMI#5",
            "payload_sha256": hashlib.sha256(
                (
                    root / "out" / "extracted" / "BIN" / "WORLD00" / "AREA030" / "5.bin"
                ).read_bytes()
            ).hexdigest(),
            "load_address": COMPANION_BASE,
            "size": 0x2250,
            "static_calls": [{"caller_address": CALLSITE, "target_address": TARGET}],
            "evidence": "reviewed direct call",
        }
    ]


def test_catalog_rejects_caller_base_mismatch(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path)
    path = (
        root
        / "config"
        / "targets"
        / "emi"
        / "world00"
        / "area030"
        / "04"
        / "target.toml"
    )
    path.write_text(path.read_text().replace("0x801D0C00", "0x801D1000"))

    with pytest.raises(ValueError, match="caller catalog load address mismatch"):
        build_catalog(root / "out" / "extracted" / "BIN")


def test_catalog_rejects_changed_jal(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path)
    caller = root / "out" / "extracted" / "BIN" / "WORLD00" / "AREA030" / "4.bin"
    data = bytearray(caller.read_bytes())
    struct.pack_into("<I", data, CALLSITE - CALLER_BASE, 0)
    caller.write_bytes(data)

    with pytest.raises(ValueError, match="companion call bytes differ"):
        build_catalog(root / "out" / "extracted" / "BIN")


def test_catalog_rejects_changed_companion_payload_identity(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path)
    payload = root / "out" / "extracted" / "BIN" / "WORLD00" / "AREA030" / "5.bin"
    payload.write_bytes(payload.read_bytes() + b"stale")

    with pytest.raises(ValueError, match="companion catalog identity mismatch"):
        build_catalog(root / "out" / "extracted" / "BIN")


def test_manifest_rejects_companion_target_base_mismatch(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path)
    path = (
        root
        / "config"
        / "targets"
        / "emi"
        / "world00"
        / "area030"
        / "05"
        / "target.toml"
    )
    path.write_text(path.read_text().replace("0x800F5000", "0x800F6000"))

    with pytest.raises(ValueError, match="companion overlay identity mismatch"):
        load_target_manifests(root)


def test_manifest_rejects_companion_outside_declared_payload(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path)
    path = (
        root
        / "config"
        / "targets"
        / "emi"
        / "world00"
        / "area030"
        / "04"
        / "target.toml"
    )
    path.write_text(
        path.read_text().replace(
            "target_address = 0x800F500C", "target_address = 0x800F8000"
        )
    )

    with pytest.raises(ValueError, match="companion call outside payload"):
        load_target_manifests(root)


def _layout(root: Path, target: str, base: int, offset: int, name: str) -> None:
    path = root / "config" / "targets" / target / "splat.yaml"
    path.write_text(
        "segments:\n"
        "- name: main\n"
        "  type: code\n"
        f"  start: {offset}\n"
        f"  vram: {base + offset}\n"
        "  subsegments:\n"
        f"  - - {offset}\n"
        "    - asm\n"
        f"    - {name}\n"
        f"- - {offset + 0x20}\n",
        encoding="utf-8",
    )


def test_companion_check_allows_callers_without_companion_static_calls(
    tmp_path: Path,
) -> None:
    root = _fixture_root(tmp_path)
    _layout(root, "emi/world00/area030/04", CALLER_BASE, 0, "func_801E0C20")
    report = build_report(root, "emi/world00/area030/04@0x801E0C20")

    assert report["companions"] == []
    assert report["ready_to_lift"]


def test_companion_check_requires_boundary_map_abi_and_declaration(
    tmp_path: Path,
) -> None:
    root = _fixture_root(tmp_path)
    _layout(
        root,
        "emi/world00/area030/04",
        CALLER_BASE,
        CALLSITE - CALLER_BASE - 8,
        "func_801E0C20",
    )
    _layout(root, "emi/world00/area030/05", COMPANION_BASE, 12, "func_800F500C")
    missing = build_report(root, "emi/world00/area030/04@0x801E0C20")
    assert not missing["ready_to_lift"]
    assert missing["companions"][0]["abi"]["status"] == "missing"
    assert missing["companions"][0]["companion_binding"]["status"] == "missing"
    assert missing["companions"][0]["consumer_declaration"]["status"] == "missing"

    (
        root
        / "config"
        / "targets"
        / "emi"
        / "world00"
        / "area030"
        / "05"
        / "symbols.txt"
    ).write_text("func_800F500C = 0x800F500C;\n", encoding="utf-8")
    header = root / "include" / "bof3" / "world" / "area03004_internal.h"
    header.parent.mkdir(parents=True)
    header.write_text(
        '#define FALSE_ABI "ignore; void func_800F500C(void); ignore" \\\n'
        "  continued tokens\n"
        "char marker = ';'; // void func_800F500C(void);\n"
        "/* void func_800F500C(void); */\n",
        encoding="utf-8",
    )
    path = (
        root
        / "config"
        / "targets"
        / "emi"
        / "world00"
        / "area030"
        / "04"
        / "target.toml"
    )
    manifest = path.read_text().replace(
        "\n[[companion_overlays]]\n",
        '\nheaders = ["include/bof3/world/area03004_internal.h"]\n'
        "\n[[companion_overlays]]\n",
    )
    path.write_text(
        manifest
        + "\n[companion_overlays.abi]\n"
        + "target_address = 0x800F500C\n"
        + 'prototype = "void func_800F500C(void)"\n'
        + 'evidence = "reviewed callers and callee assembly"\n',
        encoding="utf-8",
    )
    commented = build_report(root, "emi/world00/area030/04@0x801E0C20")
    assert commented["companions"][0]["consumer_declaration"]["status"] == "missing"

    header.write_text(
        "#ifndef AREA03004_INTERNAL_H\n"
        "#define AREA03004_INTERNAL_H \\\n"
        "  continued tokens\n"
        "void\nfunc_800F500C( void );\n"
        "#endif\n",
        encoding="utf-8",
    )
    ready = build_report(root, "emi/world00/area030/04@0x801E0C20")
    assert ready["ready_to_lift"]


def test_materialization_validates_relation_before_writing(tmp_path: Path) -> None:
    root = _fixture_root(tmp_path)
    catalog = build_catalog(root / "out" / "extracted" / "BIN")
    caller = root / "out" / "extracted" / "BIN" / "WORLD00" / "AREA030" / "4.bin"
    data = bytearray(caller.read_bytes())
    struct.pack_into("<I", data, CALLSITE - CALLER_BASE, 0)
    caller.write_bytes(data)
    with pytest.raises(ValueError, match="companion call bytes differ"):
        materialize_reviewed_targets(root=root, catalog=catalog)
    assert not (
        root / "out" / "binaries" / "emi" / "world00" / "area030" / "04.bin"
    ).exists()
