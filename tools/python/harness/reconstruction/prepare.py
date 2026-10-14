"""Prepare and authenticate complete reconstruction inputs and bundle associations."""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import asdict, dataclass
from pathlib import Path
from types import MappingProxyType

from harness.build import recipe
from harness.common.artifacts import (
    active_reader,
    ConsumedFile,
    FileIdentity,
    read_bytes,
)
from harness.common.failures import FailureEvent, PolicyFailure, primary, retain
from harness.common.digests import file_record
from harness.io import file_sha256
from harness.reconstruction import bundles, catalog, contract
from harness.toolchain.nativebuild import git_commit, read_native_build

MODULE = "harness.reconstruction.prepare"


def _require(condition: object, message: str) -> None:
    if not condition:
        raise PolicyFailure(message)


@dataclass(frozen=True)
class PreparedInputs:
    _record: Mapping
    _destination: Path

    FIELDS = frozenset(
        "snapshots fork_commits native_build_inputs tools binaries binding_inputs".split()
    )

    @classmethod
    def attach(
        cls, record: Mapping, *, run_directory: Path, events=None
    ) -> PreparedInputs:
        pointer = record.get("prepared_inputs")
        if pointer is None:
            for snapshot in record["snapshots"]:
                if "copy" in snapshot:
                    _require(
                        Path(snapshot["copy"])
                        .resolve()
                        .is_relative_to(run_directory / "snapshot"),
                        "generation snapshot copy escapes its own run",
                    )
            if "binding_inputs" in record:
                _require(
                    Path(record["binding_inputs"]["directory"])
                    == run_directory / "binding-inputs",
                    "generation catalog belongs to another run",
                )
            if "entry_observation_inputs" in record:
                _require(
                    Path(record["entry_observation_inputs"]["directory"])
                    == run_directory / "entry-observation",
                    "entry observation belongs to another run",
                )
            return cls(MappingProxyType(record), run_directory)
        prepared = cls.open_owner(pointer, events=events)
        _require(
            run_directory.parent == prepared._destination.parent / "attempts"
            and record.get("campaign") == prepared._record["campaign"]
            and record.get("problem") == prepared._record["problem"]
            and all(record.get(name) == prepared._record[name] for name in cls.FIELDS),
            "attempt differs from its direct authenticated preparation owner",
        )
        return prepared

    @classmethod
    def open_owner(cls, pointer, *, events=None) -> PreparedInputs:
        from harness.reconstruction.nativeoutput import parse_json

        _require(
            type(pointer) is dict and set(pointer) == {"path", "bytes", "sha256"},
            "shared preparation pin differs",
        )
        owner = Path(pointer["path"])
        _require(
            owner.is_absolute()
            and owner.resolve() == owner
            and not owner.is_symlink()
            and owner.name == "inputs.json"
            and owner.parent.name == "prepared",
            "shared preparation must identify a direct canonical owner",
        )
        recipe.verify_inputs([pointer], events=events)
        content = (active_reader() or read_bytes)(owner, 64 << 20, events)
        prepared = parse_json(content, 64 << 20)
        _require(
            type(prepared) is dict
            and set(prepared) == cls.FIELDS | {"problem", "campaign"},
            "shared preparation owner has a foreign structure or pointer chain",
        )
        association = prepared["campaign"]
        _require(
            type(association) is dict and set(association) == {"id", "owner_run"},
            "shared preparation campaign association differs",
        )
        campaign_root = owner.parent.parent
        campaign_pin = association["owner_run"]
        _require(
            Path(campaign_pin["path"]) == campaign_root / "run.json"
            and owner == campaign_root / "prepared/inputs.json",
            "shared preparation escapes its original campaign",
        )
        recipe.verify_inputs([campaign_pin], events=events)
        campaign = parse_json(
            (active_reader() or read_bytes)(
                Path(campaign_pin["path"]), 64 << 20, events
            ),
            64 << 20,
        )
        _require(
            type(campaign) is dict
            and set(campaign)
            == set(
                "search_version campaign selected source_forms requested problem resumed_from".split()
            )
            and campaign.get("search_version") == "member-first-v2"
            and campaign.get("campaign") == association["id"]
            and campaign.get("problem") == prepared["problem"]
            and campaign.get("requested", {}).get("problem") == prepared["problem"]
            and all(
                campaign["requested"][name] == prepared["binaries"][field]
                for name, field in (
                    ("native_receipt", "receipt"),
                    ("configuration", "configuration"),
                    ("rizin", "rizin"),
                )
            ),
            "shared preparation differs from its original campaign owner",
        )
        roots = [
            *(Path(row["copy"]) for row in prepared["snapshots"] if "copy" in row),
            *(
                Path(row["path"])
                for row in prepared["native_build_inputs"].get("copies", [])
            ),
        ]
        if "receipt_copy" in prepared["binaries"]:
            roots += [
                Path(prepared["binaries"][name]["path"])
                for name in ("plugin", "receipt_copy")
            ]
        _require(
            all(
                path.resolve() == path
                and path.is_relative_to(owner.parent / "snapshot")
                for path in roots
            )
            and Path(prepared["binding_inputs"]["directory"])
            == owner.parent / "binding-inputs",
            "shared copied inputs escape the preparation owner",
        )
        catalog.verify(prepared["tools"], events=events)
        recipe.verify_inputs([pointer, campaign_pin], events=events)
        return cls(MappingProxyType(prepared), owner.parent)

    @staticmethod
    def tree(directory: Path, *, python_only: bool = False) -> list:
        return [
            {**file_record(path), "path": str(path.relative_to(directory))}
            for path in sorted(directory.rglob("*.py" if python_only else "*"))
            if path.is_file() and not {".git", "__pycache__"}.intersection(path.parts)
        ]

    @classmethod
    def snapshot(
        cls,
        source: Path,
        name: str,
        *,
        python_only: bool = False,
        files=None,
    ) -> dict:
        manifest = (
            cls.tree(source, python_only=python_only)
            if files is None
            else sorted(files, key=lambda row: Path(row["path"]))
        )
        _require(bool(manifest), "selected source/build snapshot is empty")
        return {
            "name": name,
            "source": str(source),
            "files": manifest,
            "python_only": python_only,
        }

    @staticmethod
    def snapshot_records(record: Mapping) -> dict:
        return {
            row["name"] if "name" in row else Path(row["copy"]).name: row
            for row in record["snapshots"]
        }

    @staticmethod
    def snapshot_roots(snapshot: Mapping) -> tuple[Path, ...]:
        return tuple(
            dict.fromkeys(
                Path(snapshot[name]) for name in ("source", "copy") if name in snapshot
            )
        )

    @classmethod
    def prepare(
        cls,
        record: dict,
        *,
        destination: Path,
        native_receipt: Path,
        configuration: Path,
        rizin: Path,
        selection: Mapping,
        driver_root: Path,
    ) -> PreparedInputs:
        receipt_identity = file_record(native_receipt)
        receipt = read_native_build(native_receipt)
        sources = [
            (receipt.plugin.root, "plugin-source", receipt.plugin.after),
            (receipt.retdec.root, "retdec-source", receipt.retdec.after),
            (receipt.build_root, "native-build", receipt.build_files),
        ]
        snapshots = [
            cls.snapshot(source, name, files=manifest)
            for source, name, manifest in sources
        ]
        identities = [
            *receipt.tools,
            *receipt.external_inputs,
            *receipt.compiler_inputs,
            *receipt.link_inputs,
        ]
        identities += [prerequisite.archive for prerequisite in receipt.prerequisites]
        for execution in (receipt.configure, receipt.build, receipt.probe):
            identities += [execution.log, *execution.streams]
        input_identities = {}
        for identity in identities:
            _require(
                input_identities.setdefault(identity.path, identity) == identity,
                "native input identities disagree",
            )
        inputs = [
            {**asdict(identity), "path": str(identity.path)}
            for _, identity in sorted(input_identities.items())
        ]
        snapshots.append(
            cls.snapshot(
                driver_root,
                "driver",
                python_only=True,
            )
        )
        record.update(
            {
                "snapshots": snapshots,
                "fork_commits": [
                    {"root": str(source.root), "commit": source.commit}
                    for source in (receipt.plugin, receipt.retdec)
                ],
                "native_build_inputs": {"files": inputs},
                "tools": selection,
                "binaries": {
                    "rizin": file_record(rizin),
                    "plugin": asdict(receipt.dso) | {"path": str(receipt.dso.path)},
                    "configuration": file_record(configuration),
                    "receipt": receipt_identity,
                },
            }
        )
        return cls(MappingProxyType(record), destination)

    def verify(self, *, events: list[FailureEvent] | None = None) -> None:
        local = []
        for phase in (
            "tools",
            "native",
            "bindings",
            "forks",
            "snapshots",
            "producer",
        ):
            try:
                if phase == "tools":
                    catalog.verify(
                        self._record["tools"], events=local, reader=active_reader()
                    )
                elif phase == "native":
                    native = self._record["native_build_inputs"]
                    for group in (*native, "binaries"):
                        try:
                            records = (
                                self._record["binaries"].values()
                                if group == "binaries"
                                else native[group]
                            )
                            for record in records:
                                try:
                                    recipe.verify_inputs([record], events=local)
                                except Exception as error:
                                    retain(
                                        local,
                                        error,
                                        phase=group,
                                        path=self._destination,
                                        origin="policy"
                                        if isinstance(error, recipe.ImmutableInputDrift)
                                        else "unknown",
                                    )
                        except Exception as error:
                            retain(local, error, phase=group, path=self._destination)
                elif phase == "bindings" and "binding_inputs" in self._record:
                    inputs = self._record["binding_inputs"]
                    directory = Path(inputs["directory"])
                    observed = self.tree(directory)
                    _require(observed == inputs["files"], "catalog input drift")
                    if "entry_observation_inputs" in self._record:
                        entry = self._record["entry_observation_inputs"]
                        _require(
                            self.tree(Path(entry["directory"])) == entry["files"],
                            "entry observation input drift",
                        )
                elif phase == "forks":
                    for source in self._record["fork_commits"]:
                        try:
                            _require(
                                git_commit(Path(source["root"])) == source["commit"],
                                "native fork commit drift",
                            )
                        except Exception as error:
                            retain(
                                local,
                                error,
                                phase=phase,
                                path=self._destination,
                                origin="policy"
                                if isinstance(error, PolicyFailure)
                                else "unknown",
                            )
                elif phase == "snapshots":
                    for snapshot in self._record["snapshots"]:
                        for directory in self.snapshot_roots(snapshot):
                            try:
                                _require(
                                    self.tree(
                                        directory,
                                        python_only=snapshot["python_only"],
                                    )
                                    == snapshot["files"],
                                    "selected source/build snapshot drift",
                                )
                            except Exception as error:
                                retain(
                                    local,
                                    error,
                                    phase="snapshot",
                                    path=directory,
                                    origin="policy"
                                    if isinstance(error, PolicyFailure)
                                    else "unknown",
                                )
                elif phase == "producer" and "producer" in self._record:
                    from harness.reconstruction import invocation

                    invocation.verify_supplied_run_inputs(
                        self._record, self._destination
                    )
            except Exception as error:
                retain(
                    local,
                    error,
                    phase=phase,
                    path=self._destination,
                    origin="policy" if isinstance(error, PolicyFailure) else "unknown",
                )
        if events is not None:
            events.extend(local)
        if primary(local) is not None:
            raise primary(local)


