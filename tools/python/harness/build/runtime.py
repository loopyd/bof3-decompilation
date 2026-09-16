"""Derive bounded runtime candidates from versioned, source-pinned stage policies."""

from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

from harness.common.deadlines import check_deadline

if TYPE_CHECKING:
    from harness.build.invocation import Invocation

MASPSX_POLICY = "maspsx-local-sources/v1"
ASSEMBLER_POLICY = "bof3-assembler-wrapper/v1"
GCC_POLICY = "gcc-2.7.2-psx-search/v1"
_SOURCES = {
    MASPSX_POLICY: "38c090123ad707fb28c40013121c71036a763ec3b787a5c6ab6b55752d1166ae",
    ASSEMBLER_POLICY: "1bc786f76eb2fc72f448a00ea7b94ca5fb3a3d2ecc55a591020951c9585011ca",
    GCC_POLICY: "4eb1d9e0335ef61aa484afe1c671e807f54a2526c483a48ee6ae9e5e98f408c6",
}
_COMMON = {"provider", "root", "stage", "cwd", "operand", "spelling"}
_FIELDS = {
    MASPSX_POLICY: {"interpreter", "python_path"},
    ASSEMBLER_POLICY: {"assembler", "path"},
    GCC_POLICY: {"exec_prefix", "compiler_path", "path", "controls"},
}


def _is_search_control(argument: str) -> bool:
    return argument.startswith(("-B", "-b", "-V", "-specs", "--", "@", "-x"))


def _gcc_candidates(seed: dict) -> list[tuple[str, str, list[str] | None]]:
    machine = "mips-sony-psx/"
    suffix = machine + "2.7.2/"
    standard = ("/opt/cross/lib/gcc-lib/", "/usr/lib/gcc/")
    tool = "/opt/cross/mips-sony-psx/"
    compiler_paths = [
        (path if path.endswith("/") else path + "/") if path else "./"
        for path in seed["compiler_path"].split(os.pathsep)
    ]
    specs = [
        (seed["exec_prefix"], (suffix, "")),
        *((prefix, (suffix,)) for prefix in standard),
        (tool + "lib/", (suffix, "")),
    ]
    executables = [
        *((prefix, (suffix, "")) for prefix in [seed["exec_prefix"], *compiler_paths]),
        *((prefix, (suffix, machine)) for prefix in standard),
        (tool + "bin/", (suffix, "")),
    ]
    pending = [
        ("runtime-source", prefix + ending + "specs", None)
        for prefix, endings in specs
        for ending in endings
    ]
    for name in ("cpp", "cc1"):
        pending.extend(
            ("runtime-executable", prefix + ending + name, None)
            for prefix, endings in executables
            for ending in endings
        )
        pending.append(("runtime-executable", name, seed["path"]))
    return pending


@dataclass(frozen=True, slots=True)
class Runtime:
    provider: str
    root: Path

    def describe(self) -> dict:
        return {"provider": self.provider, "root": str(self.root)}


def describe_seeds(invocation: Invocation) -> list[dict]:
    """Bind provider inputs to actual stage operands and effective lookup settings."""
    seeds = []
    for stage in invocation.stages:
        check_deadline()
        if stage.runtime is None:
            continue
        policy = stage.runtime
        environment = dict(stage.environment)
        operand = 2 if policy.provider == MASPSX_POLICY else 0
        seed = {
            **policy.describe(),
            "stage": stage.name,
            "cwd": str(stage.cwd),
            "operand": operand,
            "spelling": stage.arguments[operand],
        }
        if policy.provider == MASPSX_POLICY:
            if operand not in stage.files or stage.arguments[1] != "-P":
                raise ValueError(
                    "maspsx policy differs from its script/interpreter roles"
                )
            path = environment.get("PYTHONPATH")
            if not isinstance(path, str):
                raise ValueError("maspsx policy requires an explicit search path")
            seed.update(
                interpreter=stage.arguments[0], python_path=path.split(os.pathsep)
            )
        elif policy.provider == GCC_POLICY:
            seed.update(
                exec_prefix=environment.get("GCC_EXEC_PREFIX"),
                compiler_path=environment.get("COMPILER_PATH"),
                path=os.get_exec_path(environment),
                controls=[
                    argument.tokens[0]
                    for argument in stage.parse_compiler_arguments()
                    if argument.role in {"option", "delimiter"}
                    and _is_search_control(argument.tokens[0])
                ],
            )
        else:
            seed.update(
                path=os.get_exec_path(environment),
                assembler=environment.get("PSX_AS")
                or str(
                    policy.root / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-as"
                ),
            )
        seeds.append(seed)
    validate_seeds(seeds)
    return seeds


