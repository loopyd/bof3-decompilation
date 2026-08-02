"""Direct-execution contract for the two Pi status scripts after typed-manifest migration.

Both scripts are run as standalone subprocesses against a minimal
production-shaped manifest workspace with no inherited ``PYTHONPATH``, proving
their local ``tools/python`` import bootstrap and ``harness.domain`` manifest
loading preserve public ordering and selected report fields.
"""

SNAPSHOT_STATUS = (
    Path(__file__).resolve().parents[3]
    / ".pi"
    / "skills"
    / "psx-rizin"
    / "scripts"
    / "snapshot-status.py"
)
LOOP_STATUS = (
    Path(__file__).resolve().parents[3]
    / ".pi"
    / "skills"
    / "bof3-lift-loop"
    / "scripts"
    / "loop-status.py"
)

# (canonical id, disc_id, kind, load_address) — created in reversed order so the
# reports must sort them.
TARGETS = (
    ("exe/slus_004_22", "SLUS_004.22", "executable", 0x80096800),
    ("emi/etc/game/00", "BIN/ETC/GAME.EMI#0", "emi", 0x80090000),
    ("emi/battle/battle/15", "BIN/BATTLE/BATTLE.EMI#15", "emi", 0x80096800),
)
IDS = tuple(entry[0] for entry in TARGETS)


def _write_target(root: Path, target_id: str, disc_id: str, kind: str, load_address: int) -> None:
    target_dir = root / "config" / "targets" / target_id
    target_dir.mkdir(parents=True)
    (target_dir / "target.toml").write_text(
        "schema = 'harness.target/v2'\n"
        f"id = '{target_id}'\n"
        f"disc_id = '{disc_id}'\n"
        f"kind = '{kind}'\n"
        f"source_dir = 'src/{target_id}'\n"
        f"binary = 'out/binaries/{target_id}.bin'\n"
        f"splat = 'config/targets/{target_id}/splat.yaml'\n"
        f"load_address = 0x{load_address:08X}\n",
        encoding="utf-8",
    )
    binary = root / "out/binaries" / f"{target_id}.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"BOF3\x00" + target_id.encode())


def _workspace(root: Path, *, fresh: bool) -> dict[str, str]:
    """Minimal production-shaped workspace; returns binary sha256 per target id."""
    for target_id, disc_id, kind, load_address in reversed(TARGETS):
        _write_target(root, target_id, disc_id, kind, load_address)
    (root / "bin").mkdir()
    (root / "bin" / "rz-project").write_text(
        "#!/usr/bin/env python3\n"
        "import json, sys\n"
        f'print(json.dumps({{"fresh": {fresh}}}))\n',
        encoding="utf-8",
    )
    (root / "bin" / "rev-query").write_text(
        "#!/usr/bin/env python3\nimport json\nprint(json.dumps({'items': []}))\n",
        encoding="utf-8",
    )
    for helper in ("bin/rz-project", "bin/rev-query"):
        (root / helper).chmod(0o755)
    return {
        target_id: hashlib.sha256(b"BOF3\x00" + target_id.encode()).hexdigest()
        for target_id, *_ in TARGETS
    }


def _run(script: Path, root: Path, *args: str) -> subprocess.CompletedProcess:
    env = dict(os.environ)
    env.pop("PYTHONPATH", None)
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    return subprocess.run(
        (sys.executable, str(script), "--root", str(root), *args),
        cwd=root,
        env=env,
        text=True,
        capture_output=True,
    )


def test_snapshot_status_sweep_sorted_typed_fields(tmp_path: Path) -> None:
    digests = _workspace(tmp_path, fresh=True)

    result = _run(SNAPSHOT_STATUS, tmp_path)
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)

    assert report["schema"] == "bof3.skill-rizin-snapshot-status/v1"
    assert [record["target"] for record in report["targets"]] == sorted(IDS)
    assert report["summary"] == {"total": 3, "fresh": 3, "stale_or_unavailable": 0}
    assert report["reverse_index"]["exit_code"] == 0
    for target_id, disc_id, kind, load_address in TARGETS:
        record = next(r for r in report["targets"] if r["target"] == target_id)
        assert record["manifest"] == f"config/targets/{target_id}/target.toml"
        assert record["kind"] == kind
        assert record["disc_id"] == disc_id
        assert record["load_address"] == f"0x{load_address:08X}"
        assert record["binary"] == {
            "path": f"out/binaries/{target_id}.bin",
            "exists": True,
            "size": len(b"BOF3\x00" + target_id.encode()),
            "sha256": digests[target_id],
        }
        assert record["snapshot"]["json"] == {"fresh": True}


def test_snapshot_status_single_selection_and_unknown_target(tmp_path: Path) -> None:
    _workspace(tmp_path, fresh=True)

    selected = _run(SNAPSHOT_STATUS, tmp_path, "emi/etc/game/00")
    assert selected.returncode == 0, selected.stderr
    report = json.loads(selected.stdout)
    assert [record["target"] for record in report["targets"]] == ["emi/etc/game/00"]
    assert report["summary"]["total"] == 1

    unknown = _run(SNAPSHOT_STATUS, tmp_path, "emi/etc/game/02")
    assert unknown.returncode == 2
    assert "unknown target: emi/etc/game/02" in unknown.stderr


def test_loop_status_sorted_ids_and_inspection_only(tmp_path: Path) -> None:
    _workspace(tmp_path, fresh=False)

    result = _run(LOOP_STATUS, tmp_path)
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)

    assert report["schema"] == "bof3.skill-lift-loop-status/v1"
    assert [entry["command"][2] for entry in report["snapshots"]] == sorted(IDS)
    assert report["stale_targets"] == sorted(IDS)
    # Default invocation inspects only: no recovery, no index, no candidates.
    assert report["recovery"] is None
    assert report["suppressed_candidates"] == {
        "reason": "stale_snapshot",
        "stale_targets": sorted(IDS),
        "hint": "use --recover to repair stale generated evidence",
    }
    assert report["index"] == {"command": ["(skipped)"], "exit_code": 1}
    assert report["candidates"] == {"command": ["(skipped)"], "exit_code": 1}
    assert "analyze" not in str(report)
    assert report["journal"]["records"] == []


def test_loop_status_fresh_path_queries_candidates_in_order(tmp_path: Path) -> None:
    _workspace(tmp_path, fresh=True)

    result = _run(LOOP_STATUS, tmp_path, "--selection", "hotspots", "--limit", "1")
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)

    assert report["schema"] == "bof3.skill-lift-loop-status/v1"
    assert [entry["command"][2] for entry in report["snapshots"]] == sorted(IDS)
    assert report["stale_targets"] == []
    assert report["suppressed_candidates"] is None
    assert report["recovery"] is None
    assert report["index"]["exit_code"] == 0
    assert report["candidates"]["command"][:2] == ["bin/rev-query", "hotspots"]
    assert report["candidates"]["exit_code"] == 0
    assert report["journal"]["records"] == []
