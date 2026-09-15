"""Select source-bound preservation history without changing caller environment."""

from __future__ import annotations

import hashlib
import os
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path

from harness.build.preservation import read_preservation_document, validate_fingerprint
from harness.common.deadlines import check_deadline
from harness.common.directory import validate_repo_path
from harness.common.files import read_file
from harness.common.observation import PathWatch

PRESERVATION_ENVIRONMENT = (
    "BOF3_PRESERVATION_RECORD",
    "BOF3_PRESERVATION_FINGERPRINT",
    "BOF3_PRESERVATION_ROUTES",
    "BOF3_PRESERVATION_ROUTES_SHA256",
)
_LIMIT = 4 * 1024 * 1024


@dataclass(frozen=True)
class Selection:
    record: Path | None
    fingerprint: str | None
    routes: Path | None
    routes_sha256: str | None
    environment: dict[str, str | None]
    watch: PathWatch | None
    reserved: frozenset[Path] = frozenset()

    def close(self) -> None:
        if self.watch is not None:
            self.watch.close()

    def validate(self) -> None:
        check_deadline()
        if self.environment != {
            name: os.environ.get(name) for name in PRESERVATION_ENVIRONMENT
        }:
            raise ValueError("preservation routing environment changed")
        if self.watch is not None:
            self.watch.validate()
            content = read_file(self.routes.parent, self.routes.name, max_bytes=_LIMIT)
            if hashlib.sha256(content).hexdigest() != self.routes_sha256:
                raise ValueError("preservation routes differ from external SHA-256")
            self.watch.validate()
        check_deadline()

    def protect_outputs(
        self, output: Path | None, *, inputs: Iterable[Path] = ()
    ) -> None:
        if output is None:
            return
        reserved = self.reserved | set(inputs)
        if self.record is not None:
            reserved |= {self.record}
        protected = reserved | {path.resolve() for path in reserved}
        for artifact in (
            output,
            output.with_name(output.name + ".s"),
            output.with_name(output.name + ".producer.json"),
        ):
            check_deadline()
            if artifact in protected or artifact.resolve() in protected:
                raise ValueError("compiler output collides with a preserved input")

    def describe(self) -> dict:
        return {
            "path": str(self.routes) if self.routes else None,
            "sha256": self.routes_sha256,
        }


def _read_pair(
    environment: dict, names: tuple[str, str]
) -> tuple[str | None, str | None]:
    path, fingerprint = (environment[name] for name in names)
    if (path is None) != (fingerprint is None) or path == "" or fingerprint == "":
        raise ValueError("preservation path and external pin must be supplied together")
    if fingerprint is not None:
        validate_fingerprint(fingerprint)
    return path, fingerprint


def _confine_path(root: Path, value: object) -> Path:
    name = validate_repo_path(value)
    if any(ord(character) < 32 or ord(character) == 127 for character in name):
        raise ValueError("preservation routing path has control characters")
    path = root / name
    if path.resolve() != path:
        raise ValueError("preservation routing requires canonical repository paths")
    return path


def _select_record(root: Path, document: dict, source: Path, grouped: bool) -> tuple:
    if (
        set(document) != {"schema", "root", "records"}
        or document["schema"] != "bof3.preservation-routes/v1"
        or document["root"] != str(root)
    ):
        raise ValueError("preservation routing schema or repository binding is invalid")
    records = document["records"]
    if not isinstance(records, dict) or not 1 <= len(records) <= 4096:
        raise ValueError("preservation routes require one to 4096 source entries")
    selected = None
    used_records = set()
    reserved = set()
    for name, entry in records.items():
        check_deadline()
        path = _confine_path(root, name)
        if not path.is_relative_to(root / "src/bof3") or path.suffix != ".c":
            raise ValueError("preservation routes require src/bof3/ C source keys")
        if not isinstance(entry, dict) or set(entry) != {"record", "fingerprint"}:
            raise ValueError("preservation route requires record and fingerprint")
        record = _confine_path(root, entry["record"])
        validate_fingerprint(entry["fingerprint"])
        if record in used_records:
            raise ValueError("one preservation record cannot route multiple sources")
        used_records.add(record)
        reserved.update((path, record))
        if source == path:
            selected = record, entry["fingerprint"]
    if grouped and selected is None:
        raise ValueError("grouped source has no explicit preservation route")
    if not grouped and selected is not None:
        raise ValueError(
            "preservation route selects a source that is no longer grouped"
        )
    record, fingerprint = selected if selected is not None else (None, None)
    return record, fingerprint, frozenset(reserved)


def select_preservation(
    root: Path, sources: list[Path], grouped: list[Path]
) -> Selection:
    """Resolve one explicit pair or an externally pinned source-routing table."""
    environment = {name: os.environ.get(name) for name in PRESERVATION_ENVIRONMENT}
    record_name, fingerprint = _read_pair(environment, PRESERVATION_ENVIRONMENT[:2])
    routes_name, routes_sha256 = _read_pair(environment, PRESERVATION_ENVIRONMENT[2:])
    if record_name is not None and routes_name is not None:
        raise ValueError("explicit preservation history conflicts with source routes")
    if routes_name is None:
        return Selection(
            Path(record_name).absolute() if record_name else None,
            fingerprint,
            None,
            None,
            environment,
            None,
        )
    routes = Path(routes_name)
    if (
        not routes.is_absolute()
        or str(routes) != routes_name
        or not routes.is_relative_to(root)
        or routes.resolve() != routes
    ):
        raise ValueError(
            "preservation routes require an absolute canonical repository path"
        )
    if len(sources) != 1:
        raise ValueError("preservation routing requires one explicit source")
    source = sources[0]
    if (
        source.resolve() != source
        or not source.is_relative_to(root / "src")
        or source.suffix != ".c"
    ):
        raise ValueError("preservation routing requires a canonical src/ C source")
    watch = PathWatch({routes})
    try:
        document = read_preservation_document(routes, expected_sha256=routes_sha256)
        record, fingerprint, reserved = _select_record(
            root, document, source, bool(grouped)
        )
        selection = Selection(
            record,
            fingerprint,
            routes,
            routes_sha256,
            environment,
            watch,
            reserved | {routes},
        )
        selection.validate()
        return selection
    except BaseException:
        watch.close()
        raise
