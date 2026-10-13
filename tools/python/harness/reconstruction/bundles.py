"""Fresh bundle publication rejects existing bundle directories."""

from __future__ import annotations

from typing import Literal
from harness.common.deadlines import check_deadline
from harness.common.failures import FailureEvent

import json
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Iterator

from harness.reconstruction import contract, records, relations
from harness.reconstruction import documents as document_records


@dataclass(frozen=True)
class SealedBundle:
    """One published bundle and the digest of its primary document."""

    name: str
    role: str
    schema: str
    document: dict
    digest: str
    bundle_digest: str


def ensure_run_root(run_root: Path) -> Path:
    try:
        run_root.mkdir(parents=True, exist_ok=True)
    except OSError as error:
        raise contract.PreparationRefused(
            f"run root is unusable: {run_root} ({error})"
        ) from error
    contract.require(run_root.is_dir(), f"run root is not a directory: {run_root}")
    contract.require(not run_root.is_symlink(), f"run root is a symlink: {run_root}")
    return run_root


def create_run(run_root: Path, run_id: str) -> Path:
    """Create the run directory exclusively; an existing run is never adopted."""

    run_dir = run_root / contract.validate_run_id(run_id)
    try:
        os.mkdir(run_dir)
    except FileExistsError as error:
        raise contract.PreparationRefused(
            f"run directory already exists and is never adopted: {run_dir}"
        ) from error
    except OSError as error:
        raise contract.PreparationRefused(
            f"run directory cannot be created: {run_dir} ({error})"
        ) from error
    return run_dir


def _validated_index(data: bytes, run_name: str, coordinate: str, subject: str) -> dict:
    index = contract.parse_json(data, subject)
    records.validate_index(index, subject, coordinate=coordinate)
    contract.require(
        index["run"] == run_name,
        f"bundle {coordinate} does not belong to the active run",
    )
    observed = contract.bundle_digest(index)
    contract.require(
        observed == index["bundle_digest"],
        f"bundle digest mismatch in {coordinate}: expected {index['bundle_digest']} observed {observed}",
    )
    return index


