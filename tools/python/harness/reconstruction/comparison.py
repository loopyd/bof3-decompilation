"""Independently rebuild candidates and compare complete linked function images."""

from __future__ import annotations

from copy import deepcopy
from collections.abc import Callable
from dataclasses import asdict, dataclass
import hashlib
from pathlib import Path
from typing import Literal

from harness.build import recipe
from harness.common.artifacts import (
    active_reader,
    ConsumedFile,
    FileIdentity,
    encode_json,
    physical_read_bytes,
    read_bytes,
    using_reader,
    write_bytes,
)
from harness.common.captureproof import verify_capture
from harness.common.failures import FailureEvent, PolicyFailure, facts, primary, retain
from harness.common.process import ProcessCleanupError, capture
from harness.match._asm_link import object_link_command
from harness.match.extraction import (
    FunctionImage,
    read_function_bytes,
    read_function_image,
    read_linked_image,
)
from harness.reconstruction.prepare import PreparedInputs
from harness.reconstruction import (
    bundles,
    catalog,
    calltargets,
    contract,
    nativeoutput,
    objectfile,
    obligations,
    prelude,
    provenance,
    invocation,
    callcontracts,
    machinecode,
    prepare,
)


class ConsumptionAllowance:
    def __init__(self, *, read_limit, open_limit, artifact_limit):
        _require_rebuild(
            all(
                type(value) is int and value > 0
                for value in (read_limit, open_limit, artifact_limit)
            ),
            "consumption limits must be positive integers",
        )
        self.read_limit, self.open_limit, self.artifact_limit = (
            read_limit,
            open_limit,
            artifact_limit,
        )
        self.charged_bytes = self.attempted_opens = self.failed_reserved_bytes = 0
        self.observations, self.requests = [], []
        self.final_bytes = self.final_opens = 0
        self.final_identities = None
        self.final_output = None
        self.final_counts = {}
        self._final_authenticated = set()

    def read_bytes(self, path, bound, events=None):
        _require_rebuild(
            isinstance(path, Path) and type(bound) is int and bound >= 0,
            "physical read requires a path and bounded artifact size",
        )
        bound = min(bound, self.artifact_limit)
        expected = None
        key = str(path.absolute())
        if self.final_identities is not None:
            expected = self.final_identities.get(key)
            _require_rebuild(
                expected is not None
                and expected.bytes <= bound
                and self.final_counts.get(key, 0) > 0,
                "final read has no known identity, sufficient bound or occurrence",
            )
            bound = expected.bytes
        reservation = bound + 1
        if self.final_identities is None:
            _require_rebuild(
                self.charged_bytes + reservation <= self.read_limit
                and self.attempted_opens < self.open_limit,
                "command input consumption allowance exhausted",
            )
        else:
            protected_output = (
                self.final_output[1] + 1 if self.final_output is not None else 0
            )
            _require_rebuild(
                reservation <= self.final_bytes - protected_output
                and self.final_opens > 0,
                "final read is outside pre-reserved complete consumption",
            )
            self.final_bytes -= reservation
            self.final_opens -= 1
            self.final_counts[key] -= 1
        self.charged_bytes += reservation
        self.attempted_opens += 1
        self.requests.append((path, bound))
        try:
            data = physical_read_bytes(path, bound, events)
            identity = FileIdentity(
                path.absolute(), len(data), hashlib.sha256(data).hexdigest()
            )
            self.observations.append(identity.as_record())
            _require_rebuild(
                len(data) <= bound, "physical snapshot exceeds requested bound"
            )
            if expected is not None:
                _require_rebuild(
                    identity == expected,
                    "physical input changed during final consumption",
                )
        except BaseException:
            self.failed_reserved_bytes += reservation
            raise
        if expected is not None:
            self._final_authenticated.add(key)
        self.charged_bytes -= reservation - len(data)
        return data

    def read(self, path, bound, events=None):
        data = self.read_bytes(path, bound, events)
        return ConsumedFile(
            FileIdentity(path.absolute(), len(data), hashlib.sha256(data).hexdigest()),
            data,
        )

    def reserve_final_consumption(self, requests, identities, *, output=None):
        _require_rebuild(
            self.final_identities is None and not self.failed_reserved_bytes,
            "uncertain consumption cannot enter terminal publication",
        )
        known = self._index_identities(identities)
        _require_rebuild(
            known == self._index_identities(self.observations),
            "final census differs from actual physical observations",
        )
        selected_output = None
        if output is not None:
            _require_rebuild(
                type(output) in (tuple, list)
                and len(output) == 2
                and isinstance(output[0], Path)
                and type(output[1]) is int
                and output[1] >= 0
                and ".." not in output[0].parts,
                "final output reservation is malformed",
            )
            selected_output = (
                output[0].absolute(),
                min(output[1], self.artifact_limit),
            )
            _require_rebuild(
                str(selected_output[0]) not in known,
                "final output aliases an observed input",
            )
        _require_rebuild(
            type(requests) in (tuple, list),
            "final consumption requires the complete request sequence",
        )
        counts, total = {}, 0
        for request in requests:
            _require_rebuild(
                type(request) in (tuple, list)
                and len(request) == 2
                and isinstance(request[0], Path)
                and type(request[1]) is int
                and request[1] >= 0
                and ".." not in request[0].parts,
                "final consumption request is malformed",
            )
            path, bound = request
            key = str(path.absolute())
            counts[key] = counts.get(key, 0) + 1
            if selected_output is not None and path.absolute() == selected_output[0]:
                _require_rebuild(
                    bound == output[1] and counts[key] == 1,
                    "final output requires one exact reserved occurrence",
                )
                total += selected_output[1] + 1
                continue
            expected = known.get(key)
            _require_rebuild(
                expected is not None
                and expected.bytes <= min(bound, self.artifact_limit),
                "final request has no known identity or sufficient bound",
            )
            total += expected.bytes + 1
        _require_rebuild(
            set(counts)
            == set(known)
            | ({str(selected_output[0])} if selected_output is not None else set()),
            "final requests omit observed inputs or the reserved output",
        )
        _require_rebuild(
            self.charged_bytes + total <= self.read_limit
            and self.attempted_opens + len(requests) <= self.open_limit,
            "complete final consumption exceeds remaining allowance",
        )
        self.final_bytes, self.final_opens = total, len(requests)
        self.final_identities = known
        self.final_output = selected_output
        self.final_counts = counts

    def bind_final_output(self, identity):
        _require_rebuild(
            self.final_output is not None
            and not self.failed_reserved_bytes
            and self._final_authenticated.issuperset(self.final_identities)
            and isinstance(identity, FileIdentity)
            and isinstance(identity.path, Path)
            and identity.path == self.final_output[0]
            and type(identity.bytes) is int
            and identity.bytes >= 0
            and identity.bytes <= self.final_output[1],
            "terminal output differs from its pre-reserved physical read",
        )
        key = str(identity.path)
        expected = self._index_identities([identity.as_record()])[key]
        _require_rebuild(
            key not in self.final_identities and self.final_counts.get(key, 0) == 1,
            "terminal output identity was already bound or consumed",
        )
        self.final_identities[key] = expected
        self.final_output = None

    @staticmethod
    def _index_identities(rows):
        _require_rebuild(
            type(rows) in (tuple, list), "physical identity census is malformed"
        )
        indexed = {}
        for row in rows:
            _require_rebuild(
                type(row) is dict
                and set(row) == {"path", "bytes", "sha256"}
                and type(row["path"]) is str
                and Path(row["path"]).is_absolute()
                and str(Path(row["path"])) == row["path"]
                and ".." not in Path(row["path"]).parts
                and type(row["bytes"]) is int
                and row["bytes"] >= 0
                and type(row["sha256"]) is str
                and len(row["sha256"]) == 64
                and all(value in "0123456789abcdef" for value in row["sha256"]),
                "physical identity is malformed",
            )
            identity = FileIdentity(Path(row["path"]), row["bytes"], row["sha256"])
            _require_rebuild(
                indexed.setdefault(row["path"], identity) == identity,
                "physical identity drift between command snapshots",
            )
        return indexed

    def as_record(self):
        return dict(
            charged_bytes=self.charged_bytes,
            attempted_opens=self.attempted_opens,
            failed_reserved_bytes=self.failed_reserved_bytes,
            reserved_final_bytes=self.final_bytes,
            reserved_final_opens=self.final_opens,
        )


@dataclass(frozen=True)
class OriginalFunction:
    start: int
    end: int
    content: bytes

    def __post_init__(self) -> None:
        if (
            type(self.start) is not int
            or type(self.end) is not int
            or not 0 <= self.start < self.end <= 2**32
            or self.start % 4
            or self.end % 4
            or type(self.content) is not bytes
            or len(self.content) != self.end - self.start
        ):
            raise ValueError(
                "original function requires a complete aligned byte interval"
            )


@dataclass(frozen=True)
class FunctionComparison:
    original: OriginalFunction
    actual: FunctionImage

    @property
    def outcome(self) -> Literal["exact", "short", "overlong", "changed"]:
        if len(self.actual.content) < len(self.original.content):
            return "short"
        if len(self.actual.content) > len(self.original.content):
            return "overlong"
        if self.actual.content != self.original.content:
            return "changed"
        return "exact"

    @property
    def first_difference(self) -> int | None:
        if self.actual.content == self.original.content:
            return None
        for offset, (original, actual) in enumerate(
            zip(self.original.content, self.actual.content)
        ):
            if original != actual:
                return offset
        return min(len(self.original.content), len(self.actual.content))


def compare_linked_function(
    path: Path, *, function_name: str, original: OriginalFunction
) -> FunctionComparison:
    actual = read_function_image(
        path, function_name=function_name, address=original.start
    )
    comparison = FunctionComparison(original=original, actual=actual)
    _ = comparison.outcome, comparison.first_difference
    return comparison


def _require_rebuild(condition: object, message: str) -> None:
    if not condition:
        raise PolicyFailure(message)


def _complete_process(record):
    return (
        type(record) is dict
        and record.get("raw_exit") == record.get("exit_code") == 0
        and record.get("failure") is None
        and record.get("owner_observation") is None
        and record.get("capture_complete") is True
    )


def _regular_record(record: dict, limit: int) -> Path:
    _require_rebuild(
        type(record) is dict
        and set(record) == {"path", "bytes", "sha256"}
        and type(record["path"]) is str
        and type(record["bytes"]) is int
        and 0 <= record["bytes"] <= limit,
        "submitted file record is absent, invalid or exceeds its bound",
    )
    path = Path(record["path"])
    _require_rebuild(
        path.is_absolute()
        and path.resolve() == path
        and not path.is_symlink()
        and path.is_file(),
        "submitted file must be a canonical regular nonsymlinked file",
    )
    return path


def _authenticated_bytes(
    record: dict,
    limit: int,
    events: list[FailureEvent],
    *,
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
) -> bytes:
    reader = reader if reader is not None else active_reader()
    path = _regular_record(record, limit)
    recipe.verify_inputs([record], events=events, reader=reader)
    with contract.bounded_reads(limit, reader=reader):
        content = contract.read_bytes(
            path, "submitted artifact", io_mode="exact", events=events, max_bytes=limit
        )
    _require_rebuild(
        len(content) == record["bytes"]
        and hashlib.sha256(content).hexdigest() == record["sha256"],
        "submitted file changed during its authenticated read",
    )
    recipe.verify_inputs([record], events=events, reader=reader)
    return content


def _submitted_document(record, limit, events, *, reader=None) -> dict:
    value = nativeoutput.parse_json(
        _authenticated_bytes(record, limit, events, reader=reader), limit
    )
    if not isinstance(value, dict) or type(value) is not dict:
        raise PolicyFailure("submitted document must be an object")
    return deepcopy(value)


def _record_inventory(records: list[dict], limit: int) -> list[dict]:
    inventory: dict[str, dict] = {}
    for record in records:
        path = _regular_record(record, limit)
        _require_rebuild(
            inventory.setdefault(str(path), record) == record,
            "submitted file identities disagree for one path",
        )
    return deepcopy(list(inventory.values()))


@dataclass(frozen=True)
class BuildSubmission:
    identity: dict
    entry_symbol: str
    source: dict
    prelude: dict
    selected: dict
    records: list[dict]
    producer: invocation.OriginalOnlySource | invocation.SuppliedOrdinarySource


