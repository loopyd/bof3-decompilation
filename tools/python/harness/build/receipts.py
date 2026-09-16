"""Bind grouped producer outputs to fresh configured provenance, never reuse rights."""

from __future__ import annotations

import hashlib
import json
import os
import stat
import sys
from contextlib import closing, nullcontext
from pathlib import Path

from harness.build.dispatch import close_dispatch, prepare_dispatch, validate_dispatch
from harness.build.driver import execute_compiler
from harness.build.execution import verify_execution
from harness.build.preservation import hash_preservation, validate_fingerprint
from harness.common.deadlines import check_deadline, use_deadline
from harness.common.files import atomic_write, read_file
from harness.common.lease import ACTIVE_WRITER, acquire_writer, verify_writer
from harness.common.observation import PathWatch
from harness.common.paths import leaf_stat
from harness.io import unique_object

_SCHEMA = "bof3.grouped-producer/v5"
_LIMIT = 16 * 1024 * 1024


def _select_receipt(dispatch) -> Path:
    if not dispatch.state["grouped"] or dispatch.output is None:
        raise ValueError("producer receipts require a guarded grouped object")
    receipt = dispatch.output.with_name(dispatch.output.name + ".producer.json")
    for path in (
        dispatch.output,
        dispatch.output.with_name(dispatch.output.name + ".s"),
        receipt,
    ):
        if str(path) in dispatch.state["observations"]:
            raise ValueError(
                f"producer artifact collides with a preserved input: {path}"
            )
    return receipt


def _collect_outputs(dispatch) -> dict:
    result = {}
    for path in (
        dispatch.output,
        dispatch.output.with_name(dispatch.output.name + ".s"),
    ):
        check_deadline()
        name = path.relative_to(dispatch.root).as_posix()
        status = leaf_stat(dispatch.root, name)
        if status is None or not stat.S_ISREG(status.st_mode) or status.st_nlink != 1:
            raise ValueError(f"missing or unsafe producer output: {name}")
        content = read_file(dispatch.root, name, max_bytes=64 * 1024 * 1024)
        if not content:
            raise ValueError(f"empty producer output: {name}")
        result[name] = {
            "sha256": hashlib.sha256(content).hexdigest(),
            "mode": stat.S_IMODE(status.st_mode),
        }
    return result


def _write_receipt(root: Path, receipt: Path, content: bytes, expected: bytes | None):
    quarantine = atomic_write(
        root, receipt.relative_to(root).as_posix(), content, expected=expected
    )
    if quarantine is not None:
        sys.stderr.write(f"producer receipt recovery retained: {quarantine}\n")


def _verify_execution(dispatch, outputs: dict, execution: object) -> str:
    return verify_execution(
        dispatch.invocation,
        dispatch.fingerprint,
        dispatch.state["grouped"],
        execution,
        outputs={
            str(dispatch.root / name): state["sha256"]
            for name, state in outputs.items()
        },
    )


def _encode_receipt(dispatch, outputs: dict, execution: object) -> bytes:
    _verify_execution(dispatch, outputs, execution)
    document = {
        "schema": _SCHEMA,
        "status": "configured-provenance",
        "reusable": False,
        "invocation": dispatch.state,
        "outputs": outputs,
        "execution": execution,
    }
    content = (
        json.dumps(document, sort_keys=True, indent=2, allow_nan=False) + "\n"
    ).encode()
    if len(content) > _LIMIT:
        raise ValueError("producer receipt exceeds its capture bound")
    return content


def _resolve_lease(root: Path):
    if ACTIVE_WRITER.get() is not None:
        verify_writer(root)
        return nullcontext()
    return acquire_writer(root)


