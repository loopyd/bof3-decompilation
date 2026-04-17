from __future__ import annotations

import argparse
import shlex
import subprocess
from pathlib import Path

from ..cli import add_logging_args, logger_from_args, package_prog
from ..config import DEFAULT_PSX_PROFILE
from . import compile_one
from . import history as history_lib
from . import pipeline_ready
from . import workspace as workspace_lib


ROOT = compile_one.ROOT


def default_build_root_for_profile(profile: str) -> Path:
    return compile_one.default_build_root_for_profile(profile)


def build_env(profile: str) -> dict[str, str]:
    return compile_one.build_env(profile)


def build_status_payload(
    workspace_payload: dict[str, object],
    *,
    profile: str,
    command: list[str],
    log_path: Path,
    build_root: Path,
    result: subprocess.CompletedProcess[str],
) -> dict[str, object]:
    return compile_one.build_status_payload(
        workspace_payload,
        profile=profile,
        build_mode="full-build",
        command=command,
        log_path=log_path,
        build_root=build_root,
        result=result,
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "build"),
        description="Run the canonical PSX matching build and record workspace build artifacts.",
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--build-root",
        type=Path,
        default=None,
    )
    parser.add_argument(
        "--build-command",
        nargs="+",
        default=["make", "build"],
        help="Command used for the fallback full build path",
    )
    parser.add_argument(
        "--full-build",
        action="store_true",
        help="Run the fallback full build instead of compile-one",
    )
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def record_build_attempt(
    workspace_dir: Path,
    workspace_payload: dict[str, object],
    *,
    build_mode: str,
    result: subprocess.CompletedProcess[str],
    log_path: Path,
    build_root: Path,
    command: list[str],
    source_file: Path | None = None,
    object_path: Path | None = None,
) -> None:
    history_lib.append_entry(
        workspace_dir,
        {
            "event": "build",
            "program_path": workspace_payload.get("program_path"),
            "entry_hex": workspace_payload.get("entry_hex"),
            "build_mode": build_mode,
            "command": list(command),
            "returncode": int(result.returncode),
            "succeeded": result.returncode == 0,
            "build_root": workspace_lib.relative_to_root(build_root),
            "log_path": workspace_lib.relative_to_root(log_path),
            "source_file": None
            if source_file is None
            else workspace_lib.relative_to_root(source_file),
            "object_path": None
            if object_path is None
            else workspace_lib.relative_to_root(object_path),
        },
    )


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_build")
    profile = DEFAULT_PSX_PROFILE
    resolved = compile_one.resolve_workspace(args, logger)
    if resolved is None:
        return 1

    workspace_json, workspace_payload = resolved
    workspace_dir = workspace_json.parent
    log_path = workspace_dir / "build.log"
    status_path = workspace_dir / "build.json"
    build_root = args.build_root or default_build_root_for_profile(profile)

    if args.full_build:
        if args.dry_run:
            logger.summary(
                f"workspace={workspace_payload.get('workspace_dir')} mode=full-build command={shlex.join(args.build_command)}"
            )
            return 0

        result = compile_one.run_command(args.build_command, env=build_env(profile))
        compile_one.write_build_outputs(
            workspace_payload,
            profile=profile,
            build_mode="full-build",
            log_path=log_path,
            status_path=status_path,
            build_root=build_root,
            result=result,
            command=args.build_command,
        )
        record_build_attempt(
            workspace_dir,
            workspace_payload,
            build_mode="full-build",
            result=result,
            log_path=log_path,
            build_root=build_root,
            command=args.build_command,
        )

        if result.returncode != 0:
            logger.error(
                f"full build failed; see {workspace_lib.relative_to_root(log_path)}"
            )
            return result.returncode

        logger.summary(
            f"workspace={workspace_payload.get('workspace_dir')} build_log={workspace_lib.relative_to_root(log_path)}"
        )
        return 0

    try:
        plan = compile_one.plan_compile_one(workspace_payload, build_root=build_root)
    except (FileNotFoundError, LookupError) as exc:
        logger.error(f"compile-one unavailable: {exc}; retry with --full-build")
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

    result, _ = compile_one.run_compile_one(
        workspace_payload,
        build_root=build_root,
        profile=profile,
    )
    compile_one.write_build_outputs(
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
    record_build_attempt(
        workspace_dir,
        workspace_payload,
        build_mode="compile-one",
        result=result,
        log_path=log_path,
        build_root=build_root,
        command=list(plan["command"]),
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
