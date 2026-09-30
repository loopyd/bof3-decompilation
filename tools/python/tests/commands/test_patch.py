"""Exercise patch command selection, exact states and recoverable publication."""

from __future__ import annotations

import difflib
from dataclasses import replace
import json
import subprocess

import pytest

from harness.commands import patch
from harness.patches import operations, targets
from harness.patches.git import git_text
from harness.patches.models import Target


@pytest.fixture
def workspace(tmp_path, monkeypatch):
    source = tmp_path / "source"
    source.mkdir()
    subprocess.run(["git", "init", "-q", str(source)], check=True)
    (source / "a.txt").write_text("one\n")
    (source / "b.txt").write_text("two\n")
    subprocess.run(["git", "-C", str(source), "add", "."], check=True)
    subprocess.run(
        [
            "git",
            "-C",
            str(source),
            "-c",
            "user.name=Patch fixture",
            "-c",
            "user.email=fixture@example.invalid",
            "-c",
            "commit.gpgsign=false",
            "commit",
            "-qm",
            "fixture",
        ],
        check=True,
    )
    revision = git_text(source, "rev-parse", "HEAD")
    folder = tmp_path / "inputs/patches/pcsx-redux"
    folder.mkdir(parents=True)

    def add(name, filename, before, after):
        text = "".join(
            difflib.unified_diff(
                before.splitlines(True),
                after.splitlines(True),
                fromfile="a/" + filename,
                tofile="b/" + filename,
            )
        )
        (folder / name).write_text(text)

    add("01.patch", "a.txt", "one\n", "ONE\n")
    add("02.patch", "b.txt", "two\n", "TWO\n")
    add("03.patch", "a.txt", "ONE\n", "THREE\n")
    target = Target(
        "pcsx-redux",
        tmp_path,
        source,
        revision,
        lambda files: {},
        lambda evidence: None,
    )
    monkeypatch.setattr(targets, "TARGETS", {"pcsx-redux": lambda layout: target})
    return target, folder


def invoke(target, action, *arguments):
    return patch.main(["--root", str(target.root), action, *arguments])


def test_list_without_checkout_or_status(workspace, monkeypatch, capsys):
    target, _ = workspace
    target.source.rename(target.root / "absent-checkout")

    def unavailable(layout):
        raise AssertionError("plain listing must not inspect a checkout")

    monkeypatch.setitem(targets.TARGETS, "pcsx-redux", unavailable)
    assert invoke(target, "list") == 0
    report = json.loads(capsys.readouterr().out)
    assert report["status_requested"] is False
    files = report["targets"][0]["patches"]
    assert [item["name"] for item in files] == ["01.patch", "02.patch", "03.patch"]
    assert all("status" not in item and item["bytes"] > 0 for item in files)


def test_list_status_and_scope(workspace, capsys):
    target, _ = workspace
    assert invoke(target, "apply", "--target", "pcsx-redux", "--patch", "02.patch") == 0
    capsys.readouterr()
    assert invoke(target, "list", "--status") == 0
    files = json.loads(capsys.readouterr().out)["targets"][0]["patches"]
    assert [item["status"] for item in files] == ["pristine", "applied", "pristine"]
    assert (
        invoke(
            target, "list", "--target", "pcsx-redux", "--patch", "02.patch", "--status"
        )
        == 0
    )
    files = json.loads(capsys.readouterr().out)["targets"][0]["patches"]
    assert [(item["name"], item["status"]) for item in files] == [
        ("02.patch", "applied")
    ]


def test_list_unresolved_status_preserves_files(workspace, capsys):
    target, _ = workspace
    (target.source / "a.txt").write_text("user edit\n")
    assert invoke(target, "list", "--status") == 2
    report = json.loads(capsys.readouterr().out)
    assert report["valid"] is False
    assert report["targets"][0]["error"]
    assert {item["status"] for item in report["targets"][0]["patches"]} == {
        "unresolved"
    }
    assert (target.source / "a.txt").read_text() == "user edit\n"


def test_list_unknown_target_without_status(workspace, capsys):
    target, folder = workspace
    folder.rename(folder.with_name("future"))
    assert invoke(target, "list") == 0
    row = json.loads(capsys.readouterr().out)["targets"][0]
    assert row["target"] == "future" and row["supported"] is False
    assert invoke(target, "list", "--status") == 2
    assert json.loads(capsys.readouterr().out)["valid"] is False


