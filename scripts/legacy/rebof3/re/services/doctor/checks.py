from __future__ import annotations

import importlib.util
import os
import shutil
import sqlite3
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

from ....common import prepend_pythonpath
from . import constants


@dataclass(frozen=True)
class DoctorCheck:
    group: str
    name: str
    required: bool
    status: str
    detail: str
    hint: str | None = None

    def as_dict(self) -> dict[str, object]:
        return {
            "group": self.group,
            "name": self.name,
            "required": self.required,
            "status": self.status,
            "detail": self.detail,
            "hint": self.hint,
        }


def detect_disk_inputs() -> list[Path]:
    disk_dir = constants.ROOT / "disk"
    if not disk_dir.exists():
        return []
    matches: list[Path] = []
    for pattern in ("*.cue", "*.bin", "*.iso"):
        matches.extend(sorted(disk_dir.glob(pattern)))
    return matches


def newest_mtime(paths: list[Path] | tuple[Path, ...]) -> float:
    return max(path.stat().st_mtime for path in paths)


def oldest_mtime(paths: list[Path] | tuple[Path, ...]) -> float:
    return min(path.stat().st_mtime for path in paths)


def missing_paths(paths: list[Path] | tuple[Path, ...]) -> list[Path]:
    return [path for path in paths if not path.exists()]


def extract_project_xml_path() -> Path:
    build_dir = constants.ROOT / "build"
    for name in constants.EXTRACT_PROJECT_XML_CANDIDATES:
        candidate = build_dir / name
        if candidate.exists():
            return candidate
    return build_dir / constants.EXTRACT_PROJECT_XML_CANDIDATES[0]


def extract_sentinels() -> tuple[Path, ...]:
    return (
        constants.ROOT / "build" / "extracted" / "SLUS_004.22",
        extract_project_xml_path(),
        constants.ROOT / "build" / "extracted" / "BIN" / "ETC" / "FIRST.EMI",
        constants.ROOT / "build" / "extracted" / "BIN" / "ETC" / "GAME.EMI",
        constants.ROOT / "build" / "extracted" / "BIN" / "SCENARIO" / "SCENA16.EMI",
    )


def unpack_input_sentinels() -> tuple[Path, ...]:
    return (
        constants.ROOT / "build" / "extracted" / "BIN" / "ETC" / "FIRST.EMI",
        constants.ROOT / "build" / "extracted" / "BIN" / "ETC" / "GAME.EMI",
        constants.ROOT / "build" / "extracted" / "BIN" / "SCENARIO" / "SCENA16.EMI",
    )


def run_smoke(command: list[str], *, env: dict[str, str] | None = None) -> bool:
    try:
        result = subprocess.run(
            command,
            cwd=constants.ROOT,
            env=env,
            capture_output=True,
            text=True,
            check=False,
        )
    except OSError:
        return False
    return result.returncode == 0


def resolve_command_path(repo_path: Path, *names: str) -> str | None:
    if repo_path.exists():
        return str(repo_path)
    for name in names:
        resolved = shutil.which(name)
        if resolved:
            return resolved
    return None


def tool_check(
    *,
    group: str,
    name: str,
    required: bool,
    repo_path: Path | None = None,
    names: tuple[str, ...] = (),
    smoke_command: list[str] | None = None,
    smoke_env: dict[str, str] | None = None,
    hint: str,
) -> DoctorCheck:
    if repo_path is not None:
        resolved = resolve_command_path(repo_path, *names)
    else:
        resolved = next(
            (shutil.which(candidate) for candidate in names if shutil.which(candidate)),
            None,
        )
    if resolved is None:
        return DoctorCheck(group, name, required, "missing", "not found", hint)
    if smoke_command is not None and not run_smoke(smoke_command, env=smoke_env):
        return DoctorCheck(
            group,
            name,
            required,
            "missing",
            f"found at {resolved} but smoke check failed",
            hint,
        )
    return DoctorCheck(group, name, required, "ok", resolved)


