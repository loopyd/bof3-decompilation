"""Bind compiler invocations to stable sources and externally pinned profile history."""

from __future__ import annotations

import hashlib
import os
import shutil
from dataclasses import dataclass
from pathlib import Path

from harness.build.arguments import inspect_arguments, observe_input, read_source
from harness.build.compiler import (
    build_compiler_arguments,
    has_compiler_annotations,
    parse_object_compilers,
    parse_object_flags,
    resolve_compiler_settings,
    validate_object_configuration,
)
from harness.build.invocation import Invocation, plan_dispatch, plan_invocation
from harness.build.programs import ProgramSnapshot
from harness.build.preservation import (
    hash_preservation,
    read_preservation_document,
    verify_preservation,
)
from harness.build.profiles import select_compiler
from harness.build.routing import (
    PRESERVATION_ENVIRONMENT,
    Selection,
    select_preservation,
)
from harness.common.deadlines import check_deadline
from harness.common.files import read_file
from harness.common.observation import PathWatch
from harness.domain.tags import count_function_metadata
from harness.io import repo_layout
from harness.toolchain.gcc_variants import (
    check_host_compatible,
    parse_variants,
    select_variant,
)

_LIMIT = 16 * 1024 * 1024
_ENVIRONMENT = (
    "PSX_CC_DRIVER",
    "PSX_GCC",
    "PSX_AS",
    "PSX_MASPSX",
    "MASPSX_PYTHON",
    "PSX_PYTHON",
    "ASPSX_VERSION",
    "CPATH",
    "C_INCLUDE_PATH",
    "CPLUS_INCLUDE_PATH",
    "OBJC_INCLUDE_PATH",
    "DEPENDENCIES_OUTPUT",
    "SUNPRO_DEPENDENCIES",
    "GCC_EXEC_PREFIX",
    "COMPILER_PATH",
    "BOF3_COMPILER_TRIAL",
    "PATH",
    *PRESERVATION_ENVIRONMENT,
)


def _resolve_executable(name: str) -> Path:
    resolved = shutil.which(name) if "/" not in name else name
    if resolved is None:
        raise ValueError(f"missing compiler executable: {name}")
    path = Path(resolved).resolve(strict=True)
    if not path.is_file():
        raise ValueError(f"compiler executable is not a file: {path}")
    return path


def close_dispatch(dispatch: Dispatch) -> None:
    """Release the invocation-owned directory event fence after terminal checks."""
    try:
        dispatch.watch.close()
    finally:
        try:
            dispatch.programs.close()
        finally:
            dispatch.selection.close()


def _select_annotated_compiler(root, arguments, sources, output, grouped):
    trial = os.environ.get("BOF3_COMPILER_TRIAL")
    if trial:
        directory = Path(trial)
        if (
            grouped
            or len(sources) != 1
            or output is None
            or directory.resolve() != directory
            or not directory.is_dir()
            or not directory.name.startswith("harness-flags-")
            or output.parent != directory
            or output.suffix != ".o"
            or (
                directory.is_relative_to(root)
                and not directory.is_relative_to(root / "out")
            )
        ):
            raise ValueError("compiler trials require one ungrouped scratch output")
    annotated = [
        source
        for source in sources
        if source.suffix == ".c"
        and (grouped or has_compiler_annotations(read_source(source)))
    ]
    if not annotated:
        return None, set()
    if len(sources) != 1 or output is None or os.environ.get("PSX_CC_DRIVER"):
        raise ValueError(
            "annotated compilation requires one configured C source and native driver"
        )
    source = annotated[0]
    paths = {
        root / "config/compiler/object-flags.cmake",
        root / "config/compiler/variants.json",
        *(
            root / name
            for name in (
                "tools/python/harness/build/compiler.py",
                "tools/python/harness/build/dispatch.py",
                "tools/python/harness/domain/functions.py",
                "tools/python/harness/domain/tags.py",
                "tools/python/harness/common/lexicon.py",
                "tools/python/harness/toolchain/gcc.py",
                "tools/python/harness/toolchain/gcc_variants.py",
            )
        ),
    }
    configuration = read_file(
        root, "config/compiler/object-flags.cmake", missing_ok=True, max_bytes=_LIMIT
    )
    text = configuration.decode("utf-8") if configuration is not None else ""
    validate_object_configuration(text)
    settings = resolve_compiler_settings(
        source.relative_to(root).as_posix(),
        read_source(source),
        parse_object_flags(text),
        parse_object_compilers(text),
    )
    layout = repo_layout(root)
    if settings.compiler_id is None:
        compiler = layout.gcc272_psx_root / "gcc"
    else:
        catalog = read_file(root, "config/compiler/variants.json", max_bytes=_LIMIT)
        variant = select_variant(
            parse_variants(catalog.decode("utf-8")), settings.compiler_id
        )
        check_host_compatible(variant.host)
        compiler = variant.install_path(layout) / variant.executable_relpath
    if trial:
        return select_compiler(root) if os.environ.get("PSX_GCC") else compiler, paths
    configured = os.environ.get("PSX_GCC")
    if configured and Path(configured) != compiler:
        raise ValueError("explicit compiler differs from attached function metadata")
    expected = [
        *build_compiler_arguments(root, settings.flags)[1:],
        "-c",
        str(source),
        "-o",
        str(output),
    ]
    if arguments != expected:
        raise ValueError(
            "compiler arguments differ from the ordered metadata profile; use configured commands or explicit flag-search trials"
        )
    return compiler, paths


