"""Reproduce owner facts and reduce coherent fixed-corpus evidence chains."""

from __future__ import annotations

from copy import deepcopy
from dataclasses import asdict, dataclass
import hashlib
from pathlib import Path

from harness.build import recipe

from harness.common.artifacts import (
    FileIdentity,
    encode_json,
    using_reader,
    write_bytes,
)
from harness.common.failures import FailureEvent, PolicyFailure, facts, retain
from harness.io import repo_layout
from harness.reconstruction import (
    authored,
    baseline,
    bundles,
    comparison,
    contract,
    prepare,
    provenance,
)

MODES = ("autonomous", "signature_only", "metadata_assisted")
SEMANTIC = (
    "current-provenance",
    "mode",
    "generation",
    "unchanged-c89-compilation",
    "linkage",
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
    "terminal-owner",
)
EXACT = (*SEMANTIC, "complete-function-bytes")


@dataclass(frozen=True)
class Limits:
    input_read_limit: int
    input_open_limit: int
    artifact_limit: int
    output_limit: int


@dataclass(frozen=True)
class SelectedMembers:
    keys: tuple[str, ...]


@dataclass(frozen=True)
class AllMembers:
    pass


@dataclass(frozen=True)
class VerificationRequest:
    baseline: Path
    prepared_run: Path
    generation_directories: tuple[Path, ...]
    selection: AllMembers | SelectedMembers
    destination: Path
    limits: Limits
    required_mode: str


@dataclass(frozen=True)
class ReportRequest:
    baseline: Path
    prepared_run: Path
    verification_directories: tuple[Path, ...]
    destination: Path
    limits: Limits
    required_mode: str


@dataclass(frozen=True)
class FixedCorpus:
    members: tuple[provenance.BoundMember, ...]
    exclusions: tuple[dict, ...]
    baseline_sha256: str
    member_set_sha256: str
    exclusion_set_sha256: str


@dataclass(frozen=True)
class _Predicate:
    name: str
    state: str
    reason: str


@dataclass(frozen=True)
class ProfileEvidence:
    profile_id: str
    ordinal: int
    identity: str
    predicates: tuple[_Predicate, ...]
    facts: dict


@dataclass(frozen=True)
class CandidateEvidence:
    member_key: str
    identity: str
    generation_run: FileIdentity | None
    mode: str | None
    predicates: tuple[_Predicate, ...]
    profiles: tuple[ProfileEvidence, ...]
    failures: tuple[dict, ...]
    stages: dict


@dataclass(frozen=True)
class Outcome:
    exit_code: int
    document: dict
    published_artifacts: tuple[FileIdentity, ...]
    failures: tuple[FailureEvent, ...]


def _require(condition, message):
    if not condition:
        raise PolicyFailure(message)


def _set_digest(value):
    return hashlib.sha256(encode_json(value, path="fixed membership")).hexdigest()