def _adapt_submission(result, run, result_record, run_record, limit):
    candidate = Path(result_record["path"]).parent
    _require_rebuild(
        Path(result_record["path"]).name == "result.json"
        and Path(run_record["path"]) == candidate.parent / "run.json",
        "member result and run record do not identify one submitted run",
    )
    if run.get("producer", {}).get("kind") == invocation.SUPPLIED_KIND:
        inspection = invocation.supplied_submission(candidate, result, run)
        _require_rebuild(inspection.unit is not None, "ordinary source is missing")
        return BuildSubmission(
            result["identity"],
            inspection.unit.entry_symbol,
            result["source"],
            result["prelude"],
            deepcopy(run["tools"]),
            _record_inventory(
                [
                    result_record,
                    run_record,
                    *(row.as_record() for row in inspection.files),
                ],
                limit,
            ),
            invocation.SuppliedOrdinarySource(inspection),
        )
    _require_rebuild(
        "producer" not in run and "supplied" not in result,
        "original-only submission has a foreign producer",
    )
    identity, native, build = (
        result.get("identity"),
        result.get("native"),
        result.get("build"),
    )
    _require_rebuild(
        type(identity) is dict
        and type(identity.get("key")) is str
        and type(run.get("selected")) is list
        and identity["key"] in run["selected"]
        and type(native) is dict
        and type(build) is dict
        and native.get("workspace") == str(candidate),
        "submission lacks a selected native source and build record",
    )
    source, copied_source, supplied_prelude = (
        native.get("source"),
        build.get("source"),
        build.get("prelude"),
    )
    for record, expected in (
        (source, candidate / "generated.c"),
        (copied_source, candidate / "build/unit.c"),
        (supplied_prelude, candidate / "build" / prelude.PRELUDE_PATH),
    ):
        _require_rebuild(
            _regular_record(record, limit) == expected,
            "submitted source or prelude path differs",
        )
    _require_rebuild(
        (source["bytes"], source["sha256"])
        == (copied_source["bytes"], copied_source["sha256"]),
        "native source and submitted build source differ",
    )
    outputs = native.get("outputs")
    _require_rebuild(type(outputs) is list, "native output inventory is absent")
    normalized = {}
    for output in outputs:
        _require_rebuild(
            type(output) is dict
            and set(output) == {"path", "bytes", "sha256"}
            and type(output["path"]) is str
            and output["path"]
            in {
                "bindings.json",
                "inferred-config.json",
                "status.json",
                "tokens.json",
            }
            and output["path"] not in normalized,
            "native output inventory has an unknown, escaping or duplicate path",
        )
        normalized[output["path"]] = {
            **output,
            "path": str(candidate / "work" / output["path"]),
        }
    _require_rebuild(
        set(normalized)
        == {"bindings.json", "inferred-config.json", "status.json", "tokens.json"},
        "native output inventory is incomplete",
    )
    bindings = native.get("bindings")
    _require_rebuild(
        type(bindings) is dict
        and bindings.get("artifact") == normalized["bindings.json"],
        "native binding identity is absent or differs from the output inventory",
    )
    selected = run.get("tools")
    _require_rebuild(
        type(selected) is dict
        and selected.get("catalog_id") == catalog.CATALOG_ID
        and selected.get("pipeline_id") == recipe.CANONICAL_PROFILE_ID
        and type(selected.get("profiles")) is list
        and bool(selected["profiles"])
        and "profiles_digest" in selected
        and type(selected.get("files")) is dict,
        "submission lacks the complete selected compiler profiles",
    )
    profiles = selected["profiles"]
    expected_profiles = []
    for ordinal, profile in enumerate(profiles):
        _require_rebuild(
            type(profile) is dict
            and profile.get("ordinal") == ordinal
            and type(profile.get("profile_id")) is str
            and type(profile.get("identity")) is str,
            "submitted compiler profiles are not ordered complete records",
        )
        expected_profiles.append(
            (profile["profile_id"], profile["identity"], profile["ordinal"])
        )
    _require_rebuild(
        profiles[0]["profile_id"] == recipe.CANONICAL_PROFILE_ID
        and len({row[0] for row in expected_profiles}) == len(profiles),
        "submitted compiler profile inventory differs",
    )
    for attempts in (result.get("profile_attempts"), build.get("profile_attempts")):
        _require_rebuild(
            type(attempts) is list
            and all(type(attempt) is dict for attempt in attempts)
            and [
                (
                    attempt.get("profile_id"),
                    attempt.get("profile_identity"),
                    attempt.get("ordinal"),
                )
                for attempt in attempts
            ]
            == expected_profiles,
            "submitted member and run compiler profile inventories differ",
        )
    records = [result_record, run_record, source, copied_source, supplied_prelude]
    if "prepared_inputs" in run:
        prepared = PreparedInputs.attach(run, run_directory=candidate.parent)
        prepared.verify()
        records.extend([run["prepared_inputs"], run["campaign"]["owner_run"]])
    for name in ("expected_inputs", "expected_outputs"):
        values = native.get(name)
        _require_rebuild(
            type(values) is list and bool(values),
            "native submitted input or output inventory is absent",
        )
        for value in values:
            _require_rebuild(
                _regular_record(value, limit).is_relative_to(candidate),
                "native submitted file escapes the member directory",
            )
        records.extend(values)
    _require_rebuild(
        source in native["expected_outputs"],
        "native source is not bound to the submitted output inventory",
    )
    records.extend(normalized.values())
    records.append(bindings["artifact"])
    return BuildSubmission(
        identity,
        native["entry_symbol"],
        source,
        supplied_prelude,
        deepcopy(selected),
        _record_inventory(records, limit),
        invocation.OriginalOnlySource(native, normalized),
    )


def _workspace_boundary(workspace: Path, run_record: dict, records, selected) -> None:
    _require_rebuild(
        isinstance(workspace, Path)
        and workspace.is_absolute()
        and workspace.parent.is_dir()
        and workspace.parent.resolve() == workspace.parent
        and not workspace.exists()
        and not workspace.is_symlink(),
        "rebuild workspace must be absolute, canonical and absent",
    )
    recipe.validate_workspace(str(workspace))
    protected = [Path(run_record["path"]).parent]
    run = _submitted_document(run_record, 64 << 20, [])
    if "prepared_inputs" in run:
        prepared = PreparedInputs.attach(run, run_directory=protected[0])
        protected.append(prepared._destination)
    for profile in selected["profiles"]:
        protected.append(Path(profile["original"]["directory"]["path"]))
        if "tools" in profile:
            protected.append(Path(profile["tools"]["maspsx_root"]))
    _require_rebuild(
        not any(
            workspace == path or workspace.is_relative_to(path) for path in protected
        )
        and not any(
            workspace == Path(record["path"])
            or Path(record["path"]).is_relative_to(workspace)
            for record in records
        ),
        "rebuild workspace overlaps submitted inputs or compiler tools",
    )


def _rebuild_checks(
    records,
    selected,
    syntax,
    attempts,
    contexts,
    receipts,
    output_limit,
    events,
    *,
    reader=None,
) -> None:
    reader = reader if reader is not None else active_reader()
    for record in records:
        try:
            _regular_record(record, 64 << 20)
            recipe.verify_inputs([record], events=events, reader=reader)
        except Exception as error:
            retain(
                events,
                error,
                phase="submitted-input",
                path=record.get("path", "input") if type(record) is dict else "input",
            )
    if selected is not None:
        try:
            catalog.verify(selected, events=events, reader=reader)
        except Exception as error:
            retain(events, error, phase="submitted-catalog", path="tools")
    if syntax is not None:
        try:
            events.extend(recipe.verify_syntax(syntax, reader=reader))
        except Exception as error:
            retain(events, error, phase="rebuild-syntax", path=syntax.call["cwd"])
        if syntax.inputs is not None:
            try:
                events.extend(
                    recipe.verify_attempts(
                        attempts,
                        syntax.inputs,
                        output_limit=output_limit,
                        contexts=contexts,
                        reader=reader,
                    )
                )
            except Exception as error:
                retain(events, error, phase="rebuild-profiles", path=syntax.call["cwd"])
    try:
        events.extend(recipe.verify_receipts(receipts, reader=reader))
    except Exception as error:
        retain(events, error, phase="rebuild-receipts", path="compiler")


def _forward_rebuild(events: list[FailureEvent]) -> None:
    pending = primary(events)
    if pending is not None:
        raise pending


def _retain_outcome(outcome, retained, contexts, receipts) -> None:
    if outcome.context is not None:
        contexts.append(outcome.context)
        retained["profile_contexts"].append(
            dict(
                ordinal=outcome.context.ordinal,
                directory=str(outcome.context.directory),
                recipe_data=outcome.context.recipe_data.hex(),
            )
        )
    if outcome.receipt is not None:
        receipts.append(outcome.receipt)
        retained["profile_receipts"].append(
            dict(
                ordinal=outcome.receipt.ordinal,
                path=str(outcome.receipt.path),
                data=outcome.receipt.data.hex(),
                bytes=outcome.receipt.bytes,
                sha256=outcome.receipt.sha256,
            )
        )
    retained["profile_outcomes"].append(
        dict(
            ordinal=outcome.ordinal,
            result=deepcopy(outcome.result),
            abort=None
            if outcome.abort is None
            else f"{type(outcome.abort).__name__}: {outcome.abort}",
            events=facts(outcome.events),
        )
    )


def rebuild_candidate(
    *,
    result_record: dict,
    run_record: dict,
    workspace: Path,
    output_limit: int,
    artifact_limit: int,
    retained: dict,
) -> dict:
    """Recompile authenticated unchanged inputs; retain evidence before propagating failure."""
    _require_rebuild(
        type(retained) is dict
        and retained is not result_record
        and retained is not run_record,
        "rebuild requires a distinct caller-owned retained dictionary",
    )
    retained.clear()
    retained.update(
        operation="rebuild",
        status="unattempted",
        profile_attempts=[],
        profile_outcomes=[],
        profile_contexts=[],
        profile_receipts=[],
        secondary_failures=[],
        semantic_admission=False,
        autonomous_exact=False,
        original_byte_comparison="unperformed",
        associated_data="unverified",
        placement="unverified",
    )
    events: list[FailureEvent] = []
    records: list[dict] = []
    contexts: list[recipe.DeclaredProfileContext] = []
    receipts: list[recipe.ProfileReceipt] = []
    selected, syntax = None, None
    attempts = retained["profile_attempts"]
    try:
        retained["status"] = "preparing"
        result_record, run_record = deepcopy((result_record, run_record))
        _require_rebuild(
            all(
                (
                    type(limit) is int and limit > 0
                    for limit in (output_limit, artifact_limit)
                )
            ),
            "rebuild requires positive integer limits",
        )
        retained.update(
            submitted_result=deepcopy(result_record),
            submitted_run=deepcopy(run_record),
            output_limit=output_limit,
            artifact_limit=artifact_limit,
        )
        _forward_rebuild(events)
        metadata_limit = min(artifact_limit, 64 << 20)
        records = [result_record, run_record]
        result = _submitted_document(result_record, metadata_limit, events)
        _forward_rebuild(events)
        run = _submitted_document(run_record, metadata_limit, events)
        submission = _adapt_submission(
            result, run, result_record, run_record, metadata_limit
        )
        identity, source_record, prelude_record, selected, records = (
            submission.identity,
            submission.source,
            submission.prelude,
            submission.selected,
            submission.records,
        )
        retained.update(
            identity=deepcopy(identity),
            entry_symbol=submission.entry_symbol,
            submitted_inputs=deepcopy(records),
            profiles_digest=selected["profiles_digest"],
        )
        for profile in selected["profiles"]:
            attempts.append(
                dict(
                    profile_id=profile["profile_id"],
                    profile_identity=profile["identity"],
                    ordinal=profile["ordinal"],
                    disposition="unattempted",
                    terminal=False,
                    status="unattempted",
                    failure=None,
                    raw_exit=None,
                    exit_code=None,
                    stages=[],
                )
            )
        _workspace_boundary(workspace, run_record, records, selected)
        _rebuild_checks(
            records,
            selected,
            syntax,
            attempts,
            contexts,
            receipts,
            output_limit,
            events,
        )
        _forward_rebuild(events)
        if isinstance(submission.producer, invocation.OriginalOnlySource):
            invocation.verify_submitted_original_outputs(
                submission.producer.outputs, metadata_limit, events
            )
        source = _authenticated_bytes(source_record, metadata_limit, events)
        header = _authenticated_bytes(prelude_record, metadata_limit, events)
        _require_rebuild(
            bool(source) and header == prelude.STDINT_PRELUDE,
            "rebuild requires nonempty unchanged source and the uniform submitted prelude",
        )
        _forward_rebuild(events)
        retained["status"] = "syntax"
        syntax = recipe.check_syntax(
            recipe.c89_call(
                workspace=workspace,
                tools=selected,
                output_limit=output_limit,
            ),
            source=source,
            prelude=header,
            prelude_path=prelude.PRELUDE_PATH,
            tools=selected,
            immutable=records,
            root_profile_id=recipe.CANONICAL_PROFILE_ID,
            artifact_limit=artifact_limit,
        )
        retained.update(deepcopy(syntax.record))
        events.extend(syntax.events)
        if syntax.pending is not None:
            raise syntax.pending
        _require_rebuild(
            syntax.record.get("c89_launch") == "observed"
            and syntax.record.get("c89", {}).get("raw_exit") == 0
            and syntax.record["c89"].get("failure") is None
            and syntax.inputs is not None,
            "C89 refused or did not observe the unchanged candidate",
        )
        retained["status"] = "profiles"
        execution = recipe.execute_profiles(
            selected["profiles"],
            workspace=workspace,
            root_profile_id=recipe.CANONICAL_PROFILE_ID,
            inputs=syntax.inputs,
            include_roots=[str(workspace / "include")],
            attempts=attempts,
            output_limit=output_limit,
            artifact_limit=artifact_limit,
        )
        while True:
            _rebuild_checks(
                records,
                selected,
                syntax,
                attempts,
                contexts,
                receipts,
                output_limit,
                events,
            )
            _forward_rebuild(events)
            outcome = next(execution, None)
            if outcome is None:
                break
            events.extend(outcome.events)
            _retain_outcome(outcome, retained, contexts, receipts)
            retained["secondary_failures"] = facts(events)
            if outcome.abort is not None:
                raise outcome.abort
        _require_rebuild(
            len(contexts) == len(attempts)
            and len(receipts) == len(attempts)
            and [context.ordinal for context in contexts] == list(range(len(attempts)))
            and [receipt.ordinal for receipt in receipts] == list(range(len(attempts)))
            and all(
                attempt.get("status") == "compiled-unverified"
                and attempt.get("terminal") is True
                and attempt.get("raw_exit") == 0
                and attempt.get("failure") is None
                and attempt.get("launch") == "observed"
                and type(attempt.get("artifact")) is dict
                and len(attempt.get("stages", [])) == len(recipe.STAGE_TEMPLATES)
                and all(
                    stage["process"].get("raw_exit") == 0
                    and stage["process"].get("failure") is None
                    and stage["process"].get("owner_observation") is None
                    for stage in attempt["stages"]
                )
                and attempt["artifact"] in attempt["stages"][-1]["produces"]
                and attempt["artifact"]["path"]
                == str(contexts[attempt["ordinal"]].directory / "unit.o")
                for attempt in attempts
            ),
            "one or more submitted compiler profiles did not produce a fresh verified execution",
        )
    except Exception as error:
        retain(
            events,
            error,
            phase="independent-rebuild",
            path=workspace,
            origin="cleanup"
            if isinstance(error, ProcessCleanupError)
            else "policy"
            if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
            else "unknown",
        )
    _rebuild_checks(
        records,
        selected,
        syntax,
        attempts,
        contexts,
        receipts,
        output_limit,
        events,
    )
    destination = None
    if syntax is not None and "source" in syntax.record:
        try:
            _require_rebuild(
                workspace.is_dir()
                and not workspace.is_symlink()
                and workspace.resolve() == workspace,
                "fresh rebuild workspace changed before terminal persistence",
            )
            path = workspace / "rebuild-result.json"
            _require_rebuild(
                not path.exists() and not path.is_symlink(),
                "terminal rebuild result already exists",
            )
            destination = path
        except Exception as error:
            retain(events, error, phase="rebuild-result", path=workspace)
    pending = primary(events)
    if pending is None:
        try:
            _forward_rebuild(events)
        except Exception as error:
            retain(events, error, phase="rebuild-completion", path=workspace)
        pending = primary(events)
    for attempt in attempts:
        if attempt["status"] == "unattempted":
            reason = (
                "unchanged source refused before profile compilation"
                if syntax is not None
                and syntax.record.get("c89", {}).get("raw_exit") not in {None, 0}
                else f"{type(pending).__name__}: {pending}"
            )
            attempt.update(
                status="source-refused"
                if syntax is not None
                and syntax.record.get("c89", {}).get("raw_exit") not in {None, 0}
                else "rebuild-interrupted",
                launch="prelaunch",
                failure=reason,
            )
    retained.update(
        status="rebuilt-unverified" if pending is None else "refused",
        failure=None if pending is None else f"{type(pending).__name__}: {pending}",
        secondary_failures=facts(events),
    )
    if destination is not None:
        try:
            pending_file = workspace / "rebuild-result.pending.json"
            data = encode_json(retained, path=destination, events=events)
            _require_rebuild(
                len(data) <= artifact_limit,
                "terminal rebuild result exceeds its artifact bound",
            )
            write_bytes(pending_file, data, events=events, create_parents=False)
            pending_file.rename(destination)
        except Exception as error:
            retain(events, error, phase="rebuild-result", path=workspace)
            pending = primary(events)
            retained.update(
                status="refused",
                failure=f"{type(pending).__name__}: {pending}",
                secondary_failures=facts(events),
            )
    if pending is not None:
        raise pending
    return retained


