    _candidate_exclusion,
    _priority_rows,
def test_candidate_exclusions_are_reported_without_ranking(tmp_path) -> None:
    target = "exe/t"
    config = tmp_path / "config" / "targets" / "exe" / "t"
    config.mkdir(parents=True)
    (config / "target.toml").write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/t"\nkind = "executable"\n'
        'source_dir = "src/exe/t"\n'
        'binary = "out/binaries/exe/t.bin"\n'
        'splat = "config/targets/exe/t/splat.yaml"\n'
        'load_address = 0x80100000\n',
        encoding="utf-8",
    )
    (config / "splat.yaml").write_text(
        "segments:\n"
        "  - [0, c, func_80100000]\n"
        "  - [8, c, func_80100008]\n"
        "  - [16, c, func_80100010]\n"
        "  - [24, c, func_80100018]\n"
        "  - [32]\n",
        encoding="utf-8",
    )
    binary = tmp_path / "out/binaries/exe/t.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(
        b"\x08\x00\xe0\x03\x00\x00\x00\x00"  # SDK body
        + b"\x00\x00\x10\x80\x04\x00\x10\x80"  # in-image pointers
        + b"DATA\x00TXT"  # printable data mislabeled as code
        + b"\x08\x00\xe0\x03\x00\x00\x00\x00"  # canonical code
    )
    sdk = tmp_path / "config/sdk"
    sdk.mkdir(parents=True)
    (sdk / "psyq-slus.txt").write_text("SdkFn = 0x80100000;\n", encoding="utf-8")

    def row(address: int) -> dict[str, object]:
        return {
            "id": f"{target}@{address:08X}",
            "target": target,
            "address": address,
            "size": 8,
            "instruction_count": 2,
            "basic_blocks": 1,
            "cfg_edges": 0,
            "loops": 0,
            "stack_frame": 0,
            "local_count": 0,
            "argument_count": 0,
            "trivial_kind": None,
            "caller_callsites": 0,
            "unique_callers": 0,
            "callee_callsites": 0,
            "unique_callees": 0,
            "unresolved_calls": 0,
            "reviewed": True,
            "lifted": False,
            "duplicate_members": 1,
            "unlifted_duplicate_members": 1,
            "duplicate_targets": 1,
            "exact_sha256": f"{address:064x}",
        }

    candidates = [row(0x80100000 + offset) for offset in (0, 8, 16, 24)]
    assert _candidate_exclusion(tmp_path, candidates[0]) == "shared_sdk_symbol"
    assert _candidate_exclusion(tmp_path, candidates[1]) == "in_image_pointer_table"
    assert _candidate_exclusion(tmp_path, candidates[2]) == "ascii_or_nul_data"
    assert _candidate_exclusion(tmp_path, candidates[3]) is None

    args = build_parser().parse_args(["quick-wins", "--exclusions", "--limit", "0"])
    args.target = target
    args.function = None
    from unittest.mock import patch

    with patch("harness.commands.rev_query._function_metrics", return_value=candidates):
        exclusions = _priority_rows(connection, args, root=tmp_path)
    assert [entry["candidate_exclusion"] for entry in exclusions] == [
        "shared_sdk_symbol",
        "in_image_pointer_table",
        "ascii_or_nul_data",
    ]

    args.exclusions = False
    with patch("harness.commands.rev_query._function_metrics", return_value=candidates):
        ranked = _priority_rows(connection, args, root=tmp_path)
    assert [entry["id"] for entry in ranked] == [f"{target}@80100018"]