def _fixed_corpus(root, baseline_path, prepared_run):
    pinned = baseline.load_baseline(root, baseline_path)
    original = baseline.member_facts(root, pinned.document)
    excluded = authored.exclusions(root, pinned.document, original)
    binding = provenance.open_problem_binding(prepared_run, io_mode="exact")
    expected = {row.key: row for row in original}
    _require(
        len(expected) == 118
        and len(binding.members) == 118
        and {row.key for row in binding.members} == set(expected),
        "active Problem exact member identity set differs from pinned baseline",
    )
    _require(
        {row.target for row in binding.targets} == {row.target for row in original}
        and {row.file_sha256 for row in binding.images}
        == {row.image.sha256 for row in original},
        "active Problem exact target/image identity sets differ",
    )
    for member in binding.members:
        fact = expected[member.key]
        image = binding.image(member.image_id)
        _require(
            (
                member.case_id,
                member.target,
                member.address,
                member.start,
                member.end,
                member.range_sha256,
            )
            == (
                fact.case_id,
                fact.target,
                fact.address,
                fact.address,
                fact.end,
                fact.range_sha256,
            ),
            "active Problem member extent/range/identity differs",
        )
        _require(
            (
                image.file_sha256,
                image.sha256,
                image.bytes,
                image.load_address,
                image.payload_offset,
                image.payload_size,
                image.format,
            )
            == (
                fact.image.sha256,
                fact.image.sha256,
                len(fact.image.content),
                fact.image.load_address,
                fact.image.payload_offset,
                fact.image.payload_size,
                fact.image.format,
            ),
            "active Problem complete image identity differs",
        )
    answer_index = bundles.verify_bundle_references(
        prepared_run, "answer", io_mode="exact", max_entries=65536
    )
    answer = bundles.resolve_document(
        prepared_run,
        contract.doc_ref("answer", answer_index["documents"][0]["digest"], None),
        owner="answer",
        io_mode="exact",
    )
    _require(
        set(answer["members"]) == set(expected),
        "active Answer exact member identity set differs",
    )
    exclusion_records = tuple({**asdict(row), "evidence": None} for row in excluded)
    actual_exclusions = answer["exclusions"]
    _require(
        len(exclusion_records) == len(actual_exclusions) == 7
        and len({row["key"] for row in actual_exclusions}) == 7
        and sorted(actual_exclusions, key=lambda row: row["case_id"])
        == sorted(exclusion_records, key=lambda row: row["case_id"]),
        "active Answer exact seven exclusion identities/reasons differ",
    )
    for member in binding.members:
        obligations = answer["members"][member.key]["obligations"]
        _require(
            obligations["function"]["extent"]
            == {"start": member.start, "end": member.end},
            "Answer original function extent differs",
        )
    members = tuple(binding.members)
    return FixedCorpus(
        members,
        exclusion_records,
        pinned.sha256,
        _set_digest([asdict(row) for row in sorted(members, key=lambda row: row.key)]),
        _set_digest(sorted(exclusion_records, key=lambda row: row["case_id"])),
    )


def _adapt_candidate(shared, inspection):
    def adapt(rows):
        return tuple(
            _Predicate(
                row.name,
                "supported" if row.status == "matched" else row.status,
                row.reason,
            )
            for row in rows
        )

    profiles = []
    for profile in inspection.profiles:
        failure_records = contract.parse_json(
            encode_json(facts(profile.failures), path=inspection.identity),
            "profile failure facts",
        )
        retained = dict(raw=profile.raw, failures=failure_records)
        if profile.source is not None:
            retained["source"] = asdict(profile.source)
        if profile.representation is not None:
            retained["representation"] = asdict(profile.representation)
        if profile.associated is not None:
            retained["associated"] = asdict(profile.associated)
        profiles.append(
            ProfileEvidence(
                profile.profile_id,
                profile.ordinal,
                profile.profile_identity,
                adapt(profile.predicates),
                retained,
            )
        )
    return CandidateEvidence(
        inspection.member.key,
        inspection.identity,
        inspection.generation_run,
        inspection.authenticated_mode,
        adapt(inspection.predicates),
        tuple(profiles),
        tuple(
            contract.parse_json(
                encode_json(facts(inspection.failures), path=inspection.identity),
                "candidate failure facts",
            )
        ),
        {
            key: inspection.raw[key]
            for key in (
                "native",
                "validated_contract",
                "runtime_capture",
                "source_attempt",
                "supplied",
            )
            if key in inspection.raw
        },
    )


def _conjunction(states):
    states = tuple(states)
    if "failed" in states:
        return "failed"
    if "missing" in states:
        return "missing"
    if "unknown" in states:
        return "unknown"
    return (
        "supported"
        if states and all(value == "supported" for value in states)
        else "missing"
    )


def _alternatives(states):
    states = tuple(states)
    if "supported" in states:
        return "supported"
    if "unknown" in states:
        return "unknown"
    if "missing" in states or not states:
        return "missing"
    return "failed"