@dataclass(frozen=True)
class Dispatch:
    root: Path
    arguments: tuple[str, ...]
    compiler: Path
    source: Path | None
    output: Path | None
    record_path: Path | None
    record_fingerprint: str | None
    state: dict
    fingerprint: str
    watch: PathWatch
    selection: Selection
    invocation: Invocation
    programs: ProgramSnapshot


def _observe_working_directory() -> dict:
    cwd = Path.cwd()
    return {"path": str(cwd), "identity": observe_input(cwd)}


def prepare_dispatch(root: Path, arguments: list[str]) -> Dispatch:
    """Freeze an invocation after checking its externally pinned grouped history."""
    root = root.resolve()
    working_directory = _observe_working_directory()
    operands = inspect_arguments(arguments, cwd=Path(working_directory["path"]))
    sources = list(operands.sources)
    grouped = [
        path for path in sources if count_function_metadata(read_source(path)) >= 2
    ]
    selection = select_preservation(root, sources, grouped)
    try:
        return _prepare_selected(
            root, arguments, operands, grouped, selection, working_directory
        )
    except BaseException:
        selection.close()
        raise


def _prepare_selected(
    root, arguments, operands, grouped, selection, working_directory
) -> Dispatch:
    sources, output = list(operands.sources), operands.output
    selection.protect_outputs(output)
    annotated_compiler, metadata_paths = _select_annotated_compiler(
        root, arguments, sources, output, grouped
    )
    compiler = (
        annotated_compiler if annotated_compiler is not None else select_compiler(root)
    )
    record_path, record_fingerprint = selection.record, selection.fingerprint
    source = sources[0] if len(sources) == 1 else None
    proof = None
    if grouped or record_path is not None:
        if len(sources) != 1 or len(grouped) != 1 or record_path is None:
            raise ValueError(
                "grouped compilation requires one source and external preservation history"
            )
        if (
            source.resolve() != source
            or not source.is_relative_to(root / "src/bof3")
            or source.suffix != ".c"
        ):
            raise ValueError(
                "grouped compilation requires a canonical src/bof3/ C source"
            )
        if (
            output is None
            or output.resolve() != output
            or output.suffix != ".o"
            or not any(output.is_relative_to(root / name) for name in ("build", "out"))
        ):
            raise ValueError("grouped output must be a canonical build/ or out/ object")
        proof = verify_preservation(
            root,
            read_preservation_document(record_path),
            expected_fingerprint=record_fingerprint,
            compiler=compiler,
        )
        if proof["destination"] != source.relative_to(root).as_posix():
            raise ValueError(
                "preservation destination differs from the actual compiler source"
            )
        expected = [
            *proof["profile"]["arguments"][1:],
            "-c",
            str(source),
            "-o",
            str(output),
        ]
        if arguments != expected:
            raise ValueError(
                "grouped compiler arguments differ from the ordered preserved invocation"
            )
    executable = _resolve_executable(os.environ.get("PSX_CC_DRIVER") or str(compiler))
    paths = {*operands.inputs, executable, *metadata_paths}
    cwd = Path(working_directory["path"])
    if cwd.parent != cwd:
        paths.add(cwd)
    if selection.routes is not None:
        paths.add(selection.routes)
    if proof:
        paths.update(root / name for name in proof["inputs"])
        paths.add(record_path)
    selection.protect_outputs(output, inputs=paths)
    environment = dict(os.environ)
    invocation = plan_invocation(
        root,
        arguments,
        sources=sources,
        output=output,
        compiler=compiler,
        executable=str(executable),
        cwd=cwd,
        environment=environment,
    )
    programs = ProgramSnapshot(root, invocation)
    watch = None
    try:
        programs.protect_outputs(
            [
                output,
                output.with_name(output.name + ".s"),
                output.with_name(output.name + ".producer.json"),
            ]
            if output
            else []
        )
        if proof and proof["profile"]["programs"] != programs.describe():
            raise ValueError("dispatch programs differ from preserved profile")
        if output is not None:
            output.parent.mkdir(parents=True, exist_ok=True)
        directories = {
            parent for path in paths for parent in path.parents if parent.is_dir()
        }
        directories.add(cwd)
        watch = PathWatch(paths)
        return _capture_dispatch(
            root,
            arguments,
            compiler,
            executable,
            source,
            output,
            record_path,
            record_fingerprint,
            grouped,
            sources,
            operands.describe_forced(),
            paths,
            directories,
            watch,
            selection,
            working_directory,
            environment,
            invocation,
            programs,
        )
    except BaseException:
        try:
            if watch is not None:
                watch.close()
        finally:
            programs.close()
        raise


