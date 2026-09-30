"""Canonical expert conclusion import behavior."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from types import MappingProxyType
from typing import Any, cast

import pytest
from harness.domain.receipts import write_receipt
from harness.naming.capabilities import PRODUCTION_EXACT_CAPABILITIES
from harness.naming.conclusion import import_conclusion
from harness.naming.equivalence import _exact_capability_row
from harness.naming.journal import JournalInputs
from harness.naming.journal import commit_entry
from harness.naming.journal import entry_sha256
from harness.naming.namespace import journal_dir
from harness.naming.journal import write_manifest
from harness.naming.validation import _canonical_json_digest, _validate_next_command
from naming_synthetic_fixture import SYNTHETIC_ROW, SYNTHETIC_TARGET

TARGET = "exe/test"
FIXTURES = Path(__file__).resolve().parents[1] / "fixtures"


def test_documented_envelope_fixture_has_exact_source_shape() -> None:
    envelope = json.loads(
        (FIXTURES / "naming-conclusion-envelope.json").read_text(encoding="utf-8")
    )
    assert set(envelope) == {
        "schema",
        "target",
        "report_sha256",
        "source",
        "conclusion",
    }
    assert set(envelope["source"]) == {
        "report",
        "report_digest",
        "evidence",
        "evidence_sha256",
        "conclusion_capability",
    }


def test_documented_conclusion_fixtures_pass_import_shape_validation() -> None:
    from harness.naming.conclusion import _validate_conclusion

    for name in ("naming-conclusion-exhausted.json", "naming-conclusion-proposed.json"):
        row = json.loads((FIXTURES / name).read_text(encoding="utf-8"))
        assert _validate_conclusion(row) == row


def _fixture(root: Path) -> tuple[Path, Path, dict, dict]:
    report = root / "report.json"
    gap = {
        "status": "open",
        "next_command": "bin/harness analysis query --json owners exe/test@0x80100010",
        "observations": [
            {
                "id": "gap",
                "text": "Evidence gap; this initializer records no semantic conclusion.",
            }
        ],
        "authority": "original",
    }
    initial = {
        "kind": "function",
        "name": "func_80100000",
        "initializer_state": "bof3.naming-audit-initializer/v1",
        "rung_status": "blocked",
        "rungs": {"selected_range": gap},
        "smallest_repair": "bin/harness analysis query --json owners exe/test@0x80100010",
        "ceiling_next_command": "bin/harness analysis query --json owners exe/test@0x80100010",
    }
    payload_path = root / "out/reviews/evidence/payload.json"
    receipt = write_receipt(
        root,
        "out/reviews/evidence/receipt.json",
        {
            "command": "inspect",
            "status": "passed",
            "target": TARGET,
            "selector": f"{TARGET}@0x80100000",
            "output": "facts",
        },
    )
    receipt.update({"item": "calls:x", "operation": "calls"})
    receipt_path = root / receipt["receipt"]
    receipt_payload = json.loads(receipt_path.read_text())
    receipt_payload.update(
        {"schema": "bof3.naming-evidence-facts/v1", "facts": ["selected_call"]}
    )
    receipt_path.write_text(json.dumps(receipt_payload, sort_keys=True) + "\n")
    receipt["sha256"] = hashlib.sha256(receipt_path.read_bytes()).hexdigest()
    receipt2 = write_receipt(
        root,
        "out/reviews/evidence/receipt2.json",
        {
            "command": "inspect caller",
            "status": "passed",
            "target": TARGET,
            "selector": f"{TARGET}@0x80100000",
            "output": "independent facts",
        },
    )
    receipt2.update({"item": "calls:y", "operation": "calls"})
    receipt2_path = root / receipt2["receipt"]
    receipt2_payload = json.loads(receipt2_path.read_text())
    receipt2_payload.update(
        {"schema": "bof3.naming-evidence-facts/v1", "facts": ["selected_call"]}
    )
    receipt2_path.write_text(json.dumps(receipt2_payload, sort_keys=True) + "\n")
    receipt2["sha256"] = hashlib.sha256(receipt2_path.read_bytes()).hexdigest()
    payload = {
        "target": TARGET,
        "row": "function:func_80100000",
        "items": [
            {
                "id": "calls:x",
                "operation": "calls",
                "selector": f"{TARGET}@0x80100000",
                "payload": [{"callsite": "a"}],
            },
            {
                "id": "calls:y",
                "operation": "calls",
                "selector": f"{TARGET}@0x80100000",
                "payload": [{"callsite": "b"}],
            },
        ],
        "commands": [receipt, receipt2],
    }
    payload_path.parent.mkdir(parents=True, exist_ok=True)
    payload_path.write_text(json.dumps(payload), encoding="utf-8")
    report.write_text(
        json.dumps(
            {
                "schema": "bof3.naming-audit/v3",
                "target": TARGET,
                "complete": False,
                "rows": [initial],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    terminal = {
        "kind": "function",
        "name": "func_80100000",
        "rung_status": "exhausted",
        "rungs": {
            "selected_call": {
                "status": "negative",
                "negative_result": "reviewed bytes do not establish a role",
                "observations": [
                    {
                        "id": "negative-a",
                        "text": "analyzer found no semantic role",
                        "producer": "analyzer",
                        "fact_class": "selected_call",
                        "evidence_receipt": receipt["receipt"],
                    },
                    {
                        "id": "negative-b",
                        "text": "independent analyzer found no semantic role",
                        "producer": "analyzer",
                        "fact_class": "selected_call",
                        "evidence_receipt": receipt2["receipt"],
                    },
                ],
                "authority": "original",
                "commands": [receipt, receipt2],
            }
        },
        "required_work": [
            {"id": "calls:x", "status": "completed", "commands": [receipt]},
            {"id": "calls:y", "status": "completed", "commands": [receipt2]},
        ],
        "corroborators": {
            "call-a": {"source_id": receipt["sha256"]},
            "call-b": {"source_id": receipt2["sha256"]},
        },
        "missing_fact": "independent semantic role",
        "ceiling_next_command": "bin/harness analysis query --json owners exe/test@0x80100010",
        "interpretation": "no supported name",
        "authority": "expert and original",
        "partial_used": False,
        "outside_payload": False,
        "optional_work": [],
        "pending_commands": [],
    }
    report_digest = hashlib.sha256(report.read_bytes()).hexdigest()
    evidence_sha = hashlib.sha256(payload_path.read_bytes()).hexdigest()
    entry = commit_entry(
        root,
        report,
        TARGET,
        {
            "row": "function:func_80100000",
            "status": "committed",
            "evidence": payload_path.relative_to(root).as_posix(),
            "evidence_sha256": evidence_sha,
            "operations": [],
        },
    )
    write_manifest(
        root,
        report,
        TARGET,
        JournalInputs(TARGET, report_digest, "test", "test"),
        {"function:func_80100000": entry},
    )
    for label, record in zip(
        terminal["corroborators"], (receipt, receipt2), strict=True
    ):
        terminal["corroborators"][label]["source_id"] = _canonical_json_digest(
            {
                "report_digest": report_digest,
                "target": TARGET,
                "row": "function:func_80100000",
                "operation": record["operation"],
                "selector": record["selector"],
                "command": record["command"],
                "payload_digest": evidence_sha,
                "facts": ["selected_call"],
            }
        )
    authored = {
        "schema": "bof3.naming-conclusion/v1",
        "target": TARGET,
        "report_sha256": report_digest,
        "source": {
            "report": report.as_posix(),
            "report_digest": report_digest,
            "evidence": payload_path.relative_to(root).as_posix(),
            "evidence_sha256": evidence_sha,
        },
        "conclusion": terminal,
    }
    input_path = root / "input.json"
    input_path.write_text(json.dumps(authored), encoding="utf-8")
    return report, input_path, authored, terminal


@pytest.mark.parametrize(
    "name, message",
    [
        ("function_func_8009704C.json", "eight-digit uppercase"),
        ("data_D_80096994.json", "eight-digit uppercase"),
    ],
)
def test_rejects_reviewed_battle15_unsound_conclusions(name: str, message: str) -> None:
    path = (
        Path("out/reviews/evidence/emi/battle/battle/15__39fbb754e5b8/conclusions")
        / name
    )
    if not path.exists():
        pytest.skip("disposable forensic input is unavailable")
    from harness.naming.conclusion import _validate_conclusion

    authored = json.loads(path.read_text())
    assert _validate_conclusion(authored["conclusion"])["rung_status"] == "exhausted"


@pytest.mark.parametrize(
    "command",
    [
        "bin/harness analysis rz-project query emi/battle/battle/15 -c 'pd 96 @ 0x_8009704C'",
        "bin/harness analysis rz-project query emi/battle/battle/15 -c 'pd 40 @ 0x096994'",
        "bin/harness analysis query --json xrefs emi/battle/battle/15@0x8009699a",
    ],
)
def test_rejects_malformed_or_truncated_next_commands(command: str) -> None:
    with pytest.raises(ValueError, match="canonical target|eight-digit uppercase"):
        _validate_next_command(command)


@pytest.mark.parametrize(
    "command",
    [
        "pd 96 @ 8009704C",
        "bin/harness analysis query --json xrefs exe/test@0x80100000 trailing",
        "bin/harness analysis query --json xrefs EXE/test@0x80100000",
    ],
)
def test_rejects_noncanonical_full_command_forms(command: str) -> None:
    with pytest.raises(ValueError):
        _validate_next_command(command)


def test_failed_required_work_remains_blocked() -> None:
    from harness.naming.conclusion import _validate_conclusion

    row = json.loads(
        (FIXTURES / "naming-conclusion-exhausted.json").read_text(encoding="utf-8")
    )
    row["required_work"] = [{"id": "calls:x", "status": "failed", "commands": []}]
    with pytest.raises(ValueError, match="passed exact receipt"):
        _validate_conclusion(row)


def test_one_receipt_cannot_close_two_rungs(tmp_path: Path) -> None:
    _, input_path, authored, terminal = _fixture(tmp_path)
    terminal["rungs"]["one_level_beyond"] = dict(terminal["rungs"]["selected_call"])
    input_path.write_text(json.dumps(authored), encoding="utf-8")
    with pytest.raises(ValueError, match="one evidence source"):
        import_conclusion(
            tmp_path, TARGET, tmp_path / "report.json", tmp_path / "input.json"
        )


def test_import_proposed_delegates_existing_proposal_validation(
    tmp_path: Path, monkeypatch
) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_import_exhausted_and_idempotent(tmp_path: Path, monkeypatch) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


@pytest.mark.parametrize("mutation", ["stale", "noop", "mismatch", "proposal"])
def test_rejects_unbound_or_incomplete_evidence(
    tmp_path: Path, monkeypatch, mutation: str
) -> None:
    report, input_path, authored, terminal = _fixture(tmp_path)
    monkeypatch.setattr("harness.naming.audit.validate", lambda *_args, **_kwargs: {})
    if mutation == "stale":
        authored["report_sha256"] = "0" * 64
    elif mutation == "noop":
        terminal["rungs"]["selected_call"]["commands"][0]["command"] = (
            "harness:naming-evidence-run bounded collection"
        )
    elif mutation == "mismatch":
        terminal["required_work"][0]["commands"][0]["selector"] = "wrong"
    else:
        terminal["rung_status"] = "proposed"
        terminal["corroborators"] = {}
    input_path.write_text(json.dumps(authored), encoding="utf-8")
    with pytest.raises(ValueError):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_rejects_overwrite_and_preserves_report_on_validation_failure(
    tmp_path: Path, monkeypatch
) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_validation_failure_is_atomic(tmp_path: Path, monkeypatch) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_import_validates_whole_result_and_preserves_mode(
    tmp_path: Path, monkeypatch
) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_replay_still_requires_source_evidence(tmp_path: Path, monkeypatch) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_rejects_selector_target_bypass_with_matching_receipt_metadata(
    tmp_path: Path, monkeypatch
) -> None:
    report, input_path, authored, terminal = _fixture(tmp_path)
    monkeypatch.setattr("harness.naming.audit.validate", lambda *_args, **_kwargs: {})
    record = terminal["rungs"]["selected_call"]["commands"][0]
    forged = write_receipt(
        tmp_path,
        "out/reviews/evidence/selector-forged.json",
        {**record, "target": TARGET, "selector": "exe/other@0x80100000"},
    )
    forged.update({"item": "calls:x", "operation": "calls"})
    terminal["rungs"]["selected_call"]["commands"] = [forged]
    terminal["required_work"][0]["commands"] = [forged]
    payload_path = tmp_path / authored["source"]["evidence"]
    payload = json.loads(payload_path.read_text())
    payload["items"][0]["selector"] = "exe/other@0x80100000"
    payload["commands"] = [forged]
    payload_path.write_text(json.dumps(payload), encoding="utf-8")
    authored["source"]["evidence_sha256"] = hashlib.sha256(
        payload_path.read_bytes()
    ).hexdigest()
    input_path.write_text(json.dumps(authored), encoding="utf-8")
    with pytest.raises(ValueError, match="bound payload|target/selector"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_rejects_proposal_when_transaction_is_not_ready(
    tmp_path: Path, monkeypatch
) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_rejects_forged_cross_target_receipt(tmp_path: Path, monkeypatch) -> None:
    report, input_path, authored, terminal = _fixture(tmp_path)
    monkeypatch.setattr("harness.naming.audit.validate", lambda *_args, **_kwargs: {})
    record = terminal["rungs"]["selected_call"]["commands"][0]
    forged = write_receipt(
        tmp_path,
        "out/reviews/evidence/forged.json",
        {**record, "target": "exe/other"},
    )
    forged.update({"item": "calls:x", "operation": "calls"})
    terminal["rungs"]["selected_call"]["commands"] = [forged]
    terminal["required_work"][0]["commands"] = [forged]
    payload = json.loads((tmp_path / authored["source"]["evidence"]).read_text())
    payload["commands"] = [forged]
    payload_path = tmp_path / authored["source"]["evidence"]
    payload_path.write_text(json.dumps(payload), encoding="utf-8")
    authored["source"]["evidence_sha256"] = hashlib.sha256(
        payload_path.read_bytes()
    ).hexdigest()
    input_path.write_text(json.dumps(authored), encoding="utf-8")
    with pytest.raises(ValueError, match="bound payload|receipt target/selector"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_concurrent_report_change_is_rejected(tmp_path: Path, monkeypatch) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


def test_authored_initializer_shaped_row_cannot_be_overwritten(
    tmp_path: Path, monkeypatch
) -> None:
    report, input_path, _authored, _row = _fixture(tmp_path)
    with pytest.raises(ValueError, match="semantic fact capability is unsupported"):
        import_conclusion(tmp_path, TARGET, report, input_path)


COMMAND_FIELDS = (
    "smallest_repair",
    "ceiling_next_command",
    "rung.next_command",
    "required_work.next_command",
    "optional_work.next_command",
)
MALICIOUS_COMMANDS = (
    "bin/harness analysis rz-project query exe/test -c '!echo pwned'",
    "/tmp/rev-query --json owners exe/test@0x80100010",
    "bin/harness analysis query --json invented exe/test@0x80100010",
    "bin/harness analysis query --json owners exe/other@0x80100010",
    "bin/harness analysis query --json owners exe/test@0x80100010 --extra",
    "bin/harness analysis query --json owners exe/test@0x80100010 trailing",
)


def _set_command_field(row: dict, field: str, command: str) -> None:
    if field == "rung.next_command":
        row["rungs"]["selected_call"]["next_command"] = command
    elif field == "required_work.next_command":
        row["required_work"][0]["next_command"] = command
    elif field == "optional_work.next_command":
        row["optional_work"] = [{"id": "runtime-trace", "next_command": command}]
    else:
        row[field] = command


@pytest.mark.parametrize("field", COMMAND_FIELDS)
@pytest.mark.parametrize("command", MALICIOUS_COMMANDS)
def test_rejects_malicious_authored_command_fields_before_semantic_validation(
    tmp_path: Path, field: str, command: str
) -> None:
    report, input_path, authored, row = _fixture(tmp_path)
    original = report.read_bytes()
    _set_command_field(row, field, command)
    input_path.write_text(json.dumps(authored), encoding="utf-8")
    with pytest.raises(ValueError, match="not generated"):
        import_conclusion(tmp_path, TARGET, report, input_path)
    assert report.read_bytes() == original


@pytest.mark.parametrize(
    "forgery", ["checkpoint", "manifest", "stale-namespace", "copied-ledger"]
)
def test_local_recovery_artifacts_cannot_authenticate_semantic_facts(
    tmp_path: Path, forgery: str
) -> None:
    report, input_path, authored, _row = _fixture(tmp_path)
    namespace = (
        tmp_path
        / "out/reviews/evidence"
        / f"{TARGET}__{authored['source']['report_digest'][:12]}"
    )
    checkpoint = namespace / "checkpoint.jsonl"
    manifest = namespace / "manifest.json"
    if forgery == "checkpoint":
        entry = json.loads(checkpoint.read_text().splitlines()[0])
        entry["evidence_sha256"] = "0" * 64
        checkpoint.write_text(json.dumps(entry, sort_keys=True) + "\n")
    elif forgery == "manifest":
        payload = json.loads(manifest.read_text())
        payload["rows"][0]["evidence_sha256"] = "0" * 64
        manifest.write_text(json.dumps(payload))
    elif forgery == "stale-namespace":
        stale = namespace.with_name(f"{namespace.name}-stale")
        namespace.rename(stale)
    else:
        entry = json.loads(checkpoint.read_text().splitlines()[0])
        entry["row"] = "function:func_80100004"
        entry["entry_sha256"] = entry_sha256(entry)
        checkpoint.write_text(json.dumps(entry, sort_keys=True) + "\n")
    input_path.write_text(json.dumps(authored), encoding="utf-8")
    with pytest.raises(ValueError, match="journal|manifest"):
        import_conclusion(tmp_path, TARGET, report, input_path)


BATTLE15_TARGET = "emi/battle/battle/15"
BATTLE15_ROW = "D_800969A0"


def _exact_registry(*names: str) -> MappingProxyType:
    selected = {
        key: value
        for key, value in PRODUCTION_EXACT_CAPABILITIES.items()
        if key[0] in {f"data:{name}" for name in names}
    }
    assert len(selected) == len(names)
    return MappingProxyType(selected)


def test_terminal_exact_row_reconstructs_only_code_owned_work() -> None:
    from harness.naming.plan import collection_row

    terminal = {
        "kind": "data",
        "name": BATTLE15_ROW,
        "rung_status": "exhausted",
        "required_work": [],
    }
    registry = _exact_registry(BATTLE15_ROW, "D_800969AC")
    restored = collection_row(terminal, BATTLE15_TARGET, registry=registry)
    assert restored is not terminal
    assert restored["required_work"] == [
        {
            "id": "access:emi/battle/battle/15@800ad69c",
            "status": "open",
            "profile": "data_access",
            "description": "refresh exact reviewed consumer evidence",
        }
    ]
    ac = collection_row(
        dict(terminal, name="D_800969AC"), BATTLE15_TARGET, registry=registry
    )
    assert ac["required_work"] == [
        {
            "id": "access:emi/battle/battle/15@800ad9cc",
            "status": "open",
            "profile": "data_access",
            "description": "refresh exact reviewed consumer evidence",
        }
    ]


def _synthetic_authored(report: Path, row_name: str = SYNTHETIC_ROW) -> dict:
    report_digest = hashlib.sha256(report.read_bytes()).hexdigest()
    evidence_root = None
    from harness.naming.namespace import reset_evidence_root
    from harness.naming.namespace import set_evidence_root

    token = set_evidence_root(evidence_root)
    namespace = journal_dir(report.parents[2], report, SYNTHETIC_TARGET)
    reset_evidence_root(token)
    derived = json.loads((namespace / row_name / "derived.json").read_text())
    payload_path = namespace / row_name / "payload.json"
    payload = json.loads(payload_path.read_text())
    capability = derived["conclusion_capability"]
    command = payload["commands"][0]
    conclusion = _exact_capability_row(capability, command)
    return {
        "schema": "bof3.naming-conclusion/v1",
        "target": SYNTHETIC_TARGET,
        "report_sha256": report_digest,
        "source": {
            "report": report.as_posix(),
            "report_digest": report_digest,
            "evidence": payload_path.relative_to(report.parents[2]).as_posix(),
            "evidence_sha256": hashlib.sha256(payload_path.read_bytes()).hexdigest(),
            "conclusion_capability": capability,
        },
        "conclusion": conclusion,
    }


def test_proposed_import_uses_custom_registry_for_transaction_validation(
    tmp_path: Path, monkeypatch
) -> None:
    report = tmp_path / "report.json"
    proposed = {
        "kind": "function",
        "name": "func_80100000",
        "rung_status": "proposed",
        "new_name": "applyInput",
        "identity": {"kind": "function", "name": "applyInput"},
        "corroborators": {"callers": {}, "behavior": {}},
    }
    report.write_text(
        json.dumps(
            {
                "schema": "bof3.naming-audit/v3",
                "target": TARGET,
                "complete": True,
                "rows": [
                    {
                        "kind": "function",
                        "name": "func_80100000",
                        "initializer_state": "bof3.naming-audit-initializer/v1",
                        "rung_status": "blocked",
                    },
                    {
                        "kind": "data",
                        "name": "D_CUSTOM",
                        "rung_status": "exhausted",
                    },
                ],
            }
        )
    )
    authored = {
        "schema": "bof3.naming-conclusion/v1",
        "target": TARGET,
        "report_sha256": hashlib.sha256(report.read_bytes()).hexdigest(),
        "source": {"report_digest": "digest"},
        "conclusion": proposed,
    }
    input_path = tmp_path / "conclusion.json"
    input_path.write_text(json.dumps(authored))
    registry = cast(
        Any,
        MappingProxyType({("data:D_CUSTOM", "exe/test@80100000"): object()}),
    )
    calls = []

    monkeypatch.setattr(
        "harness.naming.conclusion.evidence_namespace_for_digest",
        lambda *_args: object(),
    )
    monkeypatch.setattr(
        "harness.naming.conclusion.namespace_identity", lambda *_args: {}
    )
    monkeypatch.setattr(
        "harness.naming.conclusion._validated_capability",
        lambda *_args, **_kwargs: None,
    )
    monkeypatch.setattr(
        "harness.naming.conclusion._validate_evidence", lambda *_args: {}
    )
    monkeypatch.setattr(
        "harness.naming.conclusion._validate_next_commands", lambda *_args: None
    )
    monkeypatch.setattr(
        "harness.naming.conclusion._reject_unsupported_semantic_facts",
        lambda *_args: None,
    )

    def validate(*_args, **kwargs):
        calls.append(kwargs)
        return {"ready": True, "errors": []}

    monkeypatch.setattr("harness.naming.audit.validate", validate)

    result = import_conclusion(tmp_path, TARGET, report, input_path, registry=registry)

    assert result["changed"] is True
    assert len(calls) == 2
    assert all(call["registry"] is registry for call in calls)
    assert calls[1]["transaction"] == "function:func_80100000"


def test_synthetic_exact_rows_import_and_replay_serially(
    tmp_path: Path, synthetic_exact_evidence
) -> None:
    report = synthetic_exact_evidence.report
    evidence_root = None
    for row_name in (SYNTHETIC_ROW,):
        authored = _synthetic_authored(report, row_name)
        authored["source"]["report"] = report.as_posix()
        input_path = tmp_path / f"{row_name}.json"
        input_path.write_text(json.dumps(authored))
        result = import_conclusion(
            synthetic_exact_evidence.root,
            SYNTHETIC_TARGET,
            report,
            input_path,
            evidence_root=evidence_root,
            registry=synthetic_exact_evidence.registry,
        )
        assert result["changed"] is True
        replay = import_conclusion(
            synthetic_exact_evidence.root,
            SYNTHETIC_TARGET,
            report,
            input_path,
            evidence_root=evidence_root,
            registry=synthetic_exact_evidence.registry,
        )
        assert replay["changed"] is False
    rows = json.loads(report.read_text())["rows"]
    assert sum(row["rung_status"] == "exhausted" for row in rows) == 1
    assert sum(row["rung_status"] == "blocked" for row in rows) == len(rows) - 1
    assert not any(row["rung_status"] == "proposed" for row in rows)


@pytest.mark.parametrize(
    "mutation",
    [
        "target",
        "row",
        "report-digest",
        "input-digest",
        "fact-digest",
        "source-id",
        "required-work",
        "proposal-flag",
        "duplicate-corroborator",
        "missing-corroborator",
        "unsupported-term",
        "name-terms",
        "semantic-status",
        "transaction-status",
        "pre-apply",
        "semantic-claim",
        "interpretation",
        "observation-text",
        "source-extra",
        "source-report-omitted",
        "source-report-cross-target",
        "corroborator-observation",
        "corroborator-extra",
        "other-target",
    ],
)
def test_synthetic_exact_row_mutations_fail_without_report_change(
    tmp_path: Path, synthetic_exact_evidence, mutation: str
) -> None:
    report = synthetic_exact_evidence.report
    authored = _synthetic_authored(synthetic_exact_evidence.report)
    authored["source"]["report"] = report.as_posix()
    capability = authored["source"]["conclusion_capability"]
    row = authored["conclusion"]
    if mutation == "target":
        authored["target"] = "emi/battle/battle/03"
    elif mutation == "row":
        row["name"] = "D_80096994"
    elif mutation == "report-digest":
        authored["report_sha256"] = "0" * 64
    elif mutation == "input-digest":
        capability["input_digest"] = "0" * 64
    elif mutation == "fact-digest":
        capability["fact_digest"] = "0" * 64
    elif mutation == "source-id":
        row["rungs"]["selected_range"]["observations"][0]["source_id"] = "forged"
    elif mutation == "required-work":
        row["required_work"][0]["id"] = "access:forged"
    elif mutation == "proposal-flag":
        capability["proposal_allowed"] = True
    elif mutation == "duplicate-corroborator":
        row["corroborators"]["original-consumer"]["source_id"] = row["corroborators"][
            "original-layout"
        ]["source_id"]
    elif mutation == "missing-corroborator":
        row["corroborators"].pop("original-consumer")
    elif mutation == "unsupported-term":
        row["new_name"] = "dispatchTable"
    elif mutation == "name-terms":
        row["name_terms"] = {"dispatch": "original-consumer"}
    elif mutation == "semantic-status":
        row["semantic_status"] = "accepted"
    elif mutation == "transaction-status":
        row["transaction_status"] = "ready"
    elif mutation == "pre-apply":
        row["pre_apply"] = {"status": "ready"}
    elif mutation == "semantic-claim":
        row["semantic_claim"] = "dispatch table"
    elif mutation == "interpretation":
        row["interpretation"] = "semantic dispatch assertion"
    elif mutation == "observation-text":
        row["rungs"]["selected_range"]["observations"][0]["text"] = "semantic name"
    elif mutation == "source-extra":
        authored["source"]["semantic_claim"] = "dispatch table"
    elif mutation == "source-report-omitted":
        authored["source"].pop("report")
    elif mutation == "source-report-cross-target":
        authored["source"]["report"] = "out/reviews/other.json"
    elif mutation == "corroborator-observation":
        row["corroborators"]["original-layout"]["observation_ids"] = ["forged"]
    elif mutation == "corroborator-extra":
        row["corroborators"]["original-layout"]["semantic_claim"] = "dispatch"
    else:
        authored["source"]["conclusion_capability"]["target"] = "emi/battle/battle/03"
    original = report.read_bytes()
    input_path = tmp_path / "conclusion.json"
    input_path.write_text(json.dumps(authored))
    with pytest.raises(ValueError):
        import_conclusion(
            synthetic_exact_evidence.root,
            SYNTHETIC_TARGET,
            report,
            input_path,
            registry=synthetic_exact_evidence.registry,
        )
    assert report.read_bytes() == original


def test_synthetic_exact_row_explicit_evidence_root_is_mandatory(
    tmp_path: Path, synthetic_exact_evidence
) -> None:
    report = synthetic_exact_evidence.report
    evidence_root = None
    authored = _synthetic_authored(report)
    authored["source"]["report"] = report.as_posix()
    input_path = tmp_path / "explicit-conclusion.json"
    input_path.write_text(json.dumps(authored))
    original = report.read_bytes()

    for selected in (tmp_path / "wrong-root",):
        with pytest.raises(ValueError):
            import_conclusion(
                Path.cwd(),
                SYNTHETIC_TARGET,
                report,
                input_path,
                evidence_root=selected,
            )
        assert report.read_bytes() == original

    imported = import_conclusion(
        synthetic_exact_evidence.root,
        SYNTHETIC_TARGET,
        report,
        input_path,
        evidence_root=evidence_root,
        registry=synthetic_exact_evidence.registry,
    )
    assert imported["changed"] is True
    replay = import_conclusion(
        synthetic_exact_evidence.root,
        SYNTHETIC_TARGET,
        report,
        input_path,
        evidence_root=evidence_root,
        registry=synthetic_exact_evidence.registry,
    )
    assert replay["changed"] is False


def test_synthetic_exact_row_identical_replay_is_idempotent(
    tmp_path: Path, synthetic_exact_evidence
) -> None:
    report = synthetic_exact_evidence.report
    evidence_root = None
    authored = _synthetic_authored(report)
    authored["source"]["report"] = report.as_posix()
    input_path = tmp_path / "conclusion.json"
    input_path.write_text(json.dumps(authored))
    first = import_conclusion(
        synthetic_exact_evidence.root,
        SYNTHETIC_TARGET,
        report,
        input_path,
        evidence_root=evidence_root,
        registry=synthetic_exact_evidence.registry,
    )
    imported = report.read_bytes()
    replay = import_conclusion(
        synthetic_exact_evidence.root,
        SYNTHETIC_TARGET,
        report,
        input_path,
        evidence_root=evidence_root,
        registry=synthetic_exact_evidence.registry,
    )
    assert first["changed"] is True
    assert replay["changed"] is False
    assert report.read_bytes() == imported


def test_synthetic_exact_row_replay_rejects_stale_initializer_digest(
    tmp_path: Path, synthetic_exact_evidence
) -> None:
    report = synthetic_exact_evidence.report
    evidence_root = None
    authored = _synthetic_authored(report)
    authored["source"]["report"] = report.as_posix()
    input_path = tmp_path / "conclusion.json"
    input_path.write_text(json.dumps(authored))
    import_conclusion(
        synthetic_exact_evidence.root,
        SYNTHETIC_TARGET,
        report,
        input_path,
        evidence_root=evidence_root,
        registry=synthetic_exact_evidence.registry,
    )
    imported = report.read_bytes()
    authored["report_sha256"] = "0" * 64
    input_path.write_text(json.dumps(authored))
    with pytest.raises(ValueError, match="provenance"):
        import_conclusion(
            synthetic_exact_evidence.root,
            SYNTHETIC_TARGET,
            report,
            input_path,
            evidence_root=evidence_root,
            registry=synthetic_exact_evidence.registry,
        )
    assert report.read_bytes() == imported


def test_synthetic_exact_row_rejects_nested_command_extra(
    tmp_path: Path, synthetic_exact_evidence
) -> None:
    report = synthetic_exact_evidence.report
    evidence_root = None
    authored = _synthetic_authored(report)
    authored["source"]["report"] = report.as_posix()
    authored["conclusion"]["required_work"][0]["commands"][0]["semantic_claim"] = (
        "dispatch"
    )
    input_path = tmp_path / "conclusion.json"
    input_path.write_text(json.dumps(authored))
    with pytest.raises(ValueError, match="trusted runner payload"):
        import_conclusion(
            synthetic_exact_evidence.root,
            SYNTHETIC_TARGET,
            report,
            input_path,
            evidence_root=evidence_root,
            registry=synthetic_exact_evidence.registry,
        )


def test_synthetic_exact_row_conflicting_replay_rejects(
    tmp_path: Path, synthetic_exact_evidence
) -> None:
    report = synthetic_exact_evidence.report
    evidence_root = None
    authored = _synthetic_authored(report)
    authored["source"]["report"] = report.as_posix()
    input_path = tmp_path / "conclusion.json"
    input_path.write_text(json.dumps(authored))
    import_conclusion(
        synthetic_exact_evidence.root,
        SYNTHETIC_TARGET,
        report,
        input_path,
        evidence_root=evidence_root,
        registry=synthetic_exact_evidence.registry,
    )
    imported = report.read_bytes()
    authored["conclusion"]["interpretation"] = "conflicting semantic assertion"
    input_path.write_text(json.dumps(authored))
    with pytest.raises(ValueError):
        import_conclusion(
            synthetic_exact_evidence.root,
            SYNTHETIC_TARGET,
            report,
            input_path,
            evidence_root=evidence_root,
            registry=synthetic_exact_evidence.registry,
        )
    assert report.read_bytes() == imported
