#!/usr/bin/env python3
"""Self-check checkpoint no-progress host-gate semantics."""

import importlib.util
import json
import os
from pathlib import Path, PurePath
import shutil
import stat
import subprocess
import tempfile
import uuid
from unittest import mock

ROOT = Path(__file__).resolve().parents[4]
SCRIPT = ROOT / ".pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py"
SELECTOR = "emi/world00/area030/04@0x801DAE3C"
LANE = f"checkpoint-self-check-{uuid.uuid4().hex}"


def _parent_fsync_failure(fail_on: set[int]):
    """Fail os.fsync for the nth directory (parent) fsync, succeeding elsewhere."""
    real_fsync = os.fsync
    calls = 0

    def injected(fd: int) -> None:
        nonlocal calls
        if stat.S_ISDIR(os.fstat(fd).st_mode):
            calls += 1
            if calls in fail_on:
                raise OSError("injected parent fsync failure")
        real_fsync(fd)

    return mock.patch.object(os, "fsync", side_effect=injected)


def load_checkpoint_module():
    spec = importlib.util.spec_from_file_location("attempt_checkpoint", SCRIPT)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, cwd=ROOT, text=True, capture_output=True)


def test_cleanup_scope_round_trip() -> None:
    lane = LANE + "-cleanup"
    lane_dir = ROOT / "out/lift-loop/checkpoints" / lane
    target = "emi/etc/shop/00"
    config = ROOT / "config/targets" / target
    source_dir = ROOT / "src/emi/etc/shop/00"
    originals = {
        path: path.read_bytes()
        for path in (
            config / "target.toml",
            config / "symbols.txt",
            config / "splat.yaml",
        )
    }
    extra = ROOT / "src/bof3/ui/checkpoint_task_extra.h"
    extra.parent.mkdir(parents=True, exist_ok=True)
    extra.write_text("old cleanup baseline\n")
    originals[extra] = b"latest cleanup baseline\n"
    old = run(
        "python3",
        str(SCRIPT),
        "capture",
        "--lane",
        lane,
        "--selector",
        SELECTOR,
        "--attempt",
        "20",
        "--paths-only",
        extra.relative_to(ROOT).as_posix(),
    )
    assert old.returncode == 0, old.stderr
    old_record = (lane_dir / "attempt-20/record.json").read_bytes()
    first = run(
        "python3",
        str(SCRIPT),
        "capture",
        "--lane",
        lane,
        "--selector",
        SELECTOR,
        "--attempt",
        "21",
        "--replace",
        "--paths-only",
        "--target-scope",
        target,
        extra.relative_to(ROOT).as_posix(),
    )
    assert first.returncode == 0, first.stderr
    extra.write_bytes(originals[extra])
    captured = run(
        "python3",
        str(SCRIPT),
        "capture",
        "--lane",
        lane,
        "--selector",
        SELECTOR,
        "--attempt",
        "21",
        "--replace",
        "--paths-only",
        "--target-scope",
        target,
        extra.relative_to(ROOT).as_posix(),
    )
    assert captured.returncode == 0, captured.stderr
    captured_leaf = json.loads(captured.stdout)["checkpoint"]
    assert (lane_dir / "attempt-20/record.json").read_bytes() == old_record
    assert (
        lane_dir / captured_leaf / "files" / extra.relative_to(ROOT)
    ).read_bytes() == originals[extra]
    record = json.loads((lane_dir / captured_leaf / "record.json").read_text())
    assert record["schema"] == "bof3.attempt-checkpoint/v1"
    assert {"target.toml", "symbols.txt", "splat.yaml"} <= {
        Path(state["path"]).name for state in record["files"]
    }
    created = source_dir / "checkpoint_created.c"
    inside_target = config / "symbols.txt"
    inside_bytes = inside_target.read_bytes()
    with tempfile.TemporaryDirectory() as directory:
        outside_target = Path(directory) / "outside.txt"
        outside_target.write_text("outside target\n")
        inside_link = source_dir / "checkpoint_inside_link.c"
        outside_link = source_dir / "checkpoint_outside_link.c"
        created.parent.mkdir(parents=True, exist_ok=True)
        created.write_text("created\n")
        incomplete = lane_dir / "attempt-22-deadbeefdeadbeef"
        incomplete.mkdir()
        (incomplete / "record.json").write_text(
            json.dumps({"files": [{"path": created.relative_to(ROOT).as_posix()}]})
        )
        inside_link.symlink_to(inside_target)
        outside_link.symlink_to(outside_target)
        (config / "symbols.txt").write_text("changed\n")
        (config / "splat.yaml").unlink()
        extra.unlink()
        restored = run(
            "python3",
            str(SCRIPT),
            "restore",
            "--lane",
            lane,
            "--checkpoint",
            captured_leaf,
        )

        assert restored.returncode == 0, restored.stderr
        assert json.loads(restored.stdout)["clean_equality"] is True
        assert not created.exists() and not inside_link.is_symlink()
        assert not outside_link.is_symlink()
        assert inside_target.read_bytes() == inside_bytes
        assert outside_target.read_text() == "outside target\n"
        assert all(path.read_bytes() == content for path, content in originals.items())
        assert incomplete.is_dir(), "restore must not consume incomplete leaf bytes"
    extra.unlink()
    created.parent.rmdir()
    shutil.rmtree(lane_dir)