@dataclass(frozen=True)
class VerificationAccount:
    candidate: Path
    associated: FileIdentity
    generation_run: FileIdentity
    member_key: str


@dataclass(frozen=True)
class VerificationAttempt:
    source: FileIdentity
    ordinal: int
    candidate: Path
    generation_run: FileIdentity | None
    member_key: str
    associated: FileIdentity | None
    record: bytes
    observations: bytes
    state: Literal["failed", "missing"]


@dataclass(frozen=True)
class VerificationDiscovery:
    accounts: tuple[VerificationAccount, ...]
    attempts: tuple[VerificationAttempt, ...]
    terminals: tuple[FileIdentity, ...] = ()


def _source_failures(rows):
    _require_rebuild(
        type(rows) is list
        and all(
            type(row) is dict
            and set(row)
            == {"origin", "phase", "path", "type", "message", "capture_failures"}
            and row["origin"] in {"cleanup", "ordinary", "policy", "unknown"}
            and all(
                type(row[key]) is str for key in ("phase", "path", "type", "message")
            )
            and type(row["capture_failures"]) is list
            for row in rows
        ),
        "terminal source ordered failure observation shape differs",
    )


def _source_attempt(
    row, source, ordinal, member_row, expected, prepared_run, allowance, events
):
    fields = {"candidate", "member_key", "generation_run", "associated_record"}
    _require_rebuild(
        type(row) is dict
        and fields <= set(row) <= fields | {"failure", "raw"}
        and row["member_key"] in expected
        and type(row["candidate"]) is str,
        "terminal source attempt has a foreign member or unsupported shape",
    )
    candidate = Path(row["candidate"])
    _require_rebuild(
        candidate.is_absolute()
        and candidate.resolve() == candidate
        and not candidate.is_symlink(),
        "terminal source attempt candidate is not canonical",
    )
    generation = None
    if row["generation_run"] is not None:
        value = _submitted_document(
            row["generation_run"],
            allowance.artifact_limit,
            events,
            reader=allowance.read_bytes,
        )
        reference = row["generation_run"]
        _require_rebuild(
            Path(reference["path"]) == candidate.parent / "run.json"
            and type(value.get("selected")) is list
            and row["member_key"] in value["selected"]
            and len(value["selected"]) == len(set(value["selected"]))
            and set(value["selected"]) <= set(expected)
            and candidate.name
            == f"member-{value['selected'].index(row['member_key']):03d}"
            and type(value.get("problem")) is dict
            and value["problem"].get("run_dir") == str(prepared_run),
            "terminal source attempt generation/member/prepared association differs",
        )
        generation = FileIdentity(
            Path(reference["path"]), reference["bytes"], reference["sha256"]
        )
    failure = row.get("failure")
    _require_rebuild(
        "failure" not in row or type(failure) is str and bool(failure),
        "terminal source attempt failure observation differs",
    )
    raw = row.get("raw", {})
    _require_rebuild(
        type(raw) is dict
        and ("raw" not in row or set(raw) == {"rebuild", "comparison", "associated"}),
        "terminal source attempt raw owner inventory differs",
    )
    failed = failure is not None
    for name, owner in raw.items():
        _require_rebuild(
            type(owner) is dict
            and (
                not owner
                or owner.get("operation")
                == {
                    "rebuild": "rebuild",
                    "comparison": "comparison",
                    "associated": "associated-verification",
                }[name]
            ),
            "terminal source attempt raw operation differs",
        )
        _require_rebuild(
            "identity" not in owner or owner["identity"] == expected[row["member_key"]],
            "terminal source attempt raw member identity differs",
        )
        _require_rebuild(
            "submitted_run" not in owner
            or owner["submitted_run"] == row["generation_run"],
            "terminal source attempt raw generation identity differs",
        )
        if "submitted_result" in owner:
            result = _submitted_document(
                owner["submitted_result"],
                allowance.artifact_limit,
                events,
                reader=allowance.read_bytes,
            )
            _require_rebuild(
                Path(owner["submitted_result"]["path"]) == candidate / "result.json"
                and result.get("identity") == expected[row["member_key"]],
                "terminal source attempt raw submitted member differs",
            )
        _require_rebuild(
            owner.get("failure") is None
            or type(owner["failure"]) is str
            and bool(owner["failure"]),
            "terminal source attempt raw failure differs",
        )
        _source_failures(owner.get("secondary_failures", []))
        failed = (
            failed
            or owner.get("failure") is not None
            or bool(owner.get("secondary_failures"))
            or owner.get("status") == "refused"
        )
        for reference in _nested_records(owner):
            _authenticated_bytes(
                reference, allowance.artifact_limit, events, reader=allowance.read_bytes
            )
    observations = {
        "unknown_mode_attempts": [],
        "modes": {
            mode: [] for mode in ("autonomous", "signature_only", "metadata_assisted")
        },
    }
    if member_row is not None:
        _require_rebuild(
            type(member_row.get("unknown_mode_attempts")) is list,
            "terminal member lacks its unknown-mode attempt inventory",
        )
        for observed in member_row["unknown_mode_attempts"]:
            _require_rebuild(
                type(observed) is dict,
                "terminal member unknown-mode observation differs",
            )
            if observed.get("identity") == str(candidate):
                _require_rebuild(
                    observed.get("generation_run") == row["generation_run"]
                    and type(observed.get("failures")) is list,
                    "terminal member unknown-mode generation/failure association differs",
                )
                _source_failures(observed["failures"])
                observations["unknown_mode_attempts"].append(observed)
                failed = failed or bool(observed["failures"])
        for mode, view in member_row["modes"].items():
            _require_rebuild(
                type(view) is dict and type(view.get("attempts")) is list,
                "terminal member mode attempt inventory differs",
            )
            for observed in view["attempts"]:
                _require_rebuild(
                    type(observed) is dict,
                    "terminal member profile observation differs",
                )
                if observed.get("candidate_identity") == str(candidate):
                    observations["modes"][mode].append(observed)
    associated = None
    if row["associated_record"] is not None:
        _require_rebuild(
            set(row) == fields and generation is not None,
            "associated source attempt carries conflicting failure/raw evidence or lacks a generation",
        )
        path = _regular_record(row["associated_record"], allowance.artifact_limit)
        _authenticated_bytes(
            row["associated_record"],
            allowance.artifact_limit,
            events,
            reader=allowance.read_bytes,
        )
        associated = FileIdentity(
            path, row["associated_record"]["bytes"], row["associated_record"]["sha256"]
        )
    return VerificationAttempt(
        source,
        ordinal,
        candidate,
        generation,
        row["member_key"],
        associated,
        encode_json(row, path=source.path, events=events),
        encode_json(observations, path=source.path, events=events),
        "failed" if failed else "missing",
    )


def _terminal_associated(path, prepared_run, allowance, events):
    _require_rebuild(
        path.name == "associated-result.json",
        "only terminal associated-result.json may supply evidence",
    )
    _require_rebuild(
        not (path.parent / "associated-result.pending.json").exists()
        and not (path.parent / "refusal.json").exists(),
        "pending or refused consuming result cannot authorize terminal associated reuse",
    )
    record = _bounded_file_record(
        path, allowance.artifact_limit, events, reader=allowance.read_bytes
    )
    value = _submitted_document(
        record, allowance.artifact_limit, events, reader=allowance.read_bytes
    )
    _require_rebuild(
        value.get("operation") == "associated-verification"
        and value.get("status") in {"measured-failed", "measured-unresolved"}
        and value.get("failure") is None
        and value.get("secondary_failures") == []
        and "attempted_output" not in value
        and value.get("prepared_run") == str(prepared_run)
        and value.get("denominator") == contract.ADMITTED
        and value.get("failed_read_reserved_bytes") == 0
        and value.get("read_accounting") == "bounded-input-snapshots",
        "associated terminal owner is missing, refused, pending or bound elsewhere",
    )
    return FileIdentity(path, record["bytes"], record["sha256"]), value


def discover_verification_accounts(
    directories, prepared_run, allowance, events, *, members=(), exclusions=()
):
    accounts, attempts, terminals, seen = [], [], [], set()
    expected = {row.key: asdict(row) for row in members}
    for directory in directories:
        try:
            terminal = None
            _require_rebuild(
                directory.is_absolute()
                and directory.resolve() == directory
                and directory.is_dir()
                and not directory.is_symlink(),
                "verification directory must be canonical and regular",
            )
            paths, source_attempts = [], []
            _require_rebuild(
                not (directory / "refusal.json").exists()
                and not any(
                    (directory / name).exists()
                    for name in (
                        "verification-result.pending.json",
                        "report-result.pending.json",
                        "associated-result.pending.json",
                    )
                ),
                "verification directory retains pending or refused consumption",
            )
            if (directory / "associated-result.json").exists():
                paths.append((directory / "associated-result.json", None))
            else:
                names = [
                    name
                    for name in ("verification-result.json", "report-result.json")
                    if (directory / name).exists()
                ]
                _require_rebuild(
                    len(names) == 1,
                    "verification requires one terminal command or associated account",
                )
                command_record = _bounded_file_record(
                    directory / names[0],
                    allowance.artifact_limit,
                    events,
                    reader=allowance.read_bytes,
                )
                command = _submitted_document(
                    command_record,
                    allowance.artifact_limit,
                    events,
                    reader=allowance.read_bytes,
                )
                _require_rebuild(
                    command.get("schema") == "retdec118.reporting/v1"
                    and command.get("status") in {"complete", "incomplete"}
                    and command.get("exit") in {0, 1}
                    and command.get("prepared_run") == str(prepared_run)
                    and command.get("denominator") == contract.ADMITTED
                    and command.get("publication") == "terminal",
                    "command account is pending, refused or bound elsewhere",
                )
                actual = command.get("members")
                _require_rebuild(
                    type(actual) is list
                    and len(actual) == len(expected) == contract.ADMITTED
                    and all(
                        type(row) is dict
                        and type(row.get("identity")) is dict
                        and row["identity"].get("key") in expected
                        and row["identity"] == expected[row["identity"]["key"]]
                        and set(row.get("modes", {}))
                        == {"autonomous", "signature_only", "metadata_assisted"}
                        for row in actual
                    )
                    and len({row["identity"]["key"] for row in actual})
                    == contract.ADMITTED,
                    "terminal command exact member identities/modes differ from authenticated fixed corpus",
                )
                actual_exclusions = command.get("exclusions")
                _require_rebuild(
                    type(actual_exclusions) is list
                    and len(actual_exclusions) == len(exclusions) == contract.EXCLUSIONS
                    and sorted(actual_exclusions, key=lambda row: row["case_id"])
                    == sorted(exclusions, key=lambda row: row["case_id"]),
                    "terminal command exact exclusion identities/reasons differ",
                )
                _require_rebuild(
                    type(command.get("attempts")) is list
                    and len(command["attempts"]) <= 65536,
                    "terminal command attempt inventory differs",
                )
                source = FileIdentity(
                    Path(command_record["path"]),
                    command_record["bytes"],
                    command_record["sha256"],
                )
                terminal = source
                member_rows = {row["identity"]["key"]: row for row in actual}
                _require_rebuild(
                    type(command.get("generations")) is list
                    and all(type(row) is dict for row in command["generations"]),
                    "terminal command generation provenance inventory differs",
                )
                for ordinal, row in enumerate(command["attempts"]):
                    attempt = _source_attempt(
                        row,
                        source,
                        ordinal,
                        member_rows.get(row.get("member_key"))
                        if type(row) is dict
                        else None,
                        expected,
                        prepared_run,
                        allowance,
                        events,
                    )
                    _require_rebuild(
                        attempt.generation_run is None
                        or any(
                            value.get("run") == attempt.generation_run.as_record()
                            and value.get("directory")
                            == str(attempt.generation_run.path.parent)
                            for value in command["generations"]
                        ),
                        "terminal source attempt generation is absent from its command provenance inventory",
                    )
                    source_attempts.append(attempt)
                    if attempt.associated is not None:
                        paths.append((attempt.associated.path, attempt))
            for path, declared_attempt in paths:
                associated, value = _terminal_associated(
                    path, prepared_run, allowance, events
                )
                comparison_record = value.get("comparison_record")
                _require_rebuild(
                    _regular_record(comparison_record, allowance.artifact_limit).name
                    == "comparison-result.json",
                    "associated account lacks a canonical terminal comparison",
                )
                measured = _submitted_document(
                    comparison_record,
                    allowance.artifact_limit,
                    events,
                    reader=allowance.read_bytes,
                )
                _require_rebuild(
                    measured.get("status") == "measured-unverified"
                    and measured.get("failure") is None
                    and measured.get("secondary_failures") == [],
                    "refused comparison cannot discover a favorable candidate",
                )
                rebuilt, _, _, _, _, _, _ = _load_rebuild(
                    measured["rebuild_record"],
                    allowance.artifact_limit,
                    events,
                    reader=allowance.read_bytes,
                    verify_execution=False,
                )
                reference = rebuilt["submitted_run"]
                generation = FileIdentity(
                    Path(reference["path"]), reference["bytes"], reference["sha256"]
                )
                candidate = Path(rebuilt["submitted_result"]["path"]).parent
                _require_rebuild(
                    measured["identity"] == value["identity"] == rebuilt["identity"],
                    "terminal candidate chain identities differ",
                )
                _require_rebuild(
                    rebuilt["identity"].get("key") in expected
                    and rebuilt["identity"] == expected[rebuilt["identity"]["key"]],
                    "terminal associated candidate is outside the exact fixed membership",
                )
                if declared_attempt is not None:
                    _require_rebuild(
                        (
                            declared_attempt.candidate,
                            declared_attempt.associated,
                            declared_attempt.generation_run,
                            declared_attempt.member_key,
                        )
                        == (
                            candidate,
                            associated,
                            generation,
                            rebuilt["identity"]["key"],
                        ),
                        "source attempt does not identify its authenticated associated chain",
                    )
                else:
                    row = dict(
                        candidate=str(candidate),
                        member_key=rebuilt["identity"]["key"],
                        generation_run=generation.as_record(),
                        associated_record=associated.as_record(),
                    )
                    source_attempts.append(
                        _source_attempt(
                            row,
                            associated,
                            0,
                            None,
                            expected,
                            prepared_run,
                            allowance,
                            events,
                        )
                    )
                key = (associated.path, associated.bytes, associated.sha256)
                if key not in seen:
                    accounts.append(
                        VerificationAccount(
                            candidate,
                            associated,
                            generation,
                            rebuilt["identity"]["key"],
                        )
                    )
                    seen.add(key)
            attempts.extend(source_attempts)
            if terminal is not None and terminal not in terminals:
                terminals.append(terminal)
        except Exception as error:
            retain(events, error, phase="terminal-discovery", path=directory)
    return VerificationDiscovery(tuple(accounts), tuple(attempts), tuple(terminals))


