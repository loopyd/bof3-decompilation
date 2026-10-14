"""Render, execute and verify compiler recipes, C89 admission and profile attempts."""

from __future__ import annotations

from collections.abc import Callable, Iterator, Mapping, Sequence
from copy import deepcopy
from dataclasses import dataclass
import builtins
import hashlib
import json
from pathlib import Path
import shutil
from typing import cast

from harness.build.compiler import DEFAULT_COMPILER_ID
from harness.common.artifacts import (
    active_reader,
    ConsumedFile,
    FileIdentity,
    IncompleteWrite,
    encode_json,
    read_bytes,
    write_bytes,
)
from harness.common.captureproof import CaptureProofRefused, verify_capture
from harness.common.digests import digest, file_record
from harness.common.process import ProcessCleanupError, capture
from harness.common.failures import FailureEvent, PolicyFailure, facts, primary, retain
from harness.toolchain import gcc

CANONICAL_PROFILE_ID = f"{DEFAULT_COMPILER_ID}-maspsx-as"
MASPSX_VERSION = "2.56"
CC1_OPTIONS = ("-O2", "-G0", "-funsigned-char", "-msoft-float", "-gcoff")
ASSEMBLER_OPTIONS = ("-G0", "-EL", "-mips1")
REQUIRED_OPTIONS = (
    "-O2",
    "-G0",
    "-funsigned-char",
    "-msoft-float",
    "-gcoff",
    "-EL",
    "-mips1",
)
OPTION_DOMAINS = {
    "cpp": (),
    "cc1": CC1_OPTIONS,
    "as": ASSEMBLER_OPTIONS,
    "ld": (),
    "linker-script": (),
}
PIPELINE_SCHEMA = "retdec118.build-pipeline/v1"
WORKSPACE_MOUNT = "/work"

#: The fixed recipe template.  ``{name}`` markers expand to explicit paths;
#: ``{includes}``, ``{cc1}`` and ``{as}`` expand to whole argument lists.
STAGE_TEMPLATES = (
    {
        "name": "compile",
        "argv": (
            "{driver}",
            "{includes}",
            "{cc1}",
            "-S",
            "-o",
            "{workspace}/unit.s",
            "{source}",
        ),
        "environment": {
            "PATH": "{driver_dir}",
            "GCC_EXEC_PREFIX": "{driver_dir}/",
            "COMPILER_PATH": "{driver_dir}",
            "TMPDIR": "/tmp",
            "HOME": "/tmp",
            "LANG": "C.UTF-8",
            "PYTHONDONTWRITEBYTECODE": "1",
        },
        "stdin": None,
        "stdout": None,
        "consumes": ("{specs}",),
        "produces": ("{workspace}/unit.s",),
    },
    {
        "name": "maspsx",
        "argv": (
            "{maspsx_interpreter}",
            "-I",
            "-S",
            "-P",
            "-B",
            "{maspsx_bootstrap}",
            "{maspsx_entry}",
            "{maspsx_root}",
            f"--aspsx-version={MASPSX_VERSION}",
        ),
        "environment": {
            "PATH": "{workspace}",
            "TMPDIR": "/tmp",
            "HOME": "/tmp",
            "LANG": "C.UTF-8",
            "PYTHONDONTWRITEBYTECODE": "1",
        },
        "stdin": "{workspace}/unit.s",
        "stdout": "{workspace}/unit.part.s",
        "consumes": ("{workspace}/unit.s",),
        "produces": ("{workspace}/unit.part.s",),
    },
    {
        "name": "assemble",
        "argv": (
            "{assembler}",
            "{as}",
            "-o",
            "{workspace}/unit.o",
            "{workspace}/unit.part.s",
        ),
        "environment": {
            "PATH": "{assembler_dir}",
            "TMPDIR": "/tmp",
            "HOME": "/tmp",
            "LANG": "C.UTF-8",
        },
        "stdin": None,
        "stdout": None,
        "consumes": ("{workspace}/unit.part.s",),
        "produces": ("{workspace}/unit.o",),
    },
)
ARTIFACT_TEMPLATE = "{workspace}/unit.o"
INTERMEDIATE_TEMPLATES = ("{workspace}/unit.s", "{workspace}/unit.part.s")
TOOL_NAMES = (
    "driver",
    "cc1",
    "cpp",
    "specs",
    "assembler",
    "maspsx_interpreter",
    "maspsx_bootstrap",
    "maspsx_entry",
    "maspsx_root",
)
_LIST_MARKERS = ("includes", "cc1", "as")


def pipeline_description() -> dict:
    """The fixed, path-independent description whose digest identifies the recipe."""

    return {
        "schema": PIPELINE_SCHEMA,
        "maspsx_version": MASPSX_VERSION,
        "required_options": list(REQUIRED_OPTIONS),
        "option_domains": {
            phase: list(options) for phase, options in OPTION_DOMAINS.items()
        },
        "stages": [
            {
                "name": stage["name"],
                "argv": list(stage["argv"]),
                "environment": dict(stage["environment"]),
                "stdin": stage["stdin"],
                "stdout": stage["stdout"],
                "consumes": list(stage["consumes"]),
                "produces": list(stage["produces"]),
            }
            for stage in STAGE_TEMPLATES
        ],
        "artifact": ARTIFACT_TEMPLATE,
        "intermediates": list(INTERMEDIATE_TEMPLATES),
    }


def pipeline_digest() -> str:
    return digest(pipeline_description())


def validate_options(options: object) -> tuple[str, ...]:
    """Require exactly the admitted canonical option set, without duplication."""

    if not isinstance(options, (list, tuple)):
        raise ValueError("compiler options must be a list")
    values = tuple(options)
    if any(not isinstance(value, str) or not value for value in values):
        raise ValueError("compiler options must be non-empty strings")
    if len(set(values)) != len(values):
        raise ValueError("compiler options must be unique")
    if set(values) != set(REQUIRED_OPTIONS):
        raise ValueError(
            f"compiler options must be exactly the admitted set: {list(REQUIRED_OPTIONS)}"
        )
    return values


def route_options(options: Sequence[str]) -> dict[str, tuple[str, ...]]:
    """Route the validated option set to the stage that declares each switch."""

    selected = set(options)
    return {
        phase: tuple(option for option in OPTION_DOMAINS[phase] if option in selected)
        for phase in ("cc1", "as")
    }