def python_module_check(
    *,
    group: str,
    name: str,
    module_name: str,
    required: bool,
    hint: str,
) -> DoctorCheck:
    if importlib.util.find_spec(module_name) is None:
        return DoctorCheck(
            group,
            name,
            required,
            "missing",
            f"python module {module_name} is not importable",
            hint,
        )
    return DoctorCheck(group, name, required, "ok", module_name)


def psx_toolchain_check(required: bool) -> DoctorCheck:
    missing: list[str] = []
    resolved: dict[str, str] = {}
    for label, names in constants.PSX_TOOLCHAIN_NAMES.items():
        local_path = constants.PSN00B_TOOLCHAIN_BIN / names[0]
        if local_path.exists():
            resolved[label] = str(local_path)
            continue
        found = next((shutil.which(name) for name in names if shutil.which(name)), None)
        if found is None:
            missing.append(label)
            continue
        resolved[label] = found
    if missing:
        return DoctorCheck(
            "build",
            "psx-toolchain",
            required,
            "missing",
            f"missing {', '.join(missing)}",
            "run `make setup_build`, `make setup_toolchain`, or install a system mipsel cross toolchain",
        )
    source = (
        "repo-local"
        if any(
            path.startswith(str(constants.PSN00B_TOOLCHAIN_BIN))
            for path in resolved.values()
        )
        else "system"
    )
    return DoctorCheck(
        "build", "psx-toolchain", required, "ok", f"{source} toolchain ready"
    )


def psx_compiler_check(required: bool) -> DoctorCheck:
    override = os.environ.get("BOF3_PSX_GCC")
    if override:
        override_path = Path(override)
        if override_path.exists():
            return DoctorCheck(
                "build", "psx-compiler", required, "ok", str(override_path)
            )
        return DoctorCheck(
            "build",
            "psx-compiler",
            required,
            "missing",
            str(override_path),
            "set `BOF3_PSX_GCC` to an executable old-gcc driver or run `make setup_toolchain`",
        )

    if constants.GCC272_PSX_GCC.exists():
        return DoctorCheck(
            "build", "psx-compiler", required, "ok", str(constants.GCC272_PSX_GCC)
        )

    return DoctorCheck(
        "build",
        "psx-compiler",
        required,
        "missing",
        str(constants.GCC272_PSX_GCC),
        "run `make setup_toolchain` to stage the canonical gcc-2.7.2-psx compiler",
    )


def psyq_original_40_check(required: bool) -> DoctorCheck:
    include_path = constants.PSYQ_ORIGINAL_40_ROOT / "include" / "libgpu.h"
    lib_dir = constants.PSYQ_ORIGINAL_40_ROOT / "lib"
    if include_path.exists() and lib_dir.exists():
        return DoctorCheck(
            "build", "psyq40", required, "ok", str(constants.PSYQ_ORIGINAL_40_ROOT)
        )
    return DoctorCheck(
        "build",
        "psyq40",
        required,
        "missing",
        str(constants.PSYQ_ORIGINAL_40_ROOT),
        "run `make setup_psyq_40 PSYQ40_SOURCE=/path/to/psyq-4.0`",
    )