@dataclass(frozen=True)
class GenerationRunInput:
    directory: Path
    run: ConsumedFile | None
    failures: tuple[FailureEvent, ...]


@dataclass(frozen=True)
class GenerationProvenanceFacts:
    directory: Path
    run: FileIdentity | None
    prepared: PreparedInputs | None
    problem_binding_digest: str | None
    inputs: tuple[FileIdentity, ...]
    predicates: tuple
    failures: tuple[FailureEvent, ...]


@dataclass(frozen=True)
class SharedProvenanceFacts:
    binding: object
    generations: tuple[GenerationProvenanceFacts, ...]
    inputs: tuple[FileIdentity, ...]
    predicates: tuple
    failures: tuple[FailureEvent, ...]


def discover_generation_runs(directories, reader):
    import hashlib

    runs, seen = [], set()
    for directory in directories:
        events, consumed = [], None
        path = directory / "run.json"
        try:
            _require(
                directory.is_absolute()
                and directory.resolve() == directory
                and directory.is_dir()
                and not directory.is_symlink(),
                "generation directory must be canonical and regular",
            )
            _require(
                path.is_file() and not path.is_symlink(),
                "canonical generation run.json is missing",
            )
            data = reader(path, 64 << 20, events)
            consumed = ConsumedFile(
                FileIdentity(path, len(data), hashlib.sha256(data).hexdigest()),
                data,
            )
            key = (path, consumed.identity.bytes, consumed.identity.sha256)
            if key in seen:
                continue
            seen.add(key)
        except Exception as error:
            retain(events, error, phase="generation-discovery", path=path)
        runs.append(GenerationRunInput(directory, consumed, tuple(events)))
    return tuple(runs)


