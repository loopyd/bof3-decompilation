"""Durable cumulative reconstruction accounting across execution captures."""

from __future__ import annotations

from contextlib import contextmanager
from contextvars import ContextVar
from dataclasses import asdict
from functools import wraps
import json
from pathlib import Path
import re
import stat

from harness.common.digests import digest, file_record
from harness.common.files import read_file
from harness.common.journal import write_record
from harness.common.lease import acquire_writer, require_writer
from harness.common.process import ProcessCleanupError
from harness.io import unique_object
from harness.reconstruction import provenance

SCHEMA = "bof3.reconstruction-attempt-ledger/v1"
MAX_RECORD_BYTES = 128 << 10
PHASE_UNITS = dict(
    prepare=1,
    **{"self-check": 1},
    native=1,
    syntax=1,
    profile=1,
    rebuild=6,
    compare=5,
    associated=1,
)
ACTIVE: ContextVar[tuple[Ledger, str] | None] = ContextVar(
    "reconstruction_attempt_ledger", default=None
)


def corpus_identity(binding) -> dict:
    return {
        "denominator": 118,
        "exclusions": 7,
        "members": [asdict(member) for member in binding.members],
        "targets": [asdict(target) for target in binding.targets],
        "images": [
            {
                key: asdict(image)[key]
                for key in (
                    "image_id",
                    "sha256",
                    "bytes",
                    "format",
                    "load_address",
                    "payload_offset",
                    "payload_size",
                )
            }
            for image in binding.images
        ],
    }


def _require(condition, message):
    if not condition:
        raise ValueError(message)


def _units(value):
    _require(type(value) is int and value >= 1, "invalid ledger operation units")


def _check_record_size(record):
    _require(
        len((json.dumps(record, sort_keys=True, indent=2) + "\n").encode())
        <= MAX_RECORD_BYTES,
        "attempt ledger record exceeds its replay byte limit",
    )


def _check_pin(pin):
    _require(
        type(pin) is dict
        and set(pin) == {"path", "bytes", "sha256"}
        and type(pin["path"]) is str
        and Path(pin["path"]).is_absolute()
        and type(pin["bytes"]) is int
        and pin["bytes"] >= 0
        and type(pin["sha256"]) is str
        and re.fullmatch(r"[0-9a-f]{64}", pin["sha256"]),
        "invalid ledger evidence pin",
    )


def _name(root: Path, path: Path) -> str:
    path = path if path.is_absolute() else root / path
    _require(
        path.resolve() == path and not path.is_symlink(),
        "ledger path must be canonical",
    )
    name = path.relative_to(root).as_posix()
    _require(
        name.startswith("out/retdec-118-reconstruction/attempt-ledgers/"),
        "ledger must live in the reconstruction attempt-ledgers directory",
    )
    return name