def validate_tools(tools: Mapping[str, str]) -> None:
    """Require every explicit tool path and refuse a relative or unsafe spelling."""

    if set(tools) != set(TOOL_NAMES):
        raise ValueError("compiler recipe tools differ from the admitted set")
    for name in TOOL_NAMES:
        value = tools[name]
        if not isinstance(value, str) or not value.startswith("/") or ".." in value:
            raise ValueError(f"compiler recipe tool path is invalid: {name}={value!r}")


def _expand(item: str, values: Mapping[str, str | Sequence[str]]) -> list[str]:
    marker = (
        item[1:-1]
        if item.startswith("{") and item.endswith("}") and "{" not in item[1:-1]
        else None
    )
    if marker is not None:
        value = values[marker]
        if marker in _LIST_MARKERS:
            return list(value)
        return [str(value)]
    return [item.format(**{name: str(value) for name, value in values.items()})]


def _values(
    tools: Mapping[str, str],
    include_roots: Sequence[str],
    source: str,
    options: Sequence[str],
    workspace: str,
) -> dict[str, str | Sequence[str]]:
    routed = route_options(validate_options(options))

    def directory(path: str) -> str:
        return path.rsplit("/", 1)[0] if "/" in path else "."

    return {
        **dict(tools),
        "driver_dir": directory(tools["driver"]),
        "assembler_dir": directory(tools["assembler"]),
        "workspace": workspace,
        "source": source,
        "includes": [f"-I{root}" for root in include_roots],
        "cc1": routed["cc1"],
        "as": routed["as"],
    }


def render_recipe(
    tools: Mapping[str, str],
    include_roots: Sequence[str],
    source: str,
    options: Sequence[str],
    *,
    workspace: str = WORKSPACE_MOUNT,
) -> dict:
    """Render the complete recipe with the validated option set.

    The rendered value is recorded in the build audit; it is not the pipeline
    identity, which is the path-independent :func:`pipeline_digest`.
    """

    validate_tools(tools)
    validate_workspace(workspace)
    values = _values(tools, include_roots, source, options, workspace)

    def render(value: str) -> str:
        return value.format(**{key: str(item) for key, item in values.items()})

    rendered = []
    for stage in STAGE_TEMPLATES:
        argv: list[str] = []
        for item in stage["argv"]:
            argv.extend(_expand(item, values))
        rendered.append(
            {
                "name": stage["name"],
                "argv": argv,
                "environment": {
                    name: render(item) for name, item in stage["environment"].items()
                },
                "stdin": render(stage["stdin"]) if stage["stdin"] else None,
                "stdout": render(stage["stdout"]) if stage["stdout"] else None,
                "consumes": [render(item) for item in stage["consumes"]],
                "produces": [render(item) for item in stage["produces"]],
            }
        )
    return {
        "schema": "retdec118.build-recipe/v1",
        "pipeline": pipeline_digest(),
        "tools": dict(tools),
        "source": source,
        "include_roots": list(include_roots),
        "options": list(validate_options(options)),
        "stages": rendered,
        "artifact": recipe_artifact(workspace),
        "intermediates": list(recipe_intermediates(workspace)),
    }


def validate_workspace(workspace: str) -> None:
    if (
        not isinstance(workspace, str)
        or not workspace.startswith("/")
        or "\x00" in workspace
        or ".." in workspace.split("/")
    ):
        raise ValueError("compiler workspace must be an explicit absolute path")


def recipe_artifact(workspace: str = WORKSPACE_MOUNT) -> str:
    validate_workspace(workspace)
    return ARTIFACT_TEMPLATE.format(workspace=workspace)


def recipe_intermediates(workspace: str = WORKSPACE_MOUNT) -> tuple[str, ...]:
    validate_workspace(workspace)
    return tuple(
        template.format(workspace=workspace) for template in INTERMEDIATE_TEMPLATES
    )


def execute(
    rendered: dict,
    *,
    source: dict,
    output_limit: int,
    artifact_limit: int,
) -> dict:
    workspace = Path(rendered["artifact"]).parent
    result = {"status": "compile-failed", "stages": []}
    for stage in rendered["stages"]:
        if file_record(Path(source["path"])) != source:
            raise PolicyFailure("generated source changed before compilation")
        consumes = [file_record(Path(path)) for path in stage["consumes"]]
        input_data = None
        if stage["stdin"]:
            input_data = read_bytes(Path(stage["stdin"]), artifact_limit)
            if len(input_data) > artifact_limit:
                raise PolicyFailure(f"file exceeds read bound: {stage['stdin']}")
        data = encode_json(consumes, path="compiler")
        inputs_path = workspace / f"{stage['name']}-inputs.json"
        with inputs_path.open("xb") as stream:
            if stream.write(data) != len(data):
                raise IncompleteWrite("file write was incomplete")
        inputs_path.chmod(0o600)
        process = capture(
            workspace,
            stage["argv"],
            environment=stage["environment"],
            output_limit=output_limit,
            name=stage["name"],
            input_data=input_data,
        )
        if stage["stdout"]:
            shutil.copyfile(process["streams"]["stdout"]["path"], stage["stdout"])
        produced = [
            file_record(Path(path))
            for path in stage["produces"]
            if Path(path).is_file()
        ]
        result["stages"].append(
            {
                "name": stage["name"],
                "process": process,
                "tool": file_record(Path(stage["argv"][0])),
                "consumes": consumes,
                "produces": produced,
            }
        )
        if any(row["bytes"] > artifact_limit for row in produced):
            raise PolicyFailure("compiler artifact exceeds bound")
        if consumes != [file_record(Path(row["path"])) for row in consumes]:
            raise PolicyFailure("compiler stage input changed")
        if (
            process["failure"] is not None
            or process["raw_exit"] != 0
            or len(produced) != len(stage["produces"])
        ):
            break
    else:
        result.update(
            status="compiled-unverified",
            artifact=file_record(Path(rendered["artifact"])),
        )
    return result


class ImmutableInputDrift(ValueError):
    pass


@dataclass(frozen=True)
class FrozenInputs:
    source: dict
    prelude: dict
    tool_manifest: Mapping
    immutable: Sequence[dict]