def ghidra_checks() -> list[DoctorCheck]:
    checks: list[DoctorCheck] = []
    ghidra_home = os.environ.get("GHIDRA_HOME")
    detected_home = Path(ghidra_home) if ghidra_home else constants.DEFAULT_GHIDRA_HOME
    if not ghidra_home and constants.DEFAULT_GHIDRA_HOME.exists():
        checks.append(
            DoctorCheck(
                "ghidra",
                "GHIDRA_HOME",
                True,
                "ok",
                f"{constants.DEFAULT_GHIDRA_HOME} (default)",
            )
        )
    elif not ghidra_home:
        checks.append(
            DoctorCheck(
                "ghidra",
                "GHIDRA_HOME",
                True,
                "missing",
                "environment variable is not set",
                "set GHIDRA_HOME=/path/to/ghidra",
            )
        )
    else:
        checks.append(DoctorCheck("ghidra", "GHIDRA_HOME", True, "ok", ghidra_home))

    analyze_headless = detected_home / "support" / "analyzeHeadless"
    if analyze_headless.exists():
        checks.append(
            DoctorCheck("ghidra", "analyzeHeadless", True, "ok", str(analyze_headless))
        )
    else:
        checks.append(
            DoctorCheck(
                "ghidra",
                "analyzeHeadless",
                True,
                "missing",
                "support/analyzeHeadless not found under GHIDRA_HOME",
                "install Ghidra and point GHIDRA_HOME at the install root",
            )
        )

    checks.append(
        tool_check(
            group="ghidra",
            name="java",
            required=True,
            names=("java",),
            smoke_command=["java", "-version"],
            hint="install Java required by Ghidra",
        )
    )
    ghidra_env = prepend_pythonpath(constants.GHIDRA_SRC_DIR)
    checks.append(
        tool_check(
            group="ghidra",
            name="bof3-ghidra",
            required=True,
            repo_path=constants.GHIDRA_SRC_DIR
            / constants.GHIDRA_MAIN_MODULE
            / "__main__.py",
            smoke_command=[
                sys.executable,
                "-m",
                constants.GHIDRA_MAIN_MODULE,
                "--help",
            ],
            smoke_env=ghidra_env,
            hint="run `git submodule update --init third_party/tools/bof3-ghidra`",
        )
    )
    if constants.OPTIONAL_GHIDRA_PROJECT.exists():
        checks.append(
            DoctorCheck(
                "ghidra", "project", False, "ok", str(constants.OPTIONAL_GHIDRA_PROJECT)
            )
        )
    else:
        checks.append(
            DoctorCheck(
                "ghidra",
                "project",
                False,
                "missing",
                str(constants.OPTIONAL_GHIDRA_PROJECT),
                "run `make ghidra_bootstrap`",
            )
        )
    return checks


def workflow_check(
    *,
    name: str,
    required_paths: tuple[Path, ...],
    status_path: Path,
    upstream_paths: tuple[Path, ...] | list[Path],
    fallback_upstream_paths: tuple[Path, ...] | list[Path] = (),
    hint: str,
) -> DoctorCheck:
    missing = missing_paths(required_paths)
    if missing:
        missing_text = ", ".join(
            str(path.relative_to(constants.ROOT)) for path in missing
        )
        return DoctorCheck("workflow", name, True, "missing", missing_text, hint)
    existing_upstream = [path for path in upstream_paths if path.exists()]
    existing_fallback = [path for path in fallback_upstream_paths if path.exists()]
    effective_upstream = existing_upstream
    if upstream_paths and len(existing_upstream) != len(upstream_paths):
        if fallback_upstream_paths and len(existing_fallback) == len(
            fallback_upstream_paths
        ):
            effective_upstream = existing_fallback
        else:
            return DoctorCheck(
                "workflow", name, True, "stale", "upstream inputs are missing", hint
            )
    if not status_path.exists():
        if effective_upstream:
            if newest_mtime(effective_upstream) > oldest_mtime(required_paths):
                return DoctorCheck(
                    "workflow", name, True, "stale", "upstream inputs are newer", hint
                )
            return DoctorCheck(
                "workflow", name, True, "ok", "ready (derived from artifact mtimes)"
            )
        return DoctorCheck(
            "workflow",
            name,
            True,
            "stale",
            f"missing status marker {status_path.relative_to(constants.ROOT)}",
            hint,
        )
    if (
        effective_upstream
        and newest_mtime(effective_upstream) > status_path.stat().st_mtime
    ):
        return DoctorCheck(
            "workflow", name, True, "stale", "upstream inputs are newer", hint
        )
    return DoctorCheck("workflow", name, True, "ok", "ready")