def test_rejects_checkpoint_root_lane_and_attempt_symlinks() -> None:
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory) / "repo"
        script = root / ".pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py"
        script.parent.mkdir(parents=True)
        script.write_bytes(SCRIPT.read_bytes())
        outside = Path(directory) / "outside"
        outside.mkdir()
        checkpoint_root = root / "out/lift-loop/checkpoints"
        cases = (
            (checkpoint_root, "root-link"),
            (checkpoint_root / "lane-link", "lane-link"),
            (checkpoint_root / "attempt-link/attempt-21", "attempt-link"),
        )
        for link, lane in cases:
            shutil.rmtree(root / "out", ignore_errors=True)
            link.parent.mkdir(parents=True)
            link.symlink_to(outside, target_is_directory=True)
            rejected = subprocess.run(
                (
                    "python3",
                    str(script),
                    "capture",
                    "--lane",
                    lane,
                    "--selector",
                    SELECTOR,
                    "--attempt",
                    "21",
                    "--paths-only",
                ),
                cwd=root,
                text=True,
                capture_output=True,
            )
            assert rejected.returncode != 0
            assert "symlink" in rejected.stderr
            assert not any(outside.iterdir())


def test_rejects_component_substitution_at_every_checkpoint_boundary() -> None:
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory) / "repo"
        script = root / ".pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py"
        script.parent.mkdir(parents=True)
        script.write_bytes(SCRIPT.read_bytes())
        outside = Path(directory) / "outside"
        outside.mkdir()
        sentinel = outside / "sentinel"
        sentinel.write_text("untouched\n")
        boundaries = (
            root / "out",
            root / "out/lift-loop",
            root / "out/lift-loop/checkpoints",
            root / "out/lift-loop/checkpoints/lane",
            root / "out/lift-loop/checkpoints/lane/attempt-21",
            root / "out/lift-loop/checkpoints/lane/attempt-21/files",
        )
        for boundary in boundaries:
            out = root / "out"
            if out.is_symlink():
                out.unlink()
            else:
                shutil.rmtree(out, ignore_errors=True)
            for child in outside.iterdir():
                if child != sentinel:
                    if child.is_symlink() or child.is_file():
                        child.unlink()
                    else:
                        shutil.rmtree(child)
            boundary.parent.mkdir(parents=True, exist_ok=True)
            boundary.symlink_to(outside, target_is_directory=True)
            rejected = subprocess.run(
                (
                    "python3",
                    str(script),
                    "capture",
                    "--lane",
                    "lane",
                    "--selector",
                    SELECTOR,
                    "--attempt",
                    "21",
                    "--replace",
                    "--paths-only",
                ),
                cwd=root,
                text=True,
                capture_output=True,
            )
            if boundary.name != "files":
                assert rejected.returncode != 0, boundary
            assert sentinel.read_text() == "untouched\n"
            assert sorted(path.name for path in outside.iterdir()) == ["sentinel"], (
                boundary
            )


