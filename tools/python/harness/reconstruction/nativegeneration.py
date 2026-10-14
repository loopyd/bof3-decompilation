"""run requires one through 118 distinct member keys and a fresh destination."""

from __future__ import annotations

from copy import deepcopy
from dataclasses import asdict, astuple, dataclass
import hashlib
from itertools import zip_longest
from pathlib import Path
import re

from harness.build import recipe
from harness.common.artifacts import encode_json, read_bytes, write_bytes
from harness.common.digests import file_record
from harness.common.process import ProcessCleanupError
from harness.common.failures import PolicyFailure, facts, primary, retain
from harness.reconstruction.prepare import PreparedInputs
from harness.reconstruction import (
    catalog,
    calltargets,
    comparison,
    invocation,
    nativeoutput,
    objectfile,
    prelude,
    provenance,
)
from harness.match.extraction import read_function_image

MEMBER_COUNT = 118
MAX_IMAGE_BYTES = 32 << 20
MAX_PAYLOAD_BYTES = 16 << 20
ARTIFACT_LIMIT = 64 << 20


@dataclass(frozen=True, order=True)
class ByteScore:
    nonexact: int
    length_delta: int
    different_positions: int
    negative_matching_prefix: int
    form_ordinal: int
    profile_ordinal: int


def _score_links(measured, binding, run_dir, member, form_ordinal, expected_profiles):
    _require(
        measured["status"] == "measured-unverified"
        and measured["failure"] is None
        and not measured["secondary_failures"]
        and bool(expected_profiles)
        and [
            (link["ordinal"], link["profile_id"], link["profile_identity"])
            for link in measured["links"]
        ]
        == [
            (profile["ordinal"], profile["profile_id"], profile["identity"])
            for profile in expected_profiles
        ],
        "source search requires complete authenticated profile comparisons",
    )
    sliced = provenance.original_slice(
        binding,
        run_dir,
        target=member.target,
        image_id=member.image_id,
        pc=member.start,
        extent=(member.start, member.end),
        subject=member.key,
        io_mode="exact",
    )
    _require(sliced.slice_sha256 == member.range_sha256, "feedback original differs")
    original = comparison.OriginalFunction(
        member.start, member.end, sliced.instruction_bytes
    )
    scores = []
    for link in measured["links"]:
        _require(
            link["status"] == "measured" and link["failure"] is None,
            "source search cannot score a refused link",
        )
        recipe.verify_inputs([link["linked"]])
        actual = read_function_image(
            Path(link["linked"]["path"]),
            function_name=measured["entry_symbol"],
            address=member.start,
        )
        observed = comparison.FunctionComparison(original, actual)
        summary = link["comparison"]
        _require(
            len(actual.content) == summary["actual_bytes"]
            and len(original.content) == summary["original_bytes"]
            and hashlib.sha256(actual.content).hexdigest() == summary["actual_sha256"]
            and sliced.slice_sha256 == summary["original_sha256"]
            and actual.address == summary["address"] == member.start
            and actual.name == summary["symbol"] == measured["entry_symbol"]
            and observed.outcome == summary["outcome"]
            and observed.first_difference == summary["first_difference"],
            "complete linked bytes do not reproduce the public comparison",
        )
        different = sum(
            a != b for a, b in zip_longest(original.content, actual.content)
        )
        prefix = observed.first_difference
        score = ByteScore(
            int(different != 0),
            abs(len(actual.content) - len(original.content)),
            different,
            -(len(original.content) if prefix is None else prefix),
            form_ordinal,
            link["ordinal"],
        )
        recipe.verify_inputs([link["linked"]])
        scores.append(
            {
                "ordinal": link["ordinal"],
                "profile_id": link["profile_id"],
                "profile_identity": link["profile_identity"],
                "status": "measured",
                "linked": link["linked"],
                "comparison": summary,
                "different_positions": different,
                "score": list(astuple(score)),
            }
        )
    provenance.require_active_binding(binding, run_dir, io_mode="exact")
    return scores


SEARCH_VERSION = "member-first-v1"


@dataclass(frozen=True)
class _ResumePin:
    path: Path
    sha256: str


@dataclass(frozen=True)
class _CampaignGrant:
    grant_id: str
    member_limit: int
    campaign_limit: int

    @classmethod
    def parse(cls, value, selected):
        _require(
            type(value) is dict
            and set(value)
            == {
                "grant_id",
                "member_limit",
                "campaign_limit",
            },
            "campaign grant fields differ",
        )
        _require(
            type(value["grant_id"]) is str
            and re.fullmatch(r"[A-Za-z0-9._-]{1,128}", value["grant_id"]),
            "grant id must be a bounded ASCII label",
        )
        for name, maximum in (
            ("member_limit", len(selected)),
            ("campaign_limit", 4250),
        ):
            _require(
                type(value[name]) is int and 1 <= value[name] <= maximum,
                f"{name} differs from its finite bounds",
            )
        return cls(**value)


@dataclass(frozen=True)
class _MemberBudget:
    key: str
    grant_id: str
    admission: dict

    @classmethod
    def parse(cls, value):
        _require(
            type(value) is dict and set(value) == {"key", "grant_id", "admission"},
            "member budget differs",
        )
        return cls(
            value["key"],
            value["grant_id"],
            value["admission"],
        )


@dataclass(frozen=True)
class _Cursor:
    member_ordinal: int | None
    form_ordinal: int | None
    phase: str
    profile_ordinal: int | None = None
    attempt_ordinal: int | None = None

    @classmethod
    def parse(cls, value, selected):
        _require(
            type(value) is dict
            and set(value)
            == {
                "member_ordinal",
                "form_ordinal",
                "phase",
                "profile_ordinal",
                "attempt_ordinal",
            },
            "campaign cursor differs",
        )
        _require(
            value["phase"]
            in {
                "prepare",
                "self-check",
                "native",
                "syntax",
                "profile",
                "rebuild",
                "compare",
                "done",
            },
            "campaign cursor phase differs",
        )
        for name, bound in (
            ("member_ordinal", len(selected)),
            ("form_ordinal", 2),
            ("profile_ordinal", 5),
        ):
            _require(
                value[name] is None
                or type(value[name]) is int
                and 0 <= value[name] < bound,
                "campaign cursor ordinal differs",
            )
        _require(
            (value["phase"] == "profile") == (value["profile_ordinal"] is not None),
            "profile cursor lacks its exact ordinal",
        )
        _require(
            value["attempt_ordinal"] is None
            or type(value["attempt_ordinal"]) is int
            and value["attempt_ordinal"] >= 0,
            "attempt cursor ordinal differs",
        )
        return cls(**value)


@dataclass(frozen=True)
class _TerminalCandidate:
    attempt: dict
    disposition: str
    profiles: tuple[dict, ...]


@dataclass(frozen=True)
class _UnresolvedAttempt:
    attempt: dict
    reason: str
    generation_terminal: dict | None


class _BudgetExhausted(PolicyFailure):
    pass


def _document(pin):
    return comparison._submitted_document(pin, ARTIFACT_LIMIT, [], reader=read_bytes)


def _adapt_retained_native(result, run, result_record, *, reader=read_bytes):
    raw = comparison._authenticated_bytes(
        result_record, ARTIFACT_LIMIT, [], reader=reader
    )
    _require(
        encode_json(result, path=result_record["path"]) == raw,
        "retained member result differs from its physical pin",
    )
    candidate = Path(result_record["path"]).parent
    native = result["native"]
    _require(
        type(native) is dict and native.get("workspace") == str(candidate),
        "native result belongs to another member",
    )
    native_raw = comparison._authenticated_bytes(
        file_record(candidate / "native-result.json"), 1 << 20, [], reader=reader
    )
    _require(
        encode_json(native, path=candidate / "native-result.json") == native_raw,
        "physical native result differs from its pinned submission",
    )
    process = native["process"]
    _require(
        type(process) is dict
        and type(process.get("argv")) is list
        and all(type(item) is str for item in process["argv"]),
        "retained native argv must be a JSON list of strings",
    )
    receipt = comparison._authenticated_bytes(
        file_record(candidate / "native.json"),
        6 * run["limits"]["process_output_bytes"] + (64 << 10),
        [],
        reader=reader,
    )
    _require(
        encode_json(process, path=candidate / "native.json") == receipt,
        "physical native capture receipt differs",
    )
    origin = native.get("process_origin", "direct")
    _require(
        type(origin) is str and origin in {"direct", "persisted"},
        "unsupported retained native origin",
    )
    command = _native_command(
        candidate,
        Path(run["binaries"]["rizin"]["path"]),
        Path(run["binaries"]["plugin"]["path"]),
        run["limits"]["process_output_bytes"],
    )
    _require(
        process["argv"] == list(command.argv),
        "retained native argv differs from the native owner",
    )
    parsed = deepcopy(result)
    if origin == "direct":
        parsed["native"]["process"]["argv"] = tuple(process["argv"])
    _require(
        encode_json(parsed, path=result_record["path"]) == raw
        and encode_json(parsed["native"], path=candidate / "native-result.json")
        == native_raw
        and encode_json(parsed["native"]["process"], path=candidate / "native.json")
        == receipt,
        "native consumption adaptation changes canonical physical evidence",
    )
    recipe.verify_inputs([result_record], reader=reader)
    return parsed


def _request_identity(binding, arguments):
    return dict(
        problem=dict(
            run_dir=str(arguments["run_dir"]),
            binding_digest=binding.digest,
            problem_sha256=binding.problem_doc_sha256,
        ),
        native_receipt=file_record(arguments["native_receipt"]),
        configuration=file_record(arguments["configuration"]),
        rizin=file_record(arguments["rizin"]),
        driver=PreparedInputs.tree(
            Path(__file__).resolve().parents[1], python_only=True
        ),
        output_limit=arguments["output_limit"],
    )


def _attempts(report):
    return [attempt for row in report["members"] for attempt in row["attempts"]]