def reduce_member(member, candidates):
    views = {}
    for mode in MODES:
        attempts = []
        for candidate in sorted(candidates, key=lambda row: row.identity):
            if candidate.member_key != member.key or candidate.mode != mode:
                continue
            profiles = candidate.profiles or (
                ProfileEvidence("unattempted", -1, "unattempted", (), {}),
            )
            for profile in sorted(profiles, key=lambda row: row.ordinal):
                predicates = (*candidate.predicates, *profile.predicates)
                by_name = {}
                for row in predicates:
                    by_name.setdefault(row.name, []).append(row.state)
                values = {
                    name: _conjunction(by_name.get(name, ("missing",)))
                    for name in EXACT
                }
                if candidate.failures:
                    values["terminal-owner"] = "failed"
                semantic = _conjunction(values[name] for name in SEMANTIC)
                exact = _conjunction(values[name] for name in EXACT)
                attempts.append(
                    dict(
                        candidate_identity=candidate.identity,
                        profile_id=profile.profile_id,
                        ordinal=profile.ordinal,
                        profile_identity=profile.identity,
                        semantic=semantic,
                        exact=exact,
                        blockers=[
                            name for name in EXACT if values[name] != "supported"
                        ],
                        predicates=[asdict(row) for row in predicates],
                        evidence=profile.facts,
                        stages=candidate.stages,
                    )
                )
        semantic, exact = (
            _alternatives(row["semantic"] for row in attempts),
            _alternatives(row["exact"] for row in attempts),
        )
        selected = next((row for row in attempts if row["exact"] == "supported"), None)
        if selected is None:
            selected = next(
                (row for row in attempts if row["semantic"] == "supported"), None
            )
        views[mode] = dict(
            semantic=semantic,
            exact=exact,
            selected=None
            if selected is None
            else {
                key: selected[key]
                for key in (
                    "candidate_identity",
                    "profile_id",
                    "ordinal",
                    "profile_identity",
                )
            },
            attempts=attempts,
        )
    unknown = [
        dict(
            identity=row.identity,
            generation_run=None
            if row.generation_run is None
            else row.generation_run.as_record(),
            predicates=[asdict(value) for value in row.predicates],
            failures=deepcopy(list(row.failures)),
            profiles=[
                dict(
                    profile_id=profile.profile_id,
                    ordinal=profile.ordinal,
                    predicates=[asdict(value) for value in profile.predicates],
                    facts=deepcopy(profile.facts),
                )
                for profile in row.profiles
            ],
            stages=row.stages,
        )
        for row in candidates
        if row.mode is None
    ]
    return dict(identity=asdict(member), modes=views, unknown_mode_attempts=unknown)


def reduce_corpus(corpus, members, required_mode):
    _require(
        len(members) == 118
        and {row["identity"]["key"] for row in members}
        == {row.key for row in corpus.members},
        "reducer must retain the exact authenticated118 members",
    )
    return dict(
        semantic_count=sum(
            row["modes"][required_mode]["semantic"] == "supported" for row in members
        ),
        requested_exact_count=sum(
            row["modes"][required_mode]["exact"] == "supported" for row in members
        ),
        autonomous_exact_count=sum(
            row["modes"]["autonomous"]["exact"] == "supported" for row in members
        ),
    )


def _candidate_failure(member, path, generation, error, phase):
    from harness.reconstruction.obligations import PredicateResult

    events = []
    retain(events, error, phase=phase, path=path)
    return comparison.CandidateInspection(
        member,
        str(path),
        generation.run,
        None,
        (
            *generation.predicates,
            PredicateResult(
                "generation",
                "missing"
                if isinstance(error, FileNotFoundError)
                or not (path / "result.json").is_file()
                else "failed",
                f"{type(error).__name__}: {error}",
            ),
        ),
        (),
        {},
        tuple(events),
    )