def test_rejects_links_special_files_and_noncanonical_paths() -> None:
    lane = LANE + "-unsafe"
    lane_dir = ROOT / "out/lift-loop/checkpoints" / lane
    target = "emi/etc/shop/00"
    source_dir = ROOT / "src/emi/etc/shop/00"
    link = source_dir / "checkpoint_baseline_link.c"
    fifo = source_dir / "checkpoint_baseline_fifo.c"
    outside = ROOT.parent / "checkpoint-outside-target"
    outside.write_text("outside\n")
    source_dir.mkdir(parents=True, exist_ok=True)
    try:
        link.symlink_to(outside)
        rejected = run(
            "python3",
            str(SCRIPT),
            "capture",
            "--lane",
            lane,
            "--selector",
            SELECTOR,
            "--attempt",
            "21",
            "--replace",
            "--paths-only",
            "--target-scope",
            target,
        )
        assert rejected.returncode != 0
        assert "not a regular file" in rejected.stderr
        link.unlink()
        fifo.parent.mkdir(parents=True, exist_ok=True)
        fifo.unlink(missing_ok=True)
        os.mkfifo(fifo)
        rejected = run(
            "python3",
            str(SCRIPT),
            "capture",
            "--lane",
            lane,
            "--selector",
            SELECTOR,
            "--attempt",
            "21",
            "--replace",
            "--paths-only",
            "--target-scope",
            target,
        )
        assert rejected.returncode != 0
        assert "not a regular file" in rejected.stderr
        explicit = run(
            "python3",
            str(SCRIPT),
            "capture",
            "--lane",
            lane,
            "--selector",
            SELECTOR,
            "--attempt",
            "21",
            "--replace",
            "--paths-only",
            "../checkpoint-outside-target",
        )
        assert explicit.returncode != 0
        assert "canonical and repository-relative" in explicit.stderr
    finally:
        link.unlink(missing_ok=True)
        fifo.unlink(missing_ok=True)
        outside.unlink(missing_ok=True)
        source_dir.rmdir()
        shutil.rmtree(lane_dir, ignore_errors=True)


def test_rejects_crafted_best_checkpoint_escape_and_binding_drift() -> None:
    lane = LANE + "-crafted"
    lane_dir = ROOT / "out/lift-loop/checkpoints" / lane
    source = ROOT / "docs/agents/checkpoint-crafted-test.md"
    source.write_text("baseline\n", encoding="utf-8")
    try:
        captured = run(
            "python3",
            str(SCRIPT),
            "capture",
            "--lane",
            lane,
            "--selector",
            SELECTOR,
            "--attempt",
            "1",
            "--paths-only",
            source.relative_to(ROOT).as_posix(),
        )
        assert captured.returncode == 0, captured.stderr
        record_path = lane_dir / "attempt-1/record.json"
        record = json.loads(record_path.read_text())
        best_path = lane_dir / "best.json"
        best_path.write_text(json.dumps(record | {"checkpoint": "attempt-1"}))
        inspected = run("python3", str(SCRIPT), "best", "--lane", lane)
        assert inspected.returncode == 0, inspected.stderr
        assert json.loads(inspected.stdout)["best"]["checkpoint"] == "attempt-1"
        best_path.write_text("not json")
        rejected = run("python3", str(SCRIPT), "best", "--lane", lane)
        assert rejected.returncode != 0
        assert source.read_text() == "baseline\n"
        for checkpoint in ("../escape", "/tmp/escape", ".", "attempt-1/../escape"):
            best_path.write_text(json.dumps(record | {"checkpoint": checkpoint}))
            rejected = run("python3", str(SCRIPT), "restore", "--lane", lane)
            assert rejected.returncode != 0
            assert source.read_text() == "baseline\n"
        for key, value in (
            ("attempt", 2),
            ("run", "other"),
            ("selector", "other@0x1"),
            ("root", "/tmp"),
        ):
            best_path.write_text(
                json.dumps(record | {"checkpoint": "attempt-1", key: value})
            )
            rejected = run("python3", str(SCRIPT), "restore", "--lane", lane)
            assert rejected.returncode != 0
            assert "binding mismatch" in rejected.stderr
    finally:
        source.unlink(missing_ok=True)
        shutil.rmtree(lane_dir, ignore_errors=True)


