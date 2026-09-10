"""Capture a fresh parent-owned native diagnosis before any retained-lift edit."""

from __future__ import annotations

from pathlib import Path
from contextlib import ExitStack
import os
import secrets
from typing import Callable

from harness.common.deadlines import bind_deadline, check_deadline, resolve_deadline
from harness.common.digests import digest
from harness.common.directory import open_parent_fd, validate_repo_path
from harness.common.files import read_file
from harness.common.lease import acquire_writer, require_writer
from harness.common.process import ProcessCleanupError
from harness.context.capabilities import OUTPUTS, capture_policy, verify_policy
from harness.context.journal import open_stream, write_chunk, write_record
from harness.decomp.gates import check_candidate
from harness.decomp.missions import capture_mission, verify_mission


def diagnose_mission(
    root: Path,
    record: dict,
    *,
    expected_mission_digest: str,
    policy: dict,
    expected_policy_digest: str,
    publish: Callable[[str, dict], None],
    stream_gate: Callable[[str, str, bytes], None],
) -> dict:
    require_writer(root)
    check_deadline()
    verify_mission(root, record, expected_mission_digest)
    if not record["facts"]["existing_source"]:
        raise ValueError("native pre-edit diagnosis requires an existing claimed lift")
    verify_policy(root, policy, expected_policy_digest, before_dispatch=True)
    checked = check_candidate(
        root,
        record,
        expected_mission_digest=expected_mission_digest,
        policy=policy,
        expected_policy_digest=expected_policy_digest,
        publish=publish,
        stream_gate=stream_gate,
    )
    if checked["post"]["changed_paths"]:
        raise ValueError("pre-edit diagnosis observed a changed candidate")
    verify_mission(root, record, expected_mission_digest)
    verify_policy(root, policy, expected_policy_digest, before_dispatch=True)
    check_deadline()
    require_writer(root)
    result = {
        **{key: value for key, value in checked.items() if key != "digest"},
        "schema": "bof3.lift-diagnosis/v1",
        "status": "diagnosed-not-accepted",
        "source_write_authorized": False,
        "model_dispatched": False,
    }
    return {**result, "digest": digest(result)}


def reserve_directory(root: Path, output: str) -> None:
    validate_repo_path(output)
    if (
        not output.startswith("out/reviews/lift-diagnosis/")
        or len(Path(output).parts) != 4
    ):
        raise ValueError(
            "diagnosis output must be a fresh out/reviews/lift-diagnosis/NAME directory"
        )
    parent, leaf = open_parent_fd(root, output, create=True)
    try:
        try:
            os.mkdir(leaf, dir_fd=parent)
        except FileExistsError as error:
            raise ValueError(
                "diagnosis output already exists; retain prior evidence"
            ) from error
        os.fsync(parent)
    finally:
        os.close(parent)


@bind_deadline
def run_diagnosis(
    root: Path,
    request: dict,
    *,
    expected_request_digest: str,
    output: str,
    deadline: float,
) -> dict:
    if digest(request) != expected_request_digest:
        raise ValueError("lift diagnosis request pin drifted")
    if resolve_deadline() is None:
        raise ValueError("lift diagnosis requires the original parent work cutoff")
    reserved = False
    try:
        with acquire_writer(root):
            record = capture_mission(root, request)
            if not record["facts"]["existing_source"]:
                raise ValueError("native diagnosis requires an existing claimed lift")
            reserve_directory(root, output)
            reserved = True
            write_record(root, output + "/mission.json", record)
            temporary = "out/dispatch/lift-diagnosis-" + secrets.token_hex(12) + "/tmp"
            for name in (*OUTPUTS, temporary):
                descriptor, leaf = open_parent_fd(
                    root, name + "/.directory-probe", create=True
                )
                os.close(descriptor)
            policy = capture_policy(
                root, request["paths"], list(record["execution"]["inputs"]), temporary
            )
            policy_digest = digest(policy)
            write_record(
                root,
                output + "/policy.json",
                {"policy": policy, "digest": policy_digest},
            )
            write_record(
                root,
                output + "/invocation.json",
                {
                    "request_digest": expected_request_digest,
                    "work_deadline": resolve_deadline(),
                    "budget_debit_performed": False,
                    "model_dispatched": False,
                },
            )
            with ExitStack() as stack:
                streams = {}

                def publish(name: str, value: dict) -> None:
                    if name.endswith("-started"):
                        label = name.removesuffix("-started")
                        for stream in ("stdout", "stderr"):
                            streams[label, stream] = stack.enter_context(
                                open_stream(root, f"{output}/{label}-{stream}.log")
                            )
                    elif name.endswith("-terminal"):
                        label = name.removesuffix("-terminal")
                        for stream in ("stdout", "stderr"):
                            if (
                                read_file(root, f"{output}/{label}-{stream}.log")
                                != value[stream].encode()
                            ):
                                raise ValueError(
                                    "retained diagnosis stream differs from native output"
                                )
                    write_record(root, f"{output}/{name}.json", value)

                def stream_gate(label: str, stream: str, chunk: bytes) -> None:
                    write_chunk(streams[label, stream], chunk)

                result = diagnose_mission(
                    root,
                    record,
                    expected_mission_digest=record["digest"],
                    policy=policy,
                    expected_policy_digest=policy_digest,
                    publish=publish,
                    stream_gate=stream_gate,
                )
            write_record(root, output + "/result.json", result)
            check_deadline()
            require_writer(root)
        return {
            "directory": output,
            "mission_digest": record["digest"],
            "policy_digest": policy_digest,
            "diagnosis_digest": result["digest"],
            "byte_exact": result["byte_exact"],
            "match_percent": result["match_percent"],
            "source_accepted": False,
            "model_dispatched": False,
        }
    except BaseException as error:
        if reserved:
            try:
                write_record(
                    root,
                    output + "/failure.json",
                    {
                        "error_type": type(error).__name__,
                        "cleanup_unconfirmed": isinstance(error, ProcessCleanupError),
                        "source_accepted": False,
                        "restoration_authorized": False,
                        "retry_authorized": False,
                    },
                )
            except BaseException:
                if isinstance(error, ProcessCleanupError):
                    raise error
                raise
        raise