def _read_member(
    run_dir: Path,
    coordinate: str,
    member: str,
    subject: str,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> bytes:
    path = (
        run_dir
        / contract.validate_rel_path(coordinate)
        / contract.validate_rel_path(member)
    )
    for parent in [path, *path.parents]:
        if parent == run_dir:
            break
        contract.require(
            not parent.is_symlink(), f"{subject} contains a symlink: {parent}"
        )
    contract.require(path.is_file(), f"{subject} is not a regular file: {path}")
    return contract.read_bytes(
        path, subject, io_mode=io_mode, events=events, max_bytes=max_bytes
    )


def load_index(
    run_dir: Path,
    name: str,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> dict:
    """Read one published bundle index and verify its own digest and shape."""
    subject = f"bundle index {name}"
    data = _read_member(
        run_dir,
        name,
        "index.json",
        subject,
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    index = _validated_index(data, run_dir.name, name, subject)
    return index


def recomputed_bundle_digest(run_dir: Path, name: str) -> str:
    return contract.bundle_digest(load_index(run_dir, name))


def _primary_document(
    run_dir: Path,
    name: str,
    index: dict,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> dict:
    primary = index["documents"][0]
    return resolve_document(
        run_dir,
        contract.doc_ref(name, primary["digest"], None),
        owner=name,
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )


def verify_preparation_bindings(
    run_dir: Path,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> dict:
    """Resolve ``preparation/`` and enforce its publication identity bindings."""

    index = load_index(
        run_dir,
        "preparation",
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    contract.require(
        index["role"] == "preparation" and index["origin"] == "control",
        "Preparation requires the fixed control Preparation",
    )
    digests = _preparation_digests(
        run_dir, io_mode=io_mode, events=events, max_bytes=max_bytes
    )
    document = _primary_document(
        run_dir,
        "preparation",
        index,
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    relations.check_preparation_bindings(document, digests, "preparation binding")
    return document


def verify_visited_documents(
    run_dir: Path,
    visited: dict[tuple[str, str], dict],
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
) -> None:
    """Run cross-record relations for **every** visited (bundle, digest) document.

    Bind Answer to its Problem and Preparation to both publications. Secondary
    and transitively reached documents receive the same checks.
    """

    digests: dict | None = None
    for (bundle, digest), document in sorted(visited.items()):
        subject = f"document {bundle}/{digest}"
        schema = document.get("schema")
        if schema == contract.ANSWER_SCHEMA:
            problem = resolve_document(
                run_dir,
                document["problem"],
                owner=bundle,
                io_mode=io_mode,
                events=events,
            )
            relations.check_answer_against_problem(document, problem, subject)
        elif schema == contract.PREPARATION_SCHEMA:
            if digests is None:
                digests = _preparation_digests(run_dir, io_mode=io_mode, events=events)
            relations.check_preparation_bindings(document, digests, subject)


def seal_bundle(
    run_dir: Path,
    *,
    name: str,
    role: str,
    origin: str,
    producer: dict,
    documents: dict,
    files: dict,
    upstream: list | tuple = (),
) -> SealedBundle:
    """Write a fresh bundle, publish its index last and read-protect its files."""

    contract.validate_rel_path(name)
    contract.require(role in contract.ROLE_ORIGIN, f"unknown bundle role: {role!r}")
    contract.require(
        origin == contract.ROLE_ORIGIN[role],
        f"bundle origin {origin!r} does not match role {role!r}",
    )
    records.validate_producer(producer, "bundle producer")
    expected_schema = records.ROLE_DOCUMENT_SCHEMA[role]
    document_entries = []
    payloads: dict[str, bytes] = {}
    for path in sorted(documents):
        document = documents[path]
        contract.validate_rel_path(path)
        contract.require(
            isinstance(document, dict), f"document is not an object: {path}"
        )
        document_records.validate_document(document, f"document {path}")
        schema = document.get("schema")
        contract.require(
            schema == expected_schema,
            f"document {path} schema {schema!r} does not match role {role!r}",
        )
        data = json.dumps(document, indent=2, sort_keys=True).encode() + b"\n"
        payloads[path] = data
        document_entries.append(
            {
                "digest": contract.doc_digest(document),
                "path": path,
                "schema": schema,
                "sha256": contract.sha256_bytes(data),
            }
        )
    file_entries = []
    for path in sorted(files):
        contract.validate_rel_path(path)
        data = bytes(files[path])
        contract.require(
            path not in payloads or payloads[path] == data,
            f"conflicting document/file bytes: {path}",
        )
        payloads[path] = data
        file_entries.append(contract.file_ref(name, path, payloads[path]))
    contract.require(
        len({entry["path"] for entry in document_entries}) == len(document_entries),
        "bundle document paths must be unique",
    )
    contract.require(
        len({entry["path"] for entry in file_entries}) == len(file_entries),
        "bundle file paths must be unique",
    )
    index = {
        "schema": contract.BUNDLE_SCHEMA,
        "role": role,
        "run": contract.validate_run_id(run_dir.name),
        "origin": origin,
        "producer": producer,
        "documents": document_entries,
        "files": file_entries,
        "upstream": list(upstream),
        "sealed_at": contract.timestamp(),
    }
    index["bundle_digest"] = contract.bundle_digest(index)
    records.validate_index(index, f"bundle {name} index", coordinate=name)
    indexed = {entry["path"] for entry in document_entries} | {
        entry["path"] for entry in file_entries
    }
    contract.require(
        set(payloads) == indexed,
        f"staged payloads are not all indexed for {name}: {sorted(set(payloads) ^ indexed)}",
    )
    for path in payloads:
        contract.require(
            path.split("/")[0] != "index.json", f"reserved index member: {path}"
        )
        parts = path.split("/")
        contract.require(
            not any(
                "/".join(parts[:position]) in payloads
                for position in range(1, len(parts))
            ),
            f"impossible member tree: {path}",
        )
    directory = run_dir / name
    try:
        directory.mkdir()
        for path, data in [
            *payloads.items(),
            (
                "index.json",
                json.dumps(index, indent=2, sort_keys=True).encode() + b"\n",
            ),
        ]:
            destination = directory / path
            destination.parent.mkdir(parents=True, exist_ok=True)
            with destination.open("xb") as stream:
                stream.write(data)
                stream.flush()
                os.fsync(stream.fileno())
            contract.require(
                destination.read_bytes() == data, f"bundle write differs: {path}"
            )
            destination.chmod(0o444)
        for child in sorted(directory.rglob("*"), reverse=True):
            if child.is_dir():
                child.chmod(0o555)
        directory.chmod(0o555)
    except OSError as error:
        raise contract.PreparationRefused(
            f"fresh bundle {name} could not be written: {error}"
        ) from error
    return _sealed_from(run_dir, name, index)


def _sealed_from(run_dir: Path, name: str, index: dict) -> SealedBundle:
    entries = index.get("documents", [])
    contract.require(bool(entries), f"bundle {name} publishes no document")
    primary = entries[0]
    document = resolve_document(
        run_dir, contract.doc_ref(name, primary["digest"], None), owner=name
    )
    return SealedBundle(
        name=name,
        role=str(index["role"]),
        schema=str(primary["schema"]),
        document=document,
        digest=str(primary["digest"]),
        bundle_digest=str(index["bundle_digest"]),
    )


def resolve_document(
    run_dir: Path,
    ref: dict,
    owner: str | None = None,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> dict:
    """Resolve a ``DocRef``: index entry, byte hash, schema and recomputed digest."""

    records.validate_doc_ref(ref, "document reference")
    bundle = str(ref["bundle"])
    index = load_index(
        run_dir,
        bundle,
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    _check_expected_bundle(ref, index, owner)
    digest = ref.get("digest")
    entries = [entry for entry in index["documents"] if entry["digest"] == digest]
    contract.require(
        len(entries) == 1, f"document reference is ambiguous or absent: {digest}"
    )
    entry = entries[0]
    if index["role"] == "preparation":
        _preparation_digests(
            run_dir,
            io_mode=io_mode,
            events=events,
            max_bytes=max_bytes,
        )
    data = _read_member(
        run_dir,
        bundle,
        entry["path"],
        f"document {entry['path']}",
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    contract.require(
        contract.sha256_bytes(data) == entry["sha256"],
        f"document bytes changed: {entry['path']}",
    )
    document = contract.parse_json(data, f"document {entry['path']}")
    document_records.validate_document(document, f"document {entry['path']}")
    contract.require(
        document.get("schema") == entry["schema"],
        f"document schema changed: {entry['path']}",
    )
    contract.require(
        contract.doc_digest(document) == digest,
        f"document digest changed: {entry['path']}",
    )
    return document


def resolve_file(
    run_dir: Path,
    ref: dict,
    owner: str | None = None,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> bytes:
    """Resolve a ``FileRef``: index entry, declared size, then exact bytes."""

    records.validate_file_ref(ref, "file reference")
    bundle = str(ref["bundle"])
    index = load_index(
        run_dir,
        bundle,
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    _check_expected_bundle(ref, index, owner)
    entries = [
        entry
        for entry in index["files"]
        if entry["path"] == ref.get("path")
        and entry["sha256"] == ref.get("sha256")
        and entry["bytes"] == ref.get("bytes")
    ]
    contract.require(
        len(entries) == 1, f"file reference is ambiguous or absent: {ref.get('path')}"
    )
    data = _read_member(
        run_dir,
        bundle,
        entries[0]["path"],
        f"file {entries[0]['path']}",
        io_mode=io_mode,
        events=events,
        max_bytes=max_bytes,
    )
    contract.require(
        len(data) == ref["bytes"] and contract.sha256_bytes(data) == ref["sha256"],
        f"file bytes changed: {entries[0]['path']}",
    )
    return data


def _check_expected_bundle(ref: dict, index: dict, owner: str | None) -> None:

    expected = ref.get("expected_bundle")
    bundle = str(ref.get("bundle"))
    contract.require(
        isinstance(owner, str) and owner,
        f"reference resolution requires an explicit enclosing bundle: {bundle}",
    )
    contract.validate_rel_path(owner)
    if expected is None:
        contract.require(
            bundle == owner,
            f"a cross-bundle reference must pin expected_bundle: {bundle}",
        )
        return
    contract.require(
        bundle != owner,
        f"a same-bundle reference must not pin its own digest: {bundle}",
    )
    contract.require(
        expected == contract.bundle_digest(index),
        f"cross-bundle expected_bundle does not match {bundle}: expected {expected}",
    )


def _is_file_ref(value: object) -> bool:
    return isinstance(value, dict) and {"bundle", "path", "sha256", "bytes"} <= set(
        value
    )


def _is_doc_ref(value: object) -> bool:
    return (
        isinstance(value, dict)
        and {"bundle", "digest"} <= set(value)
        and isinstance(value.get("digest"), str)
        and str(value["digest"]).startswith("v1:")
    )


def iter_references(value: object) -> Iterator[tuple[str, dict]]:
    """Yield every ``DocRef``/``FileRef`` at any depth of one document."""

    if _is_file_ref(value):
        yield "file", value
        return
    if _is_doc_ref(value):
        yield "doc", value
        return
    if isinstance(value, dict):
        for child in value.values():
            yield from iter_references(child)
    elif isinstance(value, list):
        for child in value:
            yield from iter_references(child)


def verify_bundle_references(
    run_dir: Path,
    name: str,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
    max_entries: int | None = None,
) -> dict:
    with contract.bounded_reads(max_bytes) as observations:
        return _verify_references(
            run_dir,
            name,
            (),
            io_mode,
            events,
            max_entries,
            observations,
            max_bytes,
        )


def verify_content_references(run_dir: Path, value: object, *, owner: str) -> None:
    """Submit parsed content roots under their real enclosing bundle coordinate."""
    _verify_references(run_dir, owner, iter_references(value))


def _verify_references(
    run_dir,
    name,
    roots,
    io_mode="normalized",
    events=None,
    max_entries=None,
    observations=None,
    max_bytes=None,
):
    contract.validate_rel_path(name)
    contract.require(
        max_entries is None or type(max_entries) is int and max_entries > 0,
        "reference entry bound must be positive",
    )
    start = len(observations) if observations is not None else 0
    root_index = load_index(run_dir, name, io_mode=io_mode, events=events)
    expanded_bundles = {name}
    expanded_documents: set[tuple[str, str]] = set()
    visited: dict[tuple[str, str], dict] = {}
    pending: list[tuple[str, str, dict]] = []
    _schedule_index_edges(run_dir, name, pending, io_mode, events)
    pending.extend((kind, name, ref) for kind, ref in roots)
    reached = 0
    while pending:
        check_deadline()
        reached += 1
        contract.require(
            max_entries is None or reached + len(pending) <= max_entries,
            "reference traversal exceeds its entry bound",
        )
        if observations is not None and max_bytes is not None:
            contract.require(
                sum(row["bytes"] for row in observations[start:]) <= 8 * max_bytes,
                "reference traversal exceeds its aggregate read bound",
            )
        kind, owner, ref = pending.pop()
        bundle = str(ref["bundle"])
        if kind == "doc":
            document = resolve_document(
                run_dir,
                ref,
                owner=owner,
                io_mode=io_mode,
                events=events,
            )
            visited[(bundle, str(ref["digest"]))] = document
            if bundle not in expanded_bundles:
                expanded_bundles.add(bundle)
                _schedule_index_edges(run_dir, bundle, pending, io_mode, events)
            key = (bundle, str(ref["digest"]))
            if key in expanded_documents:
                continue
            expanded_documents.add(key)
            pending.extend(
                (kind, bundle, nested) for kind, nested in iter_references(document)
            )
        else:
            resolve_file(
                run_dir,
                ref,
                owner=owner,
                io_mode=io_mode,
                events=events,
            )
            if bundle not in expanded_bundles:
                expanded_bundles.add(bundle)
                _schedule_index_edges(run_dir, bundle, pending, io_mode, events)
    verify_visited_documents(run_dir, visited, io_mode=io_mode, events=events)
    if observations is not None and max_bytes is not None:
        contract.require(
            sum(row["bytes"] for row in observations[start:]) <= 8 * max_bytes,
            "reference traversal exceeds its aggregate read bound",
        )
    return root_index


def _schedule_index_edges(run_dir, bundle, pending, io_mode="normalized", events=None):
    index = load_index(run_dir, bundle, io_mode=io_mode, events=events)
    if index["role"] == "preparation":
        _preparation_digests(run_dir, io_mode=io_mode, events=events)
    pending.extend(
        ("doc", bundle, contract.doc_ref(bundle, entry["digest"], None))
        for entry in index["documents"]
    )
    pending.extend(("file", bundle, dict(entry)) for entry in index["files"])
    pending.extend(("doc", bundle, dict(ref)) for ref in index["upstream"])


def _preparation_digests(
    run_dir: Path,
    *,
    io_mode: Literal["normalized", "exact"] = "normalized",
    events: list[FailureEvent] | None = None,
    max_bytes: int | None = None,
) -> dict[str, str]:
    digests = {}
    for name, origin in (("problem", "approved"), ("answer", "evaluator")):
        index = load_index(
            run_dir,
            name,
            io_mode=io_mode,
            events=events,
            max_bytes=max_bytes,
        )
        contract.require(
            index["role"] == name and index["origin"] == origin,
            f"Preparation requires the fixed {origin} {name}",
        )
        digests[name] = contract.bundle_digest(index)
    return digests