def test_no_replacement_deletion_and_recovery_retention() -> None:
    lane = LANE + "-retention"
    lane_dir = ROOT / "out/lift-loop/checkpoints" / lane
    source = ROOT / "docs/agents/checkpoint-retention-test.md"
    source.write_text("first\n", encoding="utf-8")
    try:
        leaves = []
        for content in ("first\n", "second\n"):
            source.write_text(content, encoding="utf-8")
            captured = run(
                "python3",
                str(SCRIPT),
                "capture",
                "--lane",
                lane,
                "--selector",
                SELECTOR,
                "--attempt",
                "21",
                "--unique",
                "--paths-only",
                source.relative_to(ROOT).as_posix(),
            )
            assert captured.returncode == 0, captured.stderr
            leaves.append(json.loads(captured.stdout)["checkpoint"])
        assert leaves[0] != leaves[1]
        first_record = (lane_dir / leaves[0] / "record.json").read_bytes()
        assert (lane_dir / leaves[1] / "record.json").is_file()
        # no replacement deletion: the prior unique leaf stays byte-identical
        assert (lane_dir / leaves[0] / "record.json").read_bytes() == first_record
        # recovery retention: the prior leaf still restores its original state
        source.write_text("changed\n", encoding="utf-8")
        restored = run(
            "python3",
            str(SCRIPT),
            "restore",
            "--lane",
            lane,
            "--checkpoint",
            leaves[0],
        )
        assert restored.returncode == 0, restored.stderr
        assert source.read_text() == "first\n"
        restored = run(
            "python3",
            str(SCRIPT),
            "restore",
            "--lane",
            lane,
            "--checkpoint",
            leaves[1],
        )
        assert restored.returncode == 0, restored.stderr
        assert source.read_text() == "second\n"
    finally:
        source.unlink(missing_ok=True)
        shutil.rmtree(lane_dir, ignore_errors=True)


def test_transaction_preflight_rollback_and_fsync() -> None:
    checkpoint = load_checkpoint_module()
    prefix = f"out/checkpoint-transaction-{uuid.uuid4().hex}"
    first = ROOT / prefix / "first.txt"
    second = ROOT / prefix / "nested/second.txt"
    first.parent.mkdir(parents=True)
    first.write_text("current first\n")
    second.parent.mkdir(parents=True)
    second.write_text("current second\n")
    first.chmod(0o640)
    second.chmod(0o600)
    states = {
        first.relative_to(ROOT).as_posix(): {
            "path": first.relative_to(ROOT).as_posix(),
            "exists": True,
            "mode": 0o600,
        },
        second.relative_to(ROOT).as_posix(): {
            "path": second.relative_to(ROOT).as_posix(),
            "exists": True,
            "mode": 0o644,
        },
    }
    snapshots = {
        first.relative_to(ROOT).as_posix(): b"restored first\n",
        second.relative_to(ROOT).as_posix(): b"restored second\n",
    }
    original = {
        first: (first.read_bytes(), first.stat().st_mode & 0o777),
        second: (second.read_bytes(), second.stat().st_mode & 0o777),
    }
    real_restore = checkpoint.restore_file
    calls = 0

    def fail_second(*args, **kwargs):
        nonlocal calls
        calls += 1
        if calls == 2:
            raise OSError("injected mid-apply failure")
        return real_restore(*args, **kwargs)

    try:
        with mock.patch.object(checkpoint, "restore_file", side_effect=fail_second):
            try:
                checkpoint._apply_restore(
                    sorted(states), states, snapshots, {"roots": []}
                )
            except OSError as error:
                assert "injected" in str(error)
            else:
                raise AssertionError("injected failure did not abort restore")
        for path, (content, mode) in original.items():
            assert path.read_bytes() == content
            assert path.stat().st_mode & 0o777 == mode

        fsyncs = []
        with mock.patch.object(os, "fsync", side_effect=lambda fd: fsyncs.append(fd)):
            real_restore(b"durable\n", first.relative_to(ROOT), 0o640)
            checkpoint.remove_path(first.relative_to(ROOT))
        assert len(fsyncs) >= 3, "restore must fsync file and both parent mutations"

        conflict_parent = first.relative_to(ROOT).as_posix()
        first.write_text("late conflict sentinel\n")
        try:
            checkpoint._apply_restore(
                [conflict_parent, conflict_parent + "/child"],
                {},
                {},
                {"roots": []},
            )
        except ValueError as error:
            assert "conflict" in str(error)
        else:
            raise AssertionError("file/descendant conflict was accepted")
        assert first.read_text() == "late conflict sentinel\n"
    finally:
        shutil.rmtree(ROOT / prefix, ignore_errors=True)