@pytest.mark.parametrize(
    "arguments",
    [
        ["--patch", "01.patch"],
        ["--target", "pcsx-redux", "--patch", "missing.patch"],
        ["--target", "pcsx-redux", "--patch", "01.patch", "--patch", "01.patch"],
    ],
)
def test_list_rejects_invalid_scope(workspace, arguments):
    target, _ = workspace
    assert invoke(target, "list", *arguments) == 2


def test_default_check_apply_revert_and_idempotency(workspace, capsys):
    target, _ = workspace
    assert invoke(target, "check") == 0
    assert (target.source / "a.txt").read_text() == "one\n"
    assert invoke(target, "apply") == 0
    assert (target.source / "a.txt").read_text() == "THREE\n"
    assert (target.source / "b.txt").read_text() == "TWO\n"
    capsys.readouterr()
    assert invoke(target, "apply") == 0
    assert json.loads(capsys.readouterr().out)["targets"][0]["changed"] == []
    assert invoke(target, "revert") == 0
    assert (target.source / "a.txt").read_text() == "one\n"
    assert (target.source / "b.txt").read_text() == "two\n"
    assert git_text(target.source, "status", "--porcelain") == ""


@pytest.mark.parametrize("named_order", [False, True])
def test_patch_scope_preserves_other_patches_and_requires_dependencies(
    workspace, monkeypatch, named_order
):
    target, folder = workspace
    first, last = "01.patch", "03.patch"
    if named_order:
        first, last = "z-base.patch", "a-dependent.patch"
        (folder / "01.patch").rename(folder / first)
        (folder / "03.patch").rename(folder / last)
        target = replace(target, patch_order=(first, "02.patch", last))
        monkeypatch.setitem(targets.TARGETS, "pcsx-redux", lambda layout: target)
    assert invoke(target, "apply", "--target", "pcsx-redux", "--patch", "02.patch") == 0
    assert (target.source / "a.txt").read_text() == "one\n"
    assert invoke(target, "apply", "--target", "pcsx-redux", "--patch", last) == 2
    assert invoke(target, "apply") == 0
    assert invoke(target, "revert", "--target", "pcsx-redux", "--patch", first) == 2
    assert invoke(target, "revert", "--target", "pcsx-redux", "--patch", last) == 0
    assert (target.source / "a.txt").read_text() == "ONE\n"
    assert (target.source / "b.txt").read_text() == "TWO\n"


@pytest.mark.parametrize(
    "arguments",
    [
        ["--patch", "01.patch"],
        ["--target", "unknown"],
        ["--target", "pcsx-redux", "--patch", "unknown.patch"],
        ["--target", "pcsx-redux", "--target", "pcsx-redux"],
    ],
)
def test_invalid_scope_changes_nothing(workspace, arguments):
    target, _ = workspace
    assert invoke(target, "apply", *arguments) == 2
    assert git_text(target.source, "status", "--porcelain") == ""


def test_local_source_edits_are_preserved(workspace):
    target, _ = workspace
    (target.source / "a.txt").write_text("user work\n")
    assert invoke(target, "apply") == 2
    assert (target.source / "a.txt").read_text() == "user work\n"
    assert (target.source / "b.txt").read_text() == "two\n"


def test_publication_failure_rolls_back_owned_bytes(workspace, monkeypatch):
    target, _ = workspace
    publish = operations.publish_file

    def fail(path, data, mode):
        if path == target.source / "b.txt" and data == b"TWO\n":
            raise OSError("fixture publication failure")
        publish(path, data, mode)

    monkeypatch.setattr(operations, "publish_file", fail)
    assert invoke(target, "apply") == 2
    assert git_text(target.source, "status", "--porcelain") == ""
    receipts = list((target.root / "out/patches").glob("*/receipt.json"))
    assert (
        len(receipts) == 1 and json.loads(receipts[0].read_text())["status"] == "failed"
    )


def test_rollback_preserves_concurrent_edits(workspace, monkeypatch):
    target, _ = workspace
    publish = operations.publish_file

    def change(path, data, mode):
        if path == target.source / "a.txt" and data == b"THREE\n":
            (target.source / "b.txt").write_text("concurrent\n")
        publish(path, data, mode)

    monkeypatch.setattr(operations, "publish_file", change)
    assert invoke(target, "apply") == 2
    assert (target.source / "a.txt").read_text() == "one\n"
    assert (target.source / "b.txt").read_text() == "concurrent\n"


def test_unsafe_patch_paths_reject_before_publication(workspace):
    target, folder = workspace
    (folder / "04.patch").write_text(
        "--- a/../escape\n+++ b/../escape\n@@ -1 +1 @@\n-a\n+b\n"
    )
    assert invoke(target, "apply") == 2
    assert git_text(target.source, "status", "--porcelain") == ""