def _derive_search(report, grant, authenticated):
    by_id = {item.attempt["id"]: item for item in authenticated}
    complete = set()
    for row in report["members"]:
        row.update(
            best=None,
            best_local_exact=False,
            reader="unattempted",
            generation="unattempted",
            compilation="unattempted",
        )
        for attempt in row["attempts"]:
            item = by_id.get(attempt["id"])
            if not isinstance(item, _TerminalCandidate):
                continue
            complete.add((row["identity"]["key"], attempt["form_ordinal"]))
            if item.disposition != "measured":
                row.update(
                    reader="verified-original",
                    generation="native-refused"
                    if item.disposition == "native-refused"
                    else "captured-unqualified",
                    compilation="unattempted"
                    if item.disposition == "native-refused"
                    else item.disposition,
                )
            for profile in item.profiles:
                if row["best"] is None or profile["score"] < row["best"]["score"]:
                    row["best"] = dict(
                        attempt_id=attempt["id"],
                        form_ordinal=attempt["form_ordinal"],
                        profile_ordinal=profile["ordinal"],
                        score=profile["score"],
                    )
        if row["best"] is not None:
            row.update(
                best_local_exact=row["best"]["score"][0] == 0,
                reader="verified-original",
                generation="captured-unqualified",
                compilation="compiled-unverified",
            )
        row["candidates"] = (
            [
                dict(
                    ordinal=index,
                    source_form=form.value,
                    status="completed"
                    if (row["identity"]["key"], index) in complete
                    else "unresolved",
                )
                for index, form in enumerate(nativeoutput.SourceForm)
            ]
            if row["identity"]["key"] in report["selected"]
            else []
        )
    reservations = [
        item
        for item in report["operation_reservations"]
        if item["grant_id"] == grant.grant_id
    ]
    admissions = [
        item for item in report["member_budgets"] if item["grant_id"] == grant.grant_id
    ]
    remaining_units = grant.campaign_limit - sum(item["units"] for item in reservations)
    remaining_members = grant.member_limit - len(admissions)
    _require(
        remaining_units >= 0 and remaining_members >= 0,
        "campaign account exceeds its grant",
    )
    report["remaining"] = dict(
        units=remaining_units,
        member_admissions=remaining_members,
    )
    report["grant_balances"] = [
        dict(
            grant_id=value["grant_id"],
            units=value["campaign_limit"]
            - sum(
                item["units"]
                for item in report["operation_reservations"]
                if item["grant_id"] == value["grant_id"]
            ),
        )
        for value in report["grants"]
    ]
    for attempt in _attempts(report):
        for observed in attempt.get("cleanup_uncertainties", []):
            uncertainty = dict(attempt_id=attempt["id"], owner_observation=observed)
            if uncertainty not in report["cleanup_uncertainties"]:
                report["cleanup_uncertainties"].append(uncertainty)
    report["cleanup"] = (
        "uncertain" if report.get("cleanup_uncertainties") else "unverified"
    )
    report["completed"] = [
        dict(member_key=key, form_ordinal=index)
        for key in report["selected"]
        for index in range(2)
        if (key, index) in complete
    ]
    report["summary"] = dict(
        members=MEMBER_COUNT,
        selected=len(report["selected"]),
        local_best=sum(row["best"] is not None for row in report["members"]),
        local_byte_exact=sum(row["best_local_exact"] for row in report["members"]),
        autonomous_exact=0,
    )
    report["original_byte_comparison"] = (
        "measured-unverified" if report["summary"]["local_best"] else "unperformed"
    )
    return complete


def _publish_progress(report, destination, ordinal):
    path = destination / f"progress-{ordinal:03d}.json"
    write_bytes(path, encode_json(report, path=path), readonly=True)
    return file_record(path)


def _admit_work(
    report,
    grant,
    cursor,
    *,
    units,
    member,
    checkpoint,
    terminal_slot=None,
    attempt_id=None,
):
    report["cursor"] = asdict(cursor)
    spent = sum(
        item["units"]
        for item in report["operation_reservations"]
        if item["grant_id"] == grant.grant_id
    )
    if units > grant.campaign_limit - spent:
        raise _BudgetExhausted("campaign operation units exhausted before owner call")
    reservation = dict(
        id=len(report["operation_reservations"]),
        grant_id=grant.grant_id,
        units=units,
        cursor=asdict(cursor),
        member_key=None if member is None else member.key,
        attempt_id=attempt_id,
        terminal_slot=None if terminal_slot is None else str(terminal_slot),
        owner_run=report["run"],
    )
    report["operation_reservations"].append(reservation)
    checkpoint()
    return reservation


def _member_budget(report, grant, key):
    for record in report["member_budgets"]:
        if record["key"] == key and record["grant_id"] == grant.grant_id:
            budget = _MemberBudget.parse(record)
            return budget
    charged = sum(
        item["grant_id"] == grant.grant_id for item in report["member_budgets"]
    )
    if charged >= grant.member_limit:
        raise _BudgetExhausted("campaign member admissions exhausted")
    admission = dict(
        key=key,
        grant_id=grant.grant_id,
        owner_run=report["run"],
    )
    directory = Path(report["destination"]) / "member-admissions"
    directory.mkdir(mode=0o700, exist_ok=True)
    path = directory / f"{len(report['member_budgets']):06d}.json"
    write_bytes(path, encode_json(admission, path=path), readonly=True)
    budget = _MemberBudget(key, grant.grant_id, file_record(path))
    report["member_budgets"].append(asdict(budget))
    return budget


def _write_terminal(path, value):
    value = deepcopy(value)
    write_bytes(path, encode_json(value, path=path), readonly=True)
    return file_record(path)


def _profile_rows(selection, selected=True):
    return [
        dict(
            profile_id=profile["profile_id"],
            profile_identity=profile["identity"],
            ordinal=profile["ordinal"],
            disposition="unattempted",
            terminal=False,
            status="native-unattempted" if selected else "not-selected",
            failure=None if selected else "member not selected for this bounded run",
            raw_exit=None,
            exit_code=None,
            stages=[],
        )
        for profile in selection["profiles"]
    ]


def _generation_disposition(attempt, binding, arguments):
    run = _document(attempt["run"])
    submitted = _document(attempt["submission"])
    child = _document(attempt["report"])
    workspace = Path(attempt["workspace"])
    member = binding.member(attempt["member_key"])
    _require(
        Path(attempt["run"]["path"]) == workspace / "run.json"
        and Path(attempt["submission"]["path"]) == workspace / "member-000/result.json"
        and (Path(attempt["report"]["path"]) == workspace / "report.json")
        and (run["selected"] == [member.key])
        and (run["source_form"] == attempt["source_form"])
        and (run["campaign"]["id"] == attempt["origin_campaign"])
        and (run["campaign"]["grant_id"] == attempt["generation_grant_id"])
        and (submitted["identity"] == asdict(member))
        and (child["selected"] == [member.key])
        and (
            [row["identity"] for row in child["members"]]
            == [asdict(item) for item in binding.members]
        )
        and (
            child["members"][[item.key for item in binding.members].index(member.key)]
            == submitted
        )
        and (child.get("snapshots_unchanged") is True)
        and (not submitted.get("secondary_failures"))
        and (not child.get("secondary_failures"))
        and (not submitted.get("owner_observation"))
        and (not child.get("owner_observation")),
        "generation terminal does not bind its original run, result, complete census and guards",
    )
    prepared = PreparedInputs.attach(run, run_directory=workspace)
    prepared.verify()
    _require(
        run["problem"] == arguments["requested"]["problem"]
        and run["binaries"]["receipt"] == arguments["requested"]["native_receipt"]
        and run["binaries"]["configuration"] == arguments["requested"]["configuration"]
        and run["binaries"]["rizin"] == arguments["requested"]["rizin"],
        "generation caller input identities differ",
    )
    submitted = _adapt_retained_native(submitted, run, attempt["submission"])
    profiles = run["tools"]["profiles"]
    attempts = submitted["profile_attempts"]
    _require(
        bool(profiles)
        and len(profiles) == len(attempts)
        and [
            (row["ordinal"], row["profile_id"], row["profile_identity"])
            for row in attempts
        ]
        == [(row["ordinal"], row["profile_id"], row["identity"]) for row in profiles],
        "generation profile catalog differs",
    )
    candidate = workspace / "member-000"
    native = submitted["native"]
    form = nativeoutput.parse_source_form(attempt["source_form"])
    if submitted["generation"] == "native-refused":
        events = []
        image, original, payload = _original(
            binding, arguments["run_dir"], member, events
        )
        context = run["binding_inputs"]
        catalog_row = next(
            row for row in context["files"] if row["path"] == "image-catalog.json"
        )
        contexts = calltargets.NativeContexts(
            binding.problem_doc_sha256,
            catalog_row["sha256"],
            tuple(sorted(binding.images, key=lambda row: row.image_id)),
            tuple(sorted(binding.targets, key=lambda row: (row.target, row.image_id))),
            tuple(sorted(binding.members, key=lambda row: row.key)),
            Path(context["directory"]),
        )
        original_input = invocation.OriginalInput(
            member,
            image,
            original,
            payload,
            _call_scope(binding, arguments["run_dir"], member, events),
            contexts,
        )
        command = _native_command(
            candidate,
            arguments["rizin"],
            Path(run["binaries"]["plugin"]["path"]),
            arguments["output_limit"],
        )
        events.extend(
            invocation.verify_native(
                command,
                original_input,
                invocation.NativeAttempt(None, native, None),
                source_form=form,
            )
        )
        status = nativeoutput.validate_status(
            _read(candidate / "work/status.json", nativeoutput.STATUS_LIMIT),
            _read(candidate / "input/request.json", 64 << 10),
            _read(candidate / "native/fixed-config.json", nativeoutput.CONFIG_LIMIT),
        )
        nativeoutput._clean_input(status)
        process = native.get("process", {})
        _require(
            not events
            and native.get("launch") == "observed"
            and process.get("capture_complete") is True
            and type(process.get("raw_exit")) is int
            and process["raw_exit"] >= 0
            and process.get("failure") is None
            and process.get("owner_observation") is None
            and not native.get("secondary_failures")
            and not native.get("failure")
            and status == native.get("status")
            and status["status"] in {"refused", "generation-failed"}
            and bool(status["reason"])
            and status["retdec_failed"] is not True
            and any(
                word in status["reason"].lower()
                for word in ("unknown", "unresolved", "unsupported", "ambiguous")
            )
            and all(
                row["disposition"] == "unattempted"
                and row["terminal"] is False
                and row["raw_exit"] is None
                and row["exit_code"] is None
                and row["stages"] == []
                for row in attempts
            ),
            "native refusal is not complete original-only unresolved-evidence proof",
        )
        return "native-refused", [
            dict(status="unresolved-evidence", reason=status["reason"])
        ]
    inspection = invocation.inspect_native(
        candidate, submitted, run, binding, arguments["run_dir"], read_bytes
    )
    _require(
        inspection.unit is not None
        and not inspection.events
        and not submitted.get("failure"),
        "generation source cannot be authenticated",
    )
    build = submitted["build"]
    _require(
        _document(file_record(candidate / "build/result.json")) == build,
        "generation build terminal differs",
    )
    syntax_record = _document(file_record(candidate / "build/syntax-result.json"))
    frozen = _document(file_record(candidate / "build/inputs.json"))
    immutable = [
        *syntax_record["expected_inputs"],
        *invocation.NativeAttempt(None, native, None).required_files,
    ]
    _require(
        frozen
        == dict(
            source=syntax_record["source"],
            prelude=syntax_record["prelude"],
            tools=run["tools"],
            immutable=immutable,
        )
        and all(build.get(name) == value for name, value in syntax_record.items())
        and build["profile_attempts"] == attempts,
        "generation syntax and compiler inventories differ",
    )
    syntax = recipe.SyntaxAttempt(
        syntax_record,
        recipe.FrozenInputs(
            syntax_record["source"], syntax_record["prelude"], run["tools"], immutable
        ),
        None,
        (),
        recipe.c89_call(
            workspace=candidate / "build",
            tools=run["tools"],
            output_limit=arguments["output_limit"],
        ),
    )
    contexts, receipts = [], []
    for index, row in enumerate(attempts):
        directory = (
            candidate / "build"
            if index == 0
            else candidate / "build/profiles" / f"{index:03d}"
        )
        if row.get("terminal") is True:
            raw = _read(directory / "profile-result.json", 1 << 20)
            _require(
                nativeoutput.parse_json(raw, 1 << 20) == row,
                "generation profile receipt differs",
            )
            receipts.append(
                recipe.ProfileReceipt(
                    index,
                    directory / "profile-result.json",
                    raw,
                    len(raw),
                    hashlib.sha256(raw).hexdigest(),
                )
            )
            contexts.append(
                recipe.DeclaredProfileContext(
                    index, directory, _read(directory / "recipe.json", 1 << 20)
                )
            )
    events = list(
        _compiler_guards(
            syntax,
            attempts,
            workspace=candidate / "build",
            output_limit=arguments["output_limit"],
            contexts=contexts,
            receipts=receipts,
        )
    )
    process = syntax_record["c89"]
    _require(
        not events
        and process.get("capture_complete") is True
        and process.get("owner_observation") is None
        and process.get("failure") is None
        and type(process.get("raw_exit")) is int
        and process["raw_exit"] >= 0
        and syntax_record["c89_launch"] == "observed",
        "generation compiler ownership is unresolved",
    )
    if process["raw_exit"] != 0:
        _require(
            build["status"] == "source-refused"
            and all(
                row["disposition"] == "unattempted"
                and not row["terminal"]
                and row["status"] == "source-refused"
                for row in attempts
            ),
            "syntax refusal has incomplete nonlaunch proof",
        )
        return "source-refused", []
    complete = all(
        row.get("terminal") is True
        and row.get("launch") == "observed"
        and row.get("failure") is None
        and row.get("status") == "compiled-unverified"
        for row in attempts
    )
    if (
        complete
        and build["status"] == "compiled-unverified"
        and not build.get("entry_failure")
    ):
        return "generation-complete", []
    _require(
        all(
            row.get("terminal") is True
            and row.get("launch") == "observed"
            and not row.get("secondary_failures")
            and all(
                stage["process"].get("failure") is None
                and stage["process"].get("owner_observation") is None
                and stage["process"].get("capture_complete") is True
                and type(stage["process"].get("raw_exit")) is int
                and stage["process"]["raw_exit"] >= 0
                for stage in row["stages"]
            )
            or profiles[index]["availability"] != "available"
            and row["status"] == "catalog-refused"
            for index, row in enumerate(attempts)
        ),
        "compiler batch is unresolved",
    )
    return "compiler-refused", []