def _fsync_restore_fixture(checkpoint, tag: str):
    prefix = f"out/checkpoint-fsync-{tag}-{uuid.uuid4().hex}"
    first = ROOT / prefix / "first.txt"
    second = ROOT / prefix / "nested/second.txt"
    first.parent.mkdir(parents=True)
    second.parent.mkdir(parents=True)
    first.write_text("current first\n")
    second.write_text("current second\n")
    first.chmod(0o640)
    second.chmod(0o600)
    names = [
        first.relative_to(ROOT).as_posix(),
        second.relative_to(ROOT).as_posix(),
    ]
    original = {
        first: (first.read_bytes(), first.stat().st_mode & 0o777),
        second: (second.read_bytes(), second.stat().st_mode & 0o777),
    }
    return prefix, names, original


def test_parent_fsync_failure_after_replace_rolls_back_all_paths() -> None:
    checkpoint = load_checkpoint_module()
    prefix, names, original = _fsync_restore_fixture(checkpoint, "replace")
    states = {
        names[0]: {"path": names[0], "exists": True, "mode": 0o600},
        names[1]: {"path": names[1], "exists": True, "mode": 0o644},
    }
    snapshots = {
        names[0]: b"restored first\n",
        names[1]: b"restored second\n",
    }
    try:
        with _parent_fsync_failure({2}):  # second replace durable, parent fsync fails
            try:
                checkpoint._apply_restore(sorted(names), states, snapshots, {"roots": []})
            except OSError as error:
                assert "fsync" in str(error)
            else:
                raise AssertionError("injected parent fsync failure did not abort restore")
        for path, (content, mode) in original.items():
            assert path.read_bytes() == content
            assert path.stat().st_mode & 0o777 == mode
    finally:
        shutil.rmtree(ROOT / prefix, ignore_errors=True)


def test_parent_fsync_failure_after_unlink_restores_removed_path() -> None:
    checkpoint = load_checkpoint_module()
    prefix, names, original = _fsync_restore_fixture(checkpoint, "unlink")
    states = {
        names[0]: {"path": names[0], "exists": True, "mode": 0o600},
        names[1]: {"path": names[1], "exists": False},
    }
    snapshots = {names[0]: b"restored first\n"}
    try:
        with _parent_fsync_failure({2}):  # second unlink done, parent fsync fails
            try:
                checkpoint._apply_restore(sorted(names), states, snapshots, {"roots": []})
            except OSError as error:
                assert "fsync" in str(error)
            else:
                raise AssertionError("injected parent fsync failure did not abort restore")
        for path, (content, mode) in original.items():
            assert path.read_bytes() == content
            assert path.stat().st_mode & 0o777 == mode
    finally:
        shutil.rmtree(ROOT / prefix, ignore_errors=True)


def test_rollback_fsync_failure_reports_error_but_attempts_all_paths() -> None:
    checkpoint = load_checkpoint_module()
    prefix, names, original = _fsync_restore_fixture(checkpoint, "rollback")
    states = {
        names[0]: {"path": names[0], "exists": True, "mode": 0o600},
        names[1]: {"path": names[1], "exists": True, "mode": 0o644},
    }
    snapshots = {
        names[0]: b"restored first\n",
        names[1]: b"restored second\n",
    }
    try:
        # second apply parent fsync and the rollback of that path both fail;
        # rollback of the first path must still run and report the recovery error.
        with _parent_fsync_failure({2, 3}):
            try:
                checkpoint._apply_restore(sorted(names), states, snapshots, {"roots": []})
            except RuntimeError as error:
                assert "rollback failed" in str(error)
            else:
                raise AssertionError("rollback fsync failure was not reported")
        for path, (content, mode) in original.items():
            assert path.read_bytes() == content
            assert path.stat().st_mode & 0o777 == mode
    finally:
        shutil.rmtree(ROOT / prefix, ignore_errors=True)


def test_mkdir_fsync_failure_records_created_dir_and_prunes() -> None:
    checkpoint = load_checkpoint_module()
    prefix = f"out/checkpoint-mkdir-fsync-{uuid.uuid4().hex}"
    path = ROOT / prefix / "nested/file.txt"
    name = path.relative_to(ROOT).as_posix()
    states = {name: {"path": name, "exists": True, "mode": 0o600}}
    snapshots = {name: b"restored\n"}
    (ROOT / prefix).mkdir(parents=True)
    try:
        # fsync right after the first mkdir (the file's parent) fails: the new
        # directory must still be on the rollback list so caller rollback prunes it.
        with _parent_fsync_failure({1}):
            try:
                checkpoint._apply_restore(list(states), states, snapshots, {"roots": []})
            except OSError as error:
                assert "fsync" in str(error)
            else:
                raise AssertionError(
                    "injected mkdir fsync failure did not abort restore"
                )
        assert not (ROOT / prefix / "nested").exists(), "created dir left behind"
    finally:
        shutil.rmtree(ROOT / prefix, ignore_errors=True)