def verify_inputs(
    records: Sequence[dict],
    *,
    events: list[FailureEvent] | None = None,
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
) -> None:
    reader = reader if reader is not None else active_reader()
    for record in records:
        if (
            type(record) is not dict
            or set(record) != {"path", "bytes", "sha256"}
            or not isinstance(record["path"], str)
            or not Path(record["path"]).is_absolute()
            or type(record["bytes"]) is not int
            or record["bytes"] < 0
            or not isinstance(record["sha256"], str)
            or len(record["sha256"]) != 64
            or any(char not in "0123456789abcdef" for char in record["sha256"])
        ):
            raise ImmutableInputDrift("supplied file identity changed or is invalid")
        hasher = hashlib.sha256()
        remaining = record["bytes"]
        local, stream = [], None
        try:
            if reader is None:
                stream = Path(record["path"]).open("rb")
                while block := stream.read(min(1 << 20, remaining + 1)):
                    remaining -= len(block)
                    hasher.update(block)
                    if remaining < 0:
                        break
            else:
                raw = reader(Path(record["path"]), remaining, local)
                remaining -= len(raw)
                hasher.update(raw)
        except Exception as error:
            if not any(row[3] is error for row in local):
                retain(
                    local,
                    error,
                    phase="input-read",
                    path=record["path"],
                    origin="ordinary",
                )
        if stream is not None:
            try:
                stream.close()
            except Exception as error:
                retain(
                    local,
                    error,
                    phase="input-close",
                    path=record["path"],
                    origin="ordinary",
                )
        if events is not None:
            events.extend(
                (
                    origin,
                    {"read": "input-read", "close": "input-close"}.get(phase, phase),
                    path,
                    error,
                )
                for origin, phase, path, error in local
            )
        if local:
            raise local[0][3]
        if remaining != 0 or hasher.hexdigest() != record["sha256"]:
            raise ImmutableInputDrift("supplied file identity changed")


@dataclass(frozen=True)
class SyntaxAttempt:
    record: dict
    inputs: FrozenInputs | None
    pending: Exception | None
    events: tuple[FailureEvent, ...]
    call: dict


def _write_syntax_json(path, value, events):
    try:
        data = encode_json(value, events=events, path=path, phase="receipt")
        write_bytes(path, data, events=events)
    except Exception as error:
        retain(events, error, phase="receipt", path=path, origin="ordinary")
        raise


def _read_syntax(path, limit, events, reader=None):
    try:
        regular = not path.is_symlink() and path.is_file()
    except Exception as error:
        retain(events, error, phase="syntax-stat", path=path, origin="ordinary")
        raise
    if not regular:
        raise PolicyFailure("retained C89 file is invalid")
    if reader is None:
        raw = read_bytes(
            path, limit, events, read_phase="syntax-read", close_phase="syntax-close"
        )
    else:
        local = []
        try:
            raw = reader(path, limit, local)
        finally:
            events.extend(
                (
                    origin,
                    {"read": "syntax-read", "close": "syntax-close"}.get(phase, phase),
                    path,
                    error,
                )
                for origin, phase, path, error in local
            )
    if len(raw) > limit:
        raise PolicyFailure("C89 file exceeds recovery bound")
    return raw


def _load_syntax(path, limit, events, reader=None):
    raw = _read_syntax(path, limit, events, reader)
    try:
        value = json.loads(raw)
    except (ValueError, UnicodeError) as error:
        raise PolicyFailure("C89 receipt JSON is invalid") from error
    try:
        encoded = encode_json(value, path="compiler")
    except Exception as error:
        retain(events, error, phase="syntax-serialize", path=path, origin="ordinary")
        raise
    if raw != encoded:
        raise PolicyFailure("C89 receipt bytes differ")
    return value


def verify_syntax(
    attempt,
    *,
    receipt=True,
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
):
    reader = reader if reader is not None else active_reader()
    events = []
    result, call = (attempt.record, attempt.call)
    workspace = call["cwd"]
    for group in ("expected_inputs", "c89_outputs"):
        try:
            records = result.get(group, [])
            if type(records) is not list:
                raise PolicyFailure("compiler inventory differs")
        except Exception as error:
            retain(events, error, phase="inventory", path=workspace)
            continue
        for record in records:
            try:
                if type(record) is not dict or type(record.get("path")) is not str:
                    raise PolicyFailure("compiler file record differs")
                path = Path(record["path"])
                copied = group == "expected_inputs"
                if (
                    not copied
                    or "source" in result
                    or path.exists()
                    or path.is_symlink()
                ):
                    if path.is_symlink() or not path.is_file():
                        raise PolicyFailure("retained compiler input/output differs")
                    if copied and path.stat().st_mode & 511 != 256:
                        raise PolicyFailure("compiler copy mode differs")
                    verify_inputs([record], events=events, reader=reader)
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="identity",
                    path=workspace,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
    if attempt.inputs is not None:
        for record in attempt.inputs.immutable:
            try:
                if record in result.get("expected_inputs", []):
                    continue
                verify_inputs([record], events=events, reader=reader)
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="compiler-input",
                    path=workspace,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
    for name in ("inputs.json", "syntax-result.json"):
        if name == "syntax-result.json" and (not receipt):
            continue
        try:
            path = workspace / name
            expected = (
                result
                if name == "syntax-result.json"
                else None
                if attempt.inputs is None
                else dict(
                    source=attempt.inputs.source,
                    prelude=attempt.inputs.prelude,
                    tools=attempt.inputs.tool_manifest,
                    immutable=attempt.inputs.immutable,
                )
            )
            if path.exists() or path.is_symlink():
                limit = 64 << 10 if name == "syntax-result.json" else 1 << 20
                expected_bytes = None
                if name == "inputs.json" and expected is not None:
                    try:
                        expected_bytes = encode_json(expected, path="compiler")
                    except Exception as error:
                        retain(
                            events,
                            error,
                            phase="syntax-serialize",
                            path=path,
                            origin="ordinary",
                        )
                        raise
                    limit = len(expected_bytes)
                recovered = _load_syntax(
                    path,
                    limit,
                    events,
                    reader,
                )
                try:
                    encoded = (
                        expected_bytes
                        if expected_bytes is not None
                        else encode_json(expected, path="compiler")
                    )
                    actual = encode_json(recovered, path="compiler")
                except Exception as error:
                    retain(
                        events,
                        error,
                        phase="syntax-serialize",
                        path=path,
                        origin="ordinary",
                    )
                    raise
                if expected is None or actual != encoded:
                    raise PolicyFailure("compiler metadata receipt differs")
            elif name == "syntax-result.json" or "source" in result:
                raise PolicyFailure("compiler metadata receipt is missing")
        except Exception as error:
            retain(
                events,
                error,
                phase="syntax-metadata",
                path=workspace / name,
                origin="policy" if isinstance(error, PolicyFailure) else "unknown",
            )
    if "c89" in result:
        try:
            verify_capture(
                result["c89"],
                origin=result.get("c89_origin", "direct"),
                events=events,
                reader=reader,
                **call,
            )
        except Exception as error:
            retain(
                events,
                error,
                phase="c89-proof",
                path=workspace / "c89.json",
                origin="policy"
                if isinstance(
                    error, (PolicyFailure, ImmutableInputDrift, CaptureProofRefused)
                )
                else "unknown",
            )
    return tuple(events)