def _terminal_association(attempt, pin, terminal, reservations, *, generation):
    fields = (
        {"run", "report", "submission", "generation_grant_id"}
        if generation
        else {
            "run",
            "report",
            "submission",
            "generation_terminal",
            "rebuild",
            "comparison",
        }
    )
    attempt_id = (
        attempt.get("generation_attempt_id", attempt["id"])
        if generation
        else attempt["id"]
    )
    slot = (
        Path(attempt["workspace"]) / "member-000/terminal-member.json"
        if generation
        else Path(attempt["feedback_workspace"]) / "terminal.json"
    )
    reservation = terminal["reservation"]
    _require(
        set(terminal)
        == {"attempt_id", "reservation", "disposition", "evidence"}
        | ({"evidence_requests"} if generation else set())
        and type(terminal["evidence"]) is dict
        and (set(terminal["evidence"]) == fields)
        and (terminal["attempt_id"] == attempt_id)
        and (Path(pin["path"]) == slot)
        and (reservation in reservations)
        and (reservation["attempt_id"] == attempt_id)
        and (reservation["member_key"] == attempt["member_key"])
        and (reservation["cursor"]["phase"] == ("native" if generation else "compare"))
        and (reservation["terminal_slot"] == str(slot))
        and (
            reservation["grant_id"]
            == (attempt["generation_grant_id"] if generation else attempt["grant_id"])
        ),
        "terminal differs from its original named reservation or evidence",
    )
    _require(
        all(attempt.get(name) == value for name, value in terminal["evidence"].items()),
        "terminal replaces previously pinned evidence",
    )


def _authenticate_retained_outputs(attempt, binding, arguments):
    if "run" not in attempt:
        return
    run = _document(attempt["run"])
    workspace = Path(attempt["workspace"])
    member = binding.member(attempt["member_key"])
    _require(
        Path(attempt["run"]["path"]) == workspace / "run.json"
        and run["selected"] == [member.key]
        and run["source_form"] == attempt["source_form"]
        and run["problem"] == arguments["requested"]["problem"],
        "unresolved generation owner differs",
    )
    PreparedInputs.attach(run, run_directory=workspace).verify()
    candidate = workspace / "member-000"
    submitted = _document(attempt["submission"]) if "submission" in attempt else {}
    if submitted:
        _require(
            Path(attempt["submission"]["path"]) == candidate / "result.json"
            and submitted["identity"] == asdict(member),
            "retained unresolved submission differs",
        )
    native = submitted.get("native")
    native_path = candidate / "native-result.json"
    if native_path.exists() or native_path.is_symlink():
        pin = file_record(native_path)
        physical = _document(pin)
        _require(
            native is None or native == physical,
            "retained unresolved native metadata differs",
        )
        native = physical
        attempt["retained_native"] = pin
    if native is not None:
        _require(
            native["workspace"] == str(candidate),
            "retained native output owner differs",
        )
        for name, records in native.get("inputs", {}).items():
            _require(
                name in {"input", "native"}
                and records == PreparedInputs.tree(candidate / name),
                "retained unresolved native input inventory differs",
            )
        if "outputs" in native:
            _require(
                native["outputs"] == PreparedInputs.tree(candidate / "work"),
                "retained unresolved native output inventory differs",
            )
        if "process" in native:
            receipt = file_record(candidate / "native.json")
            _require(
                comparison._authenticated_bytes(
                    receipt, 6 * arguments["output_limit"] + (64 << 10), []
                )
                == encode_json(native["process"], path=candidate / "native.json"),
                "retained unresolved native capture metadata differs",
            )
    build_path = candidate / "build/result.json"
    build = submitted.get("build")
    if build_path.exists() or build_path.is_symlink():
        pin = file_record(build_path)
        physical = _document(pin)
        _require(
            build is None or build == physical,
            "retained unresolved compiler metadata differs",
        )
        build = physical
        attempt["retained_build"] = pin
    if build is not None:
        for row in build["profile_attempts"]:
            if row.get("terminal"):
                directory = (
                    candidate / "build"
                    if row["ordinal"] == 0
                    else candidate / "build/profiles" / f"{row['ordinal']:03d}"
                )
                _require(
                    _document(file_record(directory / "profile-result.json")) == row,
                    "retained unresolved profile metadata differs",
                )
    documents = [submitted, native, build]
    for name in ("rebuild", "comparison"):
        if name not in attempt:
            continue
        observed = _document(attempt[name])
        _require(
            observed["operation"] == name
            and (
                observed.get("identity") is None
                or observed["identity"] == asdict(member)
            ),
            "retained unresolved feedback owner differs",
        )
        if name == "rebuild":
            _require(
                observed["submitted_run"] == attempt["run"]
                and observed["submitted_result"] == attempt["submission"],
                "unresolved rebuild reparents its submission",
            )
        else:
            _require(
                observed["rebuild_record"] == attempt["rebuild"],
                "unresolved comparison changes its rebuild",
            )
        documents.append(observed)
    recipe.verify_inputs(
        [
            pin
            for document in documents
            for pin in comparison._nested_records(document)
            if Path(pin["path"]).is_absolute()
        ]
    )
    observations = [
        value for document in documents for value in _cleanup_observations(document)
    ]
    if observations:
        attempt["cleanup_uncertainties"] = observations


def _cleanup_observations(value):
    if type(value) is dict:
        observations = (
            [value["owner_observation"]]
            if value.get("owner_observation") is not None
            else []
        )
        return observations + [
            row
            for key, child in value.items()
            if key != "owner_observation"
            for row in _cleanup_observations(child)
        ]
    if type(value) is list:
        return [row for child in value for row in _cleanup_observations(child)]
    return []


def _authenticate_attempt(attempt, binding, arguments, reservations):
    attempt = deepcopy(attempt)
    _require(
        type(attempt) is dict
        and type(attempt.get("attempt_ordinal")) is int
        and attempt["attempt_ordinal"] >= 0
        and attempt["member_key"] in arguments["member_keys"]
        and type(attempt["form_ordinal"]) is int
        and attempt["form_ordinal"] in range(2)
        and attempt["source_form"]
        == tuple(nativeoutput.SourceForm)[attempt["form_ordinal"]].value,
        "retained attempt identity differs",
    )
    for pin in comparison._nested_records(attempt):
        comparison._authenticated_bytes(pin, ARTIFACT_LIMIT, [], reader=read_bytes)
    known_slots = [
        row
        for row in reservations
        if row["attempt_id"] == attempt["id"]
        and row["terminal_slot"] is not None
        and row["cursor"]["phase"] in {"native", "compare"}
    ]
    for reservation in known_slots:
        field = (
            "generation_terminal"
            if reservation["cursor"]["phase"] == "native"
            else "terminal"
        )
        path = Path(reservation["terminal_slot"])
        if field not in attempt and path.exists():
            try:
                pin = file_record(path)
                terminal = _document(pin)
                _require(
                    terminal["reservation"] == reservation
                    and terminal["attempt_id"] == attempt["id"],
                    "named terminal slot association differs",
                )
                _require(
                    type(terminal["evidence"]) is dict
                    and all(
                        name not in attempt or attempt[name] == value
                        for name, value in terminal["evidence"].items()
                    ),
                    "recovery replaces previously pinned evidence",
                )
                recovered = {**attempt, **terminal["evidence"], field: pin}
                _terminal_association(
                    recovered,
                    pin,
                    terminal,
                    reservations,
                    generation=field == "generation_terminal",
                )
                if field == "generation_terminal":
                    disposition, requests = _generation_disposition(
                        recovered, binding, arguments
                    )
                    _require(
                        terminal["disposition"] == disposition
                        and terminal["evidence_requests"] == requests,
                        "named generation terminal proof differs",
                    )
                else:
                    comparison.inspect_original_feedback(
                        comparison_record=recovered["comparison"],
                        prepared_run=arguments["run_dir"],
                        artifact_limit=ARTIFACT_LIMIT,
                        output_limit=arguments["output_limit"],
                    )
                attempt = recovered
                attempt["recovered_terminal"] = pin
            except Exception as error:
                attempt.setdefault("recovery_failures", []).append(
                    f"{type(error).__name__}: {error}"
                )
    _authenticate_retained_outputs(attempt, binding, arguments)
    pin = attempt.get("generation_terminal")
    if pin is None:
        _require(
            attempt.get("status")
            not in {
                "measured",
                "native-refused",
                "source-refused",
                "compiler-refused",
                "generation-complete",
            }
            and not attempt.get("completed"),
            "claimed terminal lacks its generation receipt",
        )
        return _UnresolvedAttempt(
            attempt, "generation has no authenticated terminal", None
        )
    terminal = _document(pin)
    _terminal_association(attempt, pin, terminal, reservations, generation=True)
    disposition, requests = _generation_disposition(attempt, binding, arguments)
    _require(
        terminal["disposition"] == disposition
        and terminal["evidence_requests"] == requests,
        "generation terminal disposition or evidence differs",
    )
    attempt["evidence_requests"] = requests
    if disposition != "generation-complete":
        _require(
            attempt.get("status") != "measured",
            "refused generation cannot claim a score",
        )
        return _TerminalCandidate(attempt, disposition, ())
    if "terminal" not in attempt:
        _require(
            attempt.get("status") != "measured",
            "measured candidate lacks its terminal receipt",
        )
        return _UnresolvedAttempt(
            attempt, "complete generation needs independent feedback", pin
        )
    final = _document(attempt["terminal"])
    _terminal_association(
        attempt, attempt["terminal"], final, reservations, generation=False
    )
    _require(
        final["attempt_id"] == attempt["id"]
        and final["disposition"] == "measured"
        and (final["reservation"] in reservations)
        and (final["reservation"]["cursor"]["phase"] == "compare")
        and all(
            (attempt.get(name) == value for name, value in final["evidence"].items())
        ),
        "measured terminal association differs",
    )
    measured = comparison.inspect_original_feedback(
        comparison_record=attempt["comparison"],
        prepared_run=arguments["run_dir"],
        artifact_limit=ARTIFACT_LIMIT,
        output_limit=arguments["output_limit"],
    )
    rebuilt = _document(attempt["rebuild"])
    _require(
        measured["rebuild_record"] == attempt["rebuild"]
        and rebuilt["submitted_result"] == attempt["submission"]
        and (rebuilt["submitted_run"] == attempt["run"]),
        "feedback reparents or changes its original pair",
    )
    expected_profiles = _document(attempt["run"])["tools"]["profiles"]
    profiles = tuple(
        _score_links(
            measured,
            binding,
            arguments["run_dir"],
            binding.member(attempt["member_key"]),
            attempt["form_ordinal"],
            expected_profiles,
        )
    )
    return _TerminalCandidate(attempt, "measured", profiles)