def inspect_shared_provenance(prepared_run, runs, reader):
    from harness.common.artifacts import using_reader
    from harness.reconstruction import provenance
    from harness.reconstruction.nativeoutput import parse_json, parse_source_form
    from harness.reconstruction.obligations import PredicateResult
    import hashlib

    generations, common_inputs = [], []

    def common_reader(path, bound, events=None):
        content = reader(path, bound, events)
        common_inputs.append(
            FileIdentity(path, len(content), hashlib.sha256(content).hexdigest())
        )
        return content

    with (
        using_reader(common_reader),
        contract.bounded_reads(64 << 20, reader=common_reader),
    ):
        binding = provenance.open_problem_binding(prepared_run, io_mode="exact")
        for selected in runs:
            local, prepared, digest, inputs = list(selected.failures), None, None, []

            def generation_reader(path, bound, events=None):
                content = reader(path, bound, events)
                inputs.append(
                    FileIdentity(
                        path, len(content), hashlib.sha256(content).hexdigest()
                    )
                )
                return content

            if selected.run is not None:
                try:
                    record = parse_json(selected.run.content, 64 << 20)
                    fields = {
                        "snapshots",
                        "fork_commits",
                        "native_build_inputs",
                        "tools",
                        "binaries",
                        "binding_inputs",
                        "problem",
                        "limits",
                        "selected",
                    }
                    supplied_fields = fields - {"binding_inputs"} | {"producer"}
                    supplied = type(record) is dict and set(record) == supplied_fields
                    _require(
                        type(record) is dict
                        and (
                            supplied
                            or set(record)
                            in (
                                fields,
                                fields | {"overlay_mounts"},
                                fields | {"overlay_mounts", "source_form"},
                                fields | {"source_form"},
                                fields
                                | {
                                    "overlay_mounts",
                                    "entry_observation",
                                    "entry_observation_inputs",
                                },
                                fields
                                | {
                                    "overlay_mounts",
                                    "source_form",
                                    "entry_observation",
                                    "entry_observation_inputs",
                                },
                                fields | {"source_form", "prepared_inputs", "campaign"},
                            )
                        ),
                        "generation run has an incomplete or foreign structure",
                    )
                    if "overlay_mounts" in record:
                        mounts = record["overlay_mounts"]
                        _require(
                            type(mounts) is list
                            and 1 <= len(mounts) <= 128
                            and all(type(image) is str and image for image in mounts)
                            and len(set(mounts)) == len(mounts)
                            and set(mounts)
                            <= {image.image_id for image in binding.images},
                            "generation overlay mounts differ from prepared images",
                        )
                    if "source_form" in record:
                        parse_source_form(record["source_form"])
                    if "entry_observation" in record:
                        from harness.reconstruction.nativeoutput import (
                            validate_entry_observation_descriptor,
                        )

                        descriptor = validate_entry_observation_descriptor(
                            record["entry_observation"]
                        )
                        captures = record["entry_observation_inputs"]
                        _require(
                            type(captures) is dict
                            and set(captures) == {"directory", "files"}
                            and captures["files"]
                            == sorted(
                                [
                                    {**row, "path": Path(row["path"]).name}
                                    for row in descriptor["files"].values()
                                ],
                                key=lambda row: row["path"],
                            ),
                            "entry observation staged inputs differ",
                        )
                    _require(
                        record["problem"]
                        == {
                            "run_dir": str(prepared_run),
                            "binding_digest": binding.digest,
                            "problem_sha256": binding.problem_doc_sha256,
                        },
                        "generation prepared Problem association differs",
                    )
                    _require(
                        type(record["selected"]) is list
                        and len(set(record["selected"])) == len(record["selected"])
                        and 1 <= len(record["selected"]) <= 118
                        and set(record["selected"])
                        <= {member.key for member in binding.members},
                        "generation selection differs from fixed membership",
                    )
                    if supplied:
                        _require(
                            type(record["producer"]) is dict
                            and record["producer"].get("kind")
                            == "supplied-ordinary-v1",
                            "supplied generation producer differs",
                        )
                    _require(
                        type(record["limits"]) is dict
                        and set(record["limits"])
                        == {
                            "process_output_bytes",
                            "artifact_bytes",
                        },
                        "generation execution limits differ",
                    )
                    digest = record["problem"]["binding_digest"]
                    with (
                        using_reader(generation_reader),
                        contract.bounded_reads(64 << 20, reader=generation_reader),
                    ):
                        prepared = PreparedInputs.attach(
                            record, run_directory=selected.directory, events=local
                        )
                        prepared.verify(events=local)
                    current = generation_reader(
                        selected.run.identity.path, selected.run.identity.bytes, local
                    )
                    _require(
                        current == selected.run.content,
                        "generation run changed during snapshot authentication",
                    )
                except Exception as error:
                    retain(
                        local,
                        error,
                        phase="generation-provenance",
                        path=selected.directory,
                    )
            status = (
                "matched"
                if selected.run is not None and prepared is not None and not local
                else "missing"
                if selected.run is None
                else "failed"
            )
            generations.append(
                GenerationProvenanceFacts(
                    selected.directory,
                    None if selected.run is None else selected.run.identity,
                    prepared,
                    digest,
                    tuple(dict.fromkeys(inputs)),
                    (
                        PredicateResult(
                            "current-provenance",
                            status,
                            "this generation's current source/build manifests and prepared binding",
                        ),
                    ),
                    tuple(local),
                )
            )
    return SharedProvenanceFacts(
        binding,
        tuple(generations),
        tuple(dict.fromkeys(common_inputs)),
        (
            PredicateResult(
                "current-provenance",
                "matched",
                "active common prepared Problem/Answer association authenticated",
            ),
        ),
        (),
    )