class Ledger:
    def __init__(self, root: Path, name: str):
        self.root, self.name = root, name
        self.records = []
        self.grants = {}
        self.spent = {}
        self.imports = set()
        self.import_hashes = set()
        self.epochs = {}
        self.finished = set()
        self.historical_units = 0
        self.reserved_units = 0
        self.operation_counts = dict.fromkeys(PHASE_UNITS, 0)
        self.cleanup_uncertain = False
        directory = root / name
        _require(
            directory.is_dir() and not directory.is_symlink(),
            "attempt ledger is absent",
        )
        names = [path.name for path in directory.iterdir()]
        _require(
            bool(names)
            and all(re.fullmatch(r"[0-9]{6,}\.json", name) for name in names),
            "ledger contains a missing, torn or foreign record",
        )
        names.sort(key=lambda name: int(name[:-5]))
        _require(
            names == [f"{i:06d}.json" for i in range(len(names))],
            "ledger contains a missing, torn or foreign record",
        )
        for leaf in names:
            state = (directory / leaf).lstat()
            _require(
                stat.S_ISREG(state.st_mode) and state.st_nlink == 1,
                "ledger record must be a single-link regular file",
            )
            raw = read_file(root, f"{name}/{leaf}", max_bytes=MAX_RECORD_BYTES)
            record = json.loads(raw, object_pairs_hook=unique_object)
            self._accept(record)

    @property
    def head(self):
        return self.records[-1]["digest"]

    def _accept(self, record):
        _require(
            type(record) is dict
            and set(record)
            == {"schema", "sequence", "previous", "kind", "payload", "digest"},
            "invalid ledger record fields",
        )
        body = {key: value for key, value in record.items() if key != "digest"}
        _require(
            record["schema"] == SCHEMA
            and type(record["sequence"]) is int
            and record["sequence"] == len(self.records)
            and record["previous"] == (self.head if self.records else None)
            and record["digest"] == digest(body),
            "ledger lineage or digest differs",
        )
        kind, payload = record["kind"], record["payload"]
        _require(type(payload) is dict, "invalid ledger payload")
        if not self.records:
            _require(
                kind == "origin"
                and set(payload) == {"mission", "corpus", "history_complete"}
                and type(payload["mission"]) is str
                and re.fullmatch(r"[A-Za-z0-9._-]{1,128}", payload["mission"])
                and payload["history_complete"] is False,
                "invalid ledger origin",
            )
            corpus = payload["corpus"]
            _require(
                type(corpus) is dict
                and corpus.get("denominator") == 118
                and corpus.get("exclusions") == 7
                and len(corpus.get("members", [])) == 118,
                "ledger requires the frozen 118-member corpus",
            )
        elif kind == "import":
            _require(
                set(payload) == {"operation_id", "units", "evidence"}
                and type(payload["operation_id"]) is str
                and payload["operation_id"]
                and payload["operation_id"] not in self.imports,
                "duplicate or invalid historical operation",
            )
            _require(
                type(payload["units"]) is int and payload["units"] >= 0,
                "invalid historical operation units",
            )
            _require(
                type(payload["evidence"]) is dict
                and set(payload["evidence"]) == {"path", "bytes", "sha256"},
                "historical operation needs an evidence pin",
            )
            _check_pin(payload["evidence"])
            _require(
                payload["evidence"]["sha256"] not in self.import_hashes,
                "historical evidence is already imported",
            )
            self.imports.add(payload["operation_id"])
            self.import_hashes.add(payload["evidence"]["sha256"])
            self.historical_units += payload["units"]
        elif kind == "grant":
            _require(
                set(payload) == {"id", "units", "members"}
                and type(payload["id"]) is str
                and re.fullmatch(r"[A-Za-z0-9._-]{1,128}", payload["id"])
                and payload["id"] not in self.grants,
                "duplicate or invalid ledger grant",
            )
            _units(payload["units"])
            allowed = {
                member["key"]
                for member in self.records[0]["payload"]["corpus"]["members"]
            }
            _require(
                type(payload["members"]) is list
                and bool(payload["members"])
                and all(type(member) is str for member in payload["members"])
                and len(set(payload["members"])) == len(payload["members"])
                and set(payload["members"]) <= allowed,
                "grant members differ from the frozen corpus",
            )
            self.grants[payload["id"]] = payload
            self.spent[payload["id"]] = 0
        elif kind == "epoch":
            _require(
                set(payload)
                == {"id", "grant", "operation", "prepared_run", "binding_digest"}
                and type(payload["id"]) is str
                and payload["id"] not in self.epochs
                and (payload["grant"] is None or payload["grant"] in self.grants)
                and payload["operation"] in {"run", "verify", "capture-supplied"},
                "invalid or repeated ledger execution epoch",
            )
            self.epochs[payload["id"]] = payload
        elif kind == "reserve":
            _require(
                set(payload) == {"epoch", "phase", "member", "units"}
                and payload["epoch"] in self.epochs
                and payload["epoch"] not in self.finished
                and payload["phase"]
                in {
                    "prepare",
                    "self-check",
                    "native",
                    "syntax",
                    "profile",
                    "rebuild",
                    "compare",
                    "associated",
                },
                "invalid ledger reservation",
            )
            _units(payload["units"])
            _require(
                payload["units"] == PHASE_UNITS[payload["phase"]],
                "reservation cost differs from its owner operation",
            )
            allowed = {
                member["key"]
                for member in self.records[0]["payload"]["corpus"]["members"]
            }
            _require(
                payload["member"] is None
                if payload["phase"] in {"prepare", "self-check"}
                else type(payload["member"]) is str and payload["member"] in allowed,
                "reservation member is outside the frozen corpus",
            )
            self.reserved_units += payload["units"]
            self.operation_counts[payload["phase"]] += 1
            grant = self.epochs[payload["epoch"]]["grant"]
            if grant is not None:
                self.spent[grant] += payload["units"]
        elif kind == "finish":
            _require(
                set(payload) == {"epoch", "status", "artifacts", "writer_review"}
                and payload["epoch"] in self.epochs
                and payload["epoch"] not in self.finished
                and payload["status"] in {"returned", "failed"}
                and type(payload["artifacts"]) is list,
                "invalid ledger completion",
            )
            for pin in payload["artifacts"]:
                _check_pin(pin)
            if payload["writer_review"] is not None:
                _check_pin(payload["writer_review"])
            self.finished.add(payload["epoch"])
        else:
            raise ValueError("unsupported ledger record kind")
        self.records.append(record)

    def append(self, kind, payload):
        require_writer(self.root)
        _require(type(payload) is dict, "invalid ledger payload")
        _require(kind != "grant", "attempt allowance grants are retired")
        _require(
            kind != "epoch" or payload.get("grant") is None,
            "grant-backed execution epochs are retired",
        )
        last = Ledger(self.root, self.name)
        _require(
            last.head == self.head,
            "ledger changed while held; no owner call is permitted",
        )
        body = dict(
            schema=SCHEMA,
            sequence=len(self.records),
            previous=self.head,
            kind=kind,
            payload=payload,
        )
        record = dict(body, digest=digest(body))
        _check_record_size(record)
        # Validate before the durable exclusive write, without changing this account.
        last._accept(record)
        write_record(self.root, f"{self.name}/{len(self.records):06d}.json", record)
        self._accept(record)

    def summary(self):
        return dict(
            schema=SCHEMA,
            path=self.name,
            digest=self.head,
            mission=self.records[0]["payload"]["mission"],
            history_complete=False,
            historical_units_minimum=self.historical_units,
            reserved_units=self.reserved_units,
            consumed_units_minimum=self.historical_units + self.reserved_units,
            operation_counts=dict(self.operation_counts),
            historical_grants=[
                dict(
                    **grant,
                    consumed_units=self.spent[key],
                )
                for key, grant in self.grants.items()
            ],
            unfinished_epochs=sorted(set(self.epochs) - self.finished),
        )


