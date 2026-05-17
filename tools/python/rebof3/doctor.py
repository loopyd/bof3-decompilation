from __future__ import annotations

import os
import shutil
from dataclasses import dataclass
from typing import Literal

from .paths import RepoLayout, repo_layout
from .toolchain.setup_psyq import find_psyq_source


@dataclass(frozen=True)
class DoctorCheck:
    name: str
    status: str
    detail: str
    required: bool = True
    hint: str | None = None


DoctorProfile = Literal["open", "full", "ghidra", "decomp", "workspace"]
DOCTOR_PROFILES: tuple[DoctorProfile, ...] = (
    "open",
    "full",
    "ghidra",
    "decomp",
    "workspace",
)

_PROFILE_PHASES: dict[DoctorProfile, tuple[str, ...]] = {
    "workspace": ("commands", "third_party_core"),
    "open": (
        "commands",
        "third_party_core",
        "third_party_match",
        "native_tools",
        "match_tools",
        "toolchains_open",
    ),
    "ghidra": (
        "commands",
        "third_party_core",
        "third_party_ghidra",
        "native_tools",
        "toolchains_open",
        "local_inputs",
        "psyq_toolchain",
        "ghidra_outputs",
    ),
    "decomp": (
        "commands",
        "third_party_core",
        "third_party_match",
        "third_party_ghidra",
        "third_party_decomp",
        "native_tools",
        "match_tools",
        "toolchains_open",
        "local_inputs",
        "psyq_toolchain",
        "ghidra_outputs",
        "ghidra_symbol_outputs",
    ),
    "full": (
        "commands",
        "third_party_core",
        "third_party_match",
        "third_party_ghidra",
        "third_party_decomp",
        "native_tools",
        "match_tools",
        "toolchains_open",
        "local_inputs",
        "psyq_toolchain",
        "ghidra_outputs",
        "ghidra_symbol_outputs",
    ),
}

_THIRD_PARTY_TOOLS: dict[str, tuple[str, ...]] = {
    "third_party_core": ("bof3-disk", "emi-ex", "maspsx"),
    "third_party_match": ("objdiff", "mipsmatch"),
    "third_party_ghidra": ("bof3-ghidra", "ghidra-mcp"),
    "third_party_decomp": ("asm-differ", "m2c", "decomp-permuter"),
}


def _command_check(command: str, *, required: bool = True) -> DoctorCheck:
    resolved = shutil.which(command)
    if resolved is None:
        return DoctorCheck(
            name=f"command/{command}",
            status="missing",
            detail=f"{command} is not on PATH",
            required=required,
            hint=f"install {command} or make it available on PATH",
        )
    return DoctorCheck(
        name=f"command/{command}",
        status="ok",
        detail=resolved,
        required=required,
    )


def _directory_check(path, *, name: str, hint: str) -> DoctorCheck:
    if path.exists() and path.is_dir() and any(path.iterdir()):
        return DoctorCheck(name=name, status="ok", detail=str(path))
    return DoctorCheck(
        name=name,
        status="missing",
        detail=f"missing {path}",
        hint=hint,
    )


def _file_check(path, *, name: str, hint: str, required: bool = True) -> DoctorCheck:
    if path.exists():
        return DoctorCheck(name=name, status="ok", detail=str(path), required=required)
    return DoctorCheck(
        name=name,
        status="missing",
        detail=f"missing {path}",
        required=required,
        hint=hint,
    )


def _executable_check(
    path, *, name: str, hint: str, required: bool = True
) -> DoctorCheck:
    if path.exists() and os.access(path, os.X_OK):
        return DoctorCheck(name=name, status="ok", detail=str(path), required=required)
    if path.exists():
        return DoctorCheck(
            name=name,
            status="missing",
            detail=f"not executable: {path}",
            required=required,
            hint=hint,
        )
    return DoctorCheck(
        name=name,
        status="missing",
        detail=f"missing {path}",
        required=required,
        hint=hint,
    )


def _third_party_checks(repo: RepoLayout, phases: frozenset[str]) -> list[DoctorCheck]:
    checks: list[DoctorCheck] = []
    seen: set[str] = set()
    for phase, tool_names in _THIRD_PARTY_TOOLS.items():
        if phase not in phases:
            continue
        for tool_name in tool_names:
            if tool_name in seen:
                continue
            seen.add(tool_name)
            checks.append(
                _directory_check(
                    repo.third_party_dir / tool_name,
                    name=f"third_party/{tool_name}",
                    hint=f"restore third_party/{tool_name} or re-sync the repository dependencies",
                )
            )
    return checks