def _read_resume(pin, binding, arguments):
    path = pin.path
    _require(
        path.is_absolute()
        and path.resolve() == path
        and not path.is_symlink()
        and path.is_file()
        and (
            path.name == "report.json"
            or re.fullmatch(r"progress-[0-9]+\.json", path.name)
        ),
        "resume requires one canonical report or progress file",
    )
    raw = _read(path, ARTIFACT_LIMIT)
    record = dict(
        path=str(path), bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest()
    )
    _require(record["sha256"] == pin.sha256, "supplied resume SHA-256 differs")
    prior = nativeoutput.parse_json(raw, ARTIFACT_LIMIT)
    _require(
        type(prior) is dict
        and prior.get("search_version") == SEARCH_VERSION
        and prior.get("run_id") == binding.run_id
        and prior.get("binding_digest") == binding.digest
        and prior.get("selected") == list(arguments["member_keys"])
        and prior.get("source_forms")
        == [form.value for form in nativeoutput.SourceForm]
        and prior.get("denominator") == MEMBER_COUNT
        and prior.get("semantic_admission") is False
        and prior.get("autonomous_exact") is False
        and prior.get("exclusions")
        == dict(
            count=7,
            association=dict(
                problem_sha256=binding.problem_doc_sha256,
                binding_digest=binding.digest,
                run_dir=str(arguments["run_dir"]),
            ),
        )
        and [row["identity"] for row in prior["members"]]
        == [asdict(member) for member in binding.members],
        "resume corpus, queue, source identity or supported record shape differs",
    )
    owner = _document(prior["run"])
    _require(
        owner["campaign"] == prior["campaign"]
        and owner["selected"] == prior["selected"]
        and owner["requested"] == prior["requested"]
        and owner["grant"] in prior["grants"]
        and Path(prior["run"]["path"]) == Path(prior["destination"]) / "run.json"
        and owner["search_version"] == SEARCH_VERSION,
        "resume campaign owner differs",
    )
    if owner["resumed_from"] is not None:
        predecessor = _document(owner["resumed_from"])
        _require(
            prior["resumed_from"] == owner["resumed_from"]
            and predecessor["campaign"] == prior["campaign"]
            and predecessor["requested"] == prior["requested"],
            "resume lineage changes its pinned predecessor",
        )
        for name in (
            "operation_reservations",
            "member_budgets",
            "grant_owners",
            "cleanup_uncertainties",
        ):
            _require(
                prior[name][: len(predecessor[name])] == predecessor[name],
                f"resume refunds or replaces retained {name}",
            )
        current_attempts = {value["id"]: value for value in _attempts(prior)}
        for previous_attempt in _attempts(predecessor):
            _require(
                previous_attempt["id"] in current_attempts,
                "resume removes a retained attempt",
            )
            current_attempt = current_attempts[previous_attempt["id"]]
            for name in (
                "attempt_ordinal",
                "member_key",
                "form_ordinal",
                "source_form",
                "grant_id",
                "workspace",
                "feedback_workspace",
                "member_budget",
                "generation_grant_id",
                "run",
                "report",
                "submission",
                "generation_terminal",
                "rebuild",
                "comparison",
                "terminal",
            ):
                _require(
                    name not in previous_attempt
                    or current_attempt.get(name) == previous_attempt[name],
                    "resume changes original retained evidence",
                )
    grants = [
        _CampaignGrant.parse(value, arguments["member_keys"])
        for value in prior["grants"]
    ]
    _require(
        len({grant.grant_id for grant in grants}) == len(grants) and bool(grants),
        "resume grant lineage repeats an id",
    )
    by_grant = {grant.grant_id: grant for grant in grants}
    _require(
        type(prior.get("grant_owners")) is list
        and len(prior["grant_owners"]) == len(grants),
        "grant lineage lacks immutable owners",
    )
    previous = None
    for grant, origin in zip(grants, prior["grant_owners"], strict=True):
        _require(
            type(origin) is dict
            and set(origin) == {"grant_id", "run"}
            and origin["grant_id"] == grant.grant_id,
            "grant owner identity differs",
        )
        original = _document(origin["run"])
        _require(
            original["search_version"] == SEARCH_VERSION
            and original["campaign"] == prior["campaign"]
            and original["grant"] == asdict(grant)
            and original["grant_origin"] is True
            and original["grant_owner"] is None
            and original["prior_grant_owner"] == previous
            and original["selected"] == prior["selected"]
            and original["problem"] == owner["problem"]
            and original["requested"] == owner["requested"],
            "imported grant differs from its immutable original outer-run owner",
        )
        _require(
            Path(origin["run"]["path"]).name == "run.json",
            "original grant owner changes its invocation slot",
        )
        previous = origin["run"]
    _require(
        owner["grant"] == asdict(grants[-1])
        and (
            prior["run"] == previous
            if owner["grant_origin"]
            else owner["grant_owner"] == previous
        ),
        "resumed owner reparents the active grant",
    )
    for record_budget in prior["member_budgets"]:
        budget = _MemberBudget.parse(record_budget)
        grant = by_grant[budget.grant_id]
        _require(
            budget.key in arguments["member_keys"],
            "retained member identity differs",
        )
        admission = _document(budget.admission)
        admitting_owner = _document(admission["owner_run"])
        _require(
            admission
            == dict(
                key=budget.key,
                grant_id=budget.grant_id,
                owner_run=admission["owner_run"],
            )
            and admitting_owner["campaign"] == prior["campaign"]
            and admitting_owner["grant"] == asdict(grant)
            and Path(budget.admission["path"]).parent
            == Path(admission["owner_run"]["path"]).parent / "member-admissions",
            "member admission changes its immutable owner",
        )
    _require(
        len({(row["key"], row["grant_id"]) for row in prior["member_budgets"]})
        == len(prior["member_budgets"]),
        "member admissions repeat",
    )
    by_attempt = {attempt["id"]: attempt for attempt in _attempts(prior)}
    _require(
        len(by_attempt) == len(_attempts(prior))
        and len({attempt["attempt_ordinal"] for attempt in by_attempt.values()})
        == len(by_attempt),
        "retained attempt identity repeats",
    )
    _Cursor.parse(prior["cursor"], arguments["member_keys"])
    for index, reservation in enumerate(prior["operation_reservations"]):
        _require(
            type(reservation) is dict
            and set(reservation)
            == {
                "id",
                "grant_id",
                "units",
                "cursor",
                "member_key",
                "attempt_id",
                "terminal_slot",
                "owner_run",
            }
            and reservation["id"] == index
            and reservation["grant_id"] in by_grant,
            "retained reservation identity differs",
        )
        cursor = _Cursor.parse(reservation["cursor"], arguments["member_keys"])
        reserving_owner = _document(reservation["owner_run"])
        _require(
            reserving_owner["grant"] == asdict(by_grant[reservation["grant_id"]])
            and reserving_owner["campaign"] == prior["campaign"],
            "reservation original owner differs",
        )
        expected_units = {
            "prepare": 1,
            "self-check": 1,
            "native": 1,
            "syntax": 1,
            "profile": 1,
            "rebuild": 6,
            "compare": 5,
        }
        _require(
            type(reservation["units"]) is int
            and reservation["units"] == expected_units.get(cursor.phase),
            "retained reservation units differ",
        )
        if reservation["attempt_id"] is not None:
            attempt = by_attempt[reservation["attempt_id"]]
            _require(
                reservation["member_key"] == attempt["member_key"]
                and cursor.attempt_ordinal == attempt["attempt_ordinal"]
                and cursor.member_ordinal
                == arguments["member_keys"].index(attempt["member_key"])
                and cursor.form_ordinal == attempt["form_ordinal"]
                and reservation["grant_id"] == attempt["grant_id"],
                "reservation attempt association differs",
            )
            _require(
                attempt["member_budget"] in prior["member_budgets"],
                "attempt reservation changes its admitted member",
            )
            if reservation["terminal_slot"] is not None:
                expected = (
                    Path(attempt["workspace"]) / "member-000/terminal-member.json"
                    if cursor.phase == "native"
                    else Path(attempt["feedback_workspace"]) / "terminal.json"
                )
                _require(
                    cursor.phase in {"native", "compare"}
                    and Path(reservation["terminal_slot"]) == expected,
                    "reservation terminal slot differs",
                )
        else:
            _require(
                cursor.phase in {"prepare", "self-check"}
                and reservation["terminal_slot"] is None,
                "unowned work reservation differs",
            )
    for grant in grants:
        _require(
            sum(
                row["units"]
                for row in prior["operation_reservations"]
                if row["grant_id"] == grant.grant_id
            )
            <= grant.campaign_limit
            and sum(
                row["grant_id"] == grant.grant_id for row in prior["member_budgets"]
            )
            <= grant.member_limit,
            "retained account exceeds a finite grant",
        )
    recipe.verify_inputs([record, prior["run"]])
    return prior, record, grants


