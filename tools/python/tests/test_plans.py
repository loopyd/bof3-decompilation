"""Behavioral plan parsing, persistence, and recoverable consolidation checks."""

from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path

import pytest

from harness.commands import _plans

ROOT = Path(__file__).resolve().parents[3]


def document(state="open"):
    return f"""<!-- bof3.plan/v1 -->
# Synthetic plan
## 1. [A1] ({state}) Phase
- Owner: reviewer
- Depends: none
- Blocker: none
- Evidence: test receipt
- Acceptance: check fixture
1. [A1.01] ({state}) Step
- Owner: worker
- Depends: none
- Blocker: none
- Evidence: test receipt
- Acceptance: check fixture
"""


def fixture(tmp_path):
    root = tmp_path / "repo"
    directory = root / "docs/plans"
    directory.mkdir(parents=True)
    for name in ("canonical.md", "old.md", "other.md"):
        (directory / name).write_text("# Original\n\nUnfinished obligation\n")
    candidate = tmp_path / "candidate.md"
    candidate.write_text(document())
    sources = _plans.inventory(root)
    data = {
        "schema": "bof3.plan-review/v1",
        "canonical": "canonical.md",
        "candidate": {
            "path": str(candidate),
            "sha256": _plans.digest(candidate.read_bytes()),
        },
        "reviewer": "independent synthetic reviewer",
        "evidence": "synthetic review receipt",
        "sources": [
            {"name": name, "sha256": _plans.digest(raw)}
            for name, raw in sources.items()
        ],
        "mappings": [
            {
                "source": name,
                "start": 1,
                "end": 3,
                "label": "Original",
                "destination": "A1.01",
                "disposition": "retained",
                "evidence": "synthetic mapping review",
            }
            for name in sources
        ],
        "reconciliations": [],
    }
    review = tmp_path / "review.json"
    review.write_text(json.dumps(data))
    return root, review, candidate, data, sources


def cli(root, *args):
    return subprocess.run(
        [sys.executable, "-m", "harness.commands.plans", "--root", str(root), *args],
        capture_output=True,
        text=True,
        env=os.environ
        | {"PYTHONPATH": str(ROOT / "tools/python"), "PYTHONDONTWRITEBYTECODE": "1"},
    )


def test_persistence_preview_apply_and_replay(tmp_path):
    root, review, candidate, _, sources = fixture(tmp_path)
    assert cli(root, "consolidate", str(review)).returncode == 0
    assert _plans.inventory(root) == sources
    backup = tmp_path / "backup"
    assert (
        cli(
            root, "consolidate", str(review), "--apply", "--backup-dir", str(backup)
        ).returncode
        == 0
    )
    assert _plans.inventory(root) == {"canonical.md": candidate.read_bytes()}
    for name, raw in sources.items():
        assert (backup / "sources" / name).read_bytes() == raw
    assert cli(root, "consolidate", str(review)).returncode == 0
    review.unlink()
    first = cli(root, "status")
    assert first.returncode == 0 and "A1.01 (open)" in first.stdout
    assert cli(root, "status").stdout == first.stdout
    (root / "docs/plans/canonical.md").write_text(document("done"))
    assert "A1.01 (done)" in cli(root, "status").stdout
    assert '"done": 1' in cli(root, "list").stdout


@pytest.mark.parametrize(
    "change",
    [
        lambda s: s.replace("[A1.01]", "[A1]"),
        lambda s: s.replace("(open)", "(unknown)"),
        lambda s: s.replace("Depends: none", "Depends: MISSING", 1),
        lambda s: s.replace("Depends: none", "Depends: A1", 1),
        lambda s: s.replace("Owner: worker", "Owner:"),
        lambda s: s.replace("(open)", "(done)", 1),
        lambda s: s.replace("Evidence: test receipt", "Evidence: none").replace(
            "(open)", "(done)"
        ),
        lambda s: s + "```\n",
        lambda s: "",
    ],
)
def test_malformed_plan(change):
    with pytest.raises(ValueError):
        _plans.parse_plan(change(document()))


def test_fenced_examples_ignored():
    parsed = _plans.parse_plan(document() + "```\n1. [FAKE] (done) PASS\n```\n")
    assert [i.id for i in parsed.items] == ["A1", "A1.01"]