def _capture_dispatch(
    root,
    arguments,
    compiler,
    executable,
    source,
    output,
    record_path,
    record_fingerprint,
    grouped,
    sources,
    forced,
    paths,
    directories,
    watch,
    selection,
    working_directory,
    environment,
    invocation,
    programs,
) -> Dispatch:
    observations = {
        str(parent): observe_input(parent) for parent in sorted(directories)
    }
    contents = {}
    for path in sorted(paths):
        check_deadline()
        observations[str(path)] = observe_input(path)
        if path.is_file():
            contents[str(path)] = hashlib.sha256(
                read_file(path.parent, path.name, max_bytes=64 * 1024 * 1024)
            ).hexdigest()
    state = {
        "working_directory": working_directory,
        "root": str(root),
        "arguments": arguments,
        "compiler": str(compiler),
        "executable": str(executable),
        "source": str(source) if source else None,
        "output": str(output) if output else None,
        "record": str(record_path) if record_path else None,
        "record_fingerprint": record_fingerprint,
        "grouped": bool(grouped),
        "sources": [str(path) for path in sources],
        "forced_inputs": forced,
        "observations": observations,
        "contents": contents,
        "environment": {name: environment.get(name) for name in _ENVIRONMENT},
        "environment_fingerprint": hash_preservation(environment),
        "routing": selection.describe(),
        "programs": programs.describe(),
    }
    if invocation.describe() != plan_dispatch(state, environment).describe():
        raise ValueError("compiler stage recipe changed during capture")
    state["recipe"] = invocation.describe()
    dispatch = Dispatch(
        root,
        tuple(arguments),
        compiler,
        source,
        output,
        record_path,
        record_fingerprint,
        state,
        hash_preservation(state),
        watch,
        selection,
        invocation,
        programs,
    )
    validate_dispatch(dispatch)
    return dispatch


