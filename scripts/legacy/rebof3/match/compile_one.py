from __future__ import annotations

import argparse
import os
import shlex
import subprocess
from pathlib import Path
from typing import Any

from ..cli import (
    add_logging_args,
    logger_from_args,
    package_prog,
)
from ..common import ROOT, run_command, utc_now, write_json_output, write_text_output
from ..config import (
    DEFAULT_PSX_PROFILE,
    GCC272_PSX_GCC,
    GCC272_PSX_ROOT,
    PSN00B_TOOLCHAIN_BIN,
    aspsx_version_for_profile,
    psyq_root_for_profile,
)
from . import permuter_compile
from . import pipeline_ready
from . import workspace as workspace_lib


LOCAL_TOOLCHAIN_BIN = PSN00B_TOOLCHAIN_BIN
PROFILE_PRESET = "bof3-psyq40"


def relative_if_possible(path: Path) -> str:
    return workspace_lib.relative_to_root(path) if path.exists() else str(path)


def default_build_root_for_profile(profile: str) -> Path:
    return ROOT / "build" / PROFILE_PRESET


def build_env(profile: str) -> dict[str, str]:
    env = dict(os.environ)
    path_entries = [str(LOCAL_TOOLCHAIN_BIN)]
    existing_path = env.get("PATH")
    if existing_path:
        path_entries.append(existing_path)
    env["PATH"] = os.pathsep.join(path_entries)
    env["BOF3_PROFILE"] = profile
    env["BOF3_PSX_GCC_ROOT"] = str(GCC272_PSX_ROOT)
    env["BOF3_PSX_GCC"] = str(GCC272_PSX_GCC)
    return env


def build_status_payload(
    workspace_payload: dict[str, object],
    *,
    profile: str,
    build_mode: str,
    command: list[str],
    log_path: Path,
    build_root: Path,
    result: subprocess.CompletedProcess[str],
    compile_commands_path: Path | None = None,
    source_file: Path | None = None,
    object_path: Path | None = None,
) -> dict[str, object]:
    psyq_root = psyq_root_for_profile(profile)
    payload: dict[str, object] = {
        "workspace_dir": workspace_payload.get("workspace_dir"),
        "program_path": workspace_payload.get("program_path"),
        "entry_hex": workspace_payload.get("entry_hex"),
        "psx_profile": profile,
        "build_mode": build_mode,
        "command": command,
        "command_text": shlex.join(command),
        "log_path": workspace_lib.relative_to_root(log_path),
        "build_root": workspace_lib.relative_to_root(build_root),
        "build_root_exists": build_root.exists(),
        "returncode": int(result.returncode),
        "succeeded": result.returncode == 0,
        "ran_at": utc_now(),
        "toolchain_bin": relative_if_possible(LOCAL_TOOLCHAIN_BIN),
        "compiler_root": relative_if_possible(GCC272_PSX_ROOT),
        "compiler_gcc": relative_if_possible(GCC272_PSX_GCC),
        "psyq_root": relative_if_possible(psyq_root),
        "aspsx_version": aspsx_version_for_profile(profile),
    }
    if compile_commands_path is not None:
        payload["compile_commands_path"] = workspace_lib.relative_to_root(
            compile_commands_path
        )
    if source_file is not None:
        payload["source_file"] = workspace_lib.relative_to_root(source_file)
    if object_path is not None:
        payload["object_path"] = workspace_lib.relative_to_root(object_path)
        payload["object_path_exists"] = object_path.exists()
    return payload


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "compile-one"),
        description="Compile one function-matching source file from compile_commands.json.",
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--build-root",
        type=Path,
        default=None,
    )
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def resolve_workspace(
    args: argparse.Namespace, logger: Any
) -> tuple[Path, dict[str, object]] | None:
    return pipeline_ready.resolve_workspace(args, logger)


def source_file_from_workspace(workspace_payload: dict[str, object]) -> Path:
    source_mapping = workspace_payload.get("source_mapping") or {}
    source_file = source_mapping.get("source_file")
    if not source_file:
        raise LookupError("workspace is missing source_mapping.source_file")
    return ROOT / str(source_file)


def compile_commands_path_for_build_root(build_root: Path) -> Path:
    return build_root / "compile_commands.json"