def run_doctor(
    *,
    layout: RepoLayout | None = None,
    profile: DoctorProfile = "full",
) -> list[DoctorCheck]:
    if profile not in DOCTOR_PROFILES:
        choices = ", ".join(DOCTOR_PROFILES)
        raise ValueError(
            f"unknown doctor profile {profile!r}; expected one of: {choices}"
        )

    repo = layout or repo_layout()
    phases = frozenset(_PROFILE_PHASES[profile])
    checks: list[DoctorCheck] = []

    if "commands" in phases:
        checks.extend(
            [
                _command_check("uv"),
                _command_check("cmake"),
                _command_check("ninja"),
                _command_check("cargo"),
            ]
        )

    checks.extend(_third_party_checks(repo, phases))

    if "native_tools" in phases:
        checks.extend(
            [
                _executable_check(
                    repo.bof3_disk_bin,
                    name="tools/bof3-disk",
                    hint="run `bin/setup-native-tools` or `bin/setup-open`",
                ),
                _executable_check(
                    repo.emi_ex_bin,
                    name="tools/emi-ex",
                    hint="run `bin/setup-native-tools` or `bin/setup-open`",
                ),
            ]
        )

    if "match_tools" in phases:
        checks.extend(
            [
                _executable_check(
                    repo.objdiff_bin,
                    name="tools/objdiff-cli",
                    hint="run `bin/setup-match-tools` or `bin/setup-open`",
                ),
                _executable_check(
                    repo.mipsmatch_bin,
                    name="tools/mipsmatch",
                    hint="run `bin/setup-match-tools` or `bin/setup-open`",
                ),
            ]
        )

    if "local_inputs" in phases:
        disc_inputs = []
        for pattern in ("*.cue", "*.bin", "*.iso"):
            disc_inputs.extend(sorted(repo.disc_dir.glob(pattern)))
        if disc_inputs:
            checks.append(
                DoctorCheck(
                    name="inputs/disc",
                    status="ok",
                    detail=f"{len(disc_inputs)} disc input file(s) under {repo.disc_dir}",
                )
            )
        else:
            checks.append(
                DoctorCheck(
                    name="inputs/disc",
                    status="missing",
                    detail=f"no .cue, .bin, or .iso files under {repo.disc_dir}",
                    hint="place one BOF3 disc image set under inputs/disc/",
                )
            )

        psyq_source = find_psyq_source(version=repo.psyq_version)
        if psyq_source is None:
            checks.append(
                DoctorCheck(
                    name="inputs/local-psyq-source",
                    status="missing",
                    detail=f"no repo-local PsyQ {repo.psyq_version} source tree or archive found",
                    hint="run `bin/download-psyq`, or pass --source-root/--archive to `bin/setup-psyq`",
                )
            )
        else:
            checks.append(
                DoctorCheck(
                    name="inputs/local-psyq-source",
                    status="ok",
                    detail=f"{psyq_source.kind}: {psyq_source.path}",
                )
            )

    if "toolchains_open" in phases:
        checks.extend(
            [
                _executable_check(
                    repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-gcc",
                    name="toolchains/psn00b",
                    hint="run `make setup-open` or `bin/setup-open`",
                ),
                _executable_check(
                    repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objdump",
                    name="toolchains/psn00b-objdump",
                    hint="run `make setup-open` or `bin/setup-open`",
                ),
                _executable_check(
                    repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-nm",
                    name="toolchains/psn00b-nm",
                    hint="run `make setup-open` or `bin/setup-open`",
                ),
                _executable_check(
                    repo.gcc272_psx_root / "gcc",
                    name="toolchains/gcc-2.7.2-psx",
                    hint="run `make setup-open` or `bin/setup-open`",
                ),
                _file_check(
                    repo.aspsx_psyq_root / "psyq4.0" / "ASPSX.EXE",
                    name="toolchains/aspsx",
                    hint="run `bin/setup-aspsx` or `bin/setup-open`",
                ),
            ]
        )

    if "psyq_toolchain" in phases:
        checks.append(
            _file_check(
                repo.psyq_root / "include" / "libgpu.h",
                name="toolchains/psyq",
                hint="run `bin/setup-psyq --archive inputs/psyq-4.7-converted-full.7z` or `bin/download-psyq`",
            )
        )

    if "ghidra_outputs" in phases:
        checks.append(
            _file_check(
                repo.ghidra_manifest_path,
                name="out/ghidra-bof3",
                hint="run `make ghidra` after extraction and unpack",
            )
        )

    if "ghidra_symbol_outputs" in phases:
        checks.extend(
            [
                _file_check(
                    repo.inventory_ghidra_symbols_index_path,
                    name="out/ghidra-symbols-index",
                    hint="export Ghidra symbols and run `bin/inventory-import-ghidra-symbols`",
                ),
                _file_check(
                    repo.inventory_ghidra_function_index_path,
                    name="out/ghidra-function-index",
                    hint="export Ghidra symbols and run `bin/inventory-import-ghidra-symbols`",
                ),
            ]
        )

    return checks


def render_doctor(checks: list[DoctorCheck]) -> None:
    grouped: dict[str, list[DoctorCheck]] = {}
    for check in checks:
        group = check.name.split("/", 1)[0]
        grouped.setdefault(group, []).append(check)

    status_labels = {
        "ok": "OK",
        "missing": "MISS",
    }
    ordered_groups = ("command", "third_party", "tools", "inputs", "toolchains", "out")

    for group in ordered_groups:
        entries = grouped.get(group, [])
        if not entries:
            continue
        print(f"{group.capitalize()}:")
        for check in entries:
            label = check.name.split("/", 1)[1] if "/" in check.name else check.name
            scope = "" if check.required else " opt"
            status = status_labels.get(check.status, check.status.upper())
            print(f"  {status:<4}{scope}  {label:<20} {check.detail}")
            if check.hint and check.status != "ok":
                print(f"          hint: {check.hint}")
        print()

    ok_count = sum(1 for check in checks if check.status == "ok")
    issue_count = len(checks) - ok_count
    print(f"Summary: {ok_count} ok, {issue_count} issue(s)")


def doctor_exit_code(checks: list[DoctorCheck], *, strict: bool = False) -> int:
    for check in checks:
        if check.status == "ok":
            continue
        if check.required or strict:
            return 1
    return 0
