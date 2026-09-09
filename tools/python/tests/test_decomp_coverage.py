"""Synthetic coverage identity, containment and nonmutating dispatch checks."""

import json
from pathlib import Path
import sqlite3
import struct
from types import SimpleNamespace

import pytest

from harness.commands import decomp_status
from harness.decomp.coverage import build_coverage, reviewed_coverage, union_intervals
from harness.decomp.coverage_inputs import CoverageInputs, inventory_archives
from harness.decomp.coverage_index import read_index, reconcile_index


def put(root, path, data):
    path = root / path
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data.encode() if isinstance(data, str) else data)
    return path


def test_union():
    assert union_intervals([(4, 9), (0, 5), (11, 12)]) == [(0, 9), (11, 12)]


@pytest.mark.parametrize(
    "header,offset,valid", [(False, 0, True), (True, 2048, True), (True, 0, False)]
)
def test_coordinates(tmp_path, header, offset, valid):
    binary = bytearray(2064 if header else 16)
    if header:
        binary[:8] = b"PS-X EXE"
        struct.pack_into("<II", binary, 24, 0x80010000, 16)
    put(
        tmp_path,
        "layout.yaml",
        f"segments:\n  - start: {offset}\n    vram: 0x80010000\n    subsegments:\n      - [{offset}, c, sample]\n  - [{offset + 16}]\n",
    )
    manifest = SimpleNamespace(
        splat="layout.yaml", binary="image", load_address=0x80010000
    )
    _, report = reviewed_coverage(CoverageInputs(tmp_path), manifest, bytes(binary))
    assert (report["executable_range_bytes"] == 16) is valid
    assert bool(report["discrepancies"]) is not valid
    if not valid:
        assert "coordinate interpretation ambiguity" in report["discrepancies"][0]


@pytest.mark.parametrize("kind", ["leaf", "parent", "fifo"])
def test_unsafe_inputs(tmp_path, kind):
    if kind == "fifo":
        import os

        os.mkfifo(tmp_path / "file")
    elif kind == "leaf":
        (tmp_path / "file").symlink_to(tmp_path / "absent")
    else:
        (tmp_path / "alias").symlink_to(tmp_path, target_is_directory=True)
    with pytest.raises(ValueError):
        CoverageInputs(tmp_path).read("alias/file" if kind == "parent" else "file")


def test_moving_input(tmp_path):
    put(tmp_path, "dir/file", "old")
    inputs = CoverageInputs(tmp_path)
    inputs.walk("dir")
    inputs.read("dir/file")
    put(tmp_path, "dir/file", "new")
    with pytest.raises(ValueError, match="changed input"):
        inputs.verify()


@pytest.mark.parametrize(
    "problem", ["none", "missing", "extra", "duplicate", "malformed"]
)
def test_archive_inventory(tmp_path, problem):
    put(tmp_path, "out/extracted/BIN/X/A.EMI", "archive")
    entries = [
        {"index": 0, "size": 4, "name": "0.bin", "type": 0, "ram_ptr": 0x80010000}
    ]
    if problem == "duplicate":
        entries *= 2
    if problem != "missing":
        put(tmp_path, "out/extracted/BIN/X/A/0.bin", b"data")
    if problem == "extra":
        put(tmp_path, "out/extracted/BIN/X/A/extra.bin", b"extra")
    put(
        tmp_path,
        "out/extracted/BIN/X/A/emi.json",
        "{" if problem == "malformed" else json.dumps({"entries": entries}),
    )
    report = inventory_archives(CoverageInputs(tmp_path))
    assert report["archive_count"] == 1
    assert bool(report["discrepancies"]) == (problem != "none")
    if problem == "none":
        assert report["slots"][0]["identity"] == "BIN/X/A.EMI#0"


@pytest.mark.parametrize("state", ["missing", "corrupt", "journal", "wal", "stale"])
def test_index_unavailable_without_writes(tmp_path, state):
    path = tmp_path / "out/index/reverse.sqlite"
    if state != "missing":
        path.parent.mkdir(parents=True)
        if state == "corrupt":
            path.write_bytes(b"corrupt")
        else:
            with sqlite3.connect(path) as connection:
                connection.execute("CREATE TABLE targets(binary, snapshot)")
            if state == "journal":
                Path(str(path) + "-journal").write_bytes(b"journal")
            if state == "wal":
                content = bytearray(path.read_bytes())
                content[18:20] = b"\x02\x02"
                path.write_bytes(content)
    before = {str(p): p.read_bytes() for p in tmp_path.rglob("*") if p.is_file()}
    result = read_index(CoverageInputs(tmp_path), [])
    assert not result["available"] and result["functions"] is None
    assert before == {
        str(p): p.read_bytes() for p in tmp_path.rglob("*") if p.is_file()
    }