def plan_compile_one(
    workspace_payload: dict[str, object], *, build_root: Path
) -> dict[str, object]:
    compile_commands_path = compile_commands_path_for_build_root(build_root)
    if not compile_commands_path.exists():
        raise FileNotFoundError(f"compile_commands not found: {compile_commands_path}")

    source_file = source_file_from_workspace(workspace_payload)
    entry = permuter_compile.load_compile_entry(
        compile_commands_path,
        source_file=source_file,
    )
    cwd = permuter_compile.compile_entry_directory(
        entry,
        fallback=compile_commands_path.parent,
    )
    object_path = permuter_compile.resolve_output_path(entry, fallback=cwd)
    command = permuter_compile.rewrite_compile_entry(
        entry,
        source_file=source_file,
        input_c=source_file,
        output=object_path,
    )
    return {
        "compile_commands_path": compile_commands_path,
        "source_file": source_file,
        "object_path": object_path,
        "cwd": cwd,
        "command": command,
    }


def run_compile_one(
    workspace_payload: dict[str, object],
    *,
    build_root: Path,
    profile: str,
) -> tuple[subprocess.CompletedProcess[str], dict[str, object]]:
    plan = plan_compile_one(workspace_payload, build_root=build_root)
    object_path = Path(str(plan["object_path"]))
    object_path.parent.mkdir(parents=True, exist_ok=True)
    object_path.unlink(missing_ok=True)
    result = run_command(
        list(plan["command"]),
        cwd=Path(str(plan["cwd"])),
        env=build_env(profile),
    )
    return result, plan


def write_build_outputs(
    workspace_payload: dict[str, object],
    *,
    profile: str,
    build_mode: str,
    log_path: Path,
    status_path: Path,
    build_root: Path,
    result: subprocess.CompletedProcess[str],
    command: list[str],
    compile_commands_path: Path | None = None,
    source_file: Path | None = None,
    object_path: Path | None = None,
) -> dict[str, object]:
    write_text_output(
        log_path, result.stdout + ("" if not result.stderr else "\n" + result.stderr)
    )
    status = build_status_payload(
        workspace_payload,
        profile=profile,
        build_mode=build_mode,
        command=command,
        log_path=log_path,
        build_root=build_root,
        result=result,
        compile_commands_path=compile_commands_path,
        source_file=source_file,
        object_path=object_path,
    )
    write_json_output(status_path, status)
    return status


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_compile_one")
    resolved = resolve_workspace(args, logger)
    if resolved is None:
        return 1

    workspace_json, workspace_payload = resolved
    workspace_dir = workspace_json.parent
    log_path = workspace_dir / "build.log"
    status_path = workspace_dir / "build.json"
    profile = DEFAULT_PSX_PROFILE
    build_root = args.build_root or default_build_root_for_profile(profile)

    try:
        plan = plan_compile_one(workspace_payload, build_root=build_root)
    except (FileNotFoundError, LookupError) as exc:
        logger.error(str(exc))
        return 1

    if args.dry_run:
        logger.summary(
            " ".join(
                [
                    f"workspace={workspace_payload.get('workspace_dir')}",
                    "mode=compile-one",
                    f"source={workspace_lib.relative_to_root(Path(str(plan['source_file'])))}",
                    f"object={workspace_lib.relative_to_root(Path(str(plan['object_path'])))}",
                ]
            )
        )
        return 0

    result, _ = run_compile_one(
        workspace_payload,
        build_root=build_root,
        profile=profile,
    )
    write_build_outputs(
        workspace_payload,
        profile=profile,
        build_mode="compile-one",
        log_path=log_path,
        status_path=status_path,
        build_root=build_root,
        result=result,
        command=list(plan["command"]),
        compile_commands_path=Path(str(plan["compile_commands_path"])),
        source_file=Path(str(plan["source_file"])),
        object_path=Path(str(plan["object_path"])),
    )

    if result.returncode != 0:
        logger.error(
            f"compile-one failed; see {workspace_lib.relative_to_root(log_path)}"
        )
        return result.returncode

    logger.summary(
        " ".join(
            [
                f"workspace={workspace_payload.get('workspace_dir')}",
                f"source={workspace_lib.relative_to_root(Path(str(plan['source_file'])))}",
                f"object={workspace_lib.relative_to_root(Path(str(plan['object_path'])))}",
                f"build_log={workspace_lib.relative_to_root(log_path)}",
            ]
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