def discover_report_generation_runs(accounts, allowance, events, *, attempts=()):
    runs, seen = [], set()
    for account in (
        *accounts,
        *(row for row in attempts if row.generation_run is not None),
    ):
        try:
            record = account.generation_run.as_record()
            content = _authenticated_bytes(
                record, allowance.artifact_limit, events, reader=allowance.read_bytes
            )
            key = (
                account.generation_run.path,
                account.generation_run.bytes,
                account.generation_run.sha256,
            )
            if key not in seen:
                runs.append(
                    prepare.GenerationRunInput(
                        account.generation_run.path.parent,
                        ConsumedFile(account.generation_run, content),
                        (),
                    )
                )
                seen.add(key)
        except Exception as error:
            retain(
                events,
                error,
                phase="report-generation-discovery",
                path=account.generation_run.path,
            )
    return tuple(runs)


@dataclass(frozen=True)
class ProfileInspection:
    profile_id: str
    ordinal: int
    profile_identity: str
    predicates: tuple[obligations.PredicateResult, ...]
    source: nativeoutput.SourceFacts | None
    representation: nativeoutput.RepresentationFacts | None
    associated: obligations.AssociatedClosureFacts | None
    raw: dict
    failures: tuple[FailureEvent, ...]


@dataclass(frozen=True)
class CandidateInspection:
    member: provenance.BoundMember
    identity: str
    generation_run: FileIdentity | None
    authenticated_mode: str | None
    predicates: tuple[obligations.PredicateResult, ...]
    profiles: tuple[ProfileInspection, ...]
    raw: dict
    failures: tuple[FailureEvent, ...]


def inspect_retained_attempt(
    attempt, member, shared, generation, allowance, prepared_run
):
    _require_rebuild(
        attempt.associated is None
        and attempt.member_key == member.key
        and (
            attempt.generation_run is None
            or generation is not None
            and generation.run == attempt.generation_run
        ),
        "retained negative attempt has no exact member/generation association",
    )
    supplied = inspect_supplied_source(
        attempt.candidate, member, shared, generation, allowance, prepared_run
    )
    return CandidateInspection(
        member,
        str(attempt.candidate),
        attempt.generation_run,
        None if supplied is None else supplied.authenticated_mode,
        (
            *shared.predicates,
            *(() if generation is None else generation.predicates),
            *(
                supplied.predicates
                if supplied is not None
                else (
                    obligations.PredicateResult(
                        "generation",
                        attempt.state,
                        "authenticated source attempt has no replayable associated result",
                    ),
                )
            ),
            obligations.PredicateResult(
                "terminal-owner",
                attempt.state,
                "retained failure/unattempted history supplies no positive owner evidence",
            ),
        ),
        (),
        dict(
            **({} if supplied is None else {"supplied": supplied.raw}),
            source_attempt=dict(
                source=attempt.source.as_record(),
                ordinal=attempt.ordinal,
                record=nativeoutput.parse_json(attempt.record, len(attempt.record)),
                observations=nativeoutput.parse_json(
                    attempt.observations, len(attempt.observations)
                ),
            ),
        ),
        () if supplied is None else supplied.events,
    )


def inspect_supplied_source(
    candidate, member, shared, generation, allowance, prepared_run
):
    """Reauthenticate an ordinary supplied source independently of evaluator outcomes."""
    if generation is None or generation.run is None:
        return None
    events = []
    run = _submitted_document(
        generation.run.as_record(),
        allowance.artifact_limit,
        events,
        reader=allowance.read_bytes,
    )
    if run.get("producer", {}).get("kind") != invocation.SUPPLIED_KIND:
        return None
    result = _submitted_document(
        _bounded_file_record(
            candidate / "result.json",
            allowance.artifact_limit,
            events,
            reader=allowance.read_bytes,
        ),
        allowance.artifact_limit,
        events,
        reader=allowance.read_bytes,
    )
    _require_rebuild(
        result.get("identity") == asdict(member),
        "supplied source names a different member",
    )
    return invocation.inspect_supplied_attempt(
        candidate, result, run, shared.binding, prepared_run, allowance.read_bytes
    )


def _inspect_supplied_profiles(inspection, owner, member, prepared_run, shared, events):
    original = provenance.original_slice(
        shared.binding,
        prepared_run,
        target=member.target,
        image_id=member.image_id,
        pc=member.start,
        extent=dict(start=member.start, end=member.end),
        subject=member.key,
        io_mode="exact",
        events=events,
    )
    profiles = []
    for measured in owner["profiles"]:
        row, profile, local = measured["row"], measured["profile"], []
        predicates = [
            obligations.PredicateResult(
                "unchanged-c89-compilation",
                "matched",
                "independent unchanged C89 and profile captures reproduced",
            )
        ]
        linked = measured["linked_bytes"]
        try:
            if linked is None:
                predicates.append(
                    obligations.PredicateResult(
                        "linkage", "missing", "profile has no complete linked ELF"
                    )
                )
            else:
                function = read_function_bytes(
                    linked,
                    function_name=inspection.unit.entry_symbol,
                    address=member.start,
                )
                exact = FunctionComparison(
                    OriginalFunction(
                        member.start, member.end, original.instruction_bytes
                    ),
                    function,
                )
                predicates.extend(
                    (
                        obligations.PredicateResult(
                            "complete-function-bytes",
                            "matched" if exact.outcome == "exact" else "failed",
                            f"complete function comparison: {exact.outcome}",
                        ),
                        obligations.PredicateResult(
                            "linkage",
                            "matched" if row["status"] == "measured" else "failed",
                            "independent link capture and complete ELF reproduced",
                        ),
                    )
                )
        except Exception as error:
            retain(
                local,
                error,
                phase="supplied-profile-inspection",
                path=profile["profile_id"],
            )
            predicates.append(
                obligations.PredicateResult("linkage", "failed", str(error))
            )
        for name in (
            "clean-c",
            "type-semantics",
            "conditional-representation",
            "entry-domain",
            "whole-effects",
            "associated-contents",
            "placement",
            "initialization",
            "relocations",
            "bindings",
            "inventory-closure",
            "required-runtime-questions",
        ):
            predicates.append(
                obligations.PredicateResult(
                    name,
                    "unknown",
                    "ordinary supplied source has no original-only machine contract; retained measurements do not establish this predicate",
                )
            )
        profiles.append(
            ProfileInspection(
                profile["profile_id"],
                profile["ordinal"],
                profile["identity"],
                tuple(predicates),
                None,
                None,
                None,
                dict(
                    comparison=row,
                    compiler=deepcopy(
                        owner["rebuilt"]["profile_attempts"][profile["ordinal"]]
                    ),
                    c89=deepcopy(owner["rebuilt"]["c89"]),
                    measurements=measured["measurement"].as_record(),
                ),
                tuple(local),
            )
        )
    return tuple(profiles)


