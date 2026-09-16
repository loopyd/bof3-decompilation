"""Run the GCC, maspsx and assembler pipeline behind one preservation gate."""

from __future__ import annotations

import os
import shutil
import sys
import tempfile
from pathlib import Path

from harness.build.arguments import inspect_arguments
from harness.build.dispatch import (
    Dispatch,
    close_dispatch,
    prepare_dispatch,
    validate_dispatch,
)
from harness.build.execution import Execution, capture_stage, complete_stage
from harness.build.invocation import Stage, select_mode
from harness.build.translation import prepare_translation
from harness.common.deadlines import check_deadline, resolve_deadline, use_deadline
from harness.common.files import atomic_write, read_file
from harness.common.process import ProcessCleanupError, run_bounded


class CompilerFailure(RuntimeError):
    def __init__(self, status: int) -> None:
        super().__init__(f"compiler stage exited {status}")
        self.status = status


def _execute(
    arguments: list[str],
    environment: dict[str, str],
    *,
    cwd: Path,
    input_text: str | None = None,
    supervisor: tuple[str, ...] | None = None,
) -> str:
    result = run_bounded(
        cwd,
        arguments,
        timeout=300.0,
        deadline=resolve_deadline(),
        input_data=input_text.encode() if input_text is not None else None,
        output_limit=64 * 1024 * 1024,
        env=environment,
        supervisor=supervisor,
    )
    failed = result["exit_code"] != 0 or result["failure"]
    status = result["exit_code"] or (124 if result["failure"] == "timeout" else 2)
    status = 128 - status if status < 0 else status
    try:
        if result["stderr"]:
            sys.stderr.write(result["stderr"])
        if failed:
            if result["stdout"]:
                sys.stdout.write(result["stdout"])
            if result["failure"]:
                sys.stderr.write(f"compiler stage failure: {result['failure']}\n")
    except BrokenPipeError as error:
        if failed:
            raise CompilerFailure(status) from error
        raise
    if failed:
        raise CompilerFailure(status)
    return result["stdout"]


def _execute_stage(stage: Stage, temporary: Path, execution: Execution) -> str:
    entry, arguments, environment, input_text = capture_stage(stage, temporary)
    execution.check_stage(stage, entry)
    rendered = _execute(
        arguments,
        environment,
        cwd=stage.cwd,
        input_text=input_text,
        supervisor=stage.supervisor,
    )
    execution.events.append(complete_stage(entry, stage, temporary, rendered))
    return rendered


def _publish(output: Path, content: bytes, original: bytes | None) -> None:
    check_deadline()
    quarantine = atomic_write(output.parent, output.name, content, expected=original)
    try:
        check_deadline()
    except BaseException:
        sys.stderr.write(
            f"compiler publication crossed cutoff; inspect output {output}; quarantine {quarantine!r}\n"
        )
        raise
    if quarantine is not None:
        retained = output.parent / quarantine
        if (
            read_file(retained.parent, retained.name, max_bytes=64 * 1024 * 1024)
            == original
        ):
            retained.unlink()
    check_deadline()


def _publish_recorded(
    execution: Execution,
    output: Path,
    content: bytes,
    original: bytes | None,
    artifact: str,
) -> None:
    execution.check_publication(content, artifact)
    _publish(output, content, original)
    execution.record_publication(output, content, artifact)