def test_empty_multiple_and_singular_options(tmp_path):
    root = tmp_path
    assert cli(root, "list").returncode == 0
    assert cli(root, "status").returncode == 2
    directory = root / "docs/plans"
    directory.mkdir(parents=True)
    for name in ("a.md", "b.md"):
        (directory / name).write_text(document())
    assert cli(root, "status").returncode == 2
    assert cli(root, "status", "a.md").returncode == 0
    for args in [
        ("status", "../a.md"),
        ("status", ""),
        ("status", "a.md", "b.md"),
        ("--root", str(root), "list"),
        ("consolidate",),
        ("consolidate", "a", "b"),
        ("consolidate", "", "--apply", "--apply"),
    ]:
        assert cli(root, *args).returncode == 2


@pytest.mark.parametrize(
    "kind",
    [
        "source",
        "candidate",
        "extra",
        "missing",
        "gap",
        "overlap",
        "destination",
        "duplicate-key",
    ],
)
def test_stale_and_bad_review_no_writes(tmp_path, kind):
    root, review, candidate, data, _ = fixture(tmp_path)
    directory = root / "docs/plans"
    if kind == "source":
        (directory / "old.md").write_text("changed")
    elif kind == "candidate":
        candidate.write_text(document("done"))
    elif kind == "extra":
        (directory / "new.md").write_text("new")
    elif kind == "missing":
        (directory / "old.md").unlink()
    elif kind == "gap":
        data["mappings"][0]["end"] = 2
    elif kind == "overlap":
        data["mappings"].append(data["mappings"][0])
    elif kind == "destination":
        data["mappings"][0]["destination"] = "MISSING"
    review.write_text(json.dumps(data))
    if kind == "duplicate-key":
        review.write_text(
            review.read_text().replace(
                '{"schema":', '{"schema":"duplicate","schema":', 1
            )
        )
    before = _plans.inventory(root)
    assert (
        cli(
            root,
            "consolidate",
            str(review),
            "--apply",
            "--backup-dir",
            str(tmp_path / "backup"),
        ).returncode
        == 2
    )
    assert _plans.inventory(root) == before
    assert not (tmp_path / "backup").exists()


@pytest.mark.parametrize(
    "kind", ["source", "candidate", "review", "ancestor", "hardlink", "backup"]
)
def test_unsafe_paths(tmp_path, kind):
    root, review, candidate, _, sources = fixture(tmp_path)
    backup = tmp_path / "backup"
    if kind in ("source", "candidate", "review"):
        path = {
            "source": root / "docs/plans/old.md",
            "candidate": candidate,
            "review": review,
        }[kind]
        external = tmp_path / "external"
        path.rename(external)
        path.symlink_to(external)
    elif kind == "ancestor":
        (root / "docs").rename(root / "actual")
        (root / "docs").symlink_to(root / "actual", target_is_directory=True)
    elif kind == "hardlink":
        os.link(root / "docs/plans/old.md", tmp_path / "hardlink")
    else:
        backup.symlink_to(tmp_path / "missing", target_is_directory=True)
    assert (
        cli(
            root, "consolidate", str(review), "--apply", "--backup-dir", str(backup)
        ).returncode
        == 2
    )
    assert (root / "docs/plans/canonical.md").read_bytes() == sources["canonical.md"]


@pytest.mark.parametrize("operation", ["copy", "fsync", "replace", "second-delete"])
def test_failure_restores_or_preserves_recovery(tmp_path, monkeypatch, operation):
    root, review, _, _, sources = fixture(tmp_path)
    backup = tmp_path / "backup"
    if operation == "copy":
        monkeypatch.setattr(
            _plans,
            "write_copy",
            lambda *a: (_ for _ in ()).throw(OSError("injected copy")),
        )
    elif operation == "fsync":
        monkeypatch.setattr(
            _plans.os,
            "fsync",
            lambda *a: (_ for _ in ()).throw(OSError("injected fsync")),
        )
    elif operation == "replace":
        monkeypatch.setattr(
            _plans.os,
            "replace",
            lambda *a: (_ for _ in ()).throw(OSError("injected replace")),
        )
    else:
        unlink = Path.unlink

        def fail(path, *args, **kwargs):
            if path.name == "other.md":
                raise OSError("injected second deletion")
            return unlink(path, *args, **kwargs)

        monkeypatch.setattr(Path, "unlink", fail)
    with pytest.raises(ValueError, match="recovery:"):
        _plans.consolidate(root, review, True, backup)
    assert _plans.inventory(root) == sources