def test_reconcile_target_qualified():
    target = {
        "target": "exe/a",
        "ranges": [
            {
                "virtual_start": 16,
                "kind": "c",
                "sha256": "a",
                "file_start": 0,
                "file_end": 4,
            }
        ],
        "authored": [{"address": 16}],
    }
    row = {
        "target_id": "exe/a",
        "address": 16,
        "reviewed": 1,
        "lifted": 1,
        "reviewed_sha256": "a",
        "reviewed_size": 4,
        "contains_data": 0,
    }
    index = {
        "available": True,
        "functions": [row, {**row, "target_id": "exe/b", "reviewed_sha256": "other"}],
    }
    assert reconcile_index(target, index) == []
    row["contains_data"] = 1
    assert "contains data" in reconcile_index(target, index)[0]


def test_cli_no_build_and_out_rejected(tmp_path, monkeypatch, capsys):
    from harness.decomp import coverage

    def forbidden(*args, **kwargs):
        pytest.fail("build/write path reached")

    monkeypatch.setattr(decomp_status, "build_report", forbidden)
    monkeypatch.setattr(decomp_status, "write_report", forbidden)
    report = {"targets": [], "blockers": ["unknown denominator"], "inputs": {}}
    monkeypatch.setattr(coverage, "build_coverage", lambda *args: report)
    parser = decomp_status.build_parser()
    for detail in ("minimal", "normal", "full"):
        args = parser.parse_args(["--coverage-only", "--no-cache", "--detail", detail])
        assert decomp_status.run(args) == 2
        assert "unknown denominator" in capsys.readouterr().out
    output = put(tmp_path, "kept", "unchanged")
    with pytest.raises(ValueError, match="stdout-only"):
        decomp_status.run(parser.parse_args(["--coverage-only", "--out", str(output)]))
    assert output.read_text() == "unchanged"


def test_default_dispatch(monkeypatch, capsys):
    called = []
    report = {"lifts": {"invalid": 0}}
    monkeypatch.setattr(
        decomp_status, "build_report", lambda *a, **kw: called.append(kw) or report
    )
    monkeypatch.setattr(decomp_status, "project_report", lambda r, detail: r)
    assert decomp_status.run(decomp_status.build_parser().parse_args(["--json"])) == 0
    assert called == [{"use_cache": True}]
    assert json.loads(capsys.readouterr().out) == report


@pytest.mark.parametrize("bad", ["../escape", "/absolute", "a//b", "a/./b"])
def test_path_spellings(tmp_path, bad):
    with pytest.raises(ValueError):
        CoverageInputs(tmp_path).read(bad)


def test_missing_original_retains_target_facts(tmp_path):
    put(
        tmp_path,
        "config/targets/exe/test/target.toml",
        """schema = "harness.target/v2"
id = "exe/test"
kind = "executable"
source_dir = "src/test"
binary = "image.bin"
splat = "layout.yaml"
load_address = 2147549184
""",
    )
    put(tmp_path, "image.bin", b"raw payload")
    put(tmp_path, "layout.yaml", "segments: [[0, asm, entry], [11]]")
    report = build_coverage(tmp_path)
    assert report["targets"][0]["image"]["bytes"] == 11
    assert report["original"]["archive_count"] is None
    assert report["full_original_function_denominator"] is None
    assert report == build_coverage(tmp_path)


@pytest.mark.parametrize("data", [b"", b"PS-X EXE", b"PS-X EXE" + bytes(2040)])
def test_invalid_images(tmp_path, data):
    from harness.decomp.coverage import target_coverage

    put(tmp_path, "image", data)
    manifest = SimpleNamespace(
        binary="image", load_address=0x80010000, disc_id="test", kind="executable"
    )
    report = target_coverage(CoverageInputs(tmp_path), "exe/test", manifest)
    assert report["discrepancies"]
    assert "ranges" not in report