def inspect_candidate(
    candidate, associated_directory, shared, generation, member, allowance, prepared_run
):
    from harness.runtime.session import (
        inspect_runtime_capture,
        inspect_state_comparison,
    )

    events, profiles, raw = [], [], {}
    mode = None
    predicates = [*shared.predicates, *generation.predicates]
    try:
        terminal, account = _terminal_associated(
            associated_directory / "associated-result.json",
            prepared_run,
            allowance,
            events,
        )
        owner = {}
        reproduced = verify_associated_candidate(
            comparison_record=Path(account["comparison_record"]["path"]),
            prepared_run=prepared_run,
            workspace=associated_directory,
            output_limit=account["output_limit"],
            artifact_limit=allowance.artifact_limit,
            input_read_limit=allowance.read_limit,
            retained=raw,
            allowance=allowance,
            publish=False,
            inspection=owner,
        )
        for field in (
            "identity",
            "authority",
            "inventories",
            "profiles",
            "input_census",
            "measured_member",
            "unmeasured_members",
            "inventory_summary",
            "inventory_closure",
        ):
            _require_rebuild(
                reproduced.get(field) == account.get(field),
                f"terminal associated {field} does not reproduce",
            )
        rebuilt, run = owner["rebuilt"], owner["run"]
        _require_rebuild(
            Path(rebuilt["submitted_result"]["path"]).parent == candidate
            and rebuilt["identity"] == asdict(member)
            and generation.run is not None
            and rebuilt["submitted_run"] == generation.run.as_record(),
            "candidate/member/generation association differs",
        )
        result = _submitted_document(
            rebuilt["submitted_result"],
            allowance.artifact_limit,
            events,
            reader=allowance.read_bytes,
        )
        if run.get("producer", {}).get("kind") == invocation.SUPPLIED_KIND:
            supplied = invocation.inspect_supplied_attempt(
                candidate,
                result,
                run,
                shared.binding,
                prepared_run,
                allowance.read_bytes,
            )
            events.extend(supplied.events)
            predicates.extend(supplied.predicates)
            mode = supplied.authenticated_mode
            raw["supplied"] = supplied.raw
            _require_rebuild(
                supplied.unit is not None and not supplied.events,
                "ordinary source cannot be reproduced",
            )
            profiles = list(
                _inspect_supplied_profiles(
                    supplied, owner, member, prepared_run, shared, events
                )
            )
            predicates.append(
                obligations.PredicateResult(
                    "terminal-owner",
                    "matched",
                    "independent terminal measurements reproduced",
                )
            )
            _authenticated_bytes(
                terminal.as_record(),
                allowance.artifact_limit,
                events,
                reader=allowance.read_bytes,
            )
            return CandidateInspection(
                member,
                str(candidate),
                generation.run,
                mode,
                tuple(predicates),
                tuple(profiles),
                deepcopy(raw),
                tuple(events),
            )
        native = invocation.inspect_native(
            candidate, result, run, shared.binding, prepared_run, allowance.read_bytes
        )
        events.extend(native.events)
        predicates.extend(native.predicates)
        mode = native.authenticated_mode
        _require_rebuild(
            native.unit is not None and not native.events,
            "native source/contract cannot be reproduced",
        )
        projected = callcontracts.project_validated_contract(native.unit)
        raw["validated_contract"] = asdict(projected) if projected else None
        body = (
            callcontracts.project_validated_body(native.unit)
            if projected is None
            else None
        )
        facts = projected if projected is not None else body
        derive_leaf = (
            machinecode.derive_empty_return_leaf
            if body
            else machinecode.derive_byte_comparison_leaf
            if facts is not None
            and facts.accesses
            and facts.accesses[0][1:] == ("read", 8)
            else machinecode.derive_zero_return_leaf
        )
        raw["native"] = deepcopy(result["native"])
        original = provenance.original_slice(
            shared.binding,
            prepared_run,
            target=member.target,
            image_id=member.image_id,
            pc=member.start,
            extent=dict(start=member.start, end=member.end),
            subject=member.key,
            io_mode="exact",
            events=events,
        )
        original_leaf = derive_leaf(member, original)
        generated = allowance.read(
            candidate / "generated.c", nativeoutput.SOURCE_LIMIT, events
        )
        tokens = allowance.read(
            candidate / "work/tokens.json", nativeoutput.TOKEN_LIMIT, events
        )
        for inspected, context in zip(
            owner["profiles"], owner["contexts"], strict=True
        ):
            local = []
            row, profile = inspected["row"], inspected["profile"]
            source_facts = representation = associated = None
            profile_predicates = []
            try:
                recipe_file = allowance.read(
                    context.directory / "recipe.json", 1 << 20, local
                )
                actual_recipe = nativeoutput.parse_json(recipe_file.content, 1 << 20)
                components = {
                    name: allowance.read(
                        Path(actual_recipe["tools"][name]),
                        allowance.artifact_limit,
                        local,
                    )
                    for name in ("driver", "cc1", "cpp")
                }
                integer = recipe.inspect_compiler_integer_facts(
                    context,
                    profile_identity=profile["identity"],
                    ordinal=profile["ordinal"],
                    recipe=recipe_file,
                    components=components,
                )
                source_file = allowance.read(
                    Path(actual_recipe["source"]), nativeoutput.SOURCE_LIMIT, local
                )
                header = allowance.read(
                    Path(rebuilt["prelude"]["path"]), 1 << 20, local
                )
                _require_rebuild(
                    actual_recipe["include_roots"]
                    == [str(Path(rebuilt["prelude"]["path"]).parent)],
                    "source rule include resolution differs from exact staged prelude",
                )
                if facts is not None:
                    source_facts = nativeoutput.derive_source_facts(
                        native.unit,
                        facts,
                        generated.content,
                        source_file.content,
                        tokens.content,
                        header.content,
                        integer,
                    )
                    profile_predicates.extend(source_facts.predicates)
                else:
                    profile_predicates.append(
                        obligations.PredicateResult(
                            "type-semantics",
                            "unknown",
                            "native body proposal retains an unknown original function declaration",
                        )
                    )
                linked_bytes = inspected["linked_bytes"]
                if linked_bytes is None:
                    profile_predicates.append(
                        obligations.PredicateResult(
                            "linkage", "missing", "profile has no complete linked ELF"
                        )
                    )
                else:
                    function = read_function_bytes(
                        linked_bytes,
                        function_name=native.unit.entry_symbol,
                        address=member.start,
                    )
                    compiled_leaf = derive_leaf(member, function)
                    symbol_producer = obligations.derive_symbol_producer_facts(
                        tuple(
                            allowance.read(
                                Path(path), allowance.artifact_limit, local
                            ).content
                            for path in actual_recipe["intermediates"]
                        ),
                        inspected["object_image"],
                        inspected["linked_image"],
                        restricted_link=inspected["_restricted_link"],
                    )
                    entry = nativeoutput.derive_entry_domain_facts(
                        member, original_leaf, compiled_leaf
                    )
                    if facts is not None:
                        representation = nativeoutput.derive_representation_facts(
                            source_facts, original_leaf, compiled_leaf, entry, facts
                        )
                        profile_predicates.extend(
                            (
                                *representation.conditional_agreement,
                                *representation.whole_schedule_agreement,
                                *representation.prerequisite_closure,
                            )
                        )
                    exact = FunctionComparison(
                        OriginalFunction(
                            member.start, member.end, original.instruction_bytes
                        ),
                        function,
                    )
                    profile_predicates.extend(
                        (
                            obligations.PredicateResult(
                                "complete-function-bytes",
                                "matched" if exact.outcome == "exact" else "failed",
                                f"complete function comparison: {exact.outcome}",
                            ),
                            obligations.PredicateResult(
                                "linkage",
                                "matched" if row["status"] == "measured" else "failed",
                                "reproduced independent link ownership and complete ELF",
                            ),
                        )
                    )
                    associated = obligations.derive_associated_closure(
                        member,
                        profile["profile_id"],
                        owner["requirements"],
                        owner["original_facts"],
                        inspected["object_image"],
                        inspected["linked_image"],
                        inspected["measurement"],
                        original_leaf,
                        compiled_leaf,
                        entry,
                        native.unit.entry_symbol,
                        symbol_producer=symbol_producer,
                        candidate_backing=native.unit.backing,
                        source=source_facts,
                        link_inputs=inspected["row"],
                    )
                    profile_predicates.extend(associated.predicates)
                profile_predicates.extend(
                    (
                        obligations.PredicateResult(
                            "unchanged-c89-compilation",
                            "matched",
                            "reproduced unchanged syntax and complete successful compiler recipe captures",
                        ),
                        obligations.PredicateResult(
                            "required-runtime-questions",
                            "matched"
                            if representation is not None
                            and all(
                                item.status == "matched"
                                for item in (
                                    *representation.conditional_agreement,
                                    *representation.whole_schedule_agreement,
                                )
                            )
                            else "unknown",
                            "complete finite conditional rule has no additional runtime behavior question; valid entry/domain remains a separate required predicate"
                            if representation is not None
                            else "unsupported effects leave runtime question/execution scope unresolved",
                        ),
                    )
                )
            except Exception as error:
                retain(local, error, phase="profile-inspection", path=context.directory)
                profile_predicates.append(
                    obligations.PredicateResult(
                        "terminal-owner", "failed", f"{type(error).__name__}: {error}"
                    )
                )
            profiles.append(
                ProfileInspection(
                    profile["profile_id"],
                    profile["ordinal"],
                    profile["identity"],
                    tuple(profile_predicates),
                    source_facts,
                    representation,
                    associated,
                    dict(
                        comparison=row,
                        compiler=deepcopy(
                            rebuilt["profile_attempts"][profile["ordinal"]]
                        ),
                        c89=deepcopy(rebuilt["c89"]),
                        measurements=inspected["measurement"].as_record(),
                    ),
                    tuple(local),
                )
            )
        runtime_dir = candidate / "runtime"
        if runtime_dir.exists():
            capture_facts = inspect_runtime_capture(runtime_dir, allowance.read_bytes)
            raw["runtime_capture"] = dict(
                receipt=capture_facts.receipt.as_record(),
                status=capture_facts.status,
                limitations=list(capture_facts.limitations),
            )
            if any(
                row.path.name == "comparison.json" for row in capture_facts.captures
            ):
                state_facts = inspect_state_comparison(
                    capture_facts, allowance.read_bytes
                )
                raw["runtime_capture"]["selected_subtree"] = dict(
                    path=state_facts.subtree,
                    report=state_facts.report.as_record(),
                    schema=state_facts.schema.as_record(),
                    before=state_facts.before.as_record(),
                    after=state_facts.after.as_record(),
                    observed_equal=state_facts.observed_equal,
                    compared_nodes=state_facts.compared_nodes,
                    compared_bytes=state_facts.compared_bytes,
                    changed_bytes=state_facts.changed_bytes,
                    difference_count=state_facts.difference_count,
                    reason=state_facts.reason,
                )
        predicates.append(
            obligations.PredicateResult(
                "terminal-owner",
                "matched",
                "terminal associated account and physical independent measurements reproduced",
            )
        )
        _authenticated_bytes(
            terminal.as_record(),
            allowance.artifact_limit,
            events,
            reader=allowance.read_bytes,
        )
    except Exception as error:
        retain(events, error, phase="candidate-inspection", path=candidate)
        predicates.append(
            obligations.PredicateResult(
                "terminal-owner", "failed", f"{type(error).__name__}: {error}"
            )
        )
    return CandidateInspection(
        member,
        str(candidate),
        generation.run,
        mode,
        tuple(predicates),
        tuple(profiles),
        deepcopy(raw),
        tuple(events),
    )


def _bounded_file_record(path: Path, limit: int, events, *, reader=None) -> dict:
    _require_rebuild(
        path.is_absolute()
        and path.resolve() == path
        and not path.is_symlink()
        and path.is_file(),
        "retained file must be canonical, regular and nonsymlinked",
    )
    if reader is None:
        data = read_bytes(path, limit, events)
    else:
        with contract.bounded_reads(limit, reader=reader):
            data = contract.read_bytes(
                path, "retained file", io_mode="exact", events=events, max_bytes=limit
            )
    _require_rebuild(len(data) <= limit, "retained file exceeds its artifact bound")
    record = dict(
        path=str(path), bytes=len(data), sha256=hashlib.sha256(data).hexdigest()
    )
    recipe.verify_inputs([record], events=events, reader=reader)
    return record


def _load_rebuild(
    rebuild_record,
    limit,
    events,
    *,
    reader=None,
    verify_execution=True,
    original_only=False,
):
    _require_rebuild(
        _regular_record(rebuild_record, limit).name == "rebuild-result.json",
        "comparison requires a pinned rebuild-result.json record",
    )
    rebuilt = _submitted_document(rebuild_record, limit, events, reader=reader)
    root = Path(rebuild_record["path"]).parent
    _require_rebuild(
        rebuilt.get("operation") == "rebuild"
        and rebuilt.get("status") == "rebuilt-unverified"
        and rebuilt.get("failure") is None
        and rebuilt.get("secondary_failures") == []
        and rebuilt.get("semantic_admission") is False
        and rebuilt.get("autonomous_exact") is False,
        "comparison requires an externally bound successful unverified rebuild",
    )
    _require_rebuild(
        all(
            type(rebuilt.get(name)) is int and rebuilt[name] > 0
            for name in ("output_limit", "artifact_limit")
        ),
        "accepted rebuild execution limits are absent or invalid",
    )
    result_record, run_record = (
        rebuilt.get("submitted_result"),
        rebuilt.get("submitted_run"),
    )
    result = _submitted_document(result_record, limit, events, reader=reader)
    run = _submitted_document(run_record, limit, events, reader=reader)
    _require_rebuild(
        not original_only or "producer" not in run,
        "original-only feedback refuses supplied or historical input",
    )
    submission = _adapt_submission(result, run, result_record, run_record, limit)
    identity, source, header, selected, records = (
        submission.identity,
        submission.source,
        submission.prelude,
        submission.selected,
        submission.records,
    )
    _require_rebuild(
        rebuilt.get("identity") == identity
        and rebuilt.get("entry_symbol") == submission.entry_symbol
        and type(rebuilt.get("entry_symbol")) is str
        and bool(rebuilt["entry_symbol"])
        and rebuilt.get("profiles_digest") == selected["profiles_digest"]
        and rebuilt.get("submitted_inputs") == records,
        "accepted rebuild submission, entry or catalog identity differs",
    )
    for name, expected_path, original in (
        ("source", root / "unit.c", source),
        ("prelude", root / prelude.PRELUDE_PATH, header),
    ):
        record = rebuilt.get(name)
        _require_rebuild(
            _regular_record(record, limit) == expected_path
            and (record["bytes"], record["sha256"])
            == (original["bytes"], original["sha256"]),
            "accepted rebuild source or prelude is not the unchanged fresh copy",
        )
    metadata = {
        name: _bounded_file_record(root / name, limit, events, reader=reader)
        for name in ("inputs.json", "syntax-result.json")
    }
    frozen_document = _submitted_document(
        metadata["inputs.json"], limit, events, reader=reader
    )
    immutable = [rebuilt["source"], rebuilt["prelude"], *records]
    _require_rebuild(
        frozen_document
        == dict(
            source=rebuilt["source"],
            prelude=rebuilt["prelude"],
            tools=selected,
            immutable=immutable,
        ),
        "accepted rebuild frozen inputs differ from its authenticated submission",
    )
    syntax_record = _submitted_document(
        metadata["syntax-result.json"], limit, events, reader=reader
    )
    syntax_keys = {
        "c89",
        "c89_launch",
        "c89_outputs",
        "c89_tool",
        "expected_inputs",
        "prelude",
        "secondary_failures",
        "source",
    }
    _require_rebuild(
        set(syntax_record) == syntax_keys
        and all(rebuilt.get(key) == value for key, value in syntax_record.items())
        and syntax_record.get("c89_launch") == "observed"
        and type(syntax_record.get("c89")) is dict
        and _complete_process(syntax_record["c89"]),
        "accepted rebuild C89 execution or receipt differs",
    )
    syntax = recipe.SyntaxAttempt(
        record=syntax_record,
        inputs=recipe.FrozenInputs(
            rebuilt["source"], rebuilt["prelude"], selected, immutable
        ),
        pending=None,
        events=(),
        call=recipe.c89_call(
            workspace=root,
            tools=selected,
            output_limit=rebuilt["output_limit"],
        ),
    )
    profiles = selected["profiles"]
    for name in (
        "profile_attempts",
        "profile_contexts",
        "profile_receipts",
        "profile_outcomes",
    ):
        _require_rebuild(
            type(rebuilt.get(name)) is list and len(rebuilt[name]) == len(profiles),
            "accepted rebuild ordered execution inventory is incomplete",
        )
    contexts, receipts, fresh_records = [], [], []
    for ordinal, profile in enumerate(profiles):
        attempt = rebuilt["profile_attempts"][ordinal]
        context, receipt, outcome = (
            rebuilt[name][ordinal]
            for name in ("profile_contexts", "profile_receipts", "profile_outcomes")
        )
        directory = root if ordinal == 0 else root / "profiles" / f"{ordinal:03d}"
        _require_rebuild(
            type(context) is dict
            and set(context) == {"ordinal", "directory", "recipe_data"}
            and type(context["ordinal"]) is int
            and context["ordinal"] == ordinal
            and context["directory"] == str(directory)
            and type(context["recipe_data"]) is str
            and len(context["recipe_data"]) <= 2 * min(limit, 1 << 20)
            and type(receipt) is dict
            and set(receipt) == {"ordinal", "path", "data", "bytes", "sha256"}
            and type(receipt["ordinal"]) is int
            and receipt["ordinal"] == ordinal
            and receipt["path"] == str(directory / "profile-result.json")
            and type(receipt["data"]) is str
            and len(receipt["data"]) <= 2 * min(limit, 1 << 20),
            "accepted rebuild typed context or receipt identity differs",
        )
        declared_context = recipe.DeclaredProfileContext(
            ordinal, directory, bytes.fromhex(context["recipe_data"])
        )
        declared_receipt = recipe.ProfileReceipt(
            ordinal,
            Path(receipt["path"]),
            bytes.fromhex(receipt["data"]),
            receipt["bytes"],
            receipt["sha256"],
        )
        _require_rebuild(
            type(attempt) is dict
            and attempt.get("profile_id") == profile["profile_id"]
            and attempt.get("profile_identity") == profile["identity"]
            and type(attempt.get("ordinal")) is int
            and attempt["ordinal"] == ordinal
            and attempt.get("status") == "compiled-unverified"
            and attempt.get("terminal") is True
            and attempt.get("raw_exit") == 0
            and attempt.get("exit_code") == 0
            and attempt.get("failure") is None
            and attempt.get("secondary_failures") == []
            and attempt.get("launch") == "observed"
            and type(attempt.get("stages")) is list
            and len(attempt["stages"]) == len(recipe.STAGE_TEMPLATES)
            and all(
                type(stage) is dict
                and type(stage.get("process")) is dict
                and _complete_process(stage["process"])
                for stage in attempt["stages"]
            )
            and type(attempt.get("artifact")) is dict
            and attempt["artifact"] in attempt["stages"][-1]["produces"]
            and _regular_record(attempt["artifact"], limit) == directory / "unit.o"
            and nativeoutput.parse_json(declared_receipt.data, limit) == attempt
            and type(outcome) is dict
            and set(outcome) == {"ordinal", "result", "abort", "events"}
            and type(outcome["ordinal"]) is int
            and outcome["ordinal"] == ordinal
            and outcome["abort"] is None
            and outcome["events"] == []
            and type(outcome["result"]) is dict
            and set(outcome["result"]) == {"artifact", "recipe", "stages", "status"}
            and all(
                attempt.get(key) == value for key, value in outcome["result"].items()
            ),
            "accepted rebuild does not bind a complete successful observed profile",
        )
        contexts.append(declared_context)
        receipts.append(declared_receipt)
        fresh_records.extend(
            [
                attempt["artifact"],
                dict(
                    path=str(directory / "recipe.json"),
                    bytes=len(declared_context.recipe_data),
                    sha256=hashlib.sha256(declared_context.recipe_data).hexdigest(),
                ),
                {key: receipt[key] for key in ("path", "bytes", "sha256")},
            ]
        )
    records = _record_inventory(
        [*records, rebuild_record, *metadata.values(), *immutable, *fresh_records],
        limit,
    )
    if verify_execution:
        _rebuild_checks(
            records,
            selected,
            syntax,
            rebuilt["profile_attempts"],
            contexts,
            receipts,
            rebuilt["output_limit"],
            events,
            reader=reader,
        )
    pending = primary(events)
    if pending is not None:
        raise pending
    return rebuilt, run, records, selected, syntax, contexts, receipts