def validate_seeds(seeds: object) -> None:
    """Validate policy settings without importing or executing their dependencies."""
    if not isinstance(seeds, list) or len(seeds) > 16:
        raise ValueError("runtime policy seed budget exceeded")
    identities = set()
    for seed in seeds:
        check_deadline()
        if (
            not isinstance(seed, dict)
            or not isinstance(seed.get("provider"), str)
            or seed["provider"] not in _SOURCES
        ):
            raise ValueError("unknown runtime candidate policy")
        python = seed["provider"] == MASPSX_POLICY
        gcc = seed["provider"] == GCC_POLICY
        if set(seed) != _COMMON | _FIELDS[seed["provider"]]:
            raise ValueError("runtime policy seed fields differ")
        spellings = (
            "root",
            "stage",
            "cwd",
            "spelling",
        ) + (
            ("exec_prefix", "compiler_path")
            if gcc
            else ("interpreter" if python else "assembler",)
        )
        for name in spellings:
            value = seed[name]
            if (
                not isinstance(value, str)
                or not value
                or "\0" in value
                or len(os.fsencode(value)) > 16384
            ):
                raise ValueError("runtime seed spelling is invalid")
        for name in ("root", "cwd"):
            path = Path(seed[name])
            if not path.is_absolute() or str(path) != seed[name] or ".." in path.parts:
                raise ValueError("runtime seed directory is not canonical")
        if type(seed["operand"]) is not int or seed["operand"] != (2 if python else 0):
            raise ValueError("runtime seed operand differs")
        paths = seed["python_path" if python else "path"]
        if (
            not isinstance(paths, list)
            or not 1 <= len(paths) <= 16
            or any(
                not isinstance(path, str)
                or "\0" in path
                or len(os.fsencode(path)) > 16384
                for path in paths
            )
        ):
            raise ValueError("runtime search roots exceed their bounds")
        if gcc:
            controls = seed["controls"]
            if (
                len(seed["compiler_path"].split(os.pathsep)) > 16
                or not isinstance(controls, list)
                or len(controls) > 256
                or any(
                    not isinstance(value, str)
                    or "\0" in value
                    or len(os.fsencode(value)) > 16384
                    or not _is_search_control(value)
                    for value in controls
                )
            ):
                raise ValueError("GCC search controls exceed their bounds")
        identity = (seed["stage"], seed["provider"])
        if identity in identities:
            raise ValueError("duplicate runtime stage policy")
        identities.add(identity)
    check_deadline()


def derive_candidates(
    root: Path, seeds: list[dict], primary: list[dict], nodes: dict[str, dict]
) -> tuple[list[dict], list[dict]]:
    """Re-derive candidate causes from captured source hashes, never guessed selection."""
    validate_seeds(seeds)
    requests = []
    edges = []
    for seed in seeds:
        check_deadline()
        if seed["root"] != str(root):
            raise ValueError("runtime seed is bound to a different repository")
        python = seed["provider"] == MASPSX_POLICY
        matches = [
            index
            for index, request in enumerate(primary)
            if request["stage"] == seed["stage"]
            and request["role"] == ("script" if python else "executable")
            and request["operand"] == seed["operand"]
        ]
        if len(matches) != 1:
            raise ValueError("runtime policy lacks one primary operand")
        source_index = matches[0]
        source = primary[source_index]
        if source["cwd"] != seed["cwd"] or source["spelling"] != seed["spelling"]:
            raise ValueError("runtime policy primary binding differs")
        if python:
            interpreters = [
                request
                for request in primary
                if request["stage"] == seed["stage"] and request["role"] == "executable"
            ]
            if (
                len(interpreters) != 1
                or interpreters[0]["spelling"] != seed["interpreter"]
            ):
                raise ValueError("runtime interpreter binding differs")
        checksum = None
        if len(source["lookups"]) == 1:
            lookup = source["lookups"][0]
            if lookup["status"] == "file":
                checksum = nodes[lookup["terminal"]]["sha256"]
        status = (
            "candidate-only"
            if checksum == _SOURCES[seed["provider"]]
            else "unrecognized-source"
        )
        gcc = seed["provider"] == GCC_POLICY
        if seed["provider"] == ASSEMBLER_POLICY and source["spelling"] != str(
            root / "bin/as"
        ):
            status = "unsupported-location"
        if gcc and status == "candidate-only" and seed["controls"]:
            status = "unsupported-controls"
        pending = []
        if status == "candidate-only":
            if python:
                pending = [
                    ("runtime-source", os.path.join(directory, name), None)
                    for directory in seed["python_path"]
                    for name in (
                        "maspsx/__init__.py",
                        "maspsx.py",
                        "maspsx/__init__.pyc",
                        "maspsx.pyc",
                    )
                ]
            elif gcc:
                pending = _gcc_candidates(seed)
            else:
                pending = [
                    ("runtime-executable", "/bin/sh", None),
                    ("runtime-executable", "dirname", seed["path"]),
                    ("runtime-executable", seed["assembler"], seed["path"]),
                ]
        indices = []
        for role, spelling, search in pending:
            candidates = (
                [spelling]
                if search is None or "/" in spelling
                else [os.path.join(directory, spelling) for directory in search]
            )
            indices.append(len(primary) + len(requests))
            requests.append(
                {
                    "stage": seed["stage"],
                    "role": role,
                    "operand": None,
                    "spelling": spelling,
                    "command": None,
                    "cwd": seed["cwd"],
                    "candidates": candidates,
                }
            )
        edges.append(
            {
                "provider": seed["provider"],
                "source": source_index,
                "source_sha256": checksum,
                "status": status,
                "requests": indices,
                "unresolved": (
                    [
                        "effective-import-selection",
                        "extension-loaders",
                        "source-bytecode-caches",
                        "startup-stdlib-runtime",
                    ]
                    if python
                    else [
                        "source-build-correspondence",
                        "executable-suffix-variants",
                        "specs-command-expansion",
                        "post-specs-prefixes",
                        "language-specific-backends",
                        "effective-executable-selection",
                        "preprocessor-inputs",
                    ]
                    if gcc
                    else [
                        "shell-command-semantics",
                        "shell-directory-resolution",
                        "assembler-loader-libraries",
                    ]
                ),
            }
        )
    check_deadline()
    return requests, edges