def coverage_fixture(root):
    for directory in ("src", "include", "out/reverse/snapshots", "out/extracted/BIN"):
        (root / directory).mkdir(parents=True, exist_ok=True)
    for name in ("test", "other"):
        put(
            root,
            f"config/targets/exe/{name}/target.toml",
            f'''schema = "harness.target/v2"
id = "exe/{name}"
disc_id = "{name}"
kind = "executable"
source_dir = "src/{name}"
binary = "{name}.bin"
splat = "{name}.yaml"
load_address = 2147549184
''',
        )
        binary = bytearray(2064)
        binary[:8] = b"PS-X EXE"
        struct.pack_into("<II", binary, 24, 0x80010000, 16)
        put(root, f"{name}.bin", binary)
        put(root, f"out/extracted/{name}", binary)
        put(root, f"{name}.yaml", "segments: [[2048, asm, entry], [2064]]")
    path = put(root, "out/index/reverse.sqlite", b"")
    with sqlite3.connect(path) as connection:
        connection.execute("CREATE TABLE targets(binary, snapshot)")
        connection.execute(
            "CREATE TABLE functions(target_id, address, size, analyzer_sha256, reviewed_sha256, reviewed_size, reviewed, lifted, source, contains_data, id)"
        )
        connection.execute(
            "CREATE TABLE function_candidates(target_id, address, provenance)"
        )
    return root / "config/targets/exe/test/target.toml"


@pytest.mark.parametrize(
    "document",
    [
        "",
        "scalar",
        "[]",
        "[",
        "segments: null",
        "segments: []\noptions: []",
        "segments: [false]",
        "segments: [[]]",
        "segments: [[0, 3]]",
        "segments: [[true]]",
        "segments: [[1.5]]",
        "segments: [[{}, asm]]",
        "segments: [{start: false}]",
        "segments: [{vram: []}]",
        "segments: [{subsegments: false}]",
        "segments: [{subsegments: [[0]]}]",
        "segments: [[0, asm, 5]]",
        "segments: [[0, asm, {name: false}]]",
        "segments: [[0, asm, {source: []}]]",
        "segments: [[0, asm, {behavior: {}}]]",
        "segments: [[0, asm, entry, 5]]",
        "segments: []\noptions: {symbol_addrs_path: false}",
    ],
)
def test_malformed_layout_retains_image(tmp_path, document, capsys):
    coverage_fixture(tmp_path)
    put(tmp_path, "test.yaml", document)
    report = build_coverage(tmp_path, ["exe/test"])
    row = report["targets"][0]
    assert row["image"]["bytes"] == 2064
    assert row["image"]["payload"]["load_address"] == 0x80010000
    assert row["shipped_identity"] == "test"
    assert any("Splat" in error for error in row["discrepancies"])
    assert not report["index"]["available"] and "Splat" in report["index"]["reason"]
    args = decomp_status.build_parser().parse_args(
        ["--root", str(tmp_path), "--coverage-only", "--json", "exe/test"]
    )
    assert decomp_status.run(args) == 2
    assert json.loads(capsys.readouterr().out)["targets"][0]["image"] == row["image"]


@pytest.mark.parametrize(
    "field",
    [
        "psyq_source = false",
        "psyq_source = 0",
        "psyq_source = []",
        "sources = false",
        "support_sources = [1]",
        "headers = {}",
        "companion_overlays = [{}]",
        "companion_overlays = false",
        "[psyq]\nspace = false",
        "[psyq.libraries.a]\nconfidence = false",
        '[psyq.libraries.a]\nmembers = "bad"',
        "[psyq.libraries.a]\nevidence = [1]",
        "[matching]\nsection_placements = [{}]",
        '[[matching.section_placements]]\nfunction = true\nsection = ".text"\naddress = 4\nsize = 4',
    ],
)
def test_malformed_manifest_fields(tmp_path, field):
    path = coverage_fixture(tmp_path)
    path.write_text(path.read_text() + field + "\n")
    report = build_coverage(tmp_path)
    assert report["targets"] == []
    assert len(report["blockers"]) > 1
    result = read_index(CoverageInputs(tmp_path), [])
    assert not result["available"]
    assert result["reason"]