def _search_forms(*, binding, arguments):
    destination, selected = arguments["destination"], arguments["member_keys"]
    _require(
        not destination.exists()
        and not destination.is_symlink()
        and destination.parent.is_dir(),
        "search destination must be new and canonical",
    )
    forms = tuple(nativeoutput.SourceForm)
    _require(len(forms) == 2, "source search requires the reviewed two-form catalog")
    _require(
        (arguments["resume_from"] is None) == (arguments["resume_sha256"] is None),
        "resume path and SHA-256 must appear together",
    )
    prior = resume_record = None
    grants = []
    if arguments["resume_from"] is not None:
        digest = arguments["resume_sha256"]
        _require(
            type(digest) is str and re.fullmatch(r"[0-9a-f]{64}", digest),
            "resume SHA-256 must be lowercase hexadecimal",
        )
        prior, resume_record, grants = _read_resume(
            _ResumePin(Path(arguments["resume_from"]), digest), binding, arguments
        )
    owner_prior_grant = (
        None
        if prior is None
        else _document(prior["grant_owners"][-1]["run"])["prior_grant_owner"]
    )
    new_id = arguments["new_budget_id"]
    _require(
        new_id is None or prior is not None, "new_budget_id requires a resumed campaign"
    )
    if prior is not None and new_id is None:
        grant = grants[-1]
        for name in ("member_limit", "campaign_limit"):
            _require(
                arguments[name] is None or arguments[name] == getattr(grant, name),
                "same-grant resume cannot change finite allowances",
            )
    else:
        if new_id is not None:
            _require(
                all(
                    arguments[name] is not None
                    for name in ("member_limit", "campaign_limit")
                ),
                "a new grant requires all explicit finite allowances",
            )
        member_limit = (
            len(selected)
            if arguments["member_limit"] is None
            else arguments["member_limit"]
        )
        campaign_limit = (
            2 + 36 * len(selected)
            if arguments["campaign_limit"] is None
            else arguments["campaign_limit"]
        )
        grant = _CampaignGrant.parse(
            dict(
                grant_id=new_id or "initial",
                member_limit=member_limit,
                campaign_limit=campaign_limit,
            ),
            selected,
        )
        _require(
            grant.grant_id not in {old.grant_id for old in grants},
            "a new grant id repeats its imported lineage",
        )
        grants.append(grant)
    campaign = (
        prior["campaign"]
        if prior
        else hashlib.sha256(str(destination).encode()).hexdigest()
    )
    report = dict(
        operation="run",
        status="incomplete",
        exit=1,
        denominator=MEMBER_COUNT,
        run_id=binding.run_id,
        binding_digest=binding.digest,
        destination=str(destination),
        selected=list(selected),
        search_forms=True,
        search_version=SEARCH_VERSION,
        source_forms=[form.value for form in forms],
        campaign=campaign,
        grants=[asdict(value) for value in grants],
        grant_owners=[] if prior is None else deepcopy(prior["grant_owners"]),
        resumed_from=resume_record,
        requested=None if prior is None else deepcopy(prior["requested"]),
        operation_reservations=[]
        if prior is None
        else deepcopy(prior["operation_reservations"]),
        member_budgets=[] if prior is None else deepcopy(prior["member_budgets"]),
        failures=[] if prior is None else deepcopy(prior["failures"]),
        cleanup_uncertainties=[]
        if prior is None
        else deepcopy(prior.get("cleanup_uncertainties", [])),
        exclusions=dict(
            count=7,
            association=dict(
                problem_sha256=binding.problem_doc_sha256,
                binding_digest=binding.digest,
                run_dir=str(arguments["run_dir"]),
            ),
        ),
        reuse_refusals=[],
        cursor=asdict(_Cursor(None, None, "prepare")),
        semantic_admission=False,
        autonomous_exact=False,
        associated_data="unverified",
        placement="unverified",
        original_byte_comparison="unperformed",
        members=[
            dict(
                identity=asdict(member),
                reader="unattempted",
                generation="unattempted",
                compilation="unattempted",
                best=None,
                best_local_exact=False,
                candidates=[],
                attempts=[],
            )
            for member in binding.members
        ],
    )
    if prior:
        for row, old in zip(report["members"], prior["members"], strict=True):
            row["attempts"] = deepcopy(old["attempts"])
            row["retained_best"] = deepcopy(old.get("best") or old.get("retained_best"))
        report["prior_preparations"] = [
            *prior.get("prior_preparations", []),
            *([prior["prepared_inputs"]] if "prepared_inputs" in prior else []),
        ]
    destination.mkdir(mode=0o700)
    (destination / "attempts").mkdir(mode=0o700)
    (destination / "feedback").mkdir(mode=0o700)
    authenticated, progress = [], 0
    fatal = None
    active = None
    owner = None

    def checkpoint():
        nonlocal progress
        _derive_search(report, grant, authenticated)
        _publish_progress(report, destination, progress)
        progress += 1

    def publish_owner():
        nonlocal owner
        if owner is None:
            owner = dict(
                search_version=SEARCH_VERSION,
                campaign=campaign,
                grant=asdict(grant),
                selected=list(selected),
                source_forms=report["source_forms"],
                requested=report["requested"],
                problem=dict(
                    run_dir=str(arguments["run_dir"]),
                    binding_digest=binding.digest,
                    problem_sha256=binding.problem_doc_sha256,
                ),
                resumed_from=resume_record,
                grant_origin=prior is None or new_id is not None,
                grant_owner=None
                if prior is None or new_id is not None
                else report["grant_owners"][-1]["run"],
                prior_grant_owner=None
                if prior is None
                else report["grant_owners"][-1]["run"]
                if new_id is not None
                else owner_prior_grant,
            )
            path = destination / "run.json"
            write_bytes(path, encode_json(owner, path=path), readonly=True)
            report["run"] = file_record(path)
            if owner["grant_origin"]:
                report["grant_owners"].append(
                    dict(grant_id=grant.grant_id, run=report["run"])
                )

    try:
        requested = _request_identity(binding, arguments)
        if prior is not None:
            _require(
                requested == prior["requested"],
                "current configuration, executable, receipt, compiler catalog or complete driver identity differs",
            )
        report["requested"] = arguments["requested"] = requested
        publish_owner()
        if prior is not None:
            for pin in report.get("prior_preparations", []):
                prepared = PreparedInputs.open_owner(pin)
                prepared.verify()
                prepared_owner = prepared._record
                _require(
                    prepared_owner["campaign"]["id"] == campaign
                    and prepared_owner["campaign"]["grant_id"]
                    in {value.grant_id for value in grants}
                    and prepared_owner["problem"] == requested["problem"]
                    and all(
                        prepared_owner["binaries"][name] == requested[argument]
                        for name, argument in (
                            ("receipt", "native_receipt"),
                            ("configuration", "configuration"),
                            ("rizin", "rizin"),
                        )
                    ),
                    "imported preparation differs from its problem, caller or grant",
                )
            imported = []
            for row in report["members"]:
                for attempt in row["attempts"]:
                    item = _authenticate_attempt(
                        attempt,
                        binding,
                        arguments,
                        report["operation_reservations"],
                    )
                    imported.append(item)
                row["attempts"] = [
                    item.attempt
                    for item in imported
                    if item.attempt["member_key"] == row["identity"]["key"]
                ]
            recipe.verify_inputs([resume_record, prior["run"]])
            authenticated.extend(imported)
        report["authentication"] = "authenticated"
        completed = _derive_search(report, grant, authenticated)
        checkpoint()
        pending = [
            (key, index)
            for key in selected
            for index in range(2)
            if (key, index) not in completed
        ]
        shared = contexts = config_bytes = shared_record = None
        has_native = any(
            not any(
                isinstance(item, _UnresolvedAttempt)
                and item.generation_terminal is not None
                and item.attempt["member_key"] == key
                and item.attempt["form_ordinal"] == index
                for item in authenticated
            )
            for key, index in pending
        )

        def member_available(key):
            budget = next(
                (
                    row
                    for row in report["member_budgets"]
                    if row["key"] == key and row["grant_id"] == grant.grant_id
                ),
                None,
            )
            return budget is not None or report["remaining"]["member_admissions"] > 0

        if has_native:
            _require(
                any(member_available(key) for key, _ in pending),
                "no member admission remains before preparation",
            )
            if (
                report["remaining"]["units"] == 0
                or report["remaining"]["member_admissions"] == 0
                and not any(
                    row["grant_id"] == grant.grant_id
                    for row in report["member_budgets"]
                )
            ):
                raise _BudgetExhausted("no work or member allowance before preparation")
            _admit_work(
                report,
                grant,
                _Cursor(None, None, "prepare"),
                units=1,
                member=None,
                checkpoint=checkpoint,
            )
            prepared_root = destination / "prepared"
            prepared_root.mkdir(mode=0o700)
            shared_record, config_bytes, shared = _prepare_run(
                arguments["root"],
                prepared_root,
                arguments["native_receipt"],
                arguments["configuration"],
                arguments["rizin"],
                report,
                [],
            )
            contexts = calltargets.prepare_native_contexts(
                binding, arguments["run_dir"], prepared_root / "binding-inputs"
            )
            shared_record.update(
                binding_inputs=dict(
                    directory=str(contexts.input_directory),
                    files=PreparedInputs.tree(contexts.input_directory),
                ),
                problem=owner["problem"],
                campaign=dict(
                    id=campaign,
                    grant_id=grant.grant_id,
                    owner_run=report["run"],
                ),
            )
            path = prepared_root / "inputs.json"
            shared.verify()
            write_bytes(path, encode_json(shared_record, path=path), readonly=True)
            report["prepared_inputs"] = file_record(path)
            _require(
                any(member_available(key) for key, _ in pending),
                "no member admission remains before self-check",
            )
            _admit_work(
                report,
                grant,
                _Cursor(None, None, "self-check"),
                units=1,
                member=None,
                checkpoint=checkpoint,
            )
            self_check = invocation.capture_native(
                _native_command(
                    prepared_root / "self-check",
                    arguments["rizin"],
                    Path(shared_record["binaries"]["plugin"]["path"]),
                    arguments["output_limit"],
                ),
                configuration=config_bytes,
                original=None,
            )
            report["self_check"] = dict(
                prepared_inputs=report["prepared_inputs"],
                evidence=self_check.evidence,
            )
            report["cleanup_uncertainties"].extend(
                dict(attempt_id=None, owner_observation=observed)
                for observed in _cleanup_observations(self_check.evidence)
            )
            if self_check.pending is not None:
                raise self_check.pending
            _require(
                self_check.launch == "observed"
                and self_check.status == "self-check-complete"
                and self_check.raw_exit == 0
                and self_check.process_failure is None,
                "shared native self-check failed",
            )
            checkpoint()
        for member_index, key in enumerate(selected):
            unresolved = [index for index in range(2) if (key, index) not in completed]
            if not unresolved:
                continue
            report["cursor"] = asdict(_Cursor(member_index, unresolved[0], "native"))
            try:
                member_budget = _member_budget(report, grant, key)
            except _BudgetExhausted as error:
                report["failures"].append(str(error))
                report["budget_status"] = "member-admissions-exhausted"
                break
            for form_index in unresolved:
                prior_generation = next(
                    (
                        item
                        for item in reversed(authenticated)
                        if isinstance(item, _UnresolvedAttempt)
                        and item.generation_terminal is not None
                        and item.attempt["member_key"] == key
                        and item.attempt["form_ordinal"] == form_index
                    ),
                    None,
                )
                ordinal = 1 + max(
                    (attempt["attempt_ordinal"] for attempt in _attempts(report)),
                    default=-1,
                )
                workspace = destination / "attempts" / f"{ordinal:06d}"
                feedback_root = destination / "feedback" / f"{ordinal:06d}"
                active = dict(
                    id=f"{campaign}/{ordinal}",
                    attempt_ordinal=ordinal,
                    member_key=key,
                    source_form=forms[form_index].value,
                    form_ordinal=form_index,
                    grant_id=grant.grant_id,
                    origin_campaign=campaign,
                    member_budget=asdict(member_budget),
                    workspace=str(workspace),
                    feedback_workspace=str(feedback_root),
                    status="unresolved",
                    failure=None,
                    evidence_requests=[],
                )
                row = next(
                    row for row in report["members"] if row["identity"]["key"] == key
                )
                row["attempts"].append(active)

                def cursor(phase, profile=None):
                    return _Cursor(
                        member_index,
                        form_index,
                        phase,
                        profile,
                        ordinal,
                    )

                if prior_generation is not None:
                    for name in (
                        "workspace",
                        "run",
                        "report",
                        "submission",
                        "generation_terminal",
                        "generation_grant_id",
                    ):
                        active[name] = prior_generation.attempt[name]
                    active["generation_attempt_id"] = prior_generation.attempt.get(
                        "generation_attempt_id",
                        prior_generation.attempt["id"],
                    )
                    active["restart_phase"] = "rebuild"
                    checkpoint()
                else:
                    workspace.mkdir(mode=0o700)
                    run_record = deepcopy(shared_record)
                    run_record.update(
                        selected=[key],
                        source_form=forms[form_index].value,
                        prepared_inputs=report["prepared_inputs"],
                        limits=dict(
                            process_output_bytes=arguments["output_limit"],
                            artifact_bytes=ARTIFACT_LIMIT,
                        ),
                    )
                    path = workspace / "run.json"
                    write_bytes(
                        path,
                        encode_json(run_record, path=path),
                        readonly=True,
                    )
                    active.update(
                        run=file_record(path),
                        generation_grant_id=grant.grant_id,
                        restart_phase="native",
                    )
                    reservation = _admit_work(
                        report,
                        grant,
                        cursor("native"),
                        units=1,
                        member=member_budget,
                        checkpoint=checkpoint,
                        terminal_slot=workspace / "member-000/terminal-member.json",
                        attempt_id=active["id"],
                    )
                    submitted = dict(
                        identity=asdict(binding.member(key)),
                        reader="unattempted",
                        generation="unattempted",
                        compilation="unattempted",
                        profile_attempts=_profile_rows(shared_record["tools"]),
                    )
                    member_events = []
                    try:
                        _run_member(
                            binding=binding,
                            run_dir=arguments["run_dir"],
                            member=binding.member(key),
                            row=submitted,
                            workspace=workspace / "member-000",
                            run_record=run_record,
                            inputs=shared,
                            contexts=contexts,
                            rizin=arguments["rizin"],
                            plugin=Path(run_record["binaries"]["plugin"]["path"]),
                            output_limit=arguments["output_limit"],
                            config_bytes=config_bytes,
                            source_form=forms[form_index],
                            run_events=member_events,
                            retained_receipts=[],
                            before_compile=lambda phase, profile: _admit_work(
                                report,
                                grant,
                                cursor(phase, profile),
                                units=1,
                                member=member_budget,
                                checkpoint=checkpoint,
                                attempt_id=active["id"],
                            ),
                        )
                    finally:
                        observations = _cleanup_observations(submitted)
                        if observations:
                            active["cleanup_uncertainties"] = observations
                        path = workspace / "member-000/result.json"
                        if path.is_file():
                            active["submission"] = file_record(path)
                        child = dict(
                            operation="run",
                            status="failed",
                            exit=1,
                            run_id=binding.run_id,
                            binding_digest=binding.digest,
                            denominator=MEMBER_COUNT,
                            selected=[key],
                            source_form=forms[form_index].value,
                            semantic_admission=False,
                            autonomous_exact=False,
                            secondary_failures=facts(member_events),
                            members=[
                                submitted
                                if member.key == key
                                else dict(
                                    identity=asdict(member),
                                    reader="unattempted",
                                    generation="unattempted",
                                    compilation="unattempted",
                                )
                                for member in binding.members
                            ],
                            snapshots_unchanged=not member_events,
                        )
                        path = workspace / "report.json"
                        write_bytes(
                            path,
                            encode_json(child, path=path),
                            readonly=True,
                        )
                        active["report"] = file_record(path)
                    disposition, requests = _generation_disposition(
                        active, binding, arguments
                    )
                    evidence = {
                        name: active[name]
                        for name in (
                            "run",
                            "report",
                            "submission",
                            "generation_grant_id",
                        )
                    }
                    active["generation_terminal"] = _write_terminal(
                        Path(reservation["terminal_slot"]),
                        dict(
                            attempt_id=active["id"],
                            reservation=reservation,
                            disposition=disposition,
                            evidence=evidence,
                            evidence_requests=requests,
                        ),
                    )
                    item = _authenticate_attempt(
                        active,
                        binding,
                        arguments,
                        report["operation_reservations"],
                    )
                    authenticated.append(item)
                    active.update(evidence_requests=requests, status=disposition)
                    checkpoint()
                    if isinstance(item, _TerminalCandidate):
                        active = None
                        continue
                feedback_root.mkdir(mode=0o700)
                rebuilt, measured = {}, {}
                try:
                    _admit_work(
                        report,
                        grant,
                        cursor("rebuild"),
                        units=6,
                        member=member_budget,
                        checkpoint=checkpoint,
                        attempt_id=active["id"],
                    )
                    comparison.rebuild_candidate(
                        result_record=active["submission"],
                        run_record=active["run"],
                        workspace=feedback_root / "rebuild",
                        output_limit=arguments["output_limit"],
                        artifact_limit=ARTIFACT_LIMIT,
                        retained=rebuilt,
                    )
                    active["rebuild"] = file_record(
                        feedback_root / "rebuild/rebuild-result.json"
                    )
                    checkpoint()
                    reservation = _admit_work(
                        report,
                        grant,
                        cursor("compare"),
                        units=5,
                        member=member_budget,
                        checkpoint=checkpoint,
                        terminal_slot=feedback_root / "terminal.json",
                        attempt_id=active["id"],
                    )
                    comparison.compare_rebuilt_candidate(
                        rebuild_record=active["rebuild"],
                        prepared_run=arguments["run_dir"],
                        workspace=feedback_root / "comparison",
                        output_limit=arguments["output_limit"],
                        artifact_limit=ARTIFACT_LIMIT,
                        retained=measured,
                    )
                    active["comparison"] = file_record(
                        feedback_root / "comparison/comparison-result.json"
                    )
                    comparison.inspect_original_feedback(
                        comparison_record=active["comparison"],
                        prepared_run=arguments["run_dir"],
                        artifact_limit=ARTIFACT_LIMIT,
                        output_limit=arguments["output_limit"],
                    )
                    active["terminal"] = _write_terminal(
                        Path(reservation["terminal_slot"]),
                        dict(
                            attempt_id=active["id"],
                            reservation=reservation,
                            disposition="measured",
                            evidence={
                                name: active[name]
                                for name in (
                                    "run",
                                    "report",
                                    "submission",
                                    "generation_terminal",
                                    "rebuild",
                                    "comparison",
                                )
                            },
                        ),
                    )
                    item = _authenticate_attempt(
                        active,
                        binding,
                        arguments,
                        report["operation_reservations"],
                    )
                    _require(
                        isinstance(item, _TerminalCandidate),
                        "feedback did not authenticate a terminal candidate",
                    )
                    authenticated.append(item)
                    active.update(status="measured", profiles=list(item.profiles))
                finally:
                    observations = _cleanup_observations(
                        rebuilt
                    ) + _cleanup_observations(measured)
                    if observations:
                        active["cleanup_uncertainties"] = observations
                    for name in ("rebuild", "comparison"):
                        path = feedback_root / name / f"{name}-result.json"
                        if path.is_file():
                            active[name] = file_record(path)
                checkpoint()
                active = None
        if shared is not None:
            shared.verify()
        provenance.require_active_binding(
            binding, arguments["run_dir"], io_mode="exact"
        )
        report["snapshots_unchanged"] = True
        completed = _derive_search(report, grant, authenticated)
        if len(completed) == len(selected) * 2:
            report.update(
                status="cleanup-uncertain"
                if report["cleanup_uncertainties"]
                else "measured-unverified"
                if report["summary"]["local_best"] == len(selected)
                else "failed",
                exit=int(
                    bool(report["cleanup_uncertainties"])
                    or report["summary"]["local_best"] != len(selected)
                ),
                budget_status="complete",
                cursor=asdict(_Cursor(None, None, "done")),
            )
        else:
            report.setdefault("budget_status", "incomplete")
    except BaseException as error:
        fatal = error
        reason = f"{type(error).__name__}: {error}"
        report["failures"].append(reason)
        report["budget_status"] = (
            "exhausted" if isinstance(error, _BudgetExhausted) else "interrupted"
        )
        if active is not None:
            active.update(status="unresolved", failure=reason)
        if isinstance(error, ProcessCleanupError):
            report["owner_observation"] = error.observed
            report["capture_failures"] = error.capture_failures
            report["cleanup_uncertainties"].append(
                dict(
                    attempt_id=None if active is None else active["id"],
                    owner_observation=error.observed,
                    capture_failures=error.capture_failures,
                )
            )
        if owner is None:
            report["reuse_refusals"].append(reason)
            publish_owner()
        if prior is not None and report.get("authentication") != "authenticated":
            authenticated.clear()
            report["authentication"] = "refused"
            report["reuse_refusals"].append(reason)
    checkpoint()
    path = destination / "report.json"
    write_bytes(path, encode_json(report, path=path), readonly=True)
    if fatal is not None and (
        not isinstance(fatal, Exception)
        or isinstance(fatal, (ProcessCleanupError, OSError))
    ):
        raise fatal
    return report


