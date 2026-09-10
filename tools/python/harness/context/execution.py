"""Owned Codex process execution with exclusive, fsynced evidence streams."""

from __future__ import annotations

from contextlib import ExitStack
from pathlib import Path
import time

from harness.common.deadlines import check_deadline
from harness.common.files import read_file
from harness.common.lease import require_writer
from harness.common.process import run_bounded
from harness.context.codex import summarize_events
from harness.context.journal import STREAMS, open_stream, write_chunk, write_record


def run_native(
    root: Path, directory: str, command: list[str], prompt: bytes, deadline: float
) -> dict:
    with ExitStack() as stack:
        streams = {
            name: stack.enter_context(open_stream(root, directory + "/" + filename))
            for name, filename in STREAMS.items()
        }

        def retain_output(name: str, chunk: bytes) -> None:
            write_chunk(streams[name], chunk)

        def retain_spawn(process) -> None:
            require_writer(root)
            write_record(
                root,
                directory + "/spawn.json",
                {
                    "supervisor_pid": process.pid,
                    "saved_pid_is_not_termination_authority": True,
                },
            )

        check_deadline()
        require_writer(root)
        observed = run_bounded(
            root,
            command,
            timeout=max(0.001, deadline - time.monotonic()),
            deadline=deadline,
            output_limit=1024 * 1024,
            errors="strict",
            input_data=prompt,
            on_output=retain_output,
            on_spawn=retain_spawn,
        )
    if observed["failure"] or observed["exit_code"] != 0:
        raise RuntimeError(
            "native Codex failed; retain debit and candidate for inspection"
        )
    events = summarize_events(observed["stdout"])
    if any(
        read_file(root, directory + "/" + filename) != observed[name].encode()
        for name, filename in STREAMS.items()
    ):
        raise ValueError("retained Codex stream differs from observed output")
    return events