def _link_call(root, row, linker, placement, output_limit):
    directory = root / "profiles" / f"{row['ordinal']:03d}"
    return dict(
        cwd=directory,
        argv=object_link_command(
            linker=linker,
            object_path=Path(row["object"]["path"]),
            output_path=directory / "linked.elf",
            text_address=placement["text_base"],
            bindings=placement["bindings"],
            section_addresses=placement["section_addresses"],
            entry_symbol=row["entry_symbol"],
        ),
        environment=dict(
            PATH="/usr/bin:/bin",
            LC_ALL="C",
            HOME=str(root / "home"),
            TMPDIR=str(root / "tmp"),
        ),
        output_limit=output_limit,
        name="link",
    )


def _comparison_checks(
    rebuilt,
    records,
    selected,
    syntax,
    contexts,
    receipts,
    binding,
    member,
    prepared_run,
    original_slice,
    original,
    calls,
    limit,
    events,
) -> None:
    _rebuild_checks(
        records,
        selected,
        syntax,
        rebuilt["profile_attempts"],
        contexts,
        receipts,
        rebuilt["output_limit"],
        events,
    )
    if binding is not None:
        try:
            current = provenance.original_slice(
                binding,
                prepared_run,
                target=member.target,
                image_id=member.image_id,
                pc=member.start,
                extent={"start": member.start, "end": member.end},
                subject=member.key,
                io_mode="exact",
                events=events,
            )
            _require_rebuild(
                current == original_slice, "complete original binding or bytes changed"
            )
        except Exception as error:
            retain(events, error, phase="comparison-original", path=prepared_run)
    for call, row in calls:
        try:
            directory = call["cwd"]
            _require_rebuild(
                directory.resolve() == directory
                and directory.is_dir()
                and not directory.is_symlink(),
                "fresh link directory changed",
            )
        except Exception as error:
            retain(events, error, phase="link-directory", path=call["cwd"])
        if row.get("link") is not None:
            try:
                verify_capture(
                    row["link"],
                    origin=row["link_origin"],
                    events=events,
                    reader=active_reader(),
                    **call,
                )
            except Exception as error:
                retain(events, error, phase="link-proof", path=call["cwd"])
        for record in row.get("raw_captures", {}).values():
            try:
                _authenticated_bytes(record, call["output_limit"], events)
            except Exception as error:
                retain(events, error, phase="link-raw-capture", path=record["path"])
        if row.get("linked") is not None:
            try:
                _authenticated_bytes(row["linked"], limit, events)
                if row.get("comparison") is not None:
                    measured = compare_linked_function(
                        Path(row["linked"]["path"]),
                        function_name=row["entry_symbol"],
                        original=original,
                    )
                    _require_rebuild(
                        row["comparison"] == _comparison_record(measured),
                        "retained complete linked comparison changed",
                    )
            except Exception as error:
                retain(events, error, phase="linked-output", path=call["cwd"])


def _comparison_record(comparison: FunctionComparison) -> dict:
    return dict(
        outcome=comparison.outcome,
        first_difference=comparison.first_difference,
        original_bytes=len(comparison.original.content),
        actual_bytes=len(comparison.actual.content),
        original_sha256=hashlib.sha256(comparison.original.content).hexdigest(),
        actual_sha256=hashlib.sha256(comparison.actual.content).hexdigest(),
        symbol=comparison.actual.name,
        address=comparison.actual.address,
    )


def _original_function(prepared_run, run, identity, events):
    _require_rebuild(
        isinstance(prepared_run, Path)
        and prepared_run.is_absolute()
        and prepared_run.resolve() == prepared_run
        and prepared_run.is_dir()
        and not prepared_run.is_symlink()
        and type(run.get("problem")) is dict
        and run["problem"].get("run_dir") == str(prepared_run),
        "submitted run does not bind the supplied canonical prepared original",
    )
    binding = provenance.open_problem_binding(
        prepared_run, io_mode="exact", events=events
    )
    _require_rebuild(
        run["problem"].get("binding_digest") == binding.digest,
        "submitted original binding digest differs from the active original",
    )
    member = binding.member(identity["key"])
    _require_rebuild(
        asdict(member) == identity,
        "complete bound member identity differs",
    )
    original_slice, original, original_record = _bound_original(
        binding, member, prepared_run, events
    )
    return binding, member, original_slice, original, original_record


def _bound_original(binding, member, prepared_run, events, limit=64 << 20):
    original_slice = provenance.original_slice(
        binding,
        prepared_run,
        target=member.target,
        image_id=member.image_id,
        pc=member.start,
        extent={"start": member.start, "end": member.end},
        subject=member.key,
        io_mode="exact",
        events=events,
        max_bytes=limit,
    )
    _require_rebuild(
        original_slice.length == member.end - member.start
        and original_slice.slice_sha256 == member.range_sha256,
        "independent original does not reproduce the complete bound member interval",
    )
    original = OriginalFunction(
        member.start, member.end, original_slice.instruction_bytes
    )
    original_record = original_slice.as_record()
    original_record["instruction_bytes_hex"] = original_record.pop(
        "instruction_bytes"
    ).hex()
    return original_slice, original, original_record


def compare_rebuilt_candidate(
    *,
    rebuild_record: dict,
    prepared_run: Path,
    workspace: Path,
    output_limit: int,
    artifact_limit: int,
    retained: dict,
) -> dict:
    """Link authenticated fresh objects and measure each complete independent original interval."""
    _require_rebuild(
        type(retained) is dict and retained is not rebuild_record,
        "comparison requires a distinct caller-owned retained dictionary",
    )
    retained.clear()
    retained.update(
        operation="comparison",
        status="unattempted",
        links=[],
        failure=None,
        secondary_failures=[],
        semantic_admission=False,
        autonomous_exact=False,
        associated_data="unverified",
        placement="unverified",
        bindings="unverified",
        original_byte_comparison="unperformed",
    )
    events: list = []
    calls: list[tuple[dict, dict]] = []
    rebuilt = selected = syntax = binding = member = original_slice = original = None
    records, contexts, receipts = [], [], []
    active_row = None
    workspace_owned = False
    try:
        retained["status"] = "preparing"
        _require_rebuild(
            all(
                type(limit) is int and limit > 0
                for limit in (output_limit, artifact_limit)
            ),
            "comparison requires positive integer limits",
        )
        rebuild_record = deepcopy(rebuild_record)
        retained.update(
            rebuild_record=rebuild_record,
            prepared_run=str(prepared_run),
            output_limit=output_limit,
            artifact_limit=artifact_limit,
        )
        _forward_rebuild(events)
        limit = min(artifact_limit, 64 << 20)
        rebuilt, run, records, selected, syntax, contexts, receipts = _load_rebuild(
            rebuild_record, limit, events
        )
        retained.update(
            identity=deepcopy(rebuilt["identity"]),
            entry_symbol=rebuilt["entry_symbol"],
            profiles_digest=selected["profiles_digest"],
        )
        for profile, attempt in zip(selected["profiles"], rebuilt["profile_attempts"]):
            retained["links"].append(
                dict(
                    profile_id=profile["profile_id"],
                    profile_identity=profile["identity"],
                    ordinal=profile["ordinal"],
                    object=deepcopy(attempt["artifact"]),
                    entry_symbol=rebuilt["entry_symbol"],
                    status="unattempted",
                    failure=None,
                    secondary_failures=[],
                    linked=None,
                    comparison=None,
                )
            )
        binding, member, original_slice, original, original_record = _original_function(
            prepared_run, run, rebuilt["identity"], events
        )
        retained["original"] = original_record
        unit = invocation.inspect_rebuild_native(
            rebuilt, run, binding, prepared_run, limit, events
        )
        _workspace_boundary(workspace, rebuilt["submitted_run"], records, selected)
        _require_rebuild(
            not any(
                workspace == protected
                or workspace.is_relative_to(protected)
                or protected.is_relative_to(workspace)
                for protected in (prepared_run, Path(rebuild_record["path"]).parent)
            ),
            "comparison workspace overlaps original or accepted rebuild evidence",
        )
        linker_record = selected["files"].get("linker")
        linker = _regular_record(linker_record, limit)
        records = _record_inventory([*records, linker_record], limit)
        _forward_rebuild(events)
        workspace.mkdir()
        workspace_owned = True
        for name in ("home", "tmp", "profiles"):
            (workspace / name).mkdir()
        retained["status"] = "linking"
        for row in retained["links"]:
            active_row = row
            _comparison_checks(
                rebuilt,
                records,
                selected,
                syntax,
                contexts,
                receipts,
                binding,
                member,
                prepared_run,
                original_slice,
                original,
                calls,
                limit,
                events,
            )
            _forward_rebuild(events)
            blob = _authenticated_bytes(row["object"], limit, events)
            placement, placeable = calltargets.measurement_placement(
                blob, row["entry_symbol"], original, limit, unit=unit
            )
            _require_rebuild(
                placeable,
                "actual object entry section cannot honor the independent original address",
            )
            row.update(placement)
            directory = workspace / "profiles" / f"{row['ordinal']:03d}"
            directory.mkdir()
            output = directory / "linked.elf"
            call = _link_call(workspace, row, linker, placement, output_limit)
            argv = call["argv"]
            calls.append((call, row))
            row.update(
                status="linking",
                argv=deepcopy(argv),
                link=None,
                link_origin="direct",
                launch="prelaunch",
            )
            _forward_rebuild(events)
            row["launch"] = "unknown"
            try:
                row["link"] = capture(**call)
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="link-capture",
                    path=directory,
                    origin="cleanup"
                    if isinstance(error, ProcessCleanupError)
                    else "unknown",
                )
                receipt_path = directory / "link.json"
                try:
                    if receipt_path.exists() or receipt_path.is_symlink():
                        receipt_record = _bounded_file_record(
                            receipt_path, limit, events
                        )
                        row["link"] = _submitted_document(receipt_record, limit, events)
                        row["link_origin"] = "persisted"
                        row["launch"] = verify_capture(
                            row["link"],
                            origin="persisted",
                            events=events,
                            reader=active_reader(),
                            **call,
                        )
                except Exception as recovery_error:
                    retain(
                        events, recovery_error, phase="link-recovery", path=receipt_path
                    )
                for channel in ("stdout", "stderr"):
                    raw_path = directory / f"link.{channel}.bin"
                    try:
                        if raw_path.exists() or raw_path.is_symlink():
                            row.setdefault("raw_captures", {})[channel] = (
                                _bounded_file_record(raw_path, output_limit, events)
                            )
                    except Exception as recovery_error:
                        retain(
                            events,
                            recovery_error,
                            phase="link-raw-recovery",
                            path=raw_path,
                        )
                raise
            finally:
                try:
                    if output.exists() or output.is_symlink():
                        row["linked"] = _bounded_file_record(output, limit, events)
                except Exception as artifact_error:
                    retain(events, artifact_error, phase="linked-artifact", path=output)
            row["launch"] = verify_capture(
                row["link"],
                origin="direct",
                events=events,
                reader=active_reader(),
                **call,
            )
            _require_rebuild(
                row["launch"] == "observed"
                and _complete_process(row["link"])
                and row["linked"] is not None,
                "linker refused, did not produce an ELF or did not confirm successful cleanup",
            )
            _authenticated_bytes(row["linked"], limit, events)
            measured = compare_linked_function(
                output, function_name=row["entry_symbol"], original=original
            )
            row.update(status="measured", comparison=_comparison_record(measured))
            _comparison_checks(
                rebuilt,
                records,
                selected,
                syntax,
                contexts,
                receipts,
                binding,
                member,
                prepared_run,
                original_slice,
                original,
                calls,
                limit,
                events,
            )
            _forward_rebuild(events)
        active_row = None
        _require_rebuild(
            all(row["status"] == "measured" for row in retained["links"]),
            "comparison did not measure the complete ordered compiler inventory",
        )
        _comparison_checks(
            rebuilt,
            records,
            selected,
            syntax,
            contexts,
            receipts,
            binding,
            member,
            prepared_run,
            original_slice,
            original,
            calls,
            limit,
            events,
        )
        _forward_rebuild(events)
        _require_rebuild(
            workspace.resolve() == workspace
            and workspace.is_dir()
            and not workspace.is_symlink(),
            "comparison workspace changed",
        )
        destination = workspace / "comparison-result.json"
        _require_rebuild(
            not destination.exists() and not destination.is_symlink(),
            "terminal comparison result already exists",
        )
        retained.update(
            status="measured-unverified",
            original_byte_comparison="measured",
            failure=None,
            secondary_failures=facts(events),
        )
        pending_file = workspace / "comparison-result.pending.json"
        data = encode_json(retained, path=destination, events=events)
        _require_rebuild(
            len(data) <= artifact_limit,
            "terminal comparison result exceeds its artifact bound",
        )
        write_bytes(pending_file, data, events=events, create_parents=False)
        _forward_rebuild(events)
        pending_file.rename(destination)
    except Exception as error:
        retain(
            events,
            error,
            phase="independent-comparison",
            path=workspace,
            origin="cleanup"
            if isinstance(error, ProcessCleanupError)
            else "policy"
            if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
            else "unknown",
        )
        if rebuilt is not None:
            _comparison_checks(
                rebuilt,
                records,
                selected,
                syntax,
                contexts,
                receipts,
                binding if original_slice is not None else None,
                member,
                prepared_run,
                original_slice,
                original,
                calls,
                min(artifact_limit, 64 << 20),
                events,
            )
        pending = primary(events) or error
        reason = f"{type(pending).__name__}: {pending}"
        if active_row is not None:
            active_row.update(
                status="refused", failure=reason, secondary_failures=facts(events)
            )
        for row in retained["links"]:
            if row["status"] == "unattempted":
                row.update(
                    status="interrupted",
                    failure=reason,
                    launch="prelaunch",
                )
        retained.update(
            status="refused", failure=reason, secondary_failures=facts(events)
        )
        if workspace_owned:
            _publish_comparison_refusal(workspace, retained, artifact_limit, events)
        raise pending
    return retained