def validate_dispatch(dispatch: Dispatch) -> None:
    """Reject source, configuration or argument drift through a live invocation."""
    check_deadline()
    dispatch.selection.validate()
    dispatch.selection.protect_outputs(dispatch.output)
    dispatch.watch.validate()
    dispatch.programs.validate(dispatch.invocation)
    if hash_preservation(dispatch.state) != dispatch.fingerprint:
        raise ValueError("compiler invocation binding changed")
    state = dispatch.state
    if state["programs"] != dispatch.programs.describe():
        raise ValueError("compiler program generation binding changed")
    if state["working_directory"] != _observe_working_directory():
        raise ValueError("compiler working directory changed during dispatch")
    dispatch.selection.protect_outputs(
        dispatch.output, inputs=(Path(name) for name in state["observations"])
    )
    operands = inspect_arguments(
        list(dispatch.arguments), cwd=Path(state["working_directory"]["path"])
    )
    sources, output = list(operands.sources), operands.output
    annotated_compiler, _ = _select_annotated_compiler(
        dispatch.root,
        list(dispatch.arguments),
        sources,
        output,
        [
            source
            for source in sources
            if count_function_metadata(read_source(source)) >= 2
        ],
    )
    selected = sources[0] if len(sources) == 1 else None
    if (
        state["sources"] != [str(path) for path in sources]
        or state.get("forced_inputs") != operands.describe_forced()
        or dispatch.source != selected
        or state["source"] != (str(selected) if selected else None)
        or dispatch.output != output
        or state["output"] != (str(output) if output else None)
    ):
        raise ValueError("compiler source or output binding changed")
    if (
        state["record"] != (str(dispatch.record_path) if dispatch.record_path else None)
        or state["record_fingerprint"] != dispatch.record_fingerprint
        or dispatch.record_path != dispatch.selection.record
        or dispatch.record_fingerprint != dispatch.selection.fingerprint
        or state["routing"] != dispatch.selection.describe()
        or dispatch.compiler
        != (
            annotated_compiler
            if annotated_compiler is not None
            else select_compiler(dispatch.root)
        )
        or state["executable"]
        != str(
            _resolve_executable(
                os.environ.get("PSX_CC_DRIVER") or str(dispatch.compiler)
            )
        )
    ):
        raise ValueError("compiler or preservation selection changed")
    if (
        state["arguments"] != list(dispatch.arguments)
        or state["compiler"] != str(dispatch.compiler)
        or state["root"] != str(dispatch.root)
    ):
        raise ValueError("compiler invocation differs from its binding")
    if state.get("environment_fingerprint") != hash_preservation(dict(os.environ)):
        raise ValueError("compiler environment changed during dispatch")
    if (
        dispatch.invocation.describe() != state["recipe"]
        or plan_dispatch(state, dict(os.environ)).describe() != state["recipe"]
    ):
        raise ValueError("compiler stage recipe changed")
    for name, observation in state["observations"].items():
        if observe_input(Path(name)) != observation:
            raise ValueError(f"compiler input or ancestor changed: {name}")
    for name, checksum in state["contents"].items():
        path = Path(name)
        if (
            hashlib.sha256(
                read_file(path.parent, path.name, max_bytes=64 * 1024 * 1024)
            ).hexdigest()
            != checksum
        ):
            raise ValueError(f"compiler input changed: {name}")
    grouped = [
        path
        for path in state["sources"]
        if count_function_metadata(read_source(Path(path))) >= 2
    ]
    if bool(grouped) != state["grouped"]:
        raise ValueError("compiler source grouping changed")
    if grouped:
        if not dispatch.record_path or not dispatch.record_fingerprint:
            raise ValueError("grouped compilation lacks preserved history")
        proof = verify_preservation(
            dispatch.root,
            read_preservation_document(dispatch.record_path),
            expected_fingerprint=dispatch.record_fingerprint,
            compiler=dispatch.compiler,
        )
        if (
            proof["destination"]
            != dispatch.source.relative_to(dispatch.root).as_posix()
            or proof["profile"]["programs"] != state["programs"]
        ):
            raise ValueError("preservation no longer selects the compiler source")
        expected = [
            *proof["profile"]["arguments"][1:],
            "-c",
            str(dispatch.source),
            "-o",
            str(dispatch.output),
        ]
        if list(dispatch.arguments) != expected:
            raise ValueError("compiler arguments no longer match preserved order")
    for name, observation in state["observations"].items():
        if observe_input(Path(name)) != observation:
            raise ValueError(f"compiler input or ancestor changed: {name}")
    dispatch.watch.validate()
    dispatch.selection.validate()
    dispatch.programs.validate(dispatch.invocation)
    if state["working_directory"] != _observe_working_directory():
        raise ValueError("compiler working directory changed during validation")
    if state.get("environment_fingerprint") != hash_preservation(dict(os.environ)):
        raise ValueError("compiler environment changed during validation")
    check_deadline()