def _require(condition: object, message: str) -> None:
    if not condition:
        raise PolicyFailure(message)


def _read(path, limit, events=None):
    data = read_bytes(path, limit, events)
    _require(len(data) <= limit, f"file exceeds read bound: {path}")
    return data


def _succeeded(record: dict) -> bool:
    return record["failure"] is None and record["raw_exit"] == 0


def _prepare_run(
    root, destination, native_receipt, configuration, rizin, report, events
):
    config_bytes = _read(configuration, 16 << 10, events)
    _require(
        hashlib.sha256(config_bytes).hexdigest() == nativeoutput.CONFIG_SHA256,
        "fixed native configuration differs",
    )
    selection = {}
    report["tools"] = selection
    try:
        report["tools"], _ = catalog.stage(
            root,
            destination / "tools",
            with_profiles=True,
            retained=selection,
            events=events,
        )
    finally:
        selection = report.get("tools")
        if selection is not None and "profiles" in selection:
            for row in report["members"]:
                selected = row["identity"]["key"] in report["selected"]
                row["profile_attempts"] = [
                    {
                        "profile_id": profile["profile_id"],
                        "profile_identity": profile["identity"],
                        "ordinal": profile["ordinal"],
                        "disposition": "unattempted",
                        "terminal": False,
                        "status": "native-unattempted" if selected else "not-selected",
                        "failure": None
                        if selected
                        else "member not selected for this bounded run",
                        **dict.fromkeys(("raw_exit", "exit_code")),
                        "stages": [],
                    }
                    for profile in selection["profiles"]
                ]
    record = {}
    inputs = PreparedInputs.prepare(
        record,
        destination=destination,
        native_receipt=native_receipt,
        configuration=configuration,
        rizin=rizin,
        selection=selection,
        driver_root=Path(__file__).resolve().parents[1],
    )
    return record, config_bytes, inputs


def _native_command(workspace, rizin, plugin, output_limit):
    workspace_text = str(workspace)
    _require(
        not any(char in workspace_text for char in '\\"\n\r'),
        "workspace cannot be quoted in the Rizin command",
    )
    return invocation.NativeCommand(
        workspace,
        (
            str(rizin),
            *("-N", "-n", "-q", "-a", "mips", "-b", "32"),
            "-l",
            str(plugin),
            "-c",
            f'pdzar "{workspace_text}"',
        ),
        {
            "HOME": str(workspace / "home"),
            "XDG_CONFIG_HOME": str(workspace / "config"),
            "PATH": "/usr/bin:/bin",
            "LC_ALL": "C",
            "RZ_NOPLUGINS": "1",
        },
        output_limit,
    )