def c89_call(*, workspace, tools, output_limit) -> dict:
    return dict(
        cwd=workspace,
        argv=[
            tools["files"]["syntax_compiler"]["path"],
            "-std=c89",
            "-pedantic-errors",
            "-O2",
            "-Werror=uninitialized",
            "-Werror=maybe-uninitialized",
            "-Werror=return-type",
            "-S",
            "-o",
            str(workspace / "syntax.s"),
            "-I" + str(workspace / "include"),
            str(workspace / "unit.c"),
        ],
        environment=dict(
            PATH="/usr/bin:/bin",
            LC_ALL="C",
            HOME=str(workspace / "home"),
            TMPDIR=str(workspace / "tmp"),
        ),
        output_limit=output_limit,
        name="c89",
    )


def check_syntax(
    call,
    *,
    source,
    prelude,
    prelude_path,
    tools,
    immutable,
    root_profile_id,
    artifact_limit,
):
    events, result, inputs = ([], {}, None)
    workspace = call["cwd"]
    try:
        copies = {"unit.c": source, prelude_path: prelude}
        expected = [
            dict(
                path=str(workspace / name),
                bytes=len(data),
                sha256=hashlib.sha256(data).hexdigest(),
            )
            for name, data in copies.items()
        ]
        result.update(
            expected_inputs=expected, c89_tool=tools["files"]["syntax_compiler"]
        )
        inputs = FrozenInputs(expected[0], expected[1], tools, [*expected, *immutable])
        try:
            workspace.mkdir(mode=448)
        except Exception as error:
            retain(events, error, phase="mkdir", path=workspace, origin="ordinary")
            raise
        for name, data in copies.items():
            write_bytes(workspace / name, data, events=events, readonly=True)
        for name in ("home", "tmp"):
            try:
                (workspace / name).mkdir(mode=448)
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="mkdir",
                    path=workspace / name,
                    origin="ordinary",
                )
                raise
        verify_inputs(inputs.immutable, events=events)
        result.update(source=expected[0], prelude=expected[1])
        _write_syntax_json(
            workspace / "inputs.json",
            dict(
                source=inputs.source,
                prelude=inputs.prelude,
                tools=tools,
                immutable=inputs.immutable,
            ),
            events,
        )
        result["c89_launch"] = "unknown"
        try:
            result["c89"] = capture(**call)
        except ProcessCleanupError as error:
            retain(events, error, phase="c89-capture", path=workspace, origin="cleanup")
            raise
        except Exception as error:
            retain(events, error, phase="c89-capture", path=workspace)
            raise
        result["c89_launch"] = verify_capture(
            result["c89"],
            origin="direct",
            events=events,
            reader=active_reader(),
            **call,
        )
    except Exception as error:
        retain(
            events,
            error,
            phase="syntax",
            path=workspace,
            origin="policy"
            if isinstance(
                error, (PolicyFailure, ImmutableInputDrift, CaptureProofRefused)
            )
            else "unknown",
        )
    if "c89" not in result and result.get("c89_launch") == "unknown":
        try:
            result["c89"] = _load_syntax(workspace / "c89.json", 64 << 10, events)
            result["c89_origin"] = "persisted"
            result["c89_launch"] = verify_capture(
                result["c89"],
                origin="persisted",
                events=events,
                reader=active_reader(),
                **call,
            )
        except Exception as error:
            retain(
                events,
                error,
                phase="c89-recovery",
                path=workspace / "c89.json",
                origin="policy"
                if isinstance(
                    error, (PolicyFailure, ImmutableInputDrift, CaptureProofRefused)
                )
                else "unknown",
            )
    try:
        path = workspace / "syntax.s"
        succeeded = (
            result.get("c89", {}).get("raw_exit") == 0
            and result.get("c89", {}).get("failure") is None
        )
        if succeeded or path.exists() or path.is_symlink():
            if path.is_symlink() or not path.is_file():
                raise PolicyFailure("C89 required syntax.s is absent or invalid")
            raw = _read_syntax(path, artifact_limit, events)
            result["c89_outputs"] = [
                dict(
                    path=str(path),
                    bytes=len(raw),
                    sha256=hashlib.sha256(raw).hexdigest(),
                )
            ]
        elif "c89" not in result and result.get("c89_launch") == "unknown":
            raise PolicyFailure("C89 launch proof is unknown")
    except Exception as error:
        retain(
            events,
            error,
            phase="syntax-output",
            path=workspace,
            origin="policy"
            if isinstance(error, (PolicyFailure, ImmutableInputDrift))
            else "unknown",
        )
    attempt = SyntaxAttempt(result, inputs, primary(events), tuple(events), call)
    events.extend(verify_syntax(attempt, receipt=False))
    result["secondary_failures"] = facts(events)
    try:
        _write_syntax_json(workspace / "syntax-result.json", result, events)
        events.extend(
            verify_syntax(
                SyntaxAttempt(result, inputs, primary(events), tuple(events), call)
            )
        )
    except Exception as error:
        retain(events, error, phase="syntax-receipt", path=workspace)
    result["secondary_failures"] = facts(events)
    return SyntaxAttempt(result, inputs, primary(events), tuple(events), call)


@dataclass(frozen=True)
class DeclaredProfileContext:
    ordinal: int
    directory: Path
    recipe_data: bytes