def initialize(root, path, binding, mission):
    root = root.resolve()
    name = _name(root, path)
    _require(
        type(mission) is str and re.fullmatch(r"[A-Za-z0-9._-]{1,128}", mission),
        "invalid ledger mission",
    )
    _require(
        len(binding.members) == 118, "ledger requires the frozen 118-member corpus"
    )
    _require(
        name == f"out/retdec-118-reconstruction/attempt-ledgers/{mission}",
        "mission must retain its canonical ledger path",
    )
    with acquire_writer(root):
        _require(not (root / name).exists(), "attempt ledger already exists")
        body = dict(
            schema=SCHEMA,
            sequence=0,
            previous=None,
            kind="origin",
            payload=dict(
                mission=mission, corpus=corpus_identity(binding), history_complete=False
            ),
        )
        record = dict(body, digest=digest(body))
        _check_record_size(record)
        write_record(root, f"{name}/000000.json", record)
        return Ledger(root, name).summary()


def inspect(root, path):
    root = root.resolve()
    return Ledger(root, _name(root, path)).summary()


def update(root, path, expected_digest, kind, payload):
    root = root.resolve()
    with acquire_writer(root):
        ledger = Ledger(root, _name(root, path))
        _require(ledger.head == expected_digest, "stale attempt ledger digest")
        ledger.append(kind, payload)
        return ledger.summary()