def _original(binding, run_dir, member, events=None):
    image = binding.image(member.image_id)
    _require(
        image.bytes <= MAX_IMAGE_BYTES and image.payload_size <= MAX_PAYLOAD_BYTES,
        "original input exceeds native bounds",
    )
    _require(
        0 < member.end - member.start <= 64 << 10, "member range exceeds native bound"
    )
    original = provenance.read_original_image(
        binding,
        run_dir,
        target=member.target,
        image_id=member.image_id,
        io_mode="exact",
        events=events,
    )
    sliced = provenance.original_slice(
        binding,
        run_dir,
        target=member.target,
        image_id=member.image_id,
        pc=member.start,
        extent={"start": member.start, "end": member.end},
        subject=member.key,
        io_mode="exact",
        events=events,
    )
    payload = original[image.payload_offset : image.payload_offset + image.payload_size]
    _require(
        hashlib.sha256(
            payload[member.start - image.load_address : member.end - image.load_address]
        ).hexdigest()
        == member.range_sha256
        == sliced.slice_sha256,
        "selected original range differs",
    )
    return image, original, payload


def _call_scope(binding, run_dir, member, events=None):
    callees = []
    image_identity = (member.target, member.image_id)
    for peer in binding.members:
        if peer.key == member.key or (peer.target, peer.image_id) != image_identity:
            continue
        _require(
            0 < peer.end - peer.start <= 64 << 10,
            "reviewed callee extent exceeds native bound",
        )
        sliced = provenance.original_slice(
            binding,
            run_dir,
            target=peer.target,
            image_id=peer.image_id,
            pc=peer.start,
            extent={"start": peer.start, "end": peer.end},
            subject=peer.key,
            io_mode="exact",
            events=events,
        )
        _require(
            sliced.slice_sha256 == peer.range_sha256, "reviewed callee bytes differ"
        )
        callees.append({key: getattr(peer, key) for key in calltargets.MEMBER_FIELDS})
    return {"problem_sha256": binding.problem_doc_sha256, "callees": callees}


def _compiler_guards(
    syntax, attempts, *, workspace, output_limit, contexts=(), receipts=()
):
    events = []
    if syntax is not None:
        for phase in ("syntax", "profiles"):
            try:
                if phase == "syntax":
                    events.extend(recipe.verify_syntax(syntax))
                elif syntax.inputs is not None:
                    events.extend(
                        recipe.verify_attempts(
                            attempts,
                            syntax.inputs,
                            output_limit=output_limit,
                            contexts=contexts,
                        )
                    )
            except Exception as error:
                retain(events, error, phase=phase, path=workspace)
    events.extend(recipe.verify_receipts(receipts))
    return tuple(events)


def _compile(
    unit,
    workspace,
    row,
    run_record,
    output_limit,
    command,
    original_input,
    native_attempt,
    binding,
    run_dir,
    member,
    *,
    before_compile=None,
):
    events = []
    abrupt = None
    attempts = row["profile_attempts"]
    result = dict(
        status="interrupted",
        stages=[],
        semantic_admission=False,
        placement="unverified",
        selected_profile_id=recipe.CANONICAL_PROFILE_ID,
        profile_attempts=attempts,
    )
    syntax = None
    receipts, contexts = [], []
    try:
        selection = run_record["tools"]
        call = recipe.c89_call(
            workspace=workspace,
            tools=selection,
            output_limit=output_limit,
        )
        if before_compile is not None:
            before_compile("syntax", None)
        syntax = recipe.check_syntax(
            call,
            source=unit.source,
            prelude=prelude.STDINT_PRELUDE,
            prelude_path=prelude.PRELUDE_PATH,
            tools=selection,
            immutable=native_attempt.required_files,
            root_profile_id=recipe.CANONICAL_PROFILE_ID,
            artifact_limit=ARTIFACT_LIMIT,
        )
        result.update(syntax.record)
        events.extend(syntax.events)
        if syntax.pending is not None:
            raise syntax.pending
        if not _succeeded(result["c89"]):
            result["status"] = "source-refused"
            for attempt in attempts:
                attempt.update(
                    status="source-refused",
                    failure="C89 diagnostic refused the unchanged unit"
                    if result["c89_launch"] == "observed"
                    else "C89 command did not launch",
                )
        else:
            execution = recipe.execute_profiles(
                selection["profiles"],
                workspace=workspace,
                root_profile_id=recipe.CANONICAL_PROFILE_ID,
                inputs=syntax.inputs,
                include_roots=[str(workspace / "include")],
                attempts=attempts,
                output_limit=output_limit,
                artifact_limit=ARTIFACT_LIMIT,
            )
            profile_index = 0
            while True:
                provenance.require_active_binding(
                    binding, run_dir, io_mode="exact", events=events
                )
                _original(binding, run_dir, member, events)
                events.extend(
                    invocation.verify_native(
                        command,
                        original_input,
                        native_attempt,
                        source_form=unit.source_form,
                    )
                )
                events.extend(
                    _compiler_guards(
                        syntax,
                        attempts,
                        workspace=workspace,
                        output_limit=output_limit,
                        contexts=contexts,
                        receipts=receipts,
                    )
                )
                if primary(events) is not None:
                    raise primary(events)
                if (
                    profile_index < len(selection["profiles"])
                    and before_compile is not None
                    and selection["profiles"][profile_index]["availability"]
                    == "available"
                ):
                    before_compile("profile", profile_index)
                outcome = next(execution, None)
                if outcome is None:
                    break
                profile_index += 1
                events.extend(outcome.events)
                if outcome.receipt is not None:
                    receipts.append(outcome.receipt)
                if outcome.context is not None:
                    contexts.append(outcome.context)
                if outcome.ordinal == 0:
                    result.update(outcome.result)
                if outcome.ordinal == 0 and result["status"] == "compiled-unverified":
                    if unit.entry_symbol is None:
                        result["entry_failure"] = (
                            "native output has no unique safe observed entry symbol"
                        )
                    else:
                        try:
                            function = objectfile.extract_function(
                                _read(
                                    Path(result["artifact"]["path"]),
                                    ARTIFACT_LIMIT,
                                    events,
                                ),
                                unit.entry_symbol,
                                limit=ARTIFACT_LIMIT,
                            )
                            result["function"] = {
                                **asdict(function),
                                "data": function.data.hex(),
                            }
                        except objectfile.ObjectRefused as error:
                            result.update(
                                status="object-refused", object_failure=str(error)
                            )
                if outcome.abort is not None:
                    raise outcome.abort
    except BaseException as error:
        abrupt = error if not isinstance(error, Exception) else None
        retain(
            events,
            error
            if isinstance(error, Exception)
            else PolicyFailure(f"{type(error).__name__}: {error}"),
            phase="compile",
            path=workspace,
            origin="policy" if isinstance(error, PolicyFailure) else "unknown",
        )
    events.extend(
        _compiler_guards(
            syntax,
            attempts,
            workspace=workspace,
            output_limit=output_limit,
            contexts=contexts,
            receipts=receipts,
        )
    )
    pending = primary(events)
    if pending is not None:
        result["interruption"] = f"{type(pending).__name__}: {pending}"
        for attempt in attempts:
            try:
                if attempt["status"] == "native-unattempted":
                    attempt.update(
                        status="run-interrupted", failure=result["interruption"]
                    )
            except Exception as error:
                retain(events, error, phase="attempt-inventory", path=workspace)
        pending = primary(events)
    result["secondary_failures"] = facts(events)
    return (
        result,
        abrupt or pending,
        tuple(events),
        syntax,
        tuple(receipts),
        tuple(contexts),
    )


def _run_member(
    *,
    binding,
    run_dir,
    member,
    row,
    workspace,
    run_record,
    inputs,
    contexts,
    rizin,
    plugin,
    output_limit,
    config_bytes,
    source_form,
    run_events,
    retained_receipts,
    before_compile=None,
):
    compiled, pending, command, abrupt = None, None, None, None
    events = []
    observed, original_input, syntax = None, None, None
    receipts, profile_contexts = (), ()
    try:
        row["reader"] = "attempted"
        image, original, payload = _original(binding, run_dir, member, events)
        row["reader"] = "verified-original"
        row["generation"] = "attempted"
        inputs.verify(events=events)
        command = _native_command(workspace, rizin, plugin, output_limit)
        original_input = invocation.OriginalInput(
            member,
            image,
            original,
            payload,
            _call_scope(binding, run_dir, member, events),
            contexts,
            tuple(run_record.get("overlay_mounts", ())),
            invocation.read_entry_observation(
                run_record.get("entry_observation"),
                run_record.get("entry_observation_inputs", {}).get(
                    "directory", workspace / "input"
                ),
            ),
        )
        observed = invocation.capture_native(
            command,
            configuration=config_bytes,
            original=original_input,
            source_form=source_form,
        )
        events.extend(observed.events)
        row.update(generation="captured-unqualified", native=observed.evidence)
        pending = observed.pending
        if pending is not None:
            raise abrupt or pending
        _require(observed.launch == "observed", "native launch was not observed")
        if (
            observed.unit is not None
            and observed.raw_exit == 0
            and observed.process_failure is None
            and observed.status == "generation-complete"
        ):
            row["compilation"] = "attempted"
            (
                compiled,
                pending,
                compile_events,
                syntax,
                receipts,
                profile_contexts,
            ) = _compile(
                observed.unit,
                workspace / "build",
                row,
                run_record,
                output_limit,
                command,
                original_input,
                observed,
                binding,
                run_dir,
                member,
                before_compile=before_compile,
            )
            if pending is not None and not isinstance(pending, Exception):
                abrupt = pending
            retained_receipts.extend(receipts)
            events.extend(compile_events)
            row.update(compilation=compiled["status"], build=compiled)
        else:
            row.update(
                generation="native-refused",
                failure="native command refused or failed",
            )
    except BaseException as error:
        abrupt = error if not isinstance(error, Exception) else None
        retain(
            events,
            error
            if isinstance(error, Exception)
            else PolicyFailure(f"{type(error).__name__}: {error}"),
            phase="member",
            path=workspace,
            origin="policy" if isinstance(error, PolicyFailure) else "unknown",
        )
        for phase in ("generation", "compilation"):
            if row[phase] == "attempted":
                row[phase] = "failed"
    for phase in ("run", "problem", "original", "native", "compiler"):
        try:
            if phase == "run":
                inputs.verify(events=events)
            elif phase == "problem":
                provenance.require_active_binding(
                    binding, run_dir, io_mode="exact", events=events
                )
            elif phase == "original" and row["reader"] != "unattempted":
                _original(binding, run_dir, member, events)
            elif phase == "native":
                if observed is not None:
                    events.extend(
                        invocation.verify_native(
                            command,
                            original_input,
                            observed,
                            source_form=source_form,
                        )
                    )
                else:
                    calltargets.verify_native_contexts(contexts)
            elif phase == "compiler" and syntax is not None:
                events.extend(
                    _compiler_guards(
                        syntax,
                        row["profile_attempts"],
                        workspace=workspace,
                        output_limit=output_limit,
                        contexts=profile_contexts,
                        receipts=receipts,
                    )
                )
        except Exception as error:
            retain(
                events,
                error,
                phase=phase,
                path=workspace,
                origin="policy" if isinstance(error, PolicyFailure) else "unknown",
            )
    for path, receipt in (
        (workspace / "build/result.json", compiled),
        (workspace / "result.json", row),
    ):
        events.extend(recipe.verify_receipts(retained_receipts))
        pending = primary(events)
        row["secondary_failures"] = facts(events)
        if compiled is not None:
            compiled["terminal_failures"] = facts(events)
        if pending is not None:
            details = dict(
                failure=f"{type(pending).__name__}: {pending}",
                capture_failures=getattr(pending, "capture_failures", ()),
            )
            if isinstance(pending, ProcessCleanupError):
                details["owner_observation"] = pending.observed
                row["generation"] = "cleanup-uncertain"
            row.update(details)
            if compiled is not None:
                compiled.update(details)
        if receipt is not None:
            try:
                write_bytes(
                    path,
                    encode_json(receipt, events=events, path=path),
                    events=events,
                )
            except Exception as error:
                retain(events, error, phase="receipt", path=path)
    row["secondary_failures"] = facts(events)
    run_events.extend(events)
    pending = primary(events)
    if pending is not None:
        row["failure"] = f"{type(pending).__name__}: {pending}"
        row["capture_failures"] = getattr(pending, "capture_failures", ())
        if isinstance(pending, ProcessCleanupError):
            row["owner_observation"] = pending.observed
        if compiled is not None:
            compiled.update(failure=row["failure"], terminal_failures=facts(events))
        raise abrupt or pending
    if abrupt is not None:
        raise abrupt
    return row


