from __future__ import annotations

from pathlib import Path
import zipfile

from harness.commands.setup import _staged_psyq_member
from harness.commands.symbols import main as symbols_main
from harness.psyq.signature_calls import find_calls
from harness.psyq.signatures import scan
from harness.analysis.snapshot import (
    SNAPSHOT_SCHEMA,
    SnapshotUnresolvedCall,
    AnalysisSnapshot,
    snapshot_path,
    write_snapshot,
)
from harness.toolchain import psyq_discovery
from harness.toolchain.psyq import import_psyq_sdk, stage_psyq_sdk


def test_staged_psyq_member_resolves_converted_obj_name(tmp_path: Path) -> None:
    converted = tmp_path / "toolchains/psyq/4.7/libsnd/vm_nowof.o"
    converted.parent.mkdir(parents=True)
    converted.touch()

    assert (
        _staged_psyq_member(tmp_path, "psyq/4.7/libsnd/VM_NOWOF.OBJ") == converted
    )
    assert _staged_psyq_member(tmp_path, "psyq/4.7/libcard/END.OBJ") == (
        tmp_path / "toolchains/psyq/4.7/libcard/END.OBJ"
    )


def make_fake_psyq_tree(root: Path) -> None:
    include_dir = root / "SDK" / "INCLUDE"
    lib_dir = root / "SDK" / "LIB"
    include_dir.mkdir(parents=True)
    lib_dir.mkdir(parents=True)
    (include_dir / "LIBGPU.H").write_text("#define TEST 1\r\n", encoding="utf-8")
    (lib_dir / "LIBGPU.LIB").write_bytes(b"fake")


def _write_check_target(root: Path, target: str) -> None:
    manifest = root / "config" / "targets" / target / "target.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        "\n".join(
            (
                'schema = "harness.target/v2"',
                f'id = "{target}"',
                'kind = "executable"',
                f'source_dir = "src/{target}"',
                f'binary = "out/binaries/{target}.bin"',
                f'splat = "config/targets/{target}/splat.yaml"',
                "load_address = 0x801CE000",
                f'sources = ["src/{target}/runtime/nested.c"]',
            )
        )
        + "\n",
        encoding="utf-8",
    )
    claimed = root / "src" / target / "runtime" / "nested.c"
    claimed.parent.mkdir(parents=True, exist_ok=True)
    if not claimed.exists():
        claimed.write_text("void placeholder(void) {}\n", encoding="utf-8")


def test_psyq_report_scans_nested_lift(tmp_path: Path, capsys) -> None:
    """psyq-report sees SDK calls from relocated (nested) lift sources."""
    (tmp_path / "config" / "sdk").mkdir(parents=True)
    (tmp_path / "config" / "sdk" / "psyq-slus.txt").write_text(
        "SomeSdkCall = 0x80010000;\n", encoding="utf-8"
    )
    _write_check_target(tmp_path, "exe/keep")
    nested = tmp_path / "src" / "exe" / "keep" / "runtime" / "nested.c"
    nested.parent.mkdir(parents=True, exist_ok=True)
    nested.write_text(
        "/* @source 0x80100000 @behavior x */\nvoid f(void) { SomeSdkCall(); }\n",
        encoding="utf-8",
    )

    code = symbols_main(["--root", str(tmp_path), "psyq-report", "exe/keep"])
    captured = capsys.readouterr()

    assert code == 0
    assert "SomeSdkCall = 0x80010000" in captured.out


def test_stage_psyq_sdk_from_tree(monkeypatch, tmp_path: Path) -> None:
    monkeypatch.setattr(psyq_discovery, "REPO_ROOT", tmp_path)
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
    monkeypatch.setattr(psyq_discovery, "REPO_ROOT", tmp_path)
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
        psyq_discovery.find_psyq_source(source_root=source_root)
    except ValueError as exc:
        assert "must stay under the repo's inputs/ tree" in str(exc)
    else:
        raise AssertionError("expected non-input source to be rejected")


def test_symbols_import_psyq_requires_write_and_replaces_raw_name(
    tmp_path: Path,
) -> None:
    manifest = tmp_path / "config" / "targets" / "exe" / "logo" / "target.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\n'
        'kind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/targets/exe/logo/splat.yaml"\n'
        "load_address = 0x801CE000\n",
        encoding="utf-8",
    )
    sdk_map = tmp_path / "config" / "sdk" / "psyq-slus.txt"
    sdk_map.parent.mkdir(parents=True, exist_ok=True)
    sdk_map.write_text("func_801CE758 = 0x801CE758;\n", encoding="utf-8")
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
    assert "func_801CE758" in sdk_map.read_text(encoding="utf-8")
    assert symbols_main([*args, "--write"]) == 0
    assert sdk_map.read_text(encoding="utf-8") == "CdInit = 0x801CE758;\n"


def _signature_fixture(root: Path) -> None:
    manifest = root / "config/targets/exe/logo/target.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\nkind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/targets/exe/logo/splat.yaml"\n'
        "load_address = 0x801CE000\n",
        encoding="utf-8",
    )
    binary = root / "out/binaries/exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0\0\0\0\x10\x20\x30\x40\xaa\xbb\xcc\xdd")
    for version in ("460", "470"):
        document = root / "toolchains/psx_psyq_signatures" / version / "LIBCD.LIB.json"
        document.parent.mkdir(parents=True, exist_ok=True)
        document.write_text(
            '[{"name":"EVENT.OBJ","sig":"10 ?? 30 40 AA BB CC DD",'
            '"labels":[{"name":"CdInit","offset":0},{"name":"loc_4","offset":4}]}]',
            encoding="utf-8",
        )
    (root / "toolchains/psx_psyq_signatures/.git").write_text(
        "gitdir: fake\n", encoding="utf-8"
    )


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
    assert index["version_evidence"] == [
        {
            "target": "exe/logo",
            "match_count": 1,
            "best_versions": ["460", "470"],
            "alignment_best_versions": ["460", "470"],
            "version_alignment_scores": [
                {"version": version, "score": 0.5 if version in {"460", "470"} else 0.0}
                for version in (
                    "260",
                    "300",
                    "330",
                    "340",
                    "350",
                    "3610",
                    "3611",
                    "370",
                    "400",
                    "410",
                    "420",
                    "430",
                    "440",
                    "450",
                    "460",
                    "470",
                )
            ],
            "historical_primary_versions": ["3610", "3611", "370", "400"],
            "historical_best_versions": [],
            "regional_rebuild_versions": ["410"],
            "version_match_counts": [
                {"version": version, "matches": int(version in {"460", "470"})}
                for version in (
                    "260",
                    "300",
                    "330",
                    "340",
                    "350",
                    "3610",
                    "3611",
                    "370",
                    "400",
                    "410",
                    "420",
                    "430",
                    "440",
                    "450",
                    "460",
                    "470",
                )
            ],
            "disagreement_count": 0,
            "disagreements": [],
        }
    ]
    index_path = tmp_path / "out/psyq/index.json"
    index_path.parent.mkdir(parents=True)
    import json

    index_path.write_text(json.dumps(index), encoding="utf-8")
    snapshot = AnalysisSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target="exe/logo",
        engine={"name": "rizin", "version": "test"},
        inputs={"binary_sha256": "ignored"},
        functions=(),
        calls=(),
        unresolved_calls=(
            SnapshotUnresolvedCall(
                "exe/logo@801CE000", 0x801CE004, 0x801CE000, "unknown"
            ),
        ),
    )
    write_snapshot(snapshot, snapshot_path(tmp_path, "exe/logo"))

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
