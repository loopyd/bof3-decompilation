from __future__ import annotations

import re
from pathlib import Path
from typing import Mapping, Sequence

ARGUMENT_NAME = re.compile(r"[A-Za-z][A-Za-z0-9_]{0,63}")
MAX_ARGUMENTS = 64
MAX_ARGUMENT_BYTES = 4096
SETTINGS_SCHEMA = "psx.runtime-settings/v1"

LAUNCH_FLAGS = (
    "-cli",
    "-interpreter",
    "-softgpu",
    "-debugger",
    "-no-gdb",
    "-no-webserver",
    "-no-pcdrv",
    "-no-fastboot",
    "-noupdate",
    "-run",
)
DISABLED_FEATURES = ("gdb", "webserver", "pcdrv", "fastboot", "updates")
SETTINGS_LIMITATIONS = (
    "Not a snapshot of all effective native settings.",
    "Lua may change settings during a mission.",
    "Host audio, physical controllers and scheduler are not pinned.",
    "Origin verification does not establish original runtime settings.",
)
PROCESS_LIMITATIONS = (
    "Trusted Lua scripts are not sandboxed.",
    "Process success alone does not establish guest behavior or fidelity.",
)


def parse_arguments(values: Sequence[str] | None) -> dict[str, str]:
    """Normalize ``NAME=VALUE`` mission arguments into one case-insensitive map."""

    parameters: dict[str, str] = {}
    for argument in values or []:
        name, separator, value = argument.partition("=")
        if (
            not separator
            or ARGUMENT_NAME.fullmatch(name) is None
            or "\x00" in value
            or len(value.encode("utf-8")) > MAX_ARGUMENT_BYTES
            or name.upper() in parameters
        ):
            raise ValueError(
                "runtime arguments require unique NAME=VALUE pairs (names ignore case)"
            )
        parameters[name.upper()] = value
    if len(parameters) > MAX_ARGUMENTS:
        raise ValueError("runtime accepts at most 64 mission arguments")
    return parameters


def emulator_arguments(
    *,
    output: Path,
    bios: str,
    script: str | None = None,
    mission: str | None = None,
    executable: str | None = None,
    media_cue: str | None = None,
) -> list[str]:
    """The exact maintained flag set for one mission, with no caller additions."""

    if (script is None) == (mission is None):
        raise ValueError("Select exactly one builtin mission or external script")
    command = [
        "-portable",
        str(output),
        *LAUNCH_FLAGS,
        "-bios",
        bios,
        *(["-exe", executable] if executable is not None else []),
        *(["-mission", mission] if mission is not None else ["-dofile", script]),
        *(["-iso", media_cue] if media_cue is not None else []),
    ]
    return command


def launch_settings(source: str) -> dict:
    """The recorded launch-settings description, never an effective-settings claim."""

    return {
        "schema": SETTINGS_SCHEMA,
        "source": source,
        "cpu": "interpreter",
        "gpu": "software",
        "debugger": True,
        "disabled": list(DISABLED_FEATURES),
        "defaults": "pinned emulator build; no preexisting portable configuration",
        "limits": list(SETTINGS_LIMITATIONS),
    }


def mission_environment(
    *,
    base: Mapping[str, str],
    directory: Path,
    arguments: Mapping[str, str],
    module_directory: Path | None = None,
    entry: int | None = None,
    origin_validated: bool = False,
    media: str = "",
    inputs: Sequence[tuple[str, Path]] = (),
    builtin: bool = False,
    binary_sha256: str | None = None,
    source_manifest_sha256: str | None = None,
) -> dict[str, str]:
    """One explicit mission environment; inherited ``PSX_RUNTIME_*`` is dropped.

    ``directory`` is the mission working directory; ``module_directory`` is the
    directory that holds the ``lua/`` module search root and defaults to
    ``directory``, so the interactive session keeps its existing Lua path while a
    confined service can point at a separate sealed module directory.
    """

    environment = {
        key: value for key, value in base.items() if not key.startswith("PSX_RUNTIME_")
    }
    lua_root = Path(directory) if module_directory is None else Path(module_directory)
    environment["LUA_PATH"] = "" if builtin else str(lua_root / "lua" / "?.lua")
    environment["LUA_CPATH"] = ""
    if binary_sha256 is not None or source_manifest_sha256 is not None:
        for name, value in (
            ("EMULATOR_SHA256", binary_sha256),
            ("SOURCE_MANIFEST_SHA256", source_manifest_sha256),
        ):
            if (
                not isinstance(value, str)
                or re.fullmatch(r"[0-9a-f]{64}", value) is None
            ):
                raise ValueError(
                    "Mission build identity must contain exact SHA-256 values"
                )
            environment["PSX_RUNTIME_" + name] = value
    environment["PSX_RUNTIME_ARGUMENT_NAMES"] = ",".join(arguments)
    environment.update(
        {f"PSX_RUNTIME_ARG_{name}": value for name, value in arguments.items()}
    )
    if entry is not None:
        environment["PSX_RUNTIME_ENTRY"] = str(entry)
    if origin_validated:
        environment["PSX_RUNTIME_ORIGIN_VALIDATED"] = "1"
    environment["PSX_RUNTIME_DIRECTORY"] = str(directory)
    environment["PSX_RUNTIME_MEDIA"] = media
    environment.update(
        {f"PSX_RUNTIME_INPUT_{name}": str(path) for name, path in inputs}
    )
    return environment


__all__ = [
    "ARGUMENT_NAME",
    "LAUNCH_FLAGS",
    "MAX_ARGUMENTS",
    "MAX_ARGUMENT_BYTES",
    "PROCESS_LIMITATIONS",
    "SETTINGS_SCHEMA",
    "emulator_arguments",
    "launch_settings",
    "mission_environment",
    "parse_arguments",
]