@pytest.mark.parametrize("selection", ["custom/maps.txt", ["custom/maps.txt"]])
@pytest.mark.parametrize(
    "problem",
    ["regular", "leaf", "parent", "dangling", "escape", "type", "syntax", "change"],
)
def test_selected_maps_all_targets(tmp_path, monkeypatch, selection, problem):
    import yaml
    from harness.decomp import coverage_index

    coverage_fixture(tmp_path)
    put(tmp_path, "custom/maps.txt", "entry = 0x80010000;\n")
    if problem in {"leaf", "dangling"}:
        (tmp_path / "custom/maps.txt").unlink()
        (tmp_path / "custom/maps.txt").symlink_to(
            tmp_path / ("test.bin" if problem == "leaf" else "absent")
        )
    if problem == "parent":
        (tmp_path / "custom").rename(tmp_path / "real")
        (tmp_path / "custom").symlink_to(tmp_path / "real", target_is_directory=True)
    if problem == "escape":
        selection = "../escape"
    if problem == "type":
        selection = [False]
    put(
        tmp_path,
        "other.yaml",
        "["
        if problem == "syntax"
        else yaml.safe_dump(
            {
                "segments": [[2048, "asm", "entry"], [2064]],
                "options": {"symbol_addrs_path": selection},
            }
        ),
    )
    reached = []
    observed_inputs = []
    original_read = CoverageInputs.read
    original_connect = sqlite3.connect
    connections = []

    def checked_read(self, relative):
        observed_inputs[:] = [self]
        return original_read(self, relative)

    def connect(*args, **kwargs):
        connections.append(True)
        return original_connect(*args, **kwargs)

    monkeypatch.setattr(CoverageInputs, "read", checked_read)
    monkeypatch.setattr(coverage_index.sqlite3, "connect", connect)

    def freshness(connection, root):
        reached.append(True)
        assert "custom/maps.txt" in observed_inputs[0].files
        if problem == "change":
            put(root, "custom/maps.txt", "changed")

    monkeypatch.setattr(coverage_index, "validate_status_index", freshness)
    inputs = CoverageInputs(tmp_path)
    result = read_index(inputs, [])
    expected = (
        "unsafe input type"
        if problem in {"leaf", "parent", "dangling"}
        else "unsafe input path"
        if problem == "escape"
        else "Splat"
        if problem in {"type", "syntax"}
        else "changed input"
        if problem == "change"
        else None
    )
    assert (
        (result["reason"] is None) if expected is None else expected in result["reason"]
    )
    assert bool(connections) == (problem in {"regular", "change"})
    assert bool(reached) == (problem in {"regular", "change"})
    if problem != "change":
        report = build_coverage(tmp_path, ["exe/test"])
        assert report["targets"][0]["image"]["bytes"] == 2064
        assert (
            report["index"]["available"]
            if expected is None
            else expected in report["index"]["reason"]
        )


@pytest.mark.parametrize(
    "key", ["id", "kind", "binary", "splat", "source_dir", "load_address"]
)
@pytest.mark.parametrize("value", ["false", "0.5", "[]", "{}"])
def test_malformed_manifest_required(tmp_path, key, value):
    path = coverage_fixture(tmp_path)
    lines = path.read_text().splitlines()
    path.write_text(
        "\n".join(
            f"{key} = {value}" if line.startswith(key + " =") else line
            for line in lines
        )
    )
    report = build_coverage(tmp_path)
    assert not report["targets"]
    assert "manifest" in report["blockers"][0]
    assert "manifest" in read_index(CoverageInputs(tmp_path), [])["reason"]