def _verify_attempt(
    candidate, member, generation, shared, prepared_run, workspace, allowance, limits
):
    workspace.mkdir()
    result_record = comparison._bounded_file_record(
        candidate / "result.json",
        limits.artifact_limit,
        [],
        reader=allowance.read_bytes,
    )
    _require(generation.run is not None, "candidate has no authenticated canonical run")
    rebuilt, measured, associated = {}, {}, {}
    supplied = comparison.inspect_supplied_source(
        candidate, member, shared, generation, allowance, prepared_run
    )
    try:
        comparison.rebuild_candidate(
            result_record=result_record,
            run_record=generation.run.as_record(),
            workspace=workspace / "rebuild",
            output_limit=limits.output_limit,
            artifact_limit=limits.artifact_limit,
            retained=rebuilt,
        )
        rebuild_record = comparison._bounded_file_record(
            workspace / "rebuild/rebuild-result.json",
            limits.artifact_limit,
            [],
            reader=allowance.read_bytes,
        )
        comparison.compare_rebuilt_candidate(
            rebuild_record=rebuild_record,
            prepared_run=prepared_run,
            workspace=workspace / "comparison",
            output_limit=limits.output_limit,
            artifact_limit=limits.artifact_limit,
            retained=measured,
        )
        comparison.verify_associated_candidate(
            comparison_record=workspace / "comparison/comparison-result.json",
            prepared_run=prepared_run,
            workspace=workspace / "associated",
            output_limit=limits.output_limit,
            artifact_limit=limits.artifact_limit,
            input_read_limit=limits.input_read_limit,
            retained=associated,
            allowance=allowance,
        )
    except Exception as error:
        failed = _candidate_failure(
            member, candidate, generation, error, "independent-verification"
        )
        if supplied is not None:
            failed = comparison.CandidateInspection(
                member,
                str(candidate),
                generation.run,
                supplied.authenticated_mode,
                (
                    *shared.predicates,
                    *generation.predicates,
                    *supplied.predicates,
                    comparison.obligations.PredicateResult(
                        "terminal-owner", "failed", f"{type(error).__name__}: {error}"
                    ),
                ),
                (),
                {"supplied": supplied.raw},
                (*supplied.events, *failed.failures),
            )
        return failed, dict(
            candidate=str(candidate),
            member_key=member.key,
            generation_run=generation.run.as_record(),
            associated_record=None,
            raw=dict(rebuild=rebuilt, comparison=measured, associated=associated),
        )
    record = comparison._bounded_file_record(
        workspace / "associated/associated-result.json",
        limits.artifact_limit,
        [],
        reader=allowance.read_bytes,
    )
    inspection = comparison.inspect_candidate(
        candidate,
        workspace / "associated",
        shared,
        generation,
        member,
        allowance,
        prepared_run,
    )
    return inspection, dict(
        candidate=str(candidate),
        member_key=member.key,
        generation_run=generation.run.as_record(),
        associated_record=record,
    )


def _inputs(allowance):
    unique = {}
    for row in allowance.observations:
        _require(
            unique.setdefault(row["path"], row) == row,
            "physical identity drift between command snapshots",
        )
    return list(unique.values())


def _reduce(corpus, shared, inspections, mode):
    adapted = tuple(_adapt_candidate(shared, row) for row in inspections)
    members = [
        reduce_member(
            member, tuple(row for row in adapted if row.member_key == member.key)
        )
        for member in corpus.members
    ]
    return members, reduce_corpus(corpus, members, mode)


def _replay_accounts(accounts, shared, corpus, allowance, prepared_run):
    inspections = []
    for account in accounts:
        member = next(
            (row for row in corpus.members if row.key == account.member_key), None
        )
        _require(member is not None, "terminal account names a foreign member")
        generation = next(
            (row for row in shared.generations if row.run == account.generation_run),
            None,
        )
        _require(
            generation is not None,
            "terminal account has no exact generation provenance association",
        )
        inspections.append(
            comparison.inspect_candidate(
                account.candidate,
                account.associated.path.parent,
                shared,
                generation,
                member,
                allowance,
                prepared_run,
            )
        )
    return tuple(inspections)


def _retain_source_attempts(attempts, shared, corpus, allowance, prepared_run):
    members = {row.key: row for row in corpus.members}
    generations = {row.run: row for row in shared.generations if row.run is not None}
    return tuple(
        comparison.inspect_retained_attempt(
            row,
            members[row.member_key],
            shared,
            generations.get(row.generation_run),
            allowance,
            prepared_run,
        )
        for row in attempts
        if row.associated is None
    )