def inventory_db_check() -> DoctorCheck:
    if not constants.INVENTORY_DB.exists():
        return DoctorCheck(
            "workflow",
            "inventory-db",
            True,
            "missing",
            str(constants.INVENTORY_DB.relative_to(constants.ROOT)),
            "run `make inventory`",
        )
    connection = None
    try:
        connection = sqlite3.connect(constants.INVENTORY_DB)
        rows = connection.execute(
            "SELECT name FROM sqlite_master WHERE type IN ('table', 'view')"
        ).fetchall()
    except sqlite3.Error as exc:
        return DoctorCheck(
            "workflow",
            "inventory-db",
            True,
            "missing",
            f"failed to open inventory sqlite: {exc}",
            "rebuild with `make inventory`",
        )
    finally:
        try:
            connection.close()
        except Exception:
            pass

    names = {str(row[0]) for row in rows}
    missing = [
        name for name in constants.INVENTORY_REQUIRED_TABLES if name not in names
    ]
    if missing:
        return DoctorCheck(
            "workflow",
            "inventory-db",
            True,
            "stale",
            f"missing tables/views: {', '.join(missing)}",
            "run `make inventory`",
        )
    return DoctorCheck(
        "workflow",
        "inventory-db",
        True,
        "ok",
        str(constants.INVENTORY_DB.relative_to(constants.ROOT)),
    )


