from argparse import Namespace
from pathlib import Path

import pytest

from harness.commands import analysis_readiness, index


def test_analysis_readiness_reports_unavailable_work_graphs_without_index(
    tmp_path: Path, monkeypatch
) -> None:
    monkeypatch.setattr(
        analysis_readiness,
        "load_target_manifests",
        lambda _root: {"emi/test/archive/00": object()},
    )
    monkeypatch.setattr(
        analysis_readiness,
        "project_status",
        lambda _root, target: {"target": target, "fresh": False},
    )
    monkeypatch.setattr(
        analysis_readiness,
        "_summaries",
        lambda *_args: (_ for _ in ()).throw(FileNotFoundError("missing index")),
    )

    payload = analysis_readiness.readiness(tmp_path)

    assert payload["schema"] == "bof3.analysis-readiness/v2"
    assert payload["ready"] is False
    assert payload["stale_facts"] == {
        "count": 2,
        "targets": ["emi/test/archive/00"],
        "index": True,
    }
    summaries = payload["summaries"]
    assert isinstance(summaries, dict)
    assert summaries["naming"] == {
        "available": False,
        "error": "missing index",
    }
    assert payload["recovery"] == "bin/index --recover"


def test_analysis_readiness_includes_naming_type_macro_work_graphs(
    tmp_path: Path, monkeypatch
) -> None:
    monkeypatch.setattr(
        analysis_readiness,
        "load_target_manifests",
        lambda _root: {"emi/test/archive/00": object()},
    )
    monkeypatch.setattr(
        analysis_readiness,
        "project_status",
        lambda _root, target: {"target": target, "fresh": True},
    )
    summaries = {
        concern: {"work_graph": []} for concern in ("naming", "types", "macros")
    }
    monkeypatch.setattr(analysis_readiness, "_summaries", lambda *_args: summaries)

    payload = analysis_readiness.readiness(tmp_path, "emi/test/archive/00")

    assert payload["ready"] is True
    assert payload["stale_facts"] == {"count": 0, "targets": [], "index": False}
    assert payload["summaries"] == summaries
    assert payload["recovery"] is None


def test_recover_reanalyzes_every_stale_target_before_rebuild(
    tmp_path: Path, monkeypatch
) -> None:
    calls: list[tuple[str, str]] = []
    monkeypatch.setattr(
        index,
        "load_target_manifests",
        lambda _root: {"fresh": object(), "stale/a": object(), "stale/b": object()},
    )
    recovered: set[str] = set()
    monkeypatch.setattr(
        index,
        "status",
        lambda _root, target: {"fresh": target == "fresh" or target in recovered},
    )

    def analyze(_root: Path, target: str, timeout: int) -> None:
        calls.append(("analyze", target))
        recovered.add(target)

    monkeypatch.setattr(index, "analyze_project", analyze)

    def rebuild(_root: Path) -> Path:
        calls.append(("rebuild", "index"))
        output = tmp_path / "out/index/reverse.sqlite"
        output.parent.mkdir(parents=True)
        output.touch()
        return output

    monkeypatch.setattr(index, "rebuild", rebuild)

    assert index.run(Namespace(root=tmp_path, recover=True, timeout=5)) == 0
    assert calls == [
        ("analyze", "stale/a"),
        ("analyze", "stale/b"),
        ("rebuild", "index"),
    ]


def test_recover_rechecks_targets_that_were_initially_fresh(
    tmp_path: Path, monkeypatch
) -> None:
    calls = {"fresh": 0}
    rebuilt = False
    monkeypatch.setattr(
        index,
        "load_target_manifests",
        lambda _root: {"fresh": object(), "stale": object()},
    )

    def status(_root: Path, target: str) -> dict[str, bool]:
        if target == "fresh":
            calls["fresh"] += 1
            return {"fresh": calls["fresh"] == 1}
        return {"fresh": False}

    monkeypatch.setattr(index, "status", status)
    monkeypatch.setattr(index, "analyze_project", lambda *_args, **_kwargs: None)

    def rebuild(_root: Path) -> Path:
        nonlocal rebuilt
        rebuilt = True
        return tmp_path / "out/index/reverse.sqlite"

    monkeypatch.setattr(index, "rebuild", rebuild)
    with pytest.raises(ValueError, match="fresh, stale"):
        index.run(Namespace(root=tmp_path, recover=True, timeout=5))
    assert calls["fresh"] == 2
    assert rebuilt is False


def test_recover_refuses_rebuild_when_target_remains_stale(
    tmp_path: Path, monkeypatch
) -> None:
    rebuilt = False
    monkeypatch.setattr(
        index, "load_target_manifests", lambda _root: {"stale": object()}
    )
    monkeypatch.setattr(index, "status", lambda *_: {"fresh": False})
    monkeypatch.setattr(index, "analyze_project", lambda *_args, **_kwargs: None)

    def rebuild(_root: Path) -> Path:
        nonlocal rebuilt
        rebuilt = True
        return tmp_path / "out/index/reverse.sqlite"

    monkeypatch.setattr(index, "rebuild", rebuild)

    with pytest.raises(ValueError, match="stale targets: stale"):
        index.run(Namespace(root=tmp_path, recover=True, timeout=5))
    assert rebuilt is False