def import_operation(
    root, path, expected_digest, operation_id, units, evidence, expected_sha256
):
    evidence = evidence if evidence.is_absolute() else root / evidence
    _require(
        evidence.resolve() == evidence
        and evidence.is_file()
        and evidence.lstat().st_nlink == 1,
        "historical evidence must be a canonical single-link file",
    )
    pin = file_record(evidence)
    _require(pin["sha256"] == expected_sha256, "historical evidence SHA-256 differs")
    root = root.resolve()
    with acquire_writer(root):
        ledger = Ledger(root, _name(root, path))
        _require(ledger.head == expected_digest, "stale attempt ledger digest")
        payload = dict(operation_id=operation_id, units=units, evidence=pin)
        previous = [
            record["payload"]
            for record in ledger.records
            if record["kind"] == "import"
            and record["payload"]["operation_id"] == operation_id
        ]
        if previous:
            _require(
                previous == [payload],
                "historical operation conflicts with its retained import",
            )
        else:
            ledger.append("import", payload)
        return ledger.summary()


def _terminal_artifacts(destination):
    if not destination.exists():
        return []
    _require(
        destination.resolve() == destination
        and destination.is_dir()
        and not destination.is_symlink(),
        "execution publication directory drifted",
    )
    progress = [
        path
        for path in destination.glob("progress-*.json")
        if re.fullmatch(r"progress-[0-9]{3,}.json", path.name)
    ]
    paths = [destination / name for name in ("report.json", "verification-result.json")]
    if progress:
        paths.append(max(progress, key=lambda path: int(path.stem.split("-")[1])))
    pins = []
    for path in paths:
        if path.exists() or path.is_symlink():
            _require(
                path.resolve() == path
                and path.is_file()
                and path.lstat().st_nlink == 1,
                "execution publication must be a canonical single-link file",
            )
            pins.append(file_record(path))
    return pins


def close_execution(root, path, expected_digest, epoch, evidence, expected_sha256):
    """Record the operator's terminal-writer review without refunding reservations."""
    root = root.resolve()
    evidence = evidence if evidence.is_absolute() else root / evidence
    with acquire_writer(root):
        ledger = Ledger(root, _name(root, path))
        _require(ledger.head == expected_digest, "stale attempt ledger digest")
        _require(
            epoch in ledger.epochs and epoch not in ledger.finished,
            "terminal review requires an unfinished execution",
        )
        _require(
            evidence.resolve() == evidence
            and evidence.is_file()
            and evidence.lstat().st_nlink == 1,
            "terminal review must be a canonical single-link file",
        )
        pin = file_record(evidence)
        _require(pin["sha256"] == expected_sha256, "terminal review SHA-256 differs")
        ledger.append(
            "finish",
            dict(
                epoch=epoch,
                status="failed",
                artifacts=_terminal_artifacts(Path(epoch)),
                writer_review=pin,
            ),
        )
        return ledger.summary()