def test_nested_mkdir_fsync_failure_prunes_all_created_dirs() -> None:
    checkpoint = load_checkpoint_module()
    prefix = f"out/checkpoint-mkdir-nested-fsync-{uuid.uuid4().hex}"
    path = ROOT / prefix / "nested/deep/file.txt"
    name = path.relative_to(ROOT).as_posix()
    states = {name: {"path": name, "exists": True, "mode": 0o600}}
    snapshots = {name: b"restored\n"}
    try:
        # fsync after the nested mkdir fails: reverse prune must remove both
        # created directories (deepest first) with no residue.
        with _parent_fsync_failure({2}):
            try:
                checkpoint._apply_restore(list(states), states, snapshots, {"roots": []})
            except OSError as error:
                assert "fsync" in str(error)
            else:
                raise AssertionError(
                    "injected nested mkdir fsync failure did not abort restore"
                )
        assert not (ROOT / prefix).exists(), "created dirs left behind"
    finally:
        shutil.rmtree(ROOT / prefix, ignore_errors=True)


def test_absent_nested_parent_and_symlink_special_preflight() -> None:
    checkpoint = load_checkpoint_module()
    prefix = PurePath(f"out/checkpoint-absence-{uuid.uuid4().hex}")
    absent = prefix / "missing/deep/file.txt"
    checkpoint._apply_restore(
        [absent.as_posix()], {}, {}, {"roots": []}
    )
    assert not (ROOT / prefix).exists(), "absent restore must not create parents"

    parent = ROOT / prefix
    parent.mkdir(parents=True)
    link = parent / "link"
    target = parent / "target"
    target.write_text("outside\n")
    link.symlink_to(target)
    checkpoint._apply_restore(
        [link.relative_to(ROOT).as_posix()], {}, {}, {"roots": []}
    )
    assert not link.exists() and target.read_text() == "outside\n"
    fifo = parent / "fifo"
    os.mkfifo(fifo)
    try:
        checkpoint._apply_restore(
            [fifo.relative_to(ROOT).as_posix()], {}, {}, {"roots": []}
        )
    except ValueError as error:
        assert "not removable" in str(error)
    else:
        raise AssertionError("special destination was accepted")
    assert fifo.exists()
    shutil.rmtree(parent)


def test_metric_semantics() -> None:
    lane_dir = ROOT / "out/lift-loop/checkpoints" / LANE
    probe = run("bin/asm-diff", "--json", SELECTOR)
    if probe.returncode not in (0, 1):
        print("attempt checkpoint metric checks: SKIP (asm-diff unavailable)")
        return
    first = run(
        "python3",
        str(SCRIPT),
        "capture",
        "--lane",
        LANE,
        "--selector",
        SELECTOR,
        "--attempt",
        "1",
        "--match",
        "83.33",
    )
    assert first.returncode == 0, first.stderr
    second = run(
        "python3",
        str(SCRIPT),
        "capture",
        "--lane",
        LANE,
        "--selector",
        SELECTOR,
        "--attempt",
        "2",
        "--match",
        "83.33",
        "--require-improvement",
        "--soft-no-improvement",
    )
    outcome = json.loads(second.stdout)
    assert (
        second.returncode == 0
        and outcome["accepted"] is False
        and outcome["exit_code"] == 1
    )
    third = run(
        "python3",
        str(SCRIPT),
        "capture",
        "--lane",
        LANE,
        "--selector",
        SELECTOR,
        "--attempt",
        "3",
        "--match",
        "0",
        "--require-improvement",
        "--soft-no-improvement",
    )
    assert third.returncode == 2, third.stdout
    subprocess.run(("rm", "-rf", str(lane_dir)), check=True)