def build_checks() -> list[DoctorCheck]:
    disk_inputs = detect_disk_inputs()
    extract_required_paths = extract_sentinels()
    checks: list[DoctorCheck] = [
        tool_check(
            group="core",
            name="python3",
            required=True,
            names=("python3",),
            smoke_command=["python3", "--version"],
            hint="install python3",
        ),
        python_module_check(
            group="core",
            name="python-pillow",
            module_name="PIL",
            required=True,
            hint="install Pillow into the active python environment",
        ),
        tool_check(
            group="core",
            name="git",
            required=True,
            names=("git",),
            smoke_command=["git", "--version"],
            hint="install git",
        ),
        tool_check(
            group="core",
            name="make",
            required=True,
            names=("make",),
            smoke_command=["make", "--version"],
            hint="install make",
        ),
        tool_check(
            group="core",
            name="bash",
            required=True,
            names=("bash",),
            smoke_command=["bash", "--version"],
            hint="install bash",
        ),
        tool_check(
            group="core",
            name="cmake",
            required=True,
            names=("cmake",),
            smoke_command=["cmake", "--version"],
            hint="install cmake",
        ),
        tool_check(
            group="disk",
            name="bof3-disk",
            required=True,
            repo_path=constants.BOF3_DISK_BINARY,
            names=("bof3-disk",),
            smoke_command=[
                str(
                    constants.BOF3_DISK_BINARY
                    if constants.BOF3_DISK_BINARY.exists()
                    else shutil.which("bof3-disk") or constants.BOF3_DISK_BINARY
                ),
                "--help",
            ],
            hint="run `make setup_tools`",
        ),
        tool_check(
            group="disk",
            name="emi-ex",
            required=True,
            repo_path=constants.EMI_EX_BINARY,
            names=("emi-ex",),
            smoke_command=[
                str(
                    constants.EMI_EX_BINARY
                    if constants.EMI_EX_BINARY.exists()
                    else shutil.which("emi-ex") or constants.EMI_EX_BINARY
                ),
                "--help",
            ],
            hint="run `make setup_tools`",
        ),
        tool_check(
            group="build",
            name="maspsx-cc",
            required=False,
            repo_path=constants.MASPSX_CC,
            hint="run `make setup_toolchain` and stage the canonical PsyQ 4.0 tree; expected repo wrapper under scripts/rebof3/toolchain/maspsx-cc",
        ),
        psx_compiler_check(required=False),
        psx_toolchain_check(required=False),
        psyq_original_40_check(required=False),
        tool_check(
            group="matching",
            name="asm-differ",
            required=False,
            repo_path=constants.ASM_DIFFER_SCRIPT,
            smoke_command=[sys.executable, str(constants.ASM_DIFFER_SCRIPT), "--help"],
            hint="run `make setup_submodules` and install asm-differ Python deps if needed",
        ),
        tool_check(
            group="matching",
            name="objdiff-cli",
            required=False,
            repo_path=constants.OBJDIFF_BINARY,
            names=("objdiff-cli",),
            smoke_command=[
                str(
                    constants.OBJDIFF_BINARY
                    if constants.OBJDIFF_BINARY.exists()
                    else shutil.which("objdiff-cli") or constants.OBJDIFF_BINARY
                ),
                "--help",
            ],
            hint="run `make setup_match_tools` or install objdiff-cli on PATH",
        ),
        tool_check(
            group="matching",
            name="m2c",
            required=False,
            repo_path=constants.M2C_SCRIPT,
            smoke_command=[sys.executable, str(constants.M2C_SCRIPT), "-h"],
            hint="run `make setup_submodules` and install m2c Python deps if needed",
        ),
        tool_check(
            group="matching",
            name="mipsmatch",
            required=False,
            repo_path=constants.MIPSMATCH_BINARY,
            names=("mipsmatch",),
            smoke_command=[
                str(
                    constants.MIPSMATCH_BINARY
                    if constants.MIPSMATCH_BINARY.exists()
                    else shutil.which("mipsmatch") or constants.MIPSMATCH_BINARY
                ),
                "--help",
            ],
            hint="run `make setup_match_tools` or install mipsmatch on PATH",
        ),
    ]
    if disk_inputs:
        checks.append(
            DoctorCheck(
                "workflow",
                "disk",
                True,
                "ok",
                ", ".join(
                    str(path.relative_to(constants.ROOT)) for path in disk_inputs
                ),
            )
        )
    else:
        checks.append(
            DoctorCheck(
                "workflow",
                "disk",
                True,
                "missing",
                "no .cue, .bin, or .iso inputs found under disk/",
                "place one BOF3 disc image set under disk/",
            )
        )
    checks.append(
        workflow_check(
            name="extract",
            required_paths=extract_required_paths,
            status_path=constants.EXTRACT_STATUS,
            upstream_paths=tuple(disk_inputs),
            fallback_upstream_paths=tuple(disk_inputs),
            hint="run `make extract`",
        )
    )
    checks.append(
        workflow_check(
            name="unpack",
            required_paths=constants.UNPACK_SENTINELS,
            status_path=constants.UNPACK_STATUS,
            upstream_paths=(constants.EXTRACT_STATUS,),
            fallback_upstream_paths=unpack_input_sentinels(),
            hint="run `make unpack`",
        )
    )
    checks.append(
        workflow_check(
            name="inventory",
            required_paths=constants.INVENTORY_SENTINELS,
            status_path=constants.INVENTORY_STATUS,
            upstream_paths=(constants.EXTRACT_STATUS, constants.UNPACK_STATUS),
            fallback_upstream_paths=(
                *extract_required_paths,
                *constants.UNPACK_SENTINELS,
            ),
            hint="run `make inventory`",
        )
    )
    checks.append(inventory_db_check())
    checks.extend(ghidra_checks())
    return checks


__all__ = [
    "DoctorCheck",
    "build_checks",
    "detect_disk_inputs",
    "extract_project_xml_path",
    "extract_sentinels",
    "ghidra_checks",
    "inventory_db_check",
    "missing_paths",
    "newest_mtime",
    "oldest_mtime",
    "psx_compiler_check",
    "psyq_original_40_check",
    "psx_toolchain_check",
    "resolve_command_path",
    "run_smoke",
    "tool_check",
    "unpack_input_sentinels",
    "workflow_check",
]