@contextmanager
def execution(root, path, expected_digest, prepared_run, destination, operation):
    root = root.resolve()
    _require(ACTIVE.get() is None, "nested reconstruction accounts are unsupported")
    with acquire_writer(root):
        ledger = Ledger(root, _name(root, path))
        _require(ledger.head == expected_digest, "stale attempt ledger digest")
        binding = provenance.open_problem_binding(prepared_run, io_mode="exact")
        _require(
            corpus_identity(binding) == ledger.records[0]["payload"]["corpus"],
            "recaptured corpus differs from attempt ledger",
        )
        _require(
            not set(ledger.epochs) - ledger.finished,
            "unfinished execution requires terminal writer review before continuation",
        )
        epoch = str(destination.resolve())
        _require(
            not destination.exists()
            and not destination.is_symlink()
            and destination.resolve() == destination
            and destination.parent.is_dir(),
            "ledger execution destination must be new and canonical",
        )
        ledger.append(
            "epoch",
            dict(
                id=epoch,
                grant=None,
                operation=operation,
                prepared_run=str(prepared_run.resolve()),
                binding_digest=binding.digest,
            ),
        )
        token = ACTIVE.set((ledger, epoch))
        failed = True
        cleanup_known = True
        try:
            yield ledger
            failed = False
        except BaseException as error:
            if isinstance(error, ProcessCleanupError) or not isinstance(
                error, Exception
            ):
                cleanup_known = False
            raise
        finally:
            ACTIVE.reset(token)
            if cleanup_known and not ledger.cleanup_uncertain:
                ledger.append(
                    "finish",
                    dict(
                        epoch=epoch,
                        status="failed" if failed else "returned",
                        artifacts=_terminal_artifacts(destination),
                        writer_review=None,
                    ),
                )


def reserve(phase, member=None, *, units=None):
    current = ACTIVE.get()
    _require(
        current is not None, "native reconstruction requires an active attempt ledger"
    )
    ledger, epoch = current
    _require(
        not ledger.cleanup_uncertain,
        "native cleanup is uncertain; further attempts are blocked",
    )
    units = PHASE_UNITS[phase] if units is None else units
    ledger.append("reserve", dict(epoch=epoch, phase=phase, member=member, units=units))


def snapshot():
    current = ACTIVE.get()
    _require(
        current is not None, "native reconstruction requires an active attempt ledger"
    )
    return current[0].summary()


def cleanup_unknown():
    current = ACTIVE.get()
    _require(
        current is not None, "cleanup observation requires an active attempt ledger"
    )
    current[0].cleanup_uncertain = True


def require_members(members):
    current = ACTIVE.get()
    _require(current is not None, "member selection requires an active attempt ledger")
    ledger, _ = current
    allowed = {
        member["key"] for member in ledger.records[0]["payload"]["corpus"]["members"]
    }
    _require(
        set(members) <= set(allowed),
        "selected members are outside the frozen corpus",
    )


def require_resume(pin):
    current = ACTIVE.get()
    _require(current is not None, "resume requires an active attempt ledger")
    ledger, active_epoch = current
    working_epochs = {
        record["payload"]["epoch"]
        for record in ledger.records
        if record["kind"] == "reserve"
        or record["kind"] == "finish"
        and record["payload"]["artifacts"]
    }
    prior = [
        epoch
        for epoch in ledger.epochs.values()
        if epoch["operation"] == "run"
        and epoch["id"] != active_epoch
        and epoch["id"] in working_epochs
    ]
    _require(bool(prior), "resume has no retained ledger execution")
    epoch = prior[-1]["id"]
    artifacts = [
        artifact
        for record in ledger.records
        if record["kind"] == "finish" and record["payload"]["epoch"] == epoch
        for artifact in record["payload"]["artifacts"]
    ]
    _require(
        any(
            artifact["path"] == str(pin.path) and artifact["sha256"] == pin.sha256
            for artifact in artifacts
        ),
        "resume checkpoint is not the current terminal ledger publication",
    )


def accounted(operation):
    """Require the same cumulative account around each native driver route."""

    def decorate(function):
        @wraps(function)
        def run(*, attempt_ledger, expected_ledger_digest, **arguments):
            with execution(
                Path(arguments["root"]),
                Path(attempt_ledger),
                expected_ledger_digest,
                Path(arguments.get("run_dir", arguments.get("prepared_run"))),
                Path(arguments["destination"]),
                operation,
            ):
                return function(**arguments)

        return run

    return decorate
