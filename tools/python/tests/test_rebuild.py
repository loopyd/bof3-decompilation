from __future__ import annotations

import importlib.machinery
import importlib.util
import json
from pathlib import Path
import sys

import pytest

from harness.io import repo_layout
from harness.match._asm_resolve import object_path_for_source
from harness.targets import load_target_manifests


ROOT = Path(__file__).resolve().parents[3]


def load_entrypoint(name: str):
    module_name = f"test_{name}_entrypoint"
    path = ROOT / "bin" / name
    loader = importlib.machinery.SourceFileLoader(module_name, str(path))
    spec = importlib.util.spec_from_loader(module_name, loader)
    assert spec is not None
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    loader.exec_module(module)
    return module


def write_manifest(root: Path, *, binary: str = "out/binaries/test.bin") -> None:
    path = root / "config" / "targets" / "exe" / "test.toml"
    path.parent.mkdir(parents=True)
    path.write_text(
        "\n".join(
            [
                'schema = "harness.target/v2"',
                'id = "exe/test"',
                'disc_id = "TEST.EXE"',
                'kind = "executable"',
                'source_dir = "src/exe/test"',
                f'binary = "{binary}"',
                'splat = "config/splat/exe/test.yaml"',
                "load_address = 0x80010000",
                'profile = "test"',
            ]
        ),
        encoding="utf-8",
    )


def write_source(root: Path, name: str, address: str) -> Path:
    source = root / "src" / "exe" / "test" / name
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(
        f"/* @source {address} test */\nvoid function(void) {{}}\n",
        encoding="utf-8",
    )
    return source


def test_rebuild_patches_text_and_records_baseline_metadata(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    rebuild = load_entrypoint("rebuild")
    write_manifest(tmp_path)
    source_a = write_source(tmp_path, "func_80010004.c", "0x80010004")
    source_b = write_source(tmp_path, "func_8001000c.c", "0x8001000c")
    original = bytes(range(32))
    binary = tmp_path / "out" / "binaries" / "test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(original)
    layout = repo_layout(tmp_path)
    for source in (source_a, source_b):
        object_path_for_source(layout, source).parent.mkdir(parents=True, exist_ok=True)
        object_path_for_source(layout, source).write_bytes(b"object")
    text = {source_a: b"ABCD", source_b: b"WXYZ"}
    monkeypatch.setattr(
        rebuild,
        "extract_text",
        lambda _objcopy, obj: text[
            next(
                source
                for source in text
                if object_path_for_source(layout, source) == obj
            )
        ],
    )

    result = rebuild.build_target(
        layout,
        load_target_manifests(tmp_path)["exe/test"],
        no_build=True,
    )

    rebuilt = result.output_path.read_bytes()
    assert rebuilt[4:8] == b"ABCD"
    assert rebuilt[12:16] == b"WXYZ"
    assert rebuilt[:4] == b"\x00" * 4
    assert rebuilt[8:12] == b"\x00" * 4
    metadata = json.loads(result.metadata_path.read_text(encoding="utf-8"))
    assert metadata["source_baseline"] == "out/binaries/test.bin"
    assert len(metadata["patched_functions"]) == 2
    assert metadata["unmatched_regions_remain_original"] is False
    assert metadata["unmatched_regions_zero_filled"] is True
    assert metadata["data_rodata_patched"] is False


def test_rebuild_rejects_text_past_next_function_boundary(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    rebuild = load_entrypoint("rebuild")
    write_manifest(tmp_path)
    source_a = write_source(tmp_path, "func_80010004.c", "0x80010004")
    write_source(tmp_path, "func_80010008.c", "0x80010008")
    binary = tmp_path / "out" / "binaries" / "test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(bytes(range(16)))
    layout = repo_layout(tmp_path)
    object_path = object_path_for_source(layout, source_a)
    object_path.parent.mkdir(parents=True, exist_ok=True)
    object_path.write_bytes(b"object")
    monkeypatch.setattr(rebuild, "extract_text", lambda _objcopy, _obj: b"12345")

    result = rebuild.build_target(
        layout,
        load_target_manifests(tmp_path)["exe/test"],
        no_build=True,
    )

    assert not result.wrote_output
    assert result.errors[0]["source"] == "src/exe/test/func_80010004.c"
    assert "overlaps next authored" in result.errors[0]["reason"]


def test_verify_requires_exact_bytes_even_when_partial_output_is_allowed(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    verify = load_entrypoint("verify")
    write_manifest(tmp_path)
    source = write_source(tmp_path, "func_80010004.c", "0x80010004")
    binary = tmp_path / "out" / "binaries" / "test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(bytes(range(16)))
    layout = repo_layout(tmp_path)
    object_path = object_path_for_source(layout, source)
    object_path.parent.mkdir(parents=True, exist_ok=True)
    object_path.write_bytes(b"object")
    rebuild = verify._load_rebuild_module()
    monkeypatch.setattr(rebuild, "extract_text", lambda _objcopy, _obj: b"DIFF")

    result = verify.verify_target(
        tmp_path,
        load_target_manifests(tmp_path)["exe/test"],
        no_build=True,
        allow_nonmatching=True,
    )

    assert result.status == "FAIL"
    assert not result.exact
    assert result.byte_match is False
    assert result.sha1_match is False
    assert result.length_match is True


def test_rebuild_does_not_allow_original_output_path(tmp_path: Path) -> None:
    rebuild = load_entrypoint("rebuild")
    write_manifest(tmp_path)
    binary = tmp_path / "out" / "binaries" / "test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"baseline")
    manifest = load_target_manifests(tmp_path)["exe/test"]

    with pytest.raises(ValueError, match="must not overwrite"):
        rebuild.build_target(
            repo_layout(tmp_path),
            manifest,
            output=binary,
        )


def test_rebuild_ignores_symbol_binding_units(tmp_path: Path) -> None:
    rebuild = load_entrypoint("rebuild")
    write_manifest(tmp_path)
    write_source(tmp_path, "func_80010004.c", "0x80010004")
    (tmp_path / "src" / "exe" / "test" / "symbols.c").write_text(
        "void bind_symbols(void) {}\n", encoding="utf-8"
    )

    functions, skipped = rebuild.collect_function_sources(
        tmp_path / "src" / "exe" / "test"
    )

    assert [function.path.name for function in functions] == ["func_80010004.c"]
    assert skipped == []