def _run_producer(root: Path, arguments: list[str]) -> tuple[int, dict | None]:
    root = root.resolve()
    deadline = os.environ.get("BOF3_WORK_DEADLINE")
    receipt = None
    published = None
    try:
        with use_deadline(float(deadline) if deadline else None):
            check_deadline()
            with _resolve_lease(root):
                dispatch = prepare_dispatch(root, arguments)
                try:
                    receipt = _select_receipt(dispatch)
                    dispatch.programs.protect_outputs(
                        [receipt, root / "out/reviews/evidence/quarantine"]
                    )
                    previous = read_file(
                        root,
                        receipt.relative_to(root).as_posix(),
                        missing_ok=True,
                        max_bytes=_LIMIT,
                    )
                    check_deadline()
                    _write_receipt(root, receipt, b"", previous)
                    check_deadline()
                    validate_dispatch(dispatch)
                    status, execution = execute_compiler(root, arguments)
                    if status:
                        return status, None
                    validate_dispatch(dispatch)
                    outputs = {
                        dispatch.output,
                        dispatch.output.with_name(dispatch.output.name + ".s"),
                    }
                    with closing(PathWatch(outputs)) as watch:
                        outputs = _collect_outputs(dispatch)
                        content = _encode_receipt(dispatch, outputs, execution)
                        watch.validate()
                        validate_dispatch(dispatch)
                        verify_writer(root)
                        check_deadline()
                        produced = {
                            "receipt": str(receipt),
                            "sha256": hashlib.sha256(content).hexdigest(),
                            "invocation_fingerprint": hash_preservation(dispatch.state),
                            "execution_fingerprint": _verify_execution(
                                dispatch, outputs, execution
                            ),
                        }
                        published = content
                        _write_receipt(root, receipt, content, b"")
                        watch.validate()
                        validate_dispatch(dispatch)
                        if (
                            read_file(
                                root,
                                receipt.relative_to(root).as_posix(),
                                max_bytes=_LIMIT,
                            )
                            != content
                        ):
                            raise ValueError(
                                "producer receipt changed during publication"
                            )
                        check_deadline()
                finally:
                    close_dispatch(dispatch)
                verify_writer(root)
            check_deadline()
        return 0, produced
    except BaseException:
        if receipt is not None and published is not None:
            try:
                with _resolve_lease(root):
                    current = read_file(
                        root,
                        receipt.relative_to(root).as_posix(),
                        missing_ok=True,
                        max_bytes=_LIMIT,
                    )
                    if current == published:
                        _write_receipt(root, receipt, b"", published)
                    elif current != b"":
                        raise ValueError("producer receipt recovery state is uncertain")
                    verify_writer(root)
            except BaseException as error:
                raise RuntimeError(
                    f"producer receipt invalidation unconfirmed; inspect {receipt} and retained recovery"
                ) from error
        raise


def run_producer(root: Path, arguments: list[str]) -> int:
    """Always compile under a writer lease, recording only configured provenance."""
    status, _ = _run_producer(root, arguments)
    return status


def produce_object(root: Path, arguments: list[str]) -> dict:
    """Return the fresh in-process receipt pin, not a later unbound receipt read."""
    status, produced = _run_producer(root, arguments)
    if status or produced is None:
        raise RuntimeError(f"grouped compilation failed with exit status {status}")
    return produced


def verify_production(root: Path, arguments: list[str], expected_sha256: str) -> dict:
    """Verify externally pinned configured provenance without granting object reuse."""
    validate_fingerprint(expected_sha256)
    root = root.resolve()
    deadline = os.environ.get("BOF3_WORK_DEADLINE")
    with use_deadline(float(deadline) if deadline else None):
        check_deadline()
        with _resolve_lease(root):
            dispatch = prepare_dispatch(root, arguments)
            try:
                receipt = _select_receipt(dispatch)
                paths = {
                    receipt,
                    dispatch.output,
                    dispatch.output.with_name(dispatch.output.name + ".s"),
                }
                with closing(PathWatch(paths)) as watch:
                    content = read_file(
                        root, receipt.relative_to(root).as_posix(), max_bytes=_LIMIT
                    )
                    if hashlib.sha256(content).hexdigest() != expected_sha256:
                        raise ValueError(
                            "producer receipt differs from its external pin"
                        )
                    try:
                        document = json.loads(content, object_pairs_hook=unique_object)
                    except (ValueError, RecursionError) as error:
                        raise ValueError("invalid producer receipt JSON") from error
                    if not isinstance(document, dict):
                        raise ValueError("producer receipt must be an object")
                    outputs = _collect_outputs(dispatch)
                    execution = document.get("execution")
                    expected = json.loads(_encode_receipt(dispatch, outputs, execution))
                    if hash_preservation(document) != hash_preservation(expected):
                        raise ValueError(
                            "producer receipt differs from current configured provenance"
                        )
                    watch.validate()
                    validate_dispatch(dispatch)
                    verify_writer(root)
                    check_deadline()
                    result = {
                        "status": "configured-provenance-verified",
                        "reusable": False,
                        "receipt": str(receipt),
                        "sha256": expected_sha256,
                        "invocation_fingerprint": hash_preservation(dispatch.state),
                        "execution_fingerprint": _verify_execution(
                            dispatch, outputs, execution
                        ),
                    }
            finally:
                close_dispatch(dispatch)
        check_deadline()
        return result
