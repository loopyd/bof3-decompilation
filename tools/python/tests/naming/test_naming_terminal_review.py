"""Terminal CLI/owner acceptance remains separate from structural exhaustion."""

from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path

import pytest
from harness.naming.cli import build_parser
from harness.naming.conclusion import import_conclusion
from harness.naming.inputs import digest
from harness.naming.terminal import (
    PARENT_SCHEMA,
    PREPARATION_SCHEMA,
    REVIEW_SCHEMA,
    terminal_binding,
    verify_terminal,
)
from naming_synthetic_fixture import SYNTHETIC_ROW, SYNTHETIC_TARGET
from test_naming_conclusion import _synthetic_authored


def _write(path, value):
    path.write_text(json.dumps(value))
    return {"path": str(path), "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}


@pytest.fixture
def terminal(synthetic_exact_evidence):
    fixture = synthetic_exact_evidence
    root, report = fixture.root, fixture.report
    authored = _synthetic_authored(report)
    source = root / "conclusion.json"
    _write(source, authored)
    import_conclusion(root, SYNTHETIC_TARGET, report, source, registry=fixture.registry)
    arguments = (root, SYNTHETIC_TARGET, report, f"data:{SYNTHETIC_ROW}")
    binding = terminal_binding(*arguments, registry=fixture.registry)
    ceiling = _write(
        root / "ceiling.json",
        {"fixture": "Synthetic reviewed finite static ceiling; no BOF3 acceptance"},
    )
    rationale = {
        "missing_fact": binding["missing_fact"],
        "explanation": "Synthetic independent review inspected the finite fixture: no further executable lead; raw name retained because no independent semantic corroboration exists.",
        "evidence": [ceiling],
    }
    preparation = {
        "schema": PREPARATION_SCHEMA,
        "evidence_preparation_run_id": "fixture-preparer",
        "binding": binding,
    }
    review = {
        "schema": REVIEW_SCHEMA,
        "reviewer_run_id": "fixture-reviewer",
        "binding": binding,
        "ladder_exhausted": True,
        "unresolved_leads": [],
        "ceiling_rationale": rationale,
    }
    parent = {
        "schema": PARENT_SCHEMA,
        "accepted": True,
        "parent_run_id": "fixture-parent",
        "evidence_preparation_run_id": "fixture-preparer",
        "reviewer_run_id": "fixture-reviewer",
        "binding": binding,
        "preparation_artifact": _write(root / "preparation.json", preparation),
        "review_artifact": _write(root / "review.json", review),
        "ladder_exhausted": True,
        "unresolved_leads": [],
        "ceiling_rationale": rationale,
    }
    path = root / "parent.json"
    _write(path, parent)
    return fixture, arguments, path, parent


def test_terminal_review_positive_replay_and_cli(terminal, monkeypatch, capsys):
    fixture, arguments, path, parent = terminal
    original = fixture.report.read_bytes()
    result = verify_terminal(
        *arguments, path, digest(parent), registry=fixture.registry
    )
    assert result["selected_row_accepted"] is True
    assert result["full_report"]["complete"] is False
    assert result["production_complete"] is False
    assert (
        verify_terminal(*arguments, path, digest(parent), registry=fixture.registry)
        == result
    )
    from harness.naming import terminal as terminal_review

    real = terminal_review.verify_terminal
    monkeypatch.setattr(
        terminal_review,
        "verify_terminal",
        lambda *a: real(*a, registry=fixture.registry),
    )
    args = build_parser().parse_args(
        [
            "--root",
            str(fixture.root),
            "terminal-verify",
            SYNTHETIC_TARGET,
            str(fixture.report),
            "--transaction",
            f"data:{SYNTHETIC_ROW}",
            "--parent-attestation",
            str(path),
            "--expected-parent-digest",
            digest(parent),
        ]
    )
    assert args.handler(args) == 0
    assert json.loads(capsys.readouterr().out) == result
    assert fixture.report.read_bytes() == original


@pytest.mark.parametrize(
    "mutation",
    [
        "structural-only",
        "open-leads",
        "false-ladder",
        "selfreview",
        "parent-selfreview",
        "wrong-pin",
        "stale-report",
        "stale-row",
        "wrong-selector",
        "missing-review",
        "replaced-review",
        "contrary-review",
        "preparation-binding",
        "preparation-id",
        "missing-ceiling",
        "empty-rationale",
        "wrong-missing-fact",
        "unknown-parent",
        "duplicate-parent",
        "current-tooling",
        "current-source",
        "retained-evidence",
        "derived-open",
        "derived-unavailable",
        "numeric-review-ladder",
        "missing-preparation",
        "empty-ceiling",
        "stale-index",
    ],
)
def test_terminal_review_rejects_unaccepted_or_stale(terminal, mutation):
    fixture, arguments, path, parent = terminal
    original = fixture.report.read_bytes()
    pin = None
    if mutation == "structural-only":
        parent = {"binding": parent["binding"]}
    elif mutation == "open-leads":
        parent["unresolved_leads"] = ["inspect remaining handler"]
    elif mutation == "false-ladder":
        parent["ladder_exhausted"] = False
    elif mutation == "selfreview":
        parent["reviewer_run_id"] = parent["evidence_preparation_run_id"]
    elif mutation == "parent-selfreview":
        parent["parent_run_id"] = parent["reviewer_run_id"]
    elif mutation == "wrong-pin":
        pin = "0" * 64
    elif mutation == "stale-report":
        fixture.report.write_bytes(original + b"\n")
    elif mutation == "stale-row":
        parent["binding"]["row_digest"] = "0" * 64
    elif mutation == "wrong-selector":
        parent["binding"]["selector"] = "exe/test@0x80100084"
    elif mutation == "missing-review":
        Path(parent["review_artifact"]["path"]).unlink()
    elif mutation == "replaced-review":
        Path(parent["review_artifact"]["path"]).write_text("replaced")
    elif mutation in {"contrary-review", "preparation-binding", "preparation-id"}:
        key = (
            "review_artifact"
            if mutation == "contrary-review"
            else "preparation_artifact"
        )
        artifact = Path(parent[key]["path"])
        value = json.loads(artifact.read_text())
        if mutation == "contrary-review":
            value["ladder_exhausted"] = False
            value["unresolved_leads"] = ["executable handler lead"]
        elif mutation == "preparation-binding":
            value["binding"]["row_digest"] = "0" * 64
        else:
            value["evidence_preparation_run_id"] = "different-preparer"
        parent[key] = _write(artifact, value)
    elif mutation == "missing-ceiling":
        Path(parent["ceiling_rationale"]["evidence"][0]["path"]).unlink()
    elif mutation == "empty-rationale":
        parent["ceiling_rationale"]["explanation"] = ""
    elif mutation == "wrong-missing-fact":
        parent["ceiling_rationale"]["missing_fact"] = "other"
    elif mutation == "unknown-parent":
        parent["extra"] = True
    elif mutation == "current-tooling":
        tooling = fixture.root / "tools/python/harness/new.py"
        tooling.parent.mkdir(parents=True)
        tooling.write_text("# changed tooling\n")
    elif mutation == "current-source":
        (fixture.root / "include/base/types.h").write_text(
            "typedef unsigned short u32;\n"
        )
    elif mutation == "numeric-review-ladder":
        artifact = Path(parent["review_artifact"]["path"])
        value = json.loads(artifact.read_text())
        value["ladder_exhausted"] = 1
        parent["review_artifact"] = _write(artifact, value)
    elif mutation == "missing-preparation":
        Path(parent["preparation_artifact"]["path"]).unlink()
    elif mutation == "empty-ceiling":
        artifact = Path(parent["ceiling_rationale"]["evidence"][0]["path"])
        artifact.write_text("")
    elif mutation == "stale-index":
        from harness.analysis.index import index_path

        with index_path(fixture.root).open("ab") as stream:
            stream.write(b"stale index")
    elif mutation in {"derived-open", "derived-unavailable"}:
        evidence = next((fixture.root / "out/reviews/evidence").rglob("derived.json"))
        value = json.loads(evidence.read_text())
        value[mutation.removeprefix("derived-")] = [
            {"reason": "unresolved executable lead"}
        ]
        _write(evidence, value)
    elif mutation == "retained-evidence":
        evidence = next((fixture.root / "out/reviews/evidence").rglob("derived.json"))
        evidence.write_text(evidence.read_text() + "\n")
    _write(path, parent)
    if mutation == "duplicate-parent":
        path.write_text(path.read_text()[:-1] + ', "accepted": true}')
    with pytest.raises(ValueError):
        verify_terminal(
            *arguments, path, pin or digest(parent), registry=fixture.registry
        )
    assert fixture.report.read_bytes() == (
        original + b"\n" if mutation == "stale-report" else original
    )


def test_terminal_review_separates_fullreport_blocker(terminal):
    fixture, arguments, path, parent = terminal
    report = json.loads(fixture.report.read_text())
    other = next(row for row in report["rows"] if row["kind"] == "function")
    other["rung_status"] = "proposed"
    _write(fixture.report, report)
    binding = terminal_binding(*arguments, registry=fixture.registry)
    parent = copy.deepcopy(parent)
    parent["binding"] = binding
    for key in ("preparation_artifact", "review_artifact"):
        artifact = Path(parent[key]["path"])
        value = json.loads(artifact.read_text())
        value["binding"] = binding
        parent[key] = _write(artifact, value)
    _write(path, parent)
    result = verify_terminal(
        *arguments, path, digest(parent), registry=fixture.registry
    )
    assert result["selected_row_accepted"] is True
    assert result["full_report"]["blocker"]
    assert result["full_report"]["complete"] is False
    assert result["production_complete"] is False
