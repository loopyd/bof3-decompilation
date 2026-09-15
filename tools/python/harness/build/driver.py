"""Run the GCC, maspsx and assembler pipeline behind one preservation gate."""

from __future__ import annotations

import os
import shutil
import sys
import tempfile
from pathlib import Path

from harness.build.dispatch import (
    close_dispatch,
    inspect_arguments,
    prepare_dispatch,
    validate_dispatch,
)
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
) -> str:
    result = run_bounded(
        cwd,
        arguments,
        timeout=300.0,
        deadline=resolve_deadline(),
        input_data=input_text.encode() if input_text is not None else None,
        output_limit=64 * 1024 * 1024,
        env=environment,
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


def _compiler_environment(
    root: Path, compiler: Path, environment: dict[str, str]
) -> dict[str, str]:
    executable = str(compiler)
    resolved = shutil.which(executable) if "/" not in executable else executable
    if resolved is None:
        raise ValueError(f"missing compiler: {compiler}")
    directory = str(Path(resolved).absolute().parent)
    return {
        **environment,
        "GCC_EXEC_PREFIX": directory + "/",
        "COMPILER_PATH": directory,
        "PATH": os.pathsep.join(
            [
                directory,
                str(root / "toolchains/psn00b_toolchain/bin"),
                environment.get("PATH", ""),
            ]
        ),
    }


def _resolve_version(arguments: list[str]) -> str:
    version = os.environ.get("ASPSX_VERSION", "2.56")
    for argument in arguments:
        if argument.startswith("-Wa,"):
            for flag in argument[4:].split(","):
                if flag.startswith("--aspsx-version="):
                    version = flag.split("=", 1)[1]
    return version


def _replace_output(arguments: list[str], output: Path) -> list[str]:
    result = []
    skip = False
    for argument in arguments:
        if skip:
            skip = False
        elif argument == "-o":
            skip = True
        elif not argument.startswith("-o"):
            result.append(argument)
    return [*result, "-o", str(output)]


def _compiler_arguments(arguments: list[str], assembly: Path) -> list[str]:
    result = []
    skip_output = False
    for argument in arguments:
        if skip_output:
            skip_output = False
        elif argument == "-o":
            skip_output = True
        elif argument == "-c" or argument.startswith("-o"):
            continue
        elif argument.startswith("-Wa,"):
            retained = [
                flag
                for flag in argument[4:].split(",")
                if flag
                and flag != "--expand-div"
                and not flag.startswith("--aspsx-version=")
            ]
            if retained:
                result.append("-Wa," + ",".join(retained))
        else:
            result.append(argument)
    return [*result, "-S", "-o", str(assembly)]


def _assembler_arguments(arguments: list[str], output: Path) -> list[str]:
    flags = []
    for argument in arguments:
        if argument.startswith("-Wa,"):
            flags.extend(
                flag
                for flag in argument[4:].split(",")
                if flag
                and flag != "--expand-div"
                and not flag.startswith("--aspsx-version=")
            )
    return [*flags, "-o", str(output)]


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


def _run(root: Path, arguments: list[str]) -> int:
    cwd = Path.cwd()
    sources, output = inspect_arguments(arguments, cwd=cwd)
    named = [path for path in sources if path.suffix == ".c"]
    named.extend(
        cwd / argument
        for argument in arguments
        if not argument.startswith("-")
        and argument.endswith((".s", ".S"))
        and cwd / argument != output
    )
    compile_mode = "-c" in arguments and not any(
        flag in arguments for flag in ("-E", "-S", "-M", "-MM")
    )
    if (
        compile_mode
        and not os.environ.get("PSX_CC_DRIVER")
        and (len(named) != 1 or output is None)
    ):
        raise ValueError(
            "bin/cc expects exactly one .c/.s/.S source and a -o output for -c"
        )
    if (
        not compile_mode
        and not os.environ.get("PSX_CC_DRIVER")
        and not any(argument in {"-E", "-M", "-MM", "-S"} for argument in arguments)
    ):
        raise ValueError(
            "link mode is unsupported; compile with -c and link with bin/ld"
        )
    temporary = Path(tempfile.mkdtemp(prefix=".bof3-cc-"))
    cleanup = True
    dispatch = None
    publication_started = False
    try:
        dispatch = prepare_dispatch(root, arguments)
        if dispatch.state["working_directory"]["path"] != str(cwd):
            raise ValueError("compiler working directory changed during preparation")
        environment = {
            **os.environ,
            "PYTHONDONTWRITEBYTECODE": "1",
            "TMPDIR": str(temporary),
        }
        compiler_environment = _compiler_environment(
            root, dispatch.compiler, environment
        )
        driver = os.environ.get("PSX_CC_DRIVER")
        if driver or not compile_mode:
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
            native_arguments = (
                _replace_output(arguments, staged_output) if output else arguments
            )
            rendered = _execute(
                [dispatch.state["executable"], *native_arguments],
                environment if driver else compiler_environment,
                cwd=cwd,
            )
            validate_dispatch(dispatch)
            if output:
                publication_started = True
                _publish(
                    output,
                    read_file(
                        temporary, staged_output.name, max_bytes=64 * 1024 * 1024
                    ),
                    original,
                )
            validate_dispatch(dispatch)
            sys.stdout.write(rendered)
            sys.stdout.flush()
            validate_dispatch(dispatch)
            return 0
        source = named[0]
        assembler = os.environ.get("PSX_AS", str(root / "bin/as"))
        original = read_file(
            output.parent, output.name, missing_ok=True, max_bytes=64 * 1024 * 1024
        )
        staged_object = temporary / "translation.o"
        if original is not None:
            (temporary / "previous-object").write_bytes(original)
        if source.suffix != ".c":
            operand = next(
                argument
                for argument in arguments
                if not argument.startswith("-") and cwd / argument == source
            )
            rendered = _execute(
                [assembler, *_assembler_arguments(arguments, staged_object), operand],
                environment,
                cwd=cwd,
            )
        else:
            assembly = temporary / "compiler.s"
            retained_assembly = output.with_name(output.name + ".s")
            original_assembly = read_file(
                retained_assembly.parent,
                retained_assembly.name,
                missing_ok=True,
                max_bytes=64 * 1024 * 1024,
            )
            if original_assembly is not None:
                (temporary / "previous-assembly").write_bytes(original_assembly)
            _execute(
                [
                    dispatch.state["executable"],
                    *_compiler_arguments(arguments, assembly),
                ],
                compiler_environment,
                cwd=cwd,
            )
            validate_dispatch(dispatch)
            maspsx = os.environ.get(
                "PSX_MASPSX", str(root / "third_party/maspsx/maspsx.py")
            )
            interpreter = os.environ.get("MASPSX_PYTHON", "python3")
            python_environment = {
                **environment,
                "PYTHONPATH": os.pathsep.join(
                    [str(root / "third_party/maspsx"), str(root / "tools/python")]
                ),
                "PYTHONSAFEPATH": "1",
            }
            translated = _execute(
                [
                    interpreter,
                    "-P",
                    maspsx,
                    "--aspsx-version=" + _resolve_version(arguments),
                    *(
                        ["--expand-div"]
                        if any(
                            "--expand-div" in argument[4:].split(",")
                            for argument in arguments
                            if argument.startswith("-Wa,")
                        )
                        else []
                    ),
                ],
                python_environment,
                cwd=cwd,
                input_text=read_file(
                    temporary, "compiler.s", max_bytes=64 * 1024 * 1024
                ).decode("utf-8"),
            )
            translated = prepare_translation(
                root, source, translated, dispatch=dispatch
            )
            validate_dispatch(dispatch)
            partitioned = temporary / "partitioned.s"
            partitioned.write_text(translated)
            rendered = _execute(
                [
                    assembler,
                    *_assembler_arguments(arguments, staged_object),
                    str(partitioned),
                ],
                environment,
                cwd=cwd,
            )

        validate_dispatch(dispatch)
        content = read_file(temporary, "translation.o", max_bytes=64 * 1024 * 1024)
        if dispatch.state["grouped"] and not content:
            raise ValueError("grouped assembler produced an empty object")
        validate_dispatch(dispatch)
        if source.suffix == ".c":
            publication_started = True
            _publish(
                retained_assembly,
                read_file(temporary, "compiler.s", max_bytes=64 * 1024 * 1024),
                original_assembly,
            )
        validate_dispatch(dispatch)
        publication_started = True
        _publish(output, content, original)
        validate_dispatch(dispatch)
        sys.stdout.write(rendered)
        sys.stdout.flush()
        validate_dispatch(dispatch)
        return 0
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
            if cleanup:
                shutil.rmtree(temporary)
                if dispatch is not None and sys.exc_info()[0] is None:
                    validate_dispatch(dispatch)
        except BaseException:
            sys.stderr.write(
                f"compiler cleanup or terminal check failed; inspect output {output}; staging may have been removed: {temporary}\n"
            )
            raise
        finally:
            if dispatch is not None:
                close_dispatch(dispatch)


def run_compiler(root: Path, arguments: list[str]) -> int:
    """Preserve normal compiler behavior while gating grouped-unit production."""
    deadline = os.environ.get("BOF3_WORK_DEADLINE")
    try:
        with use_deadline(float(deadline) if deadline else None):
            status = _run(root.resolve(), arguments)
            check_deadline()
            return status
    except CompilerFailure as error:
        return error.status
    except BrokenPipeError:
        return 1
