from __future__ import annotations

import os
import shutil
from dataclasses import dataclass

from .paths import RepoLayout, repo_layout
from .toolchain.setup_psyq import find_psyq_source


@dataclass(frozen=True)
class DoctorCheck:
    name: str
    status: str
    detail: str
    required: bool = True
    hint: str | None = None


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


def run_doctor(
    *,
    layout: RepoLayout | None = None,
    include_local_inputs: bool = True,
    include_generated_outputs: bool = True,
) -> list[DoctorCheck]:
    repo = layout or repo_layout()
    checks: list[DoctorCheck] = [
        _command_check("uv"),
        _command_check("cmake"),
        _command_check("ninja"),
        _command_check("cargo"),
    ]

    for tool_name in ("bof3-disk", "emi-ex", "maspsx", "objdiff", "mipsmatch"):
        checks.append(
            _directory_check(
                repo.third_party_dir / tool_name,
                name=f"third_party/{tool_name}",
                hint=f"restore third_party/{tool_name} or re-sync the repository dependencies",
            )
        )

    if include_local_inputs:
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

    checks.extend(
        [
            _executable_check(
                repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-gcc",
                name="toolchains/psn00b",
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

    if include_local_inputs:
        checks.append(
            _file_check(
                repo.psyq_root / "include" / "libgpu.h",
                name="toolchains/psyq",
                hint="run `bin/setup-psyq --archive inputs/psyq-4.7-converted-full.7z` or `bin/download-psyq`",
            )
        )

    if include_generated_outputs:
        checks.append(
            _file_check(
                repo.ghidra_manifest_path,
                name="out/ghidra-bootstrap",
                required=False,
                hint="run `make ghidra` or `bin/ghidra-bootstrap` after extraction and unpack",
            )
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
    ordered_groups = ("command", "third_party", "inputs", "toolchains", "out")

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