def _publish_comparison_refusal(
    workspace: Path, retained: dict, limit: int, events
) -> None:
    try:
        _require_rebuild(
            workspace.resolve() == workspace
            and workspace.is_dir()
            and not workspace.is_symlink(),
            "comparison refusal workspace changed",
        )
        destination = workspace / "comparison-result.json"
        pending = workspace / "comparison-refusal.pending.json"
        _require_rebuild(
            not destination.exists() and not destination.is_symlink(),
            "terminal comparison result already exists",
        )
        data = encode_json(retained, path=destination, events=events)
        _require_rebuild(
            len(data) <= limit, "terminal comparison refusal exceeds its bound"
        )
        write_bytes(pending, data, create_parents=False, events=events)
        pending.rename(destination)
    except Exception as error:
        retain(events, error, phase="comparison-refusal-publication", path=workspace)
        retained["secondary_failures"] = facts(events)


def _associated_authority(prepared_run: Path, identity: dict, limit: int, events):
    preparation = bundles.verify_preparation_bindings(
        prepared_run, io_mode="exact", events=events, max_bytes=limit
    )
    answer_index = bundles.verify_bundle_references(
        prepared_run,
        "answer",
        io_mode="exact",
        events=events,
        max_bytes=limit,
        max_entries=65536,
    )
    _require_rebuild(
        answer_index["role"] == "answer" and answer_index["origin"] == "evaluator",
        "associated verification requires active evaluator Answer",
    )
    answer = bundles.resolve_document(
        prepared_run,
        contract.doc_ref("answer", answer_index["documents"][0]["digest"], None),
        owner="answer",
        io_mode="exact",
        events=events,
        max_bytes=limit,
    )
    binding = provenance.open_problem_binding(
        prepared_run, io_mode="exact", events=events, max_bytes=limit
    )
    member = binding.member(identity["key"])
    _require_rebuild(
        asdict(member) == identity,
        "associated member identity differs from active Problem",
    )
    _require_rebuild(
        len(answer["members"]) == contract.ADMITTED,
        "associated Answer does not retain the complete fixed denominator",
    )
    inventory = {
        key: dict(
            inventory=value["obligations"]["inventory"],
            unresolved=deepcopy(value["obligations"]["unresolved"]),
            known={
                name: deepcopy(value["obligations"][name])
                for name in (
                    "initialized_data",
                    "zero_initialized",
                    "relocations",
                    "external_bindings",
                )
            },
            function=deepcopy(value["obligations"]["function"]),
            selected=key == member.key,
        )
        for key, value in answer["members"].items()
    }
    value = answer["members"][member.key]["obligations"]
    _require_rebuild(
        value["function"]["extent"] == dict(start=member.start, end=member.end),
        "Answer obligation extent differs from complete active member",
    )
    contents = {
        row["id"]: bundles.resolve_file(
            prepared_run,
            row["contents"],
            owner="answer",
            io_mode="exact",
            events=events,
            max_bytes=limit,
        )
        for row in value["initialized_data"]
    }
    requirements = obligations.parse_member_requirements(member.key, value, contents)
    targets = {member.target, *(row["target"] for row in value["external_bindings"])}
    images = []
    for target in sorted(targets):
        bound_targets = [row for row in binding.targets if row.target == target]
        if not bound_targets:
            continue
        image = binding.image(bound_targets[0].image_id)
        data = provenance.read_original_image(
            binding,
            prepared_run,
            target=target,
            image_id=image.image_id,
            io_mode="exact",
            events=events,
            max_bytes=limit,
        )
        images.append(
            obligations.OriginalImageFacts(
                target, image.image_id, image.load_address, image.payload_offset, data
            )
        )
    original_slice, original, original_record = _bound_original(
        binding, member, prepared_run, events, limit
    )
    authority = dict(
        preparation_digest=contract.doc_digest(preparation),
        answer_index_digest=answer_index["bundle_digest"],
        answer_document_digest=contract.doc_digest(answer),
        problem_binding=asdict(binding),
        obligations=deepcopy(value),
    )
    return (
        binding,
        member,
        requirements,
        obligations.OriginalAssociatedFacts(member.target, tuple(images)),
        inventory,
        authority,
        original_record,
        original,
    )


def _nested_records(value):
    if type(value) is dict:
        if set(value) == {"path", "bytes", "sha256"}:
            yield value
        else:
            for child in value.values():
                yield from _nested_records(child)
    elif type(value) is list:
        for child in value:
            yield from _nested_records(child)


def _associated_profiles(
    comparison: dict,
    root: Path,
    rebuilt: dict,
    selected: dict,
    requirements,
    original_facts,
    original,
    limit: int,
    events,
    *,
    unit,
    reader=None,
    inspection=None,
):
    rows = comparison["links"]
    profiles = selected["profiles"]
    _require_rebuild(
        type(rows) is list and len(rows) == len(profiles),
        "terminal comparison ordered profile census is incomplete",
    )
    linker_record = selected["files"].get("linker")
    linker = _regular_record(linker_record, limit)
    _authenticated_bytes(linker_record, limit, events, reader=reader)
    results, records = [], [linker_record]
    base_fields = {
        "profile_id",
        "profile_identity",
        "ordinal",
        "object",
        "entry_symbol",
        "status",
        "failure",
        "secondary_failures",
        "linked",
        "comparison",
    }
    extra_fields = {
        "entry_section",
        "entry_offset",
        "text_base",
        "sections",
        "relocations",
        "text_relocations",
        "bindings",
        "section_addresses",
        "argv",
        "link",
        "link_origin",
        "launch",
        "raw_captures",
    }
    for ordinal, (row, profile, attempt) in enumerate(
        zip(rows, profiles, rebuilt["profile_attempts"])
    ):
        _require_rebuild(
            type(row) is dict
            and base_fields <= set(row) <= base_fields | extra_fields
            and row["profile_id"] == profile["profile_id"]
            and row["profile_identity"] == profile["identity"]
            and type(row["ordinal"]) is int
            and row["ordinal"] == ordinal
            and row["object"] == attempt["artifact"]
            and row["entry_symbol"] == rebuilt["entry_symbol"]
            and row["status"] in {"measured", "refused", "interrupted"}
            and type(row["secondary_failures"]) is list,
            "terminal comparison profile identity or shape differs",
        )
        blob = _authenticated_bytes(row["object"], limit, events, reader=reader)
        records.append(row["object"])
        derived, placeable = calltargets.measurement_placement(
            blob, row["entry_symbol"], original, limit, unit=unit
        )
        measured_object = objectfile.read_object_image(
            blob, row["entry_symbol"], limit=limit
        )
        _require_rebuild(
            all(row[name] == value for name, value in derived.items() if name in row),
            "retained comparison object facts changed",
        )
        _require_rebuild(
            row["status"] != "measured" or placeable,
            "claimed measured link cannot reproduce original placement",
        )
        failures = []
        if comparison["status"] == "refused":
            failures.append(comparison["failure"])
        if row["failure"] is not None:
            _require_rebuild(
                type(row["failure"]) is str and bool(row["failure"]),
                "invalid retained profile failure",
            )
            failures.append(row["failure"])
        if row["secondary_failures"]:
            failures.append("retained profile secondary failures remain present")
        if not placeable:
            failures.append(
                "actual object entry section cannot honor independent original placement"
            )
        directory = root / "profiles" / f"{ordinal:03d}"
        output = directory / "linked.elf"
        call = _link_call(
            root,
            row,
            linker,
            derived,
            comparison["output_limit"],
        )
        argv = call["argv"]
        if row.get("argv") is not None:
            _require_rebuild(
                row["argv"] == argv,
                "retained link argv differs from independent reproduction",
            )
        linked_image = None
        function_comparison = None
        if row.get("link") is not None:
            _require_rebuild(
                directory.resolve() == directory
                and directory.is_dir()
                and not directory.is_symlink(),
                "retained link directory changed",
            )
            launch = verify_capture(
                row["link"],
                origin=row.get("link_origin"),
                events=events,
                reader=reader,
                **call,
            )
            _require_rebuild(
                row.get("launch") == launch, "retained link launch differs"
            )
            records.extend(row["link"]["streams"].values())
            records.append(
                _bounded_file_record(
                    directory / "link.json", limit, events, reader=reader
                )
            )
            if row["status"] == "measured":
                _require_rebuild(
                    launch == "observed" and _complete_process(row["link"]),
                    "claimed measured comparison does not reproduce successful link ownership",
                )
            elif (
                row["link"].get("failure") is not None
                or row["link"].get("exit_code") != 0
            ):
                failures.append("retained link execution did not succeed")
        elif row["status"] == "measured":
            raise PolicyFailure("measured comparison lacks physical link capture")
        for record in row.get("raw_captures", {}).values():
            _authenticated_bytes(
                record, comparison["output_limit"], events, reader=reader
            )
            records.append(record)
        if row["linked"] is not None:
            _require_rebuild(
                _regular_record(row["linked"], limit) == output,
                "retained linked artifact path differs",
            )
            linked_bytes = _authenticated_bytes(
                row["linked"], limit, events, reader=reader
            )
            records.append(row["linked"])
            linked_image = read_linked_image(linked_bytes, limit=limit)
            if row["comparison"] is not None:
                function_comparison = _comparison_record(
                    FunctionComparison(
                        original=original,
                        actual=read_function_bytes(
                            linked_bytes,
                            function_name=row["entry_symbol"],
                            address=original.start,
                        ),
                    )
                )
                _require_rebuild(
                    row["comparison"] == function_comparison,
                    "retained complete function comparison changed",
                )
        if row["status"] == "measured":
            _require_rebuild(
                set(derived) <= set(row)
                and linked_image is not None
                and function_comparison is not None
                and row["failure"] is None
                and not row["secondary_failures"],
                "terminal measured profile is incomplete",
            )
        if requirements is None:
            results.append(deepcopy(row))
            continue
        measurement = obligations.measure_associated_requirements(
            requirements,
            object_image=measured_object,
            linked_image=linked_image,
            original_facts=original_facts,
            prior_failures=tuple(failures),
            profile_id=profile["profile_id"],
        )
        result = measurement.as_record()
        if inspection is not None:
            inspection.append(
                dict(
                    profile=deepcopy(profile),
                    row=deepcopy(row),
                    object_bytes=blob,
                    object_image=measured_object,
                    linked_bytes=None if linked_image is None else linked_bytes,
                    linked_image=linked_image,
                    measurement=measurement,
                    _restricted_link=(
                        row["status"] == "measured"
                        and not derived["bindings"]
                        and not derived["section_addresses"]
                    ),
                )
            )
        result.update(
            ordinal=ordinal,
            profile_identity=profile["identity"],
            object=deepcopy(row["object"]),
            linked=deepcopy(row["linked"]),
            function_comparison=function_comparison,
            placement="failed" if not placeable or failures else "unresolved",
        )
        results.append(result)
    if comparison["status"] == "measured-unverified":
        _require_rebuild(
            all(row["status"] == "measured" for row in rows),
            "successful terminal comparison omits an ordered profile",
        )
    return results, records


def inspect_original_feedback(
    *, comparison_record, prepared_run, artifact_limit, output_limit, reader=None
):
    """Authenticate complete native feedback using only submitted artifacts and original bytes."""
    events = []
    with using_reader(reader if reader is not None else active_reader()):
        observed, rebuilt, run, records, selected, syntax, contexts, receipts = (
            _load_comparison(
                comparison_record,
                prepared_run,
                output_limit,
                artifact_limit,
                events,
                reader=reader,
                original_only=True,
            )
        )
        _require_rebuild(
            observed["status"] == "measured-unverified",
            "feedback is not complete measured work",
        )
        prepared = PreparedInputs.attach(
            run,
            run_directory=Path(rebuilt["submitted_run"]["path"]).parent,
            events=events,
        )
        prepared.verify(events=events)
        binding, member, original_slice, original, original_record = _original_function(
            prepared_run, run, rebuilt["identity"], events
        )
        _require_rebuild(
            observed["original"] == original_record, "feedback original bytes differ"
        )
        unit = invocation.inspect_rebuild_native(
            rebuilt, run, binding, prepared_run, artifact_limit, events, reader=reader
        )
        _, linked_records = _associated_profiles(
            observed,
            Path(comparison_record["path"]).parent,
            rebuilt,
            selected,
            None,
            None,
            original,
            min(artifact_limit, 64 << 20),
            events,
            reader=reader,
            unit=unit,
        )
        _rebuild_checks(
            [comparison_record, *records, *linked_records],
            selected,
            syntax,
            rebuilt["profile_attempts"],
            contexts,
            receipts,
            rebuilt["output_limit"],
            events,
            reader=reader,
        )
        provenance.require_active_binding(
            binding, prepared_run, io_mode="exact", events=events
        )
        if primary(events) is not None:
            raise primary(events)
        return observed


def _associated_read_bounds(
    observations: list[dict],
    records: list[dict],
    input_read_limit: int,
    input_open_limit: int,
) -> None:
    _require_rebuild(
        len(observations) + len(records) <= input_open_limit,
        "associated evidence census exceeds its entry bound",
    )
    _require_rebuild(
        sum(row["bytes"] for row in observations) <= input_read_limit,
        "associated evidence exceeds its aggregate read bound",
    )