def _run(
    root: Path, arguments: list[str], *, dispatch: Dispatch | None = None
) -> Execution:
    owns_dispatch = dispatch is None
    if dispatch is not None:
        if dispatch.root != root or dispatch.arguments != tuple(arguments):
            raise ValueError("borrowed compiler dispatch differs from the invocation")
        validate_dispatch(dispatch)
    cwd = Path.cwd()
    operands = inspect_arguments(arguments, cwd=cwd)
    sources, output = list(operands.sources), operands.output
    select_mode(
        arguments, sources=sources, output=output, cwd=cwd, environment=dict(os.environ)
    )
    temporary = None
    cleanup = True
    publication_started = False
    try:
        if owns_dispatch:
            dispatch = prepare_dispatch(root, arguments)
        temporary_parent = Path(tempfile.gettempdir())
        dispatch.programs.protect_temporary(
            temporary_parent,
            ".bof3-cc-",
            inputs=tuple(Path(name) for name in dispatch.state["observations"]),
        )
        temporary = Path(tempfile.mkdtemp(prefix=".bof3-cc-", dir=temporary_parent))
        if dispatch.state["working_directory"]["path"] != str(cwd):
            raise ValueError("compiler working directory changed during preparation")
        invocation = dispatch.invocation
        execution = Execution(
            invocation, dispatch.fingerprint, dispatch.state["grouped"], temporary
        )
        validate_dispatch(dispatch)
        if invocation.mode == "direct":
            original = (
                read_file(
                    output.parent,
                    output.name,
                    missing_ok=True,
                    max_bytes=64 * 1024 * 1024,
                )
                if output
                else None
            )
            staged_output = temporary / "direct-output"
            if original is not None:
                (temporary / "previous-object").write_bytes(original)
            rendered = _execute_stage(invocation.stages[0], temporary, execution)
            validate_dispatch(dispatch)
            if output:
                publication_started = True
                _publish_recorded(
                    execution,
                    output,
                    read_file(
                        temporary, staged_output.name, max_bytes=64 * 1024 * 1024
                    ),
                    original,
                    "direct-output",
                )
            validate_dispatch(dispatch)
            sys.stdout.write(rendered)
            sys.stdout.flush()
            validate_dispatch(dispatch)
            return execution
        source = invocation.source
        original = read_file(
            output.parent, output.name, missing_ok=True, max_bytes=64 * 1024 * 1024
        )
        if original is not None:
            (temporary / "previous-object").write_bytes(original)
        if invocation.mode == "assembly":
            rendered = _execute_stage(invocation.stages[0], temporary, execution)
        else:
            retained_assembly = output.with_name(output.name + ".s")
            original_assembly = read_file(
                retained_assembly.parent,
                retained_assembly.name,
                missing_ok=True,
                max_bytes=64 * 1024 * 1024,
            )
            if original_assembly is not None:
                (temporary / "previous-assembly").write_bytes(original_assembly)
            _execute_stage(invocation.stages[0], temporary, execution)
            validate_dispatch(dispatch)
            translated = _execute_stage(invocation.stages[1], temporary, execution)
            unpartitioned = translated
            translated = prepare_translation(
                root, source, translated, dispatch=dispatch
            )
            validate_dispatch(dispatch)
            partitioned = temporary / "partitioned.s"
            partitioned.write_text(translated)
            execution.record_partition(
                unpartitioned,
                read_file(temporary, "partitioned.s", max_bytes=64 * 1024 * 1024),
            )
            rendered = _execute_stage(invocation.stages[2], temporary, execution)

        validate_dispatch(dispatch)
        content = read_file(temporary, "translation.o", max_bytes=64 * 1024 * 1024)
        if dispatch.state["grouped"] and not content:
            raise ValueError("grouped assembler produced an empty object")
        validate_dispatch(dispatch)
        if source.suffix == ".c":
            publication_started = True
            _publish_recorded(
                execution,
                retained_assembly,
                read_file(temporary, "compiler.s", max_bytes=64 * 1024 * 1024),
                original_assembly,
                "compiler.s",
            )
        validate_dispatch(dispatch)
        publication_started = True
        _publish_recorded(execution, output, content, original, "translation.o")
        validate_dispatch(dispatch)
        sys.stdout.write(rendered)
        sys.stdout.flush()
        validate_dispatch(dispatch)
        return execution
    except ProcessCleanupError:
        cleanup = False
        sys.stderr.write(
            f"compiler process cleanup unconfirmed; staging retained: {temporary}\n"
        )
        raise
    except BaseException:
        if publication_started:
            cleanup = False
            sys.stderr.write(
                f"compiler publication failed; inspect output {output} and diagnostic; staging retained: {temporary}\n"
            )
        raise
    finally:
        try:
            if cleanup and temporary is not None:
                shutil.rmtree(temporary)
                if dispatch is not None and sys.exc_info()[0] is None:
                    validate_dispatch(dispatch)
        except BaseException:
            sys.stderr.write(
                f"compiler cleanup or terminal check failed; inspect output {output}; staging may have been removed: {temporary}\n"
            )
            raise
        finally:
            if owns_dispatch and dispatch is not None:
                close_dispatch(dispatch)


def execute_compiler(
    root: Path, arguments: list[str], *, dispatch: Dispatch | None = None
) -> tuple[int, dict | None]:
    """Return checked execution evidence; a supplied dispatch remains caller-owned."""
    deadline = os.environ.get("BOF3_WORK_DEADLINE")
    try:
        with use_deadline(float(deadline) if deadline else None):
            execution = _run(root.resolve(), arguments, dispatch=dispatch)
            check_deadline()
            evidence = execution.finish()
            check_deadline()
            return 0, evidence
    except CompilerFailure as error:
        return error.status, None
    except BrokenPipeError:
        return 1, None


def run_compiler(root: Path, arguments: list[str]) -> int:
    """Preserve the compiler CLI exit-status interface without dropping its gates."""
    status, _ = execute_compiler(root, arguments)
    return status