def _run(request, operation):
    events, published, inspections, attempts = [], [], [], []
    corpus = shared = allowance = None
    destination, prepared_run = request.destination, request.prepared_run
    document = dict(
        schema="retdec118.reporting/v1",
        operation=operation,
        status="refused",
        exit=2,
        prepared_run=str(prepared_run),
        destination=str(destination),
        denominator=118,
        required_mode=request.required_mode,
        publication="refused",
        members=[],
        exclusions=[],
        attempts=attempts,
    )
    owned = False
    try:
        limits = request.limits
        _require(
            request.required_mode in MODES,
            "reporting requires a supported mode",
        )
        _require(
            all(
                type(value) is int and value > 0
                for value in (
                    limits.input_read_limit,
                    limits.input_open_limit,
                    limits.artifact_limit,
                    limits.output_limit,
                )
            ),
            "reporting consumption/output limits must be positive integers",
        )
        allowance = comparison.ConsumptionAllowance(
            read_limit=limits.input_read_limit,
            open_limit=limits.input_open_limit,
            artifact_limit=limits.artifact_limit,
        )
        sources = (
            request.generation_directories
            if operation == "verify"
            else request.verification_directories
        )
        _require(
            bool(sources),
            "at least one explicit generation or verification directory required",
        )
        _require(
            destination.is_absolute()
            and destination.parent.resolve() == destination.parent
            and destination.parent.is_dir()
            and not destination.exists()
            and not destination.is_symlink(),
            "report destination must be canonical, absent and have an existing parent",
        )
        protected = (prepared_run, *sources)
        _require(
            not any(
                destination == path
                or destination.is_relative_to(path)
                or path.is_relative_to(destination)
                for path in protected
            ),
            "report destination overlaps immutable source evidence",
        )
        if operation == "verify":
            destination.mkdir()
            owned = True
        with (
            using_reader(allowance.read_bytes),
            contract.bounded_reads(limits.artifact_limit, reader=allowance.read_bytes),
        ):
            corpus = _fixed_corpus(repo_layout().root, request.baseline, prepared_run)
            document.update(
                members=[reduce_member(row, ()) for row in corpus.members],
                exclusions=list(corpus.exclusions),
                baseline_sha256=corpus.baseline_sha256,
                member_set_sha256=corpus.member_set_sha256,
                exclusion_set_sha256=corpus.exclusion_set_sha256,
                limits=asdict(limits),
            )
            accounts = ()
            discovery = None
            if operation == "verify":
                runs = prepare.discover_generation_runs(sources, allowance.read_bytes)
            else:
                discovery = comparison.discover_verification_accounts(
                    sources,
                    prepared_run,
                    allowance,
                    events,
                    members=corpus.members,
                    exclusions=corpus.exclusions,
                )
                accounts = discovery.accounts
                attempts.extend(
                    contract.parse_json(row.record, "retained source attempt")
                    for row in discovery.attempts
                )
                document["source_attempts"] = [
                    dict(
                        source=row.source.as_record(),
                        ordinal=row.ordinal,
                        record=contract.parse_json(
                            row.record, "retained source attempt"
                        ),
                        observations=contract.parse_json(
                            row.observations, "source attempt observations"
                        ),
                    )
                    for row in discovery.attempts
                ]
                runs = comparison.discover_report_generation_runs(
                    accounts, allowance, events, attempts=discovery.attempts
                )
            shared = prepare.inspect_shared_provenance(
                prepared_run, runs, allowance.read_bytes
            )
            if operation == "report":
                destination.mkdir()
                owned = True
            document["generations"] = [
                dict(
                    directory=str(row.directory),
                    run=None if row.run is None else row.run.as_record(),
                    problem_binding_digest=row.problem_binding_digest,
                    inputs=[value.as_record() for value in row.inputs],
                    predicates=[asdict(value) for value in row.predicates],
                    failures=facts(row.failures),
                )
                for row in shared.generations
            ]
            if operation == "verify":
                selected = (
                    tuple(row.key for row in corpus.members)
                    if isinstance(request.selection, AllMembers)
                    else request.selection.keys
                )
                _require(
                    1 <= len(selected) <= 118
                    and len(set(selected)) == len(selected)
                    and set(selected) <= {row.key for row in corpus.members},
                    "selection must contain distinct authenticated member keys",
                )
                (destination / "candidates").mkdir()
                for generation in shared.generations:
                    if generation.prepared is None:
                        continue
                    record = generation.prepared._record
                    for index, key in enumerate(record["selected"]):
                        if key not in selected:
                            continue
                        member = shared.binding.member(key)
                        candidate = generation.directory / f"member-{index:03d}"
                        try:
                            if generation.failures:
                                raise PolicyFailure(
                                    "this generation's current physical provenance was refused"
                                )
                            inspection, attempt = _verify_attempt(
                                candidate,
                                member,
                                generation,
                                shared,
                                prepared_run,
                                destination / "candidates" / f"{len(attempts):04d}",
                                allowance,
                                limits,
                            )
                        except Exception as error:
                            inspection = _candidate_failure(
                                member,
                                candidate,
                                generation,
                                error,
                                "candidate-attempt",
                            )
                            attempt = dict(
                                candidate=str(candidate),
                                member_key=key,
                                generation_run=None
                                if generation.run is None
                                else generation.run.as_record(),
                                associated_record=None,
                                failure=f"{type(error).__name__}: {error}",
                            )
                        inspections.append(inspection)
                        attempts.append(attempt)
                document["selected"] = list(selected)
                accounts = tuple(
                    comparison.VerificationAccount(
                        Path(row["candidate"]),
                        FileIdentity(
                            Path(row["associated_record"]["path"]),
                            row["associated_record"]["bytes"],
                            row["associated_record"]["sha256"],
                        ),
                        FileIdentity(
                            Path(row["generation_run"]["path"]),
                            row["generation_run"]["bytes"],
                            row["generation_run"]["sha256"],
                        ),
                        row["member_key"],
                    )
                    for row in attempts
                    if row.get("associated_record") is not None
                )
            else:
                inspections.extend(
                    _replay_accounts(accounts, shared, corpus, allowance, prepared_run)
                )
                inspections.extend(
                    _retain_source_attempts(
                        discovery.attempts, shared, corpus, allowance, prepared_run
                    )
                )
            members, summary = _reduce(
                corpus,
                shared,
                inspections,
                request.required_mode,
            )
            document.update(members=members, summary=summary, attempts=attempts)
            _require(
                not events
                and not shared.failures
                and not any(row.failures for row in shared.generations),
                "invalid terminal discovery or current generation provenance refuses publication",
            )
            _require(
                not allowance.failed_reserved_bytes,
                "failed physical read/close consumption refuses publication",
            )
            identities = _inputs(allowance)
            document.update(
                input_census=identities,
                failures=facts(events),
                publication="terminal",
                exit=0 if summary["requested_exact_count"] == 118 else 1,
                status="complete"
                if summary["requested_exact_count"] == 118
                else "incomplete",
            )
            name = (
                "verification-result.json"
                if operation == "verify"
                else "report-result.json"
            )
            terminal = destination / name
            pending = destination / name.replace(".json", ".pending.json")
            initial_requests = tuple(allowance.requests)
            output_bound = min(limits.output_limit, limits.artifact_limit)
            final_requests = (
                *initial_requests,
                *(
                    (row.directory / "run.json", min(limits.artifact_limit, 64 << 20))
                    for row in shared.generations
                ),
                *((Path(row["path"]), max(1, row["bytes"])) for row in identities),
                (pending, output_bound),
            )
            allowance.reserve_final_consumption(
                final_requests, identities, output=(pending, output_bound)
            )
            fresh_corpus = _fixed_corpus(
                repo_layout().root, request.baseline, prepared_run
            )
            _require(
                fresh_corpus == corpus,
                "fixed membership changed during final consumption",
            )
            if operation == "report":
                fresh_discovery = comparison.discover_verification_accounts(
                    sources,
                    prepared_run,
                    allowance,
                    events,
                    members=fresh_corpus.members,
                    exclusions=fresh_corpus.exclusions,
                )
                _require(
                    not events and fresh_discovery == discovery,
                    "terminal source envelopes or full retained attempt inventory changed during final consumption",
                )
            fresh_runs = prepare.discover_generation_runs(
                tuple(row.directory for row in shared.generations),
                allowance.read_bytes,
            )
            fresh_shared = prepare.inspect_shared_provenance(
                prepared_run, fresh_runs, allowance.read_bytes
            )
            _require(
                not fresh_shared.failures
                and not any(row.failures for row in fresh_shared.generations)
                and tuple(
                    (
                        row.directory,
                        row.run,
                        row.problem_binding_digest,
                        row.inputs,
                        row.predicates,
                    )
                    for row in fresh_shared.generations
                )
                == tuple(
                    (
                        row.directory,
                        row.run,
                        row.problem_binding_digest,
                        row.inputs,
                        row.predicates,
                    )
                    for row in shared.generations
                ),
                "generation provenance changed during final consumption",
            )
            fresh_inspections = _replay_accounts(
                accounts, fresh_shared, fresh_corpus, allowance, prepared_run
            )
            if operation == "report":
                fresh_inspections = (
                    *fresh_inspections,
                    *_retain_source_attempts(
                        fresh_discovery.attempts,
                        fresh_shared,
                        fresh_corpus,
                        allowance,
                        prepared_run,
                    ),
                )
            fresh_adapted = tuple(
                _adapt_candidate(fresh_shared, row) for row in fresh_inspections
            )
            expected_adapted = tuple(
                _adapt_candidate(shared, row)
                for row in inspections
                if operation == "report"
                or any(
                    row.identity == str(account.candidate)
                    and row.generation_run == account.generation_run
                    for account in accounts
                )
            )
            _require(
                fresh_adapted == expected_adapted,
                "member/profile evidence changed during final consuming replay",
            )
            if operation == "verify":
                for previous in inspections:
                    if "supplied" not in previous.raw or any(
                        previous.identity == str(account.candidate)
                        for account in accounts
                    ):
                        continue
                    generation = next(
                        row
                        for row in fresh_shared.generations
                        if row.run == previous.generation_run
                    )
                    fresh = comparison.inspect_supplied_source(
                        Path(previous.identity),
                        previous.member,
                        fresh_shared,
                        generation,
                        allowance,
                        prepared_run,
                    )
                    _require(
                        fresh is not None
                        and not fresh.events
                        and fresh.authenticated_mode == previous.authenticated_mode
                        and fresh.raw == previous.raw["supplied"],
                        "failed supplied attempt changed during final consuming replay",
                    )
            for row in identities:
                recipe.verify_inputs([row], events=events, reader=allowance.read_bytes)
            consumed = allowance.as_record()
            output_bytes = 0
            for _ in range(16):
                document["consumption"] = {
                    **consumed,
                    "charged_bytes": consumed["charged_bytes"] + output_bytes,
                    "attempted_opens": consumed["attempted_opens"] + 1,
                    "reserved_final_bytes": consumed["reserved_final_bytes"]
                    - output_bytes
                    - 1,
                    "reserved_final_opens": consumed["reserved_final_opens"] - 1,
                }
                data = encode_json(document, path=terminal, events=events)
                _require(
                    len(data) <= output_bound,
                    "complete fixed-denominator account exceeds output/artifact allowance",
                )
                if len(data) == output_bytes:
                    break
                output_bytes = len(data)
            else:
                raise PolicyFailure("terminal consumption encoding did not stabilize")
            pending_identity = FileIdentity(
                pending, len(data), hashlib.sha256(data).hexdigest()
            )
            allowance.bind_final_output(pending_identity)
            document["attempted_output"] = pending_identity.as_record()
            write_bytes(pending, data, create_parents=False, events=events)
            _require(
                allowance.read_bytes(pending, len(data), events) == data,
                "pending report bytes changed before publication",
            )
            _require(
                not events and not allowance.failed_reserved_bytes,
                "terminal publication has retained consumption failures",
            )
            _require(
                document["consumption"] == allowance.as_record(),
                "terminal publication consumption differs from its account",
            )
            pending.rename(terminal)
            published.append(FileIdentity(terminal, len(data), pending_identity.sha256))
            document.pop("attempted_output", None)
            return Outcome(document["exit"], document, tuple(published), tuple(events))
    except (Exception, KeyboardInterrupt) as error:
        retain(
            events,
            error,
            phase=f"{operation}-reporting",
            path=destination,
            origin="policy"
            if isinstance(error, (PolicyFailure, recipe.ImmutableInputDrift))
            else "unknown",
        )
        if corpus is not None:
            if shared is not None:
                members, summary = _reduce(
                    corpus,
                    shared,
                    inspections,
                    request.required_mode,
                )
                document.update(members=members, summary=summary)
            document["exclusions"] = list(corpus.exclusions)
        document.update(
            status="refused",
            exit=2,
            publication="refused",
            failures=facts(events),
            consumption=None if allowance is None else allowance.as_record(),
        )
        if owned:
            try:
                refusal = destination / "refusal.json"
                raw = encode_json(document, path=refusal, events=events)
                _require(
                    len(raw) <= request.limits.output_limit,
                    "retained refusal exceeds output allowance",
                )
                write_bytes(refusal, raw, create_parents=False, events=events)
                published.append(
                    FileIdentity(refusal, len(raw), hashlib.sha256(raw).hexdigest())
                )
            except Exception as publication_error:
                retain(
                    events,
                    publication_error,
                    phase="refusal-publication",
                    path=destination,
                )
                document["failures"] = facts(events)
        return Outcome(2, document, tuple(published), tuple(events))


def verify(request: VerificationRequest) -> Outcome:
    return _run(request, "verify")


def report(request: ReportRequest) -> Outcome:
    return _run(request, "report")