@pytest.mark.parametrize(
    "part,key",
    [
        ("overlay", "target"),
        ("overlay", "disc_id"),
        ("overlay", "payload_sha256"),
        ("overlay", "load_address"),
        ("overlay", "size"),
        ("overlay", "evidence"),
        ("call", "caller_address"),
        ("call", "target_address"),
        ("abi", "target_address"),
        ("abi", "prototype"),
        ("abi", "evidence"),
    ],
)
@pytest.mark.parametrize("bad", ["false", "[]", "1.5", "missing"])
def test_malformed_companion_fields(tmp_path, part, key, bad):
    path = coverage_fixture(tmp_path)
    sections = {
        "overlay": {
            "target": '"emi/X/A/0"',
            "disc_id": '"BIN/X/A.EMI#0"',
            "payload_sha256": '"' + "a" * 64 + '"',
            "load_address": "2147549184",
            "size": "16",
            "evidence": '"reviewed"',
        },
        "call": {"caller_address": "2147549184", "target_address": "2147549184"},
        "abi": {
            "target_address": "2147549184",
            "prototype": '"void entry(void);"',
            "evidence": '"reviewed"',
        },
    }
    if bad == "missing":
        del sections[part][key]
    else:
        sections[part][key] = bad
    text = path.read_text()
    for section, header in [
        ("overlay", "[[companion_overlays]]"),
        ("call", "[[companion_overlays.static_calls]]"),
        ("abi", "[companion_overlays.abi]"),
    ]:
        text += (
            "\n"
            + header
            + "\n"
            + "\n".join(f"{k} = {v}" for k, v in sections[section].items())
            + "\n"
        )
    path.write_text(text)
    result = read_index(CoverageInputs(tmp_path), [])
    assert not result["available"] and "manifest" in result["reason"]
    assert not build_coverage(tmp_path)["targets"]


@pytest.mark.parametrize(
    "source", ["src/../alias", "src//alias", "src/./alias", "/absolute"]
)
def test_malformed_source_directory(tmp_path, source):
    path = coverage_fixture(tmp_path)
    path.write_text(path.read_text().replace("src/test", source))
    assert "unsafe input path" in build_coverage(tmp_path)["blockers"][0]


@pytest.mark.parametrize(
    "document",
    [
        'segments: [["0x800", asm], ["0x810"]]',
        "segments: [{subsegments: [[0, asm, null]]}, [16]]",
        'segments: [[0, asm, {name: null, source: "", behavior: null}], [16]]',
        'segments: [[0, asm, entry, "@source: file.c", "@behavior: entry"] , [16]]',
        'segments: [{start: "0x0", vram: "0x80010000", subsegments: []}]',
    ],
)
def test_malformed_layout_supported_forms(tmp_path, document):
    from harness.decomp.coverage_inputs import read_layout_document, read_manifests
    from harness.domain.layout import parse_splat_layout

    path = coverage_fixture(tmp_path)
    path.write_text(
        path.read_text().replace("load_address = 2147549184\n", "")
        + 'psyq_source = ""\n'
    )
    assert read_manifests(CoverageInputs(tmp_path))["exe/test"].load_address == 0
    put(tmp_path, "test.yaml", document)
    read_layout_document(CoverageInputs(tmp_path), "test.yaml")
    parse_splat_layout(tmp_path / "test.yaml", 0x80010000)


def test_malformed_manifest_syntax_cli(tmp_path, capsys):
    path = coverage_fixture(tmp_path)
    path.write_text("[")
    args = decomp_status.build_parser().parse_args(
        ["--root", str(tmp_path), "--coverage-only", "--json"]
    )
    assert decomp_status.run(args) == 2
    report = json.loads(capsys.readouterr().out)
    assert not report["targets"] and "manifest" in report["blockers"][0]
    assert "manifest" in read_index(CoverageInputs(tmp_path), [])["reason"]


@pytest.mark.parametrize(
    "field",
    [
        "st_atime_ns",
        "st_dev",
        "st_ino",
        "st_mode",
        "st_size",
        "st_mtime_ns",
        "st_ctime_ns",
    ],
)
def test_malformed_stable_input_identity(tmp_path, monkeypatch, field):
    import os
    from harness.decomp import coverage_inputs

    path = put(tmp_path, "file", b"data")
    observed = path.stat()
    fields = [
        "st_atime_ns",
        "st_dev",
        "st_ino",
        "st_mode",
        "st_size",
        "st_mtime_ns",
        "st_ctime_ns",
    ]
    changed = SimpleNamespace(
        **{key: getattr(observed, key) + (key == field) for key in fields}
    )
    real_fstat = os.fstat
    monkeypatch.setattr(
        coverage_inputs.os,
        "fstat",
        lambda fd: (
            changed if real_fstat(fd).st_ino == observed.st_ino else real_fstat(fd)
        ),
    )
    inputs = CoverageInputs(tmp_path)
    if field == "st_atime_ns":
        assert inputs.read("file") == b"data"
    else:
        with pytest.raises(ValueError, match="moving input"):
            inputs.read("file")