def code_sha256() -> str:
    """Closure digest over the reconstruction package that produced a bundle."""

    package = Path(__file__).resolve().parent
    lines = [
        f"{path.name}:{file_sha256(path)}" for path in sorted(package.glob("*.py"))
    ]
    return contract.sha256_bytes(("\n".join(lines) + "\n").encode())


def producer(revision: object) -> dict:
    contract.require(
        isinstance(revision, str) and contract.REVISION.fullmatch(revision) is not None,
        f"producer revision is invalid: {revision!r}",
    )
    return {"module": MODULE, "revision": str(revision), "code_sha256": code_sha256()}


def run_prepare(
    *,
    root: Path,
    run_root: Path,
    run_id: str,
    baseline_path: Path,
    native_receipt: Path,
) -> dict:
    """Verify every frozen predicate, then publish problem, answer and preparation."""

    from harness.reconstruction import authored as authored_module
    from harness.reconstruction import baseline as baseline_module
    from harness.reconstruction import payloads

    baseline_document = baseline_module.load_baseline(root, baseline_path)
    closure = baseline_module.source_closure(root, native_receipt=native_receipt)
    members = baseline_module.member_facts(root, baseline_document.document)
    authored = authored_module.authored_members(
        root, baseline_document.document, members
    )
    exclusions = authored_module.exclusions(root, baseline_document.document, members)
    producer_document = producer(git_commit(root))

    bundles.ensure_run_root(run_root)
    run_dir = bundles.create_run(run_root, run_id)
    problem_document, problem_files = payloads.problem_payload(
        baseline_document.document,
        members,
        contract.RULES_PATH,
        contract.RULES_BYTES,
    )
    problem = bundles.seal_bundle(
        run_dir,
        name="problem",
        role="problem",
        origin="approved",
        producer=producer_document,
        documents={"problem.json": problem_document},
        files=problem_files,
    )
    answer_document, answer_files = payloads.answer_payload(
        baseline_bytes=baseline_document.data,
        closure=closure,
        authored=authored,
        exclusions=exclusions,
        problem_digest=problem.digest,
        problem_bundle_digest=problem.bundle_digest,
    )
    answer = bundles.seal_bundle(
        run_dir,
        name="answer",
        role="answer",
        origin="evaluator",
        producer=producer_document,
        documents={"answer.json": answer_document},
        files=answer_files,
        upstream=[contract.doc_ref("problem", problem.digest, problem.bundle_digest)],
    )
    preparation_document = {
        "schema": contract.PREPARATION_SCHEMA,
        "problem": {
            "role": "problem",
            "bundle": "problem",
            "bundle_digest": problem.bundle_digest,
        },
        "answer": {
            "role": "answer",
            "bundle": "answer",
            "bundle_digest": answer.bundle_digest,
        },
        "complete": True,
        "sealed_at": contract.timestamp(),
    }
    preparation = bundles.seal_bundle(
        run_dir,
        name="preparation",
        role="preparation",
        origin="control",
        producer=producer_document,
        documents={"preparation.json": preparation_document},
        files={},
    )
    bundles.verify_preparation_bindings(run_dir)
    bundles.verify_bundle_references(run_dir, "problem")
    bundles.verify_bundle_references(run_dir, "answer")
    return {
        "schema": contract.COMMAND_SCHEMA,
        "operation": "prepare",
        "status": "ok",
        "exit": 0,
        "run_id": contract.validate_run_id(run_id),
        "artifacts": [
            {
                "role": bundle.role,
                "bundle_digest": bundle.bundle_digest,
                "schema": bundle.schema,
                "path": bundle.name,
            }
            for bundle in (problem, answer, preparation)
        ],
        "summary": {
            "admitted": contract.ADMITTED,
            "exclusions": contract.EXCLUSIONS,
            "problem": problem.digest,
        },
        "message": None,
    }