def run(
    *,
    root: Path,
    run_dir: Path,
    destination: Path,
    member_keys: tuple[str, ...],
    native_receipt: Path,
    configuration: Path,
    rizin: Path,
    all_members: bool = False,
    output_limit: int = 16 << 20,
    source_form: nativeoutput.SourceForm | None = None,
    overlay_mounts: tuple[str, ...] = (),
    entry_receipt: Path | None = None,
    entry_cpu_receipt: Path | None = None,
    search_forms: bool = False,
    resume_from: Path | None = None,
    resume_sha256: str | None = None,
    member_limit: int | None = None,
    campaign_limit: int | None = None,
    new_budget_id: str | None = None,
) -> dict:
    _require(
        (entry_receipt is None) == (entry_cpu_receipt is None)
        and (
            entry_receipt is None
            or (
                not search_forms
                and not all_members
                and len(member_keys) == 1
                and overlay_mounts
            )
        ),
        "entry observation requires both receipts, one reviewed root and explicit original mounts",
    )
    _require(
        type(overlay_mounts) is tuple
        and len(overlay_mounts) <= 128
        and all(type(image) is str and image for image in overlay_mounts)
        and len(set(overlay_mounts)) == len(overlay_mounts),
        "overlay mounts must be distinct original image IDs in order",
    )
    _require(
        not (overlay_mounts and search_forms),
        "overlay mounts require a single selected source form run",
    )
    _require(
        type(all_members) is bool and not (all_members and member_keys),
        "all-member selection and explicit member keys are mutually exclusive",
    )
    _require(
        type(search_forms) is bool and not (search_forms and source_form is not None),
        "source search and an explicit source form are mutually exclusive",
    )
    _require(
        source_form is None or type(source_form) is nativeoutput.SourceForm,
        "source_form must be a validated native source form",
    )
    root, run_dir, destination = (
        Path(path).resolve() for path in (root, run_dir, destination)
    )
    native_receipt, configuration, rizin = (
        Path(path).resolve() for path in (native_receipt, configuration, rizin)
    )
    _require(
        search_forms
        or all(
            value is None
            for value in (
                resume_from,
                resume_sha256,
                member_limit,
                campaign_limit,
                new_budget_id,
            )
        ),
        "resume and campaign allowances require original-only source search",
    )
    _require(
        type(output_limit) is int and 0 < output_limit <= ARTIFACT_LIMIT,
        "raw process output limit must be at most 64 MiB",
    )
    _require(
        (all_members or 1 <= len(member_keys) <= MEMBER_COUNT)
        and len(set(member_keys)) == len(member_keys),
        f"one through {MEMBER_COUNT} distinct member keys required",
    )
    run_events = []
    binding = provenance.open_problem_binding(
        run_dir, io_mode="exact", events=run_events
    )
    _require(
        len(binding.members) == MEMBER_COUNT,
        "prepared Problem must contain all 118 members",
    )
    if all_members:
        member_keys = tuple(member.key for member in binding.members)
    selected_members = tuple(binding.member(key) for key in member_keys)
    _require(
        set(overlay_mounts) <= {image.image_id for image in binding.images},
        "overlay mount image is outside the permitted Problem",
    )
    if search_forms:
        return _search_forms(
            binding=binding,
            arguments={
                "root": root,
                "run_dir": run_dir,
                "destination": destination,
                "member_keys": member_keys,
                "native_receipt": native_receipt,
                "configuration": configuration,
                "rizin": rizin,
                "output_limit": output_limit,
                "resume_from": resume_from,
                "resume_sha256": resume_sha256,
                "member_limit": member_limit,
                "campaign_limit": campaign_limit,
                "new_budget_id": new_budget_id,
            },
        )
    destination.mkdir(mode=0o700)
    report = {
        "operation": "run",
        "status": "failed",
        "exit": 1,
        "denominator": MEMBER_COUNT,
        "run_id": binding.run_id,
        "binding_digest": binding.digest,
        "destination": str(destination),
        "selected": list(member_keys),
        "members": [
            {
                "identity": asdict(member),
                **dict.fromkeys(("reader", "generation", "compilation"), "unattempted"),
            }
            for member in binding.members
        ],
        **dict.fromkeys(("semantic_admission", "autonomous_exact"), False),
        "original_byte_comparison": "unperformed",
        "associated_data": "unverified",
        "placement": "unverified",
        "failures": [],
    }
    if source_form is not None:
        report["source_form"] = source_form.value
    rows = {row["identity"]["key"]: row for row in report["members"]}
    run_error, abrupt = None, None
    retained_receipts = []
    entry_sources = ()
    try:
        run_record, config_bytes, inputs = _prepare_run(
            root, destination, native_receipt, configuration, rizin, report, run_events
        )
        contexts = calltargets.prepare_native_contexts(
            binding, run_dir, destination / "binding-inputs"
        )
        run_record["binding_inputs"] = {
            "directory": str(contexts.input_directory),
            "files": PreparedInputs.tree(contexts.input_directory),
        }
        run_record.update(
            problem={
                "run_dir": str(run_dir),
                "binding_digest": binding.digest,
                "problem_sha256": binding.problem_doc_sha256,
            },
            limits={
                "process_output_bytes": output_limit,
                "artifact_bytes": ARTIFACT_LIMIT,
            },
            selected=list(member_keys),
        )
        if source_form is not None:
            run_record["source_form"] = source_form.value
        if overlay_mounts:
            run_record["overlay_mounts"] = list(overlay_mounts)
        if entry_receipt is not None:
            observation, entry_sources = invocation.capture_entry_observation(
                entry_receipt, entry_cpu_receipt, contexts, overlay_mounts
            )
            directory = destination / "entry-observation"
            directory.mkdir(mode=0o700)
            for name, raw in observation.files.items():
                write_bytes(
                    directory / f"entry-{name}", raw, readonly=True, events=run_events
                )
            run_record["entry_observation"] = observation.descriptor
            run_record["entry_observation_inputs"] = {
                "directory": str(directory),
                "files": PreparedInputs.tree(directory),
            }
        plugin = Path(run_record["binaries"]["plugin"]["path"])
        write_bytes(
            destination / "run.json",
            encode_json(run_record, events=run_events, path="driver"),
            events=run_events,
        )
        inputs.verify(events=run_events)
        self_check = invocation.capture_native(
            _native_command(destination / "self-check", rizin, plugin, output_limit),
            configuration=config_bytes,
            original=None,
        )
        report["self_check"] = self_check.evidence
        run_events.extend(self_check.events)
        if self_check.pending is not None:
            raise self_check.pending
        _require(
            self_check.launch == "observed"
            and self_check.status == "self-check-complete"
            and self_check.raw_exit == 0
            and self_check.process_failure is None,
            "native self-check failed",
        )
        for index, member in enumerate(selected_members):
            _run_member(
                binding=binding,
                run_dir=run_dir,
                member=member,
                row=rows[member.key],
                workspace=destination / f"member-{index:03d}",
                run_record=run_record,
                inputs=inputs,
                contexts=contexts,
                rizin=rizin,
                plugin=plugin,
                output_limit=output_limit,
                config_bytes=config_bytes,
                source_form=source_form,
                run_events=run_events,
                retained_receipts=retained_receipts,
            )
        provenance.require_active_binding(
            binding, run_dir, io_mode="exact", events=run_events
        )
        calltargets.verify_native_contexts(contexts)
        for target in binding.targets:
            provenance.read_original_image(
                binding,
                run_dir,
                target=target.target,
                image_id=target.image_id,
                io_mode="exact",
                events=run_events,
            )
        inputs.verify(events=run_events)
        recipe.verify_inputs(entry_sources, events=run_events)
        report["snapshots_unchanged"] = True
        attempted = [rows[key] for key in member_keys]
        report["exit"] = (
            0
            if all(
                row["compilation"] == "compiled-unverified"
                and not row.get("build", {}).get("entry_failure")
                for row in attempted
            )
            else 1
        )
        report["status"] = "measured-unverified" if report["exit"] == 0 else "failed"
    except BaseException as error:
        abrupt = error if not isinstance(error, Exception) else None
        retain(
            run_events,
            error
            if isinstance(error, Exception)
            else PolicyFailure(f"{type(error).__name__}: {error}"),
            phase="run",
            path=destination,
            origin="policy" if isinstance(error, PolicyFailure) else "unknown",
        )
        run_error = primary(run_events)
        error = run_error
        report["capture_failures"] = getattr(error, "capture_failures", ())
        report["failures"].append(f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            report.update(
                status="cleanup-uncertain",
                owner_observation=error.observed,
                capture_failures=error.capture_failures,
            )
    for row in report["members"]:
        try:
            attempts = row.get("profile_attempts", [])
            if type(attempts) is not list:
                raise PolicyFailure("member compiler inventory differs")
            for attempt in attempts:
                try:
                    if attempt["status"] == "native-unattempted":
                        attempt.update(
                            status="native-refused",
                            failure=row.get("failure")
                            or "; ".join(report["failures"])
                            or row["generation"],
                        )
                except Exception as error:
                    retain(
                        run_events, error, phase="attempt-inventory", path=destination
                    )
        except Exception as error:
            retain(
                run_events,
                error,
                phase="member-inventory",
                path=destination,
                origin="policy" if isinstance(error, PolicyFailure) else "unknown",
            )
    run_events.extend(recipe.verify_receipts(retained_receipts))
    run_error = primary(run_events)
    if run_error is not None:
        report.update(
            status="failed",
            exit=1,
            capture_failures=getattr(run_error, "capture_failures", ()),
        )
        if isinstance(run_error, ProcessCleanupError):
            report.update(
                status="cleanup-uncertain", owner_observation=run_error.observed
            )
    report["secondary_failures"] = facts(run_events)
    path = destination / "report.json"
    try:
        write_bytes(
            path, encode_json(report, events=run_events, path=path), events=run_events
        )
    except Exception as error:
        retain(run_events, error, phase="report-receipt", path=path)
        raise primary(run_events)
    if abrupt is not None:
        raise abrupt
    if isinstance(run_error, ProcessCleanupError):
        raise run_error
    return report