def test_crash_after_publication_has_all_originals(tmp_path, monkeypatch):
    root, review, candidate, _, sources = fixture(tmp_path)
    publish = _plans.publish

    def crash(*args):
        publish(*args)
        raise KeyboardInterrupt("simulated crash")

    monkeypatch.setattr(_plans, "publish", crash)
    backup = tmp_path / "backup"
    with pytest.raises(KeyboardInterrupt):
        _plans.consolidate(root, review, True, backup)
    assert (root / "docs/plans/canonical.md").read_bytes() == candidate.read_bytes()
    for name, raw in sources.items():
        assert (backup / "sources" / name).read_bytes() == raw
    with pytest.raises(ValueError, match="stale/partial"):
        _plans.validate(root, review)


@pytest.mark.parametrize("value", [[], {}, None, 7, True])
def test_mapping_source_types_fail_cleanly(tmp_path, value):
    root, review, _, data, sources = fixture(tmp_path)
    data["mappings"][0]["source"] = value
    review.write_text(json.dumps(data))
    result = cli(
        root,
        "consolidate",
        str(review),
        "--apply",
        "--backup-dir",
        str(tmp_path / "backup"),
    )
    assert result.returncode == 2 and "invalid coverage range" in result.stderr
    assert "Traceback" not in result.stderr
    assert _plans.inventory(root) == sources and not (tmp_path / "backup").exists()


@pytest.mark.parametrize("replay", [False, True])
def test_huge_ranges_are_bounded(tmp_path, replay):
    root, review, _, data, sources = fixture(tmp_path)
    if replay:
        _plans.consolidate(root, review, True, tmp_path / "original-backup")
    data["mappings"][0]["end"] = 10**12
    review.write_text(json.dumps(data))
    result = cli(root, "consolidate", str(review))
    assert result.returncode == (0 if replay else 2)
    assert "Traceback" not in result.stderr
    if not replay:
        assert "out-of-range" in result.stderr and _plans.inventory(root) == sources


@pytest.mark.parametrize("value", ["z" * 64, [], None, 64, "a" * 63])
def test_invalid_hashes(tmp_path, value):
    root, review, _, data, sources = fixture(tmp_path)
    for holder in [data["candidate"], data["sources"][0]]:
        original = holder["sha256"]
        holder["sha256"] = value
        review.write_text(json.dumps(data))
        result = cli(root, "consolidate", str(review))
        assert result.returncode == 2 and "Traceback" not in result.stderr
        assert _plans.inventory(root) == sources
        holder["sha256"] = original


def test_one_way_ownership_including_lazy_imports():
    import ast

    directory = ROOT / "tools/python/harness/commands"
    for name in ["plans", "_plans"]:
        tree = ast.parse((directory / f"{name}.py").read_text())
        imports = [node for node in ast.walk(tree) if isinstance(node, ast.ImportFrom)]
        assert not any(
            node.module == "plans" or any(alias.name == "plans" for alias in node.names)
            for node in imports
        )
        assert len((directory / f"{name}.py").read_text().splitlines()) <= 450


def test_inherited_and_unit_dependencies(tmp_path):
    text = document() + document().split("# Synthetic plan\n")[1].replace(
        "A1", "B1"
    ).replace("Depends: none", "Depends: A1@unit", 1)
    parsed = _plans.parse_plan(text)
    assert parsed.items[-1].parent == "B1"
    directory = tmp_path / "docs/plans"
    directory.mkdir(parents=True)
    (directory / "canonical.md").write_text(text)
    assert "Inherited Depends: A1@unit" in cli(tmp_path, "status").stdout
    with pytest.raises(ValueError, match="cycle"):
        _plans.parse_plan(text.replace("Depends: A1@unit", "Depends: B1.01@unit"))


def test_blocked_gate_fields_survive_status(tmp_path):
    text = (
        document()
        .replace("(open)", "(blocked)")
        .replace(
            "Blocker: none",
            "Blocker: missing original proof; next independent access review",
        )
        .replace("Evidence: test receipt", "Evidence: source.md:102 and receipt hash")
    )
    directory = tmp_path / "docs/plans"
    directory.mkdir(parents=True)
    (directory / "canonical.md").write_text(text)
    parsed = _plans.parse_plan(text)
    result = cli(tmp_path, "status")
    assert result.returncode == 0
    for item in parsed.items:
        assert f"{item.id} ({item.state})" in result.stdout
        for key, value in item.fields.items():
            assert f"{key}: {value}" in result.stdout
