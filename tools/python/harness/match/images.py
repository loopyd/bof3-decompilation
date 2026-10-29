"""Capture exact emitted source ranges through current native comparison owners."""

from __future__ import annotations

import tempfile
from contextlib import ExitStack, closing
from pathlib import Path

from harness.build.dependencies import IncludeSnapshot
from harness.build.dispatch import (
    close_dispatch,
    prepare_configured_dispatch,
    validate_dispatch,
)
from harness.build.driver import execute_compiler, protect_staging
from harness.build.execution import verify_execution
from harness.common.inputs import relative
from harness.common.lease import require_writer
from harness.common.observation import PathWatch
from harness.common.paths import leaf_stat
from harness.domain.policy import validate_matching_text
from harness.io import repo_layout
from harness.match._asm_diff_payload import AsmDiffRequest, NativeComparison
from harness.match._asm_diff_run import (
    _asm_diff_compare,
    _asm_diff_resolve,
)
from harness.match._asm_resolve import compiler_asm_path_for_object
from harness.match.grouped import _capture_states, compare_member_images
from harness.match.members import capture_members
from harness.match.status_programs import StatusPrograms


def capture_source_images(
    root: Path, source: str, output_root: Path
) -> NativeComparison:
    """Require a writer lease and return only complete current exact source bytes."""
    root = root.resolve(strict=True)
    require_writer(root)
    if relative(source) != source:
        raise ValueError("emitted source requires a canonical repository path")
    output_root = output_root.absolute()
    if (
        not output_root.is_relative_to(root / "out")
        or output_root.resolve() != output_root
        or not output_root.is_dir()
    ):
        raise ValueError(
            "emitted capture requires an existing canonical out/ directory"
        )
    members = capture_members(root, source)
    if len(members["functions"]) > 1:
        output = Path(tempfile.mkdtemp(prefix="grouped-", dir=output_root)) / "group.o"
        return compare_member_images(root, source, output.relative_to(root).as_posix())
    return _capture_ordinary(root, source, output_root, members)


def _capture_ordinary(root, source, output_root, members):
    layout = repo_layout(root)
    member = members["functions"][0]
    source_path = root / source
    configuration = {
        root / "config/compiler/object-flags.cmake",
        root / "config/compiler/variants.json",
    }
    inputs = {root / name for name in members["inputs"]} | configuration
    present = {
        root / name for name, state in members["inputs"].items() if state is not None
    } | configuration
    with ExitStack() as stack:
        watch = stack.enter_context(closing(PathWatch(inputs)))
        states = _capture_states(root, present)
        programs = stack.enter_context(closing(StatusPrograms(root)))
        request = AsmDiffRequest(
            source_path=source_path,
            address=member["address"],
            size=len(member["original"]),
            binary_path=root / members["binary"],
            load_address=members["load_address"],
            output_root=output_root,
            canonical_bindings=members["bindings"],
            section_placements=members["section_placements"][member["address"]],
            diagnostics=False,
            image_evidence=True,
        )
        resolved = _asm_diff_resolve(layout, request)
        if any(
            resolved[key] != value
            for key, value in (
                ("source_path", source_path),
                ("address", member["address"]),
                ("function_name", member["symbol"]),
                ("binary_path", root / members["binary"]),
                ("load_address", members["load_address"]),
                ("original_size", len(member["original"])),
            )
        ):
            raise ValueError(
                "ordinary comparison differs from captured member identity"
            )
        dispatch = prepare_configured_dispatch(
            root, source_path, resolved["object_path"]
        )
        stack.callback(close_dispatch, dispatch)
        includes = stack.enter_context(
            closing(
                IncludeSnapshot(
                    root,
                    source_path,
                    forced=tuple(
                        Path(item["path"]) for item in dispatch.state["forced_inputs"]
                    ),
                )
            )
        )
        for path, content in includes.content.items():
            if not path.is_relative_to(root / "toolchains"):
                validate_matching_text(content.decode("utf-8"))
        outputs = {
            resolved["object_path"],
            compiler_asm_path_for_object(resolved["object_path"]),
        }
        linked_path = resolved["object_path"].with_suffix(".linked.o")
        prospective = outputs | {
            linked_path,
            resolved["object_path"].with_suffix(".placed.o"),
        }
        for path in prospective:
            status = leaf_stat(root, path.relative_to(root).as_posix())
            if status is not None and status.st_nlink != 1:
                raise ValueError("ordinary output has multiple filesystem links")
        for _, snapshot, _ in programs.snapshots.values():
            snapshot.protect_outputs(
                sorted(prospective), inputs=tuple(inputs | includes.paths)
            )
        dispatch.programs.protect_outputs(
            sorted(prospective), inputs=tuple(inputs | includes.paths)
        )
        protect_staging(dispatch, inputs=tuple(inputs | includes.paths))

        def guard():
            require_writer(root)
            watch.validate()
            includes.validate()
            programs.validate()
            validate_dispatch(dispatch)
            if capture_members(root, source) != members:
                raise ValueError("ordinary member inputs changed during capture")
            if _capture_states(root, present) != states:
                raise ValueError("ordinary comparison inputs changed")

        guard()
        status, execution = execute_compiler(
            root, list(dispatch.arguments), dispatch=dispatch
        )
        if status:
            raise RuntimeError(f"ordinary compiler exited with status {status}")
        guard()
        object_watch = stack.enter_context(closing(PathWatch(outputs)))
        object_states = _capture_states(root, outputs)
        execution_fingerprint = verify_execution(
            dispatch.invocation,
            dispatch.fingerprint,
            dispatch.state["grouped"],
            execution,
            outputs={
                str(root / name): state["sha256"]
                for name, state in object_states.items()
            },
        )
        linked_watch = None
        linked_states = None

        def capture_linked(path):
            nonlocal linked_watch, linked_states
            if path != linked_path or linked_watch is not None:
                raise ValueError(
                    "ordinary linker published a different or repeated output"
                )
            guard()
            object_watch.validate()
            if _capture_states(root, outputs) != object_states:
                raise ValueError("fresh object changed during linking")
            linked_watch = stack.enter_context(closing(PathWatch({path})))
            linked_states = _capture_states(root, {path})
            linked_watch.validate()

        emitted = []
        report = _asm_diff_compare(
            layout,
            request,
            resolved,
            emitted=emitted,
            on_linked=capture_linked,
        )
        if linked_watch is None or linked_states is None:
            raise ValueError("ordinary comparison omitted its linked output capture")
        report.update(
            fresh_compilation=True,
            configured_fingerprint=dispatch.fingerprint,
            configured_profile=dispatch.state["recipe"],
            execution_fingerprint=execution_fingerprint,
            native_inputs=states,
            include_inputs=includes.describe_inputs(),
            source_image=includes.image.describe(),
            native_outputs={**object_states, **linked_states},
        )
        guard()
        object_watch.validate()
        if _capture_states(root, outputs) != object_states:
            raise ValueError("fresh ordinary object changed during capture")
        if (
            verify_execution(
                dispatch.invocation,
                dispatch.fingerprint,
                dispatch.state["grouped"],
                execution,
                outputs={
                    str(root / name): state["sha256"]
                    for name, state in object_states.items()
                },
            )
            != execution_fingerprint
        ):
            raise ValueError("ordinary compiler execution binding changed")
        if linked_watch is not None:
            linked_watch.validate()
            if _capture_states(root, {linked_path}) != linked_states:
                raise ValueError("linked ordinary output changed during capture")
        return NativeComparison(report, tuple(emitted))