@dataclass(frozen=True)
class CompilerIntegerFacts:
    compiler: FileIdentity
    recipe: FileIdentity
    profile_identity: str
    ordinal: int
    target_rule: str
    target_contract: FileIdentity | None
    c_dialect: str
    int_bits: int | None
    signed_representation: str | None
    status: str
    reason: str
    char_bits: int | None = None


def inspect_compiler_integer_facts(
    context: DeclaredProfileContext,
    *,
    profile_identity: str,
    ordinal: int,
    recipe: ConsumedFile,
    components: Mapping[str, ConsumedFile],
) -> CompilerIntegerFacts:
    if (
        context.ordinal != ordinal
        or recipe.identity.path != context.directory / "recipe.json"
        or recipe.content != context.recipe_data
    ):
        raise ImmutableInputDrift(
            "compiler integer context differs from consumed recipe"
        )
    if set(components) != {"driver", "cc1", "cpp", "specs"}:
        raise ImmutableInputDrift("compiler integer components differ")
    for value in (recipe, *components.values()):
        if (
            len(value.content) != value.identity.bytes
            or hashlib.sha256(value.content).hexdigest() != value.identity.sha256
        ):
            raise ImmutableInputDrift("compiler integer physical input differs")
    value = json.loads(recipe.content)
    if type(value) is not dict or set(value) != {
        "schema",
        "pipeline",
        "tools",
        "source",
        "include_roots",
        "options",
        "stages",
        "artifact",
        "intermediates",
    }:
        raise ImmutableInputDrift("compiler integer recipe has unsupported fields")
    expected = render_recipe(
        value["tools"],
        value["include_roots"],
        value["source"],
        value["options"],
        workspace=str(context.directory),
    )
    if value != expected or any(
        value["tools"][name] != str(component.identity.path)
        for name, component in components.items()
    ):
        raise ImmutableInputDrift(
            "compiler integer recipe/compiler association differs"
        )
    compiler = components["driver"]
    if (
        any(
            components[name].identity.path != compiler.identity.path.parent / name
            for name in ("cc1", "cpp", "specs")
        )
        or components["specs"].content
    ):
        raise ImmutableInputDrift("compiler integer companions or empty specs differ")
    int_bits = gcc.inspect_canonical_int_bits(
        {name: components[name].identity for name in ("driver", "cc1", "cpp")}
    )
    return CompilerIntegerFacts(
        compiler.identity,
        recipe.identity,
        profile_identity,
        ordinal,
        gcc.CANONICAL_INTEGER_RULE
        if int_bits is not None
        else "unsupported-compiler-target/v1",
        components["specs"].identity,
        "c89",
        int_bits,
        None,
        "verified" if int_bits is not None else "unknown",
        "authenticated canonical GCC and empty specs establish signed int width 32 for literal zero"
        if int_bits is not None
        else "authenticated current recipe/compiler lacks an independently justified C89 signed32-bit target contract",
        char_bits=8 if int_bits == 32 else None,
    )


@dataclass(frozen=True)
class ProfileReceipt:
    ordinal: int
    path: Path
    data: bytes
    bytes: int
    sha256: str

    @classmethod
    def declare(cls, ordinal: int, path: Path, data: builtins.bytes) -> ProfileReceipt:
        return cls(ordinal, path, data, len(data), hashlib.sha256(data).hexdigest())


@dataclass(frozen=True)
class ProfileOutcome:
    ordinal: int
    result: dict
    abort: Exception | None = None
    events: tuple[FailureEvent, ...] = ()
    receipt: ProfileReceipt | None = None
    context: DeclaredProfileContext | None = None


def _profile_inventory(values, events, phase, path):
    available = []
    try:
        iterator = iter(values)
        while True:
            try:
                value = next(iterator)
            except StopIteration:
                break
            available.append(value)
    except Exception as error:
        retain(events, error, phase=phase, path=path)
    return available


def verify_receipts(
    receipts: Sequence[ProfileReceipt],
    *,
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
) -> tuple[FailureEvent, ...]:
    events: list[FailureEvent] = []
    for receipt in _profile_inventory(
        receipts, events, "receipt-inventory", "compiler"
    ):
        path: Path | str = "compiler"
        try:
            if (
                not isinstance(receipt, ProfileReceipt)
                or type(receipt.ordinal) is not int
                or receipt.ordinal < 0
                or not isinstance(receipt.path, Path)
                or not receipt.path.is_absolute()
                or receipt.path.name != "profile-result.json"
                or type(receipt.data) is not bytes
                or type(receipt.bytes) is not int
                or receipt.bytes != len(receipt.data)
                or receipt.sha256 != hashlib.sha256(receipt.data).hexdigest()
            ):
                raise ImmutableInputDrift("compiler declared receipt differs")
            path = receipt.path
            raw = _read_profile(path, 1 << 20, events, reader)
            if (
                raw != receipt.data
                or len(raw) != receipt.bytes
                or hashlib.sha256(raw).hexdigest() != receipt.sha256
            ):
                raise ImmutableInputDrift("compiler result receipt differs")
        except Exception as error:
            retain(
                events,
                error,
                phase="receipt",
                path=path,
                origin="policy"
                if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                else "unknown",
            )
    return tuple(events)


def _read_profile(path, limit, events, reader=None):
    try:
        regular = (
            not path.is_symlink() and path.is_file() and path.stat().st_size <= limit
        )
    except Exception as error:
        retain(events, error, phase="capture-stat", path=path, origin="ordinary")
        raise
    if not regular:
        raise ImmutableInputDrift("compiler capture file is invalid or exceeds bound")
    data = (
        read_bytes(path, limit, events)
        if reader is None
        else reader(path, limit, events)
    )
    if len(data) > limit:
        raise ImmutableInputDrift("compiler capture file exceeds bound")
    return data


def _verify_declared_stage(stage, index):
    if (
        type(stage) is not dict
        or index >= len(STAGE_TEMPLATES)
        or stage.get("name") != STAGE_TEMPLATES[index]["name"]
        or type(stage.get("argv")) is not list
        or not stage["argv"]
        or any(type(value) is not str for value in stage["argv"])
        or type(stage.get("environment")) is not dict
        or any(
            type(key) is not str or type(value) is not str
            for key, value in stage["environment"].items()
        )
    ):
        raise ImmutableInputDrift("compiler declared context differs")


