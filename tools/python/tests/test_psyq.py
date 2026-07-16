from __future__ import annotations

from pathlib import Path
import zipfile

from harness.commands.symbols import main as symbols_main
from harness.psyq.discovery import _matches
from harness.psyq.signatures import find_calls, scan
from harness.snapshot import (
    SNAPSHOT_SCHEMA,
    SnapshotUnresolvedCall,
    TargetSnapshot,
    write_snapshot,
)
from harness.toolchain import setup_psyq as psyq_lib
from harness.toolchain.setup_psyq import import_psyq_sdk, stage_psyq_sdk


def make_fake_psyq_tree(root: Path) -> None:
    include_dir = root / "SDK" / "INCLUDE"
    lib_dir = root / "SDK" / "LIB"
    include_dir.mkdir(parents=True)
    lib_dir.mkdir(parents=True)
    (include_dir / "LIBGPU.H").write_text("#define TEST 1\r\n", encoding="utf-8")
    (lib_dir / "LIBGPU.LIB").write_bytes(b"fake")


def test_stage_psyq_sdk_from_tree(monkeypatch, tmp_path: Path) -> None:
    monkeypatch.setattr(psyq_lib, "REPO_ROOT", tmp_path)
    source_root = tmp_path / "inputs" / "psyq-source"
    dest_root = tmp_path / "psyq-staged"
    make_fake_psyq_tree(source_root)

    staged = stage_psyq_sdk(dest=dest_root, source_root=source_root)

    assert staged == dest_root.resolve()
    assert (dest_root / "include" / "LIBGPU.H").exists()
    assert (dest_root / "include" / "libgpu.h").exists()
    assert not (dest_root / "include" / "libgpu.h").is_symlink()
    assert (dest_root / "lib" / "LIBGPU.LIB").exists()
    assert (dest_root / ".gitkeep").exists()


def test_import_psyq_sdk_stages_original_archive(monkeypatch, tmp_path: Path) -> None:
    monkeypatch.setattr(psyq_lib, "REPO_ROOT", tmp_path)
    archive_path = tmp_path / "inputs" / "psyq47.zip"
    private_root = tmp_path / "inputs" / "external" / "private-assets"
    dest_root = tmp_path / "psyq-staged"
    archive_path.parent.mkdir(parents=True)
    with zipfile.ZipFile(archive_path, "w") as archive:
        archive.writestr("SDK/INCLUDE/LIBGPU.H", "#define TEST 1\r\n")
        archive.writestr("SDK/LIB/LIBGPU.LIB", b"fake")

    staged = import_psyq_sdk(
        dest=dest_root,
        archive=archive_path,
        private_assets_root=private_root,
        archive_url=None,
    )

    assert staged == dest_root.resolve()
    assert (private_root / "psyq" / "4.7" / "source-media" / archive_path.name).exists()
    assert (dest_root / "include" / "libgpu.h").exists()


def test_find_psyq_source_rejects_explicit_paths_outside_inputs(tmp_path: Path) -> None:
    source_root = tmp_path / "external" / "private-assets" / "psyq-source"
    make_fake_psyq_tree(source_root)

    try:
        psyq_lib.find_psyq_source(source_root=source_root)
    except ValueError as exc:
        assert "must stay under the repo's inputs/ tree" in str(exc)
    else:
        raise AssertionError("expected non-input source to be rejected")


def test_relocation_aware_match_is_not_an_exact_match() -> None:
    function = {
        "payload": b"abcdefghijklmnoq",
        "relocations": [(8, 12)],
        "relocation_hash": "",
    }
    from harness.psyq.fingerprints import relocation_masked_hash

    function["relocation_hash"] = relocation_masked_hash(
        function["payload"], function["relocations"]
    )

    assert list(_matches(b"abcdefghWXYZmnoq", function)) == [(0, "relocation_aware")]


def test_symbols_import_psyq_requires_write_and_replaces_raw_name(
    tmp_path: Path,
) -> None:
    manifest = tmp_path / "config" / "targets" / "exe" / "logo.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\n'
        'kind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/splat/exe/logo.yaml"\n'
        "load_address = 0x801CE000\n"
        'profile = "compat/capcom97"\n',
        encoding="utf-8",
    )
    target_map = tmp_path / "config" / "symbols" / "exe" / "logo.txt"
    target_map.parent.mkdir(parents=True)
    target_map.write_text("func_801CE758 = 0x801CE758;\n", encoding="utf-8")
    proposal = tmp_path / "proposal.json"
    proposal.write_text(
        '{"schema":"bof3.psyq-find/v1","matches":['
        '{"target":"exe/logo","address":"0x801CE758","name":"CdInit",'
        '"confidence":"exact","external":true}'
        "]}",
        encoding="utf-8",
    )

    args = [
        "--root",
        str(tmp_path),
        "import-psyq",
        str(proposal),
        "exe/logo@0x801CE758",
    ]
    assert symbols_main(args) == 1
    assert "func_801CE758" in target_map.read_text(encoding="utf-8")
    assert symbols_main([*args, "--write"]) == 0
    assert target_map.read_text(encoding="utf-8") == "CdInit = 0x801CE758;\n"


def _signature_fixture(root: Path) -> None:
    manifest = root / "config/targets/exe/logo.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\nkind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/splat/exe/logo.yaml"\n'
        "load_address = 0x801CE000\nprofile = \"test\"\n",
        encoding="utf-8",
    )
    binary = root / "out/binaries/exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0\0\0\0\x10\x20\x30\x40\xAA\xBB\xCC\xDD")
    for version in ("460", "470"):
        document = root / "toolchains/psx_psyq_signatures" / version / "LIBCD.LIB.json"
        document.parent.mkdir(parents=True, exist_ok=True)
        document.write_text(
            '[{"name":"EVENT.OBJ","sig":"10 ?? 30 40 AA BB CC DD",'
            '"labels":[{"name":"CdInit","offset":0},{"name":"loc_4","offset":4}]}]',
            encoding="utf-8",
        )
    (root / "toolchains/psx_psyq_signatures/.git").write_text("gitdir: fake\n", encoding="utf-8")


def test_signature_scan_merges_versions_and_calls_rizin_xrefs(tmp_path: Path) -> None:
    _signature_fixture(tmp_path)

    index = scan(tmp_path)

    assert index["matches"] == [
        {
            "target": "exe/logo",
            "address": "0x801CE004",
            "library": "LIBCD.LIB",
            "object": "EVENT.OBJ",
            "versions": ["460", "470"],
            "symbols": ["CdInit"],
            "labels": [{"name": "CdInit", "address": "0x801CE004"}],
        }
    ]
    index_path = tmp_path / "out/psyq/index.json"
    index_path.parent.mkdir(parents=True)
    import json

    index_path.write_text(json.dumps(index), encoding="utf-8")
    snapshot = TargetSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target="exe/logo",
        engine={"name": "rizin", "version": "test"},
        inputs={"binary_sha256": "ignored"},
        functions=(),
        calls=(),
        unresolved_calls=(
            SnapshotUnresolvedCall("exe/logo@801CE000", 0x801CE004, 0x801CE000, "unknown"),
        ),
    )
    write_snapshot(snapshot, tmp_path / "out/reverse/exe/logo/snapshot.json")

    calls = find_calls(tmp_path)

    assert calls["calls"] == [
        {
            "target": "exe/logo",
            "caller": "exe/logo@801CE000",
            "callsite": "0x801CE000",
            "address": "0x801CE004",
            "library": "LIBCD.LIB",
            "object": "EVENT.OBJ",
            "versions": ["460", "470"],
            "symbol": "CdInit",
        }
    ]