def test_portable_unique_checkpoint_round_trip() -> None:
    lane = LANE + "-portable"
    lane_dir = ROOT / "out/lift-loop/checkpoints" / lane
    source = ROOT / "docs/agents/checkpoint-portable-test.md"
    source.write_text("first\n", encoding="utf-8")
    try:
        leaves = []
        for content in ("first\n", "second\n"):
            source.write_text(content, encoding="utf-8")
            captured = run(
                "python3",
                str(SCRIPT),
                "capture",
                "--lane",
                lane,
                "--selector",
                SELECTOR,
                "--attempt",
                "21",
                "--replace",
                "--paths-only",
                source.relative_to(ROOT).as_posix(),
            )
            assert captured.returncode == 0, captured.stderr
            leaves.append(json.loads(captured.stdout)["checkpoint"])
        assert leaves[0] != leaves[1]
        assert all((lane_dir / leaf / "complete.json").is_file() for leaf in leaves)
        incomplete = lane_dir / "attempt-22-deadbeefdeadbeef"
        incomplete.mkdir()
        (incomplete / "record.json").write_text("{}")
        source.write_text("changed\n")
        restored = run(
            "python3",
            str(SCRIPT),
            "restore",
            "--lane",
            lane,
            "--checkpoint",
            leaves[0],
        )
        assert restored.returncode == 0, restored.stderr
        assert source.read_text() == "first\n"
        snapshot = lane_dir / leaves[0] / "files" / source.relative_to(ROOT)
        snapshot.write_text("corrupt\n")
        source.write_text("must remain\n")
        rejected = run(
            "python3",
            str(SCRIPT),
            "restore",
            "--lane",
            lane,
            "--checkpoint",
            leaves[0],
        )
        assert rejected.returncode != 0
        assert "corrupt checkpoint" in rejected.stderr
        assert source.read_text() == "must remain\n"
        collision = run(
            "python3",
            str(SCRIPT),
            "capture",
            "--lane",
            lane,
            "--selector",
            SELECTOR,
            "--attempt",
            "23",
            "--paths-only",
            source.relative_to(ROOT).as_posix(),
        )
        assert collision.returncode == 0
        repeated = run(
            "python3",
            str(SCRIPT),
            "capture",
            "--lane",
            lane,
            "--selector",
            SELECTOR,
            "--attempt",
            "23",
            "--paths-only",
            source.relative_to(ROOT).as_posix(),
        )
        assert repeated.returncode == 0
        assert json.loads(repeated.stdout)["checkpoint"] == "attempt-23"
        (lane_dir / "attempt-23/complete.json").unlink()
        rejected = run(
            "python3",
            str(SCRIPT),
            "capture",
            "--lane",
            lane,
            "--selector",
            SELECTOR,
            "--attempt",
            "23",
            "--paths-only",
            source.relative_to(ROOT).as_posix(),
        )
        assert rejected.returncode != 0
        assert "corrupt checkpoint" in rejected.stderr
    finally:
        source.unlink(missing_ok=True)
        shutil.rmtree(lane_dir, ignore_errors=True)


def _assert_every_test_invoked(tests: tuple) -> None:
    defined = {
        name
        for name, value in list(globals().items())
        if name.startswith("test_") and callable(value)
    }
    missing = defined - {test.__name__ for test in tests}
    if missing:
        raise AssertionError(
            "test functions defined but not invoked by main: "
            + ", ".join(sorted(missing))
        )


def main() -> int:
    tests = (
        test_cleanup_scope_round_trip,
        test_rejects_checkpoint_root_lane_and_attempt_symlinks,
        test_rejects_component_substitution_at_every_checkpoint_boundary,
        test_rejects_links_special_files_and_noncanonical_paths,
        test_rejects_crafted_best_checkpoint_escape_and_binding_drift,
        test_no_replacement_deletion_and_recovery_retention,
        test_portable_unique_checkpoint_round_trip,
        test_transaction_preflight_rollback_and_fsync,
        test_parent_fsync_failure_after_replace_rolls_back_all_paths,
        test_parent_fsync_failure_after_unlink_restores_removed_path,
        test_rollback_fsync_failure_reports_error_but_attempts_all_paths,
        test_mkdir_fsync_failure_records_created_dir_and_prunes,
        test_nested_mkdir_fsync_failure_prunes_all_created_dirs,
        test_absent_nested_parent_and_symlink_special_preflight,
        test_metric_semantics,
    )
    _assert_every_test_invoked(tests)
    for test in tests:
        test()
    print("attempt checkpoint self-check: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