def verify_attempts(
    attempts,
    inputs,
    *,
    output_limit,
    contexts: Sequence[DeclaredProfileContext] | None = None,
    receipts: Sequence[ProfileReceipt] = (),
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
):
    reader = reader if reader is not None else active_reader()
    events: list[FailureEvent] = []
    declared: dict[int, tuple[DeclaredProfileContext, dict]] = {}
    duplicates, used = set(), set()
    if contexts is not None:
        for context in _profile_inventory(
            contexts, events, "context-inventory", "compiler"
        ):
            path: Path | str = "compiler"
            try:
                if (
                    not isinstance(context, DeclaredProfileContext)
                    or type(context.ordinal) is not int
                    or context.ordinal < 0
                ):
                    raise ImmutableInputDrift(
                        "compiler declared context ordinal differs"
                    )
                if context.ordinal in declared or context.ordinal in duplicates:
                    duplicates.add(context.ordinal)
                    declared.pop(context.ordinal, None)
                    raise ImmutableInputDrift(
                        "compiler declared context ordinal differs"
                    )
                if (
                    not isinstance(context.directory, Path)
                    or not context.directory.is_absolute()
                    or type(context.recipe_data) is not bytes
                    or len(context.recipe_data) > 1 << 20
                ):
                    raise ImmutableInputDrift("compiler declared context differs")
                path = context.directory
                try:
                    rendered = json.loads(context.recipe_data)
                except Exception as error:
                    retain(events, error, phase="context-parse", path=path)
                    raise
                if type(rendered) is not dict or rendered.get("artifact") != str(
                    context.directory / "unit.o"
                ):
                    raise ImmutableInputDrift("compiler declared context differs")
                if encode_json(rendered, path=path) != context.recipe_data:
                    raise ImmutableInputDrift("compiler declared context differs")
                declared[context.ordinal] = (context, rendered)
                try:
                    stages = rendered["stages"]
                    if type(stages) is not list or len(stages) != len(STAGE_TEMPLATES):
                        raise ImmutableInputDrift("compiler declared context differs")
                except Exception as error:
                    retain(
                        events,
                        error,
                        phase="context",
                        path=path,
                        origin="policy"
                        if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                        else "unknown",
                    )
                    stages = []
                for index, stage in enumerate(stages):
                    try:
                        _verify_declared_stage(stage, index)
                    except Exception as error:
                        retain(
                            events,
                            error,
                            phase="context",
                            path=path,
                            origin="policy"
                            if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                            else "unknown",
                        )
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="context",
                    path=path,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
    for attempt in _profile_inventory(attempts, events, "inventory", "compiler"):
        for group in ("consumed_inputs", "captures", "stages"):
            try:
                values = attempt.get(group, [])
                if type(values) is not list:
                    raise ImmutableInputDrift("compiler retained inventory differs")
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="inventory",
                    path="compiler",
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
                continue
            for value in values:
                for field in (
                    ("consumes", "produces", "tool") if group == "stages" else (None,)
                ):
                    try:
                        records = (
                            [value]
                            if field is None
                            else [value[field]]
                            if field == "tool"
                            else value[field]
                        )
                        if type(records) is not list:
                            raise ImmutableInputDrift(
                                "compiler retained inventory differs"
                            )
                    except Exception as error:
                        retain(
                            events,
                            error,
                            phase="stage-inventory",
                            path="compiler",
                            origin="policy"
                            if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                            else "unknown",
                        )
                        continue
                    for record in records:
                        try:
                            if field == "produces":
                                path = Path(record["path"])
                                try:
                                    regular = not path.is_symlink() and path.is_file()
                                except Exception as error:
                                    retain(
                                        events,
                                        error,
                                        phase="artifact-stat",
                                        path=path,
                                        origin="ordinary",
                                    )
                                    raise
                                if not regular:
                                    raise ImmutableInputDrift(
                                        "compiler produced artifact is absent or invalid"
                                    )
                            verify_inputs([record], events=events, reader=reader)
                        except Exception as error:
                            retain(
                                events,
                                error,
                                phase="identity",
                                path="compiler",
                                origin="policy"
                                if isinstance(
                                    error, (PolicyFailure, ImmutableInputDrift)
                                )
                                else "unknown",
                            )
        directory, rendered, recipe_data = None, None, None
        try:
            if contexts is None:
                live = attempt.get("recipe")
                if live is not None:
                    rendered = live
                    directory = Path(rendered["artifact"]).parent
            else:
                ordinal = attempt["ordinal"]
                if type(ordinal) is not int or ordinal < 0:
                    raise ImmutableInputDrift(
                        "compiler declared context ordinal differs"
                    )
                used.add(ordinal)
                if ordinal in declared:
                    context, rendered = declared[ordinal]
                    directory, recipe_data = context.directory, context.recipe_data
                else:
                    if attempt.get("recipe") is not None or attempt.get(
                        "partial_processes"
                    ):
                        raise ImmutableInputDrift("compiler declared context is absent")
        except Exception as error:
            retain(
                events,
                error,
                phase="context" if contexts is not None else "recipe-proof",
                path="compiler",
                origin="policy"
                if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                else "unknown",
            )
        if contexts is not None and directory is not None:
            try:
                if attempt.get("recipe") != rendered:
                    raise ImmutableInputDrift("retained compiler recipe differs")
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="context",
                    path=directory,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
        recipe_ok = False
        if directory is not None and rendered is not None:
            try:
                raw = _read_profile(directory / "recipe.json", 1 << 20, events, reader)
                encoded = (
                    recipe_data
                    if contexts is not None
                    else encode_json(rendered, path=directory / "recipe.json")
                )
                if raw != encoded:
                    raise ImmutableInputDrift("retained compiler recipe differs")
                recipe_ok = True
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="recipe-proof",
                    path="compiler",
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
        if (
            rendered is not None
            and directory is not None
            and (contexts is not None or recipe_ok)
        ):
            try:
                processes = attempt.get("partial_processes", [])
                if type(processes) is not list:
                    raise ImmutableInputDrift("compiler process inventory differs")
            except Exception as error:
                retain(
                    events,
                    error,
                    phase="capture-proof",
                    path=directory,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
                processes = []
            for index, process in enumerate(processes):
                try:
                    stages = rendered["stages"]
                    if contexts is not None and type(stages) is not list:
                        raise ImmutableInputDrift("compiler declared context differs")
                    stage = stages[index]
                    if contexts is not None:
                        _verify_declared_stage(stage, index)
                    verify_capture(
                        process,
                        directory,
                        stage["argv"],
                        environment=stage["environment"],
                        output_limit=output_limit,
                        name=stage["name"],
                        origin="persisted",
                        events=cast("list[tuple[str, str, str, Exception]]", events),
                        reader=reader,
                    )
                except Exception as error:
                    retain(
                        events,
                        error,
                        phase="capture-proof",
                        path=directory,
                        origin="policy"
                        if isinstance(
                            error,
                            (PolicyFailure, ImmutableInputDrift, CaptureProofRefused),
                        )
                        else "unknown",
                    )
    for ordinal in declared.keys() - used:
        retain(
            events,
            ImmutableInputDrift("compiler declared context ordinal differs"),
            phase="context-ordinal",
            path="compiler",
            origin="policy",
        )
    try:
        immutable = inputs.immutable
    except Exception as error:
        retain(events, error, phase="immutable", path="compiler")
        immutable = ()
    for record in _profile_inventory(immutable, events, "immutable", "compiler"):
        try:
            verify_inputs([record], events=events, reader=reader)
        except Exception as error:
            retain(
                events,
                error,
                phase="immutable",
                path="compiler",
                origin="policy"
                if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                else "unknown",
            )
    events.extend(verify_receipts(receipts, reader=reader))
    return tuple(events)


def _profile_evidence(rendered, attempt, output_limit, events):
    directory = Path(rendered["artifact"]).parent
    attempt.update(
        captures=[], partial_processes=[], consumed_inputs=[], launch="unknown"
    )
    local, classifications, gap = ([], [], False)
    for stage in rendered["stages"]:
        records, contents = ({}, {})
        for suffix in ("-inputs.json", ".json", ".stdout.bin", ".stderr.bin"):
            path = directory / f"{stage['name']}{suffix}"
            try:
                if path.exists() or path.is_symlink():
                    raw = _read_profile(
                        path,
                        output_limit if suffix.endswith(".bin") else 1 << 20,
                        local,
                    )
                    records[suffix] = dict(
                        path=str(path.absolute()),
                        bytes=len(raw),
                        sha256=hashlib.sha256(raw).hexdigest(),
                    )
                    contents[suffix] = raw
                    attempt["captures"].append(records[suffix])
            except Exception as error:
                retain(
                    local,
                    error,
                    phase="capture-file",
                    path=path,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
        if not records:
            gap = True
            continue
        try:
            if gap or "-inputs.json" not in records:
                raise ImmutableInputDrift(
                    "compiler stage evidence is incomplete or out of order"
                )
            try:
                consumed = json.loads(contents["-inputs.json"])
            except (ValueError, UnicodeError) as error:
                raise ImmutableInputDrift("compiler input JSON differs") from error
            if (
                type(consumed) is not list
                or [row["path"] for row in consumed] != stage["consumes"]
            ):
                raise ImmutableInputDrift("compiler consumed-input evidence differs")
            attempt["consumed_inputs"].extend(consumed)
            if ".json" not in contents:
                raise ImmutableInputDrift(
                    "compiler launch is unknown without a process record"
                )
            try:
                process = json.loads(contents[".json"])
            except (ValueError, UnicodeError) as error:
                raise ImmutableInputDrift("compiler process JSON differs") from error
            attempt["partial_processes"].append(process)
            classification = verify_capture(
                process,
                directory,
                stage["argv"],
                environment=stage["environment"],
                output_limit=output_limit,
                name=stage["name"],
                origin="persisted",
                events=local,
                reader=active_reader(),
            )
            if any(
                (
                    identity != records.get(f".{channel}.bin")
                    for channel, identity in process["streams"].items()
                )
            ):
                raise ImmutableInputDrift("compiler raw capture identity differs")
            classifications.append(classification)
        except Exception as error:
            retain(
                local,
                error,
                phase="stage-proof",
                path=directory / stage["name"],
                origin="policy"
                if isinstance(
                    error, (PolicyFailure, ImmutableInputDrift, CaptureProofRefused)
                )
                else "unknown",
            )
        for record in records.values():
            try:
                verify_inputs([record], events=local)
            except Exception as error:
                retain(
                    local,
                    error,
                    phase="capture-identity",
                    path=directory,
                    origin="policy"
                    if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                    else "unknown",
                )
    for record in attempt["consumed_inputs"]:
        try:
            verify_inputs([record], events=local)
        except Exception as error:
            retain(
                local,
                error,
                phase="consumed-identity",
                path=directory,
                origin="policy"
                if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                else "unknown",
            )
    if classifications:
        attempt["launch"] = "observed" if "observed" in classifications else "prelaunch"
        attempt["disposition"] = (
            "unattempted"
            if attempt["launch"] == "prelaunch"
            else "complete"
            if attempt["terminal"]
            else "attempted"
        )
    else:
        retain(
            local,
            ImmutableInputDrift("compiler launch is unknown without process evidence"),
            phase="launch-proof",
            path=directory,
            origin="policy",
        )
    events.extend(local)
    if primary(local) is not None:
        raise primary(local)


def execute_profiles(
    profiles: Sequence[Mapping],
    *,
    workspace: Path,
    root_profile_id: str,
    inputs: FrozenInputs,
    include_roots: Sequence[str],
    attempts: list[dict],
    output_limit: int,
    artifact_limit: int,
) -> Iterator[ProfileOutcome]:
    profiles, inputs, include_roots = deepcopy((profiles, inputs, include_roots))
    validate_workspace(str(workspace))
    if (
        not profiles
        or len(profiles) != len(attempts)
        or profiles[0]["profile_id"] != root_profile_id
        or (len({profile["profile_id"] for profile in profiles}) != len(profiles))
        or any(
            (
                type(limit) is not int or limit <= 0
                for limit in (output_limit, artifact_limit)
            )
        )
        or (inputs.source not in inputs.immutable)
        or (inputs.prelude not in inputs.immutable)
    ):
        raise PolicyFailure("compiler profile execution inputs differ")
    for ordinal, (profile, attempt) in enumerate(zip(profiles, attempts, strict=True)):
        expected = {
            "profile_id": profile["profile_id"],
            "profile_identity": profile["identity"],
            "ordinal": ordinal,
        }
        if profile["ordinal"] != ordinal or any(
            (attempt.get(key) != value for key, value in expected.items())
        ):
            raise PolicyFailure("ordered compiler attempt identity differs")
        if profile["availability"] == "available":
            validate_tools(profile["tools"])
    verify_inputs(inputs.immutable)

    def advance():
        for profile, attempt in zip(profiles, attempts, strict=True):
            abort = None
            receipt, context = None, None
            directory = None
            interrupted = None
            events = []
            phase = "prepare"
            try:
                if tuple(
                    (
                        attempt.get(name)
                        for name in ("profile_id", "profile_identity", "ordinal")
                    )
                ) != (profile["profile_id"], profile["identity"], profile["ordinal"]):
                    raise ImmutableInputDrift(
                        "caller-owned compiler attempt identity changed"
                    )
                verify_inputs(inputs.immutable, events=events)
                if profile["availability"] != "available":
                    attempt.update(
                        status="catalog-refused",
                        launch="prelaunch",
                        failure=profile["refusal"],
                    )
                else:
                    directory = (
                        workspace
                        if profile["ordinal"] == 0
                        else workspace / f"profiles/{profile['ordinal']:03d}"
                    )
                    try:
                        directory.mkdir(
                            mode=448, parents=True, exist_ok=profile["ordinal"] == 0
                        )
                    except Exception as error:
                        retain(
                            events,
                            error,
                            phase="mkdir",
                            path=directory,
                            origin="ordinary",
                        )
                        raise
                    rendered = render_recipe(
                        profile["tools"],
                        include_roots,
                        inputs.source["path"],
                        REQUIRED_OPTIONS,
                        workspace=str(directory),
                    )
                    attempt["recipe"] = rendered
                    recipe_data = encode_json(
                        rendered,
                        events=events,
                        path=directory / "recipe.json",
                        phase="receipt-write",
                    )
                    context = DeclaredProfileContext(
                        profile["ordinal"], directory, recipe_data
                    )
                    write_bytes(
                        directory / "recipe.json",
                        recipe_data,
                        events=events,
                        create_parents=False,
                        write_phase="receipt-write",
                    )
                    try:
                        phase = "execute"
                        completed = execute(
                            rendered,
                            source=inputs.source,
                            output_limit=output_limit,
                            artifact_limit=artifact_limit,
                        )
                        attempt.update(
                            completed,
                            terminal=True,
                        )
                        last = completed["stages"][-1]["process"]
                        attempt.update(
                            raw_exit=last["raw_exit"], exit_code=last["exit_code"]
                        )
                        if completed["status"] != "compiled-unverified":
                            attempt["failure"] = (
                                last["failure"]
                                or "recipe refused process exit or produced artifacts"
                            )
                    except Exception as error:
                        interrupted = error
                        abort = error
                        retain(
                            events,
                            error,
                            phase=phase,
                            path=directory,
                            origin="cleanup"
                            if isinstance(error, ProcessCleanupError)
                            else "policy"
                            if isinstance(error, (PolicyFailure, ImmutableInputDrift))
                            else "ordinary"
                            if isinstance(error, IncompleteWrite)
                            else "unknown",
                        )
                        attempt.update(
                            status="interrupted",
                            raw_exit=None,
                            exit_code=None,
                            failure=f"{type(error).__name__}: {error}",
                        )
                        if isinstance(error, ProcessCleanupError):
                            attempt["owner_observation"] = error.observed
                            attempt["capture_failures"] = error.capture_failures
                    phase = "evidence"
                    _profile_evidence(rendered, attempt, output_limit, events)
                    if (
                        attempt["terminal"]
                        and [row["process"] for row in attempt["stages"]]
                        != attempt["partial_processes"]
                    ):
                        raise ImmutableInputDrift(
                            "returned compiler stages differ from retained captures"
                        )
                    if isinstance(interrupted, ProcessCleanupError):
                        abort = interrupted
                    elif any(
                        (
                            process["owner_observation"] is not None
                            for process in attempt["partial_processes"]
                        )
                    ):
                        raise ImmutableInputDrift("compiler ownership is unresolved")
                verify_inputs(inputs.immutable, events=events)
            except Exception as error:
                attempt.setdefault("launch", "unknown")
                attempt["evidence_failure"] = f"{type(error).__name__}: {error}"
                retain(
                    events,
                    error,
                    phase=phase,
                    path=directory or workspace,
                    origin="cleanup"
                    if isinstance(error, ProcessCleanupError)
                    else "policy"
                    if isinstance(
                        error, (PolicyFailure, ImmutableInputDrift, CaptureProofRefused)
                    )
                    else "unknown",
                )
            events.extend(
                verify_attempts(
                    [attempt],
                    inputs,
                    output_limit=output_limit,
                    contexts=() if context is None else (context,),
                )
            )
            abort = primary(events)
            attempt["secondary_failures"] = facts(events)
            if abort is not None:
                attempt.update(
                    failure=f"{type(abort).__name__}: {abort}",
                    capture_failures=getattr(abort, "capture_failures", ()),
                )
                if isinstance(abort, ProcessCleanupError):
                    attempt["owner_observation"] = abort.observed
            if directory is not None:
                path = directory / "profile-result.json"
                try:
                    data = encode_json(
                        attempt, events=events, path=path, phase="receipt-write"
                    )
                    receipt = ProfileReceipt.declare(profile["ordinal"], path, data)
                except Exception as error:
                    retain(events, error, phase="receipt", path=directory)
                if receipt is not None:
                    try:
                        write_bytes(
                            path,
                            receipt.data,
                            events=events,
                            create_parents=False,
                            write_phase="receipt-write",
                        )
                    except Exception as error:
                        retain(events, error, phase="receipt", path=directory)
                    events.extend(verify_receipts((receipt,)))
            abort = primary(events)
            attempt["secondary_failures"] = facts(events)
            if abort is not None:
                attempt.update(
                    failure=f"{type(abort).__name__}: {abort}",
                    capture_failures=getattr(abort, "capture_failures", ()),
                )
                if isinstance(abort, ProcessCleanupError):
                    attempt["owner_observation"] = abort.observed
            projected = {
                key: attempt[key]
                for key in ("status", "stages", "recipe", "artifact")
                if key in attempt
            }
            yield ProfileOutcome(
                profile["ordinal"], projected, abort, tuple(events), receipt, context
            )
            if abort is not None:
                return

    return advance()