def _load_comparison(
    terminal,
    prepared_run,
    output_limit,
    artifact_limit,
    events,
    *,
    reader=None,
    original_only=False,
):
    limit = min(artifact_limit, 64 << 20)
    comparison = _submitted_document(terminal, limit, events, reader=reader)
    base = {
        "operation",
        "status",
        "links",
        "failure",
        "secondary_failures",
        "semantic_admission",
        "autonomous_exact",
        "associated_data",
        "placement",
        "bindings",
        "original_byte_comparison",
    }
    identity_fields = {
        "rebuild_record",
        "prepared_run",
        "output_limit",
        "artifact_limit",
        "identity",
        "entry_symbol",
        "profiles_digest",
        "original",
    }
    _require_rebuild(
        base | identity_fields == set(comparison)
        and comparison["operation"] == "comparison"
        and comparison["status"] in {"measured-unverified", "refused"}
        and comparison["semantic_admission"] is False
        and comparison["autonomous_exact"] is False
        and all(
            comparison[name] == "unverified"
            for name in ("associated_data", "placement", "bindings")
        )
        and type(comparison["secondary_failures"]) is list,
        "physical terminal comparison shape or admission differs",
    )
    _require_rebuild(
        type(comparison["output_limit"]) is int
        and 0 < comparison["output_limit"] <= output_limit
        and (type(comparison["artifact_limit"]) is int)
        and (0 < comparison["artifact_limit"] <= artifact_limit),
        "original comparison limits are absent or exceed replay bounds",
    )
    _require_rebuild(
        prepared_run.is_absolute()
        and prepared_run.resolve() == prepared_run
        and prepared_run.is_dir()
        and not prepared_run.is_symlink()
        and comparison["prepared_run"] == str(prepared_run),
        "comparison prepared-run binding differs",
    )
    if comparison["status"] == "measured-unverified":
        _require_rebuild(
            comparison["failure"] is None
            and not comparison["secondary_failures"]
            and comparison["original_byte_comparison"] == "measured",
            "successful terminal comparison retains failures",
        )
    else:
        _require_rebuild(
            type(comparison["failure"]) is str and bool(comparison["failure"]),
            "terminal refused comparison lacks its original failure",
        )
    rebuilt, run, records, selected, syntax, contexts, receipts = _load_rebuild(
        comparison["rebuild_record"],
        limit,
        events,
        reader=reader,
        original_only=original_only,
    )
    _require_rebuild(
        comparison["identity"] == rebuilt["identity"]
        and comparison["entry_symbol"] == rebuilt["entry_symbol"]
        and comparison["profiles_digest"] == selected["profiles_digest"],
        "comparison identity differs from its authenticated rebuild",
    )
    return comparison, rebuilt, run, records, selected, syntax, contexts, receipts


def verify_associated_candidate(
    *,
    comparison_record: Path,
    prepared_run: Path,
    workspace: Path,
    output_limit: int,
    artifact_limit: int,
    input_read_limit: int,
    retained: dict,
    allowance: ConsumptionAllowance | None = None,
    publish: bool = True,
    inspection: dict | None = None,
) -> dict:
    _require_rebuild(
        type(retained) is dict,
        "associated verification requires a caller-owned retained dictionary",
    )
    retained.clear()
    retained.update(
        operation="associated-verification",
        status="unattempted",
        profiles=[],
        failure=None,
        secondary_failures=[],
        semantic_admission=False,
        autonomous_exact=False,
        denominator=contract.ADMITTED,
        associated_data="unverified",
        placement="unverified",
        bindings="unverified",
    )
    events, observations, records = [], [], []
    input_read_attempts = failed_read_reserved_bytes = charged_read_bytes = 0
    attempted_output = None
    try:
        _require_rebuild(
            isinstance(comparison_record, Path)
            and isinstance(prepared_run, Path)
            and isinstance(workspace, Path),
            "associated inputs require physical Path values",
        )
        _require_rebuild(
            all(
                type(value) is int and value > 0
                for value in (output_limit, artifact_limit, input_read_limit)
            ),
            "associated verification requires positive integer bounds",
        )
        limit = min(artifact_limit, 64 << 20)
        shared_allowance = allowance or ConsumptionAllowance(
            read_limit=input_read_limit,
            open_limit=65536,
            artifact_limit=limit,
        )
        started_bytes, started_opens, started_failed = (
            shared_allowance.charged_bytes,
            shared_allowance.attempted_opens,
            shared_allowance.failed_reserved_bytes,
        )
        local_open_limit = shared_allowance.open_limit - started_opens
        local_read_limit = min(
            input_read_limit, shared_allowance.read_limit - started_bytes
        )

        def reader(
            path: Path, bound: int, read_events: list[FailureEvent] | None
        ) -> bytes:
            nonlocal input_read_attempts, failed_read_reserved_bytes, charged_read_bytes
            _require_rebuild(
                isinstance(path, Path) and type(bound) is int and bound >= 0,
                "associated snapshot requires a physical path and nonnegative bound",
            )
            _require_rebuild(
                input_read_attempts < local_open_limit,
                "associated physical read attempts exceed their entry bound",
            )
            try:
                data = shared_allowance.read_bytes(path, min(bound, limit), read_events)
            finally:
                input_read_attempts = shared_allowance.attempted_opens - started_opens
                failed_read_reserved_bytes = (
                    shared_allowance.failed_reserved_bytes - started_failed
                )
                charged_read_bytes = shared_allowance.charged_bytes - started_bytes
            observations.append(
                dict(
                    path=str(path.absolute()),
                    bytes=len(data),
                    sha256=hashlib.sha256(data).hexdigest(),
                )
            )
            _require_rebuild(
                len(data) <= min(bound, limit),
                "associated snapshot exceeds its bounded allowance",
            )
            return data

        retained.update(
            output_limit=output_limit,
            artifact_limit=artifact_limit,
            input_read_limit=input_read_limit,
            prepared_run=str(prepared_run),
        )
        with (
            using_reader(reader),
            contract.bounded_reads(limit, observations=observations, reader=reader),
        ):
            _require_rebuild(
                comparison_record.name == "comparison-result.json",
                "associated verification requires a terminal comparison-result.json",
            )
            terminal = _bounded_file_record(
                comparison_record, limit, events, reader=reader
            )
            retained["comparison_record"] = deepcopy(terminal)
            comparison, rebuilt, run, records, selected, syntax, contexts, receipts = (
                _load_comparison(
                    terminal,
                    prepared_run,
                    output_limit,
                    artifact_limit,
                    events,
                    reader=reader,
                )
            )
            if publish:
                _workspace_boundary(
                    workspace, rebuilt["submitted_run"], records, selected
                )
            protected = [
                prepared_run,
                comparison_record.parent,
                Path(comparison["rebuild_record"]["path"]).parent,
            ]
            _require_rebuild(
                not publish
                or not any(
                    workspace == path
                    or workspace.is_relative_to(path)
                    or path.is_relative_to(workspace)
                    for path in protected
                ),
                "associated report workspace overlaps immutable inputs",
            )
            _associated_read_bounds(
                observations, records, local_read_limit, local_open_limit
            )
            (
                binding,
                member,
                requirements,
                original_facts,
                inventory,
                authority,
                original_record,
                original,
            ) = _associated_authority(prepared_run, rebuilt["identity"], limit, events)
            _require_rebuild(
                run["problem"]["run_dir"] == str(prepared_run)
                and run["problem"]["binding_digest"] == binding.digest
                and comparison["original"] == original_record,
                "comparison does not reproduce its complete original authority",
            )
            retained.update(
                identity=deepcopy(rebuilt["identity"]),
                authority=authority,
                inventories=inventory,
                measured_member=member.key,
                unmeasured_members=[key for key in inventory if key != member.key],
                inventory_summary=dict(
                    members=len(inventory),
                    unresolved_members=sum(
                        row["inventory"] == "unresolved" for row in inventory.values()
                    ),
                    unresolved_categories={
                        name: sum(
                            item["category"] == name
                            for row in inventory.values()
                            for item in row["unresolved"]
                        )
                        for name in (
                            "initialized_data",
                            "zero_initialized",
                            "relocations",
                            "external_bindings",
                        )
                    },
                ),
                inventory_closure="unknown",
            )
            unit = invocation.inspect_rebuild_native(
                rebuilt, run, binding, prepared_run, limit, events, reader=reader
            )
            results, linked_records = _associated_profiles(
                comparison,
                comparison_record.parent,
                rebuilt,
                selected,
                requirements,
                original_facts,
                original,
                limit,
                events,
                reader=reader,
                inspection=None
                if inspection is None
                else inspection.setdefault("profiles", []),
                unit=unit,
            )
            if inspection is not None:
                inspection.update(
                    binding=binding,
                    member=member,
                    requirements=requirements,
                    original_facts=original_facts,
                    original=original,
                    comparison=deepcopy(comparison),
                    rebuilt=deepcopy(rebuilt),
                    run=deepcopy(run),
                    selected=deepcopy(selected),
                    contexts=tuple(contexts),
                )
            retained["profiles"] = results
            records = _record_inventory([terminal, *records, *linked_records], limit)
            identities = {row["path"]: row for row in records}
            observed_identities = {}
            for row in observations:
                _require_rebuild(
                    observed_identities.setdefault(row["path"], row) == row,
                    "associated input changed between bounded snapshots",
                )
            for row in [*_nested_records(rebuilt), *observations]:
                _require_rebuild(
                    type(row) is dict
                    and set(row) == {"path", "bytes", "sha256"}
                    and observed_identities.get(row["path"]) == row
                    and identities.setdefault(row["path"], row) == row,
                    "associated physical census differs from authenticated owner observations",
                )
            records = deepcopy(list(identities.values()))
            records.append(
                _bounded_file_record(
                    syntax.call["cwd"] / f"{syntax.call['name']}.json",
                    limit,
                    events,
                    reader=reader,
                )
            )
            for attempt in rebuilt["profile_attempts"]:
                for stage in attempt["stages"]:
                    directory = Path(stage["process"]["cwd"])
                    records.append(
                        _bounded_file_record(
                            directory / f"{stage['name']}.json",
                            limit,
                            events,
                            reader=reader,
                        )
                    )
                    records.append(
                        _bounded_file_record(
                            directory / f"{stage['name']}-inputs.json",
                            limit,
                            events,
                            reader=reader,
                        )
                    )
            unique = {}
            for row in records:
                _require_rebuild(
                    unique.setdefault(row["path"], row) == row,
                    "associated physical census has conflicting identities",
                )
            records = deepcopy(list(unique.values()))
            _associated_read_bounds(
                observations, records, local_read_limit, local_open_limit
            )
            recipe.verify_inputs(records, events=events, reader=reader)
            fresh_rebuild = _load_rebuild(
                comparison["rebuild_record"], limit, events, reader=reader
            )
            _require_rebuild(
                fresh_rebuild[0] == rebuilt and fresh_rebuild[3] == selected,
                "replayed rebuild changed before publication",
            )
            fresh_authority = _associated_authority(
                prepared_run, rebuilt["identity"], limit, events
            )
            _require_rebuild(
                fresh_authority
                == (
                    binding,
                    member,
                    requirements,
                    original_facts,
                    inventory,
                    authority,
                    original_record,
                    original,
                ),
                "independent authority changed before publication",
            )
            unit = invocation.inspect_rebuild_native(
                rebuilt, run, binding, prepared_run, limit, events, reader=reader
            )
            reproduced, _ = _associated_profiles(
                comparison,
                comparison_record.parent,
                rebuilt,
                selected,
                requirements,
                original_facts,
                original,
                limit,
                events,
                reader=reader,
                unit=unit,
            )
            _require_rebuild(
                reproduced == results,
                "associated measurements changed before publication",
            )
            _authenticated_bytes(terminal, limit, events, reader=reader)
            recipe.verify_inputs(records, events=events, reader=reader)
            _associated_read_bounds(
                observations, records, local_read_limit, local_open_limit
            )
            _forward_rebuild(events)
            expected_read_bytes = (
                sum(row["bytes"] for row in observations)
                + sum(row["bytes"] for row in records)
                + 3 * terminal["bytes"]
            )
            expected_read_attempts = input_read_attempts + len(records) + 3
            final_request_upper = sum(row["bytes"] + 1 for row in records) + 3 * (
                terminal["bytes"] + 1
            )
            _require_rebuild(
                not publish
                or failed_read_reserved_bytes == 0
                and charged_read_bytes + final_request_upper <= local_read_limit
                and expected_read_attempts + len(records) <= local_open_limit,
                "associated final reauthentication exceeds its remaining allowance",
            )
            retained.update(
                status="measured-failed"
                if any(row["status"] == "measured-failed" for row in results)
                else "measured-unresolved",
                input_census=records,
                observed_read_bytes=expected_read_bytes,
                input_read_attempts=expected_read_attempts,
                failed_read_reserved_bytes=0,
                read_accounting="bounded-input-snapshots",
                failure=None,
                secondary_failures=facts(events),
            )
            if not publish:
                retained.update(
                    observed_read_bytes=sum(row["bytes"] for row in observations),
                    input_read_attempts=input_read_attempts,
                )
            else:
                workspace.mkdir()
            destination = workspace / "associated-result.json"
            data = encode_json(retained, path=destination, events=events)
            published = contract.parse_json(data, "associated terminal result")
            retained.clear()
            retained.update(published)
            _forward_rebuild(events)
            if not publish:
                return retained
            pending = workspace / "associated-result.pending.json"
            _require_rebuild(
                len(data) <= artifact_limit,
                "associated terminal result exceeds its bound",
            )
            attempted_output = dict(
                path=str(pending),
                bytes=len(data),
                sha256=hashlib.sha256(data).hexdigest(),
                write_completed=False,
            )
            write_bytes(pending, data, create_parents=False, events=events)
            attempted_output["write_completed"] = True
            recipe.verify_inputs(records, events=events, reader=reader)
            _authenticated_bytes(terminal, limit, events, reader=reader)
            _require_rebuild(
                sum(row["bytes"] for row in observations) == expected_read_bytes
                and input_read_attempts == expected_read_attempts
                and failed_read_reserved_bytes == 0,
                "associated final physical read counters differ from the pending result",
            )
            _associated_read_bounds(
                observations, records, local_read_limit, local_open_limit
            )
            _forward_rebuild(events)
            pending.rename(destination)
    except Exception as error:
        retain(
            events,
            error,
            phase="associated-verification",
            path=workspace,
            origin="policy"
            if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
            else "unknown",
        )
        pending_error = primary(events) or error
        retained.update(
            status="refused",
            failure=f"{type(pending_error).__name__}: {pending_error}",
            secondary_failures=facts(events),
            observed_read_bytes=sum(row["bytes"] for row in observations),
            input_read_attempts=input_read_attempts,
            failed_read_reserved_bytes=failed_read_reserved_bytes,
            read_accounting="bounded-input-snapshots; failed consumption is unknown",
        )
        if attempted_output is not None:
            retained["attempted_output"] = attempted_output
        raise pending_error
    return retained
