from __future__ import annotations

import argparse
import json
import re
import os
import re
import shutil
import subprocess
from hashlib import sha256
from pathlib import Path

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import write_json_output
from ..config import DECOMP_PERMUTER_SCRIPT, PSN00B_TOOLCHAIN_BIN, ROOT
from . import (
    asm_differ_backend,
    history as history_lib,
    permuter_compile,
    pipeline_backend,
    pipeline_ready,
    seed_sources,
    workspace as workspace_lib,
)
from .seed_sources import VARIANT_CHOICES

DEFAULT_COMPILE_COMMANDS = ROOT / "build" / "bof3-psyq40" / "compile_commands.json"
VARIANT_FUNCTION_RE = re.compile(
    r"\b(FUN_[0-9A-Fa-f]{8}|func_0x[0-9A-Fa-f]+|func_[0-9A-Fa-f]{8})\b"
)
DEFAULT_TIMEOUT_SECONDS = 60
PERMUTER_SETUP_VERSION = 2


def positive_int_from_env(name: str) -> int | None:
    value = os.environ.get(name)
    if value is None:
        return None
    try:
        parsed = int(value)
    except ValueError:
        return None
    return parsed if parsed > 0 else None


def detect_active_opencode_agents(proc_root: Path = Path("/proc")) -> int:
    pair_ids: set[tuple[str, str]] = set()
    agent_ids: set[str] = set()
    session_ids: set[str] = set()
    if not proc_root.exists():
        return 1
    for entry in proc_root.iterdir():
        if not entry.name.isdigit():
            continue
        try:
            data = (entry / "environ").read_bytes()
        except OSError:
            continue
        if b"OPENCODE=1\0" not in data:
            continue
        env_values: dict[str, str] = {}
        for item in data.split(b"\0"):
            if not item or b"=" not in item:
                continue
            key, value = item.split(b"=", 1)
            if key in {b"AGENT", b"OPENCODE_PID"}:
                env_values[key.decode("utf-8")] = value.decode(
                    "utf-8", errors="replace"
                )
        agent_id = env_values.get("AGENT")
        session_id = env_values.get("OPENCODE_PID")
        if agent_id and session_id:
            pair_ids.add((session_id, agent_id))
            continue
        if agent_id:
            agent_ids.add(agent_id)
            continue
        if session_id:
            session_ids.add(session_id)
    if pair_ids:
        return len(pair_ids)
    if agent_ids:
        return len(agent_ids)
    if session_ids:
        return len(session_ids)
    return 1


def active_agent_count() -> int:
    for name in ("REBOF3_ACTIVE_AGENTS", "OPENCODE_ACTIVE_AGENTS"):
        configured = positive_int_from_env(name)
        if configured is not None:
            return configured
    if os.environ.get("OPENCODE") == "1":
        return max(detect_active_opencode_agents(), 1)
    return 1


def default_threads() -> int:
    cpu_total = max(os.cpu_count() or 1, 1)
    return max(cpu_total // max(active_agent_count(), 1), 1)


def _text_from_timeout_output(output: str | bytes | None) -> str:
    if output is None:
        return ""
    if isinstance(output, bytes):
        return output.decode("utf-8", errors="replace")
    return output


def run_permuter(
    command: list[str],
    *,
    timeout_seconds: int | None,
    log_path: Path | None = None,
    stream_stdout: bool = False,
) -> tuple[subprocess.CompletedProcess[str], bool]:
    if stream_stdout:
        try:
            return (
                subprocess.run(
                    command,
                    check=False,
                    timeout=timeout_seconds,
                ),
                False,
            )
        except subprocess.TimeoutExpired as exc:
            return (
                subprocess.CompletedProcess(
                    command,
                    124,
                    stdout=_text_from_timeout_output(exc.stdout),
                    stderr=_text_from_timeout_output(exc.stderr),
                ),
                True,
            )
    if log_path is not None:
        log_path.parent.mkdir(parents=True, exist_ok=True)
        with log_path.open("w", encoding="utf-8") as log_handle:
            process = subprocess.Popen(
                command,
                stdout=log_handle,
                stderr=subprocess.STDOUT,
                text=True,
            )
            try:
                returncode = process.wait(timeout=timeout_seconds)
                return (
                    subprocess.CompletedProcess(
                        command,
                        returncode,
                        stdout="",
                        stderr="",
                    ),
                    False,
                )
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
                log_handle.write(
                    f"\n[match_permuter] timed out after {timeout_seconds} seconds\n"
                )
                log_handle.flush()
                return (
                    subprocess.CompletedProcess(
                        command,
                        124,
                        stdout="",
                        stderr="",
                    ),
                    True,
                )
    try:
        return (
            subprocess.run(
                command,
                capture_output=True,
                text=True,
                check=False,
                timeout=timeout_seconds,
            ),
            False,
        )
    except subprocess.TimeoutExpired as exc:
        return (
            subprocess.CompletedProcess(
                command,
                124,
                stdout=_text_from_timeout_output(exc.stdout),
                stderr=_text_from_timeout_output(exc.stderr),
            ),
            True,
        )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "permuter"),
        description="Prepare and run decomp-permuter for one function workspace.",
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--compile-commands",
        type=Path,
        default=DEFAULT_COMPILE_COMMANDS,
    )
    parser.add_argument(
        "--variant",
        choices=VARIANT_CHOICES,
        default="repo",
    )
    parser.add_argument(
        "--threads",
        type=int,
        default=default_threads(),
        help=(
            "decomp-permuter -j thread count "
            "(default: CPU count divided by active agent count)"
        ),
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=DEFAULT_TIMEOUT_SECONDS,
        help=(
            "Stop permuter after this many seconds and keep the best outputs "
            "generated so far (default: 60)"
        ),
    )
    parser.add_argument("--setup-only", action="store_true")
    parser.add_argument(
        "--stdout",
        action="store_true",
        help="Stream decomp-permuter output directly to stdout/stderr instead of permuter.log.",
    )
    parser.add_argument(
        "permuter_args",
        nargs=argparse.REMAINDER,
        help="Additional arguments passed to permuter.py after '--'",
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def permuter_dir_for_workspace(workspace_dir: Path, *, variant: str) -> Path:
    return workspace_dir / "permuter" / variant


def resolve_variant_source(
    workspace_payload: dict[str, object], *, variant: str
) -> tuple[Path, str]:
    return seed_sources.resolve_variant_source(workspace_payload, variant=variant)


def source_function_name(workspace_payload: dict[str, object]) -> str:
    return seed_sources.source_function_name(workspace_payload)


def file_digest(path: Path) -> str:
    digest = sha256()
    with path.open("rb") as handle:
        while True:
            chunk = handle.read(65536)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def _path_text(path: Path) -> str:
    try:
        return workspace_lib.relative_to_root(path)
    except ValueError:
        return str(path)


def build_setup_inputs(
    *,
    variant_name: str,
    source_file: Path,
    compile_commands: Path,
    compile_source_file: Path,
    expected_s: Path,
    expected_o: Path,
    func_name: str,
    objdump_command: str,
) -> dict[str, object]:
    return {
        "tool_version": PERMUTER_SETUP_VERSION,
        "variant": variant_name,
        "source_file": _path_text(source_file),
        "source_digest": file_digest(source_file),
        "compile_commands": _path_text(compile_commands),
        "compile_commands_digest": file_digest(compile_commands),
        "compile_source_file": _path_text(compile_source_file),
        "expected_s": _path_text(expected_s),
        "expected_s_digest": file_digest(expected_s),
        "expected_o": _path_text(expected_o),
        "expected_o_digest": file_digest(expected_o),
        "func_name": func_name,
        "objdump_command": objdump_command,
    }


def load_existing_permuter_payload(path: Path) -> dict[str, object] | None:
    if not path.exists():
        return None
    try:
        return dict(json.loads(path.read_text(encoding="utf-8")))
    except (OSError, ValueError, TypeError):
        return None


def can_reuse_permuter_dir(
    *,
    existing_payload: dict[str, object] | None,
    setup_inputs: dict[str, object],
    perm_dir: Path,
) -> bool:
    if existing_payload is None:
        return False
    if existing_payload.get("setup_inputs") != setup_inputs:
        return False
    required_paths = (
        perm_dir / "base.c",
        perm_dir / "target.s",
        perm_dir / "target.o",
        perm_dir / "compile.sh",
        perm_dir / "settings.toml",
    )
    return all(path.exists() for path in required_paths)


def compile_script_text(*, compile_commands: Path, source_file: Path) -> str:
    return (
        "#!/bin/sh\n"
        "set -eu\n"
        f'python3 -m scripts.rebof3.match.permuter_compile --compile-commands "{compile_commands}" '
        f'--source-file "{source_file}" "$@"\n'
    )


def settings_toml_text(*, func_name: str, objdump_command: str) -> str:
    return (
        f'func_name = "{func_name}"\n'
        'compiler_type = "gcc"\n'
        f'objdump_command = "{objdump_command}"\n'
    )


REPO_SOURCE_ATTRIBUTE_RE = re.compile(r"__attribute__\s*\(\((?:[^()]|\([^()]*\))*\)\)")
UNKNOWN_FUNCTION_POINTER_CAST_RE = re.compile(
    r"\((?P<ret>\?|[A-Za-z_][A-Za-z0-9_\s\*]*)\s*\(\*\)\((?P<params>[^)]*)\)\)"
)
GENERIC_DATA_SYMBOL_RE = re.compile(r"\bD_([0-9A-F]{8})\b")


def sanitize_repo_source_text(text: str) -> str:
    sanitized = REPO_SOURCE_ATTRIBUTE_RE.sub("", text)
    sanitized = sanitized.replace(
        '__attribute__((optimize("no-optimize-sibling-calls")))',
        "",
    )
    return sanitized


def sanitize_unknown_function_pointer_casts(text: str) -> str:
    def repl(match: re.Match[str]) -> str:
        ret = match.group("ret").strip()
        params = match.group("params").strip()
        if ret == "?":
            ret = "void*"
        if params:
            params = ", ".join(
                "void*" if item.strip() == "?" else item.strip()
                for item in params.split(",")
            )
        return f"({ret} (*)({params}))"

    return UNKNOWN_FUNCTION_POINTER_CAST_RE.sub(repl, text)


def ghidra_variant_prelude() -> str:
    return (
        "typedef unsigned char undefined;\n"
        "typedef unsigned char undefined1;\n"
        "typedef unsigned short undefined2;\n"
        "typedef unsigned int undefined4;\n"
        "typedef unsigned char bool;\n"
        "typedef unsigned char byte;\n"
        "typedef unsigned short ushort;\n"
        "typedef unsigned int uint;\n"
        "typedef signed char s8;\n"
        "typedef signed short s16;\n"
        "typedef signed int s32;\n"
        "typedef unsigned char u8;\n"
        "typedef unsigned short u16;\n"
        "typedef unsigned int u32;\n"
        "typedef void code(void);\n"
        "typedef void* M2C_UNK;\n"
        "#define M2C_FIELD(ptr, type, offset) (*(type)((u8*)(ptr) + (offset)))\n"
        "#define true 1\n"
        "#define false 0\n\n"
    )


def sanitize_variant_source(text: str, *, func_name: str) -> str:
    target_suffix = func_name.split("func_")[-1]
    rewritten = re.sub(
        rf"\b(?:FUN|func)_{re.escape(target_suffix)}\b",
        func_name,
        text,
        flags=re.IGNORECASE,
    )
    rewritten = VARIANT_FUNCTION_RE.sub(func_name, rewritten, count=1)
    rewritten = sanitize_unknown_function_pointer_casts(rewritten)
    rewritten = GENERIC_DATA_SYMBOL_RE.sub(
        lambda match: f"DAT_{match.group(1).lower()}",
        rewritten,
    )
    rewritten = re.sub(
        rf"^.*\b{re.escape(func_name)}\s*\([^;]*\);\n",
        "",
        rewritten,
        flags=re.MULTILINE,
    )
    return ghidra_variant_prelude() + rewritten


def preprocess_repo_source(
    source_file: Path, *, compile_commands: Path, output_path: Path
) -> None:
    entry = permuter_compile.load_compile_entry(
        compile_commands, source_file=source_file
    )
    compile_args = permuter_compile.rewrite_compile_command(
        str(entry.get("command") or ""),
        source_file=source_file,
        input_c=source_file,
        output=output_path,
    )
    command: list[str] = ["cpp", "-P"]
    skip_next = False
    for index, value in enumerate(compile_args):
        if skip_next:
            skip_next = False
            continue
        if index == 0:
            continue
        if value == "-o":
            skip_next = True
            continue
        if value == "-c":
            continue
        if value.startswith("-I") or value.startswith("-D") or value.startswith("-U"):
            command.append(value)
    command.extend([str(source_file.resolve()), "-o", str(output_path.resolve())])
    result = subprocess.run(
        command,
        cwd=str(entry.get("directory") or compile_commands.parent),
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or "failed to preprocess repo variant")
    output_path.write_text(
        sanitize_repo_source_text(output_path.read_text(encoding="utf-8")),
        encoding="utf-8",
    )


def write_base_source(
    source_file: Path,
    *,
    variant: str,
    func_name: str,
    compile_commands: Path,
    output_path: Path,
) -> None:
    if variant == "repo":
        preprocess_repo_source(
            source_file, compile_commands=compile_commands, output_path=output_path
        )
        return
    text = source_file.read_text(encoding="utf-8")
    output_path.write_text(
        sanitize_variant_source(text, func_name=func_name),
        encoding="utf-8",
    )


def prepare_permuter_dir(
    workspace_json: Path,
    workspace_payload: dict[str, object],
    *,
    compile_commands: Path,
    variant: str,
) -> dict[str, object]:
    workspace_dir = workspace_json.parent
    prepared = asm_differ_backend.prepare_backend(workspace_dir, workspace_payload)
    backend_dir = ROOT / str(prepared["backend_dir"])

    source_file, variant_name = resolve_variant_source(
        workspace_payload, variant=variant
    )
    perm_dir = permuter_dir_for_workspace(workspace_dir, variant=variant_name)
    perm_dir.mkdir(parents=True, exist_ok=True)
    source_mapping = workspace_payload.get("source_mapping") or {}
    compile_source = source_mapping.get("source_file")
    if not compile_source:
        raise LookupError("workspace is missing source_mapping.source_file")
    compile_source_file = ROOT / str(compile_source)
    expected_s_source = backend_dir / "expected" / "expected.s"
    expected_o_source = backend_dir / "expected" / "objects" / "current.o"
    base_c = perm_dir / "base.c"
    target_s = perm_dir / "target.s"
    target_o = perm_dir / "target.o"
    compile_sh = perm_dir / "compile.sh"
    settings_toml = perm_dir / "settings.toml"
    func_name = source_function_name(workspace_payload)
    objdump_command = str(PSN00B_TOOLCHAIN_BIN / "mipsel-none-elf-objdump") + " -dr"
    setup_inputs = build_setup_inputs(
        variant_name=variant_name,
        source_file=source_file,
        compile_commands=compile_commands,
        compile_source_file=compile_source_file,
        expected_s=expected_s_source,
        expected_o=expected_o_source,
        func_name=func_name,
        objdump_command=objdump_command,
    )
    existing_payload = load_existing_permuter_payload(perm_dir / "workspace.json")

    payload = {
        "workspace_dir": workspace_payload.get("workspace_dir"),
        "program_path": workspace_payload.get("program_path"),
        "entry_hex": workspace_payload.get("entry_hex"),
        "variant": variant_name,
        "source_file": workspace_lib.relative_to_root(source_file),
        "permuter_dir": workspace_lib.relative_to_root(perm_dir),
        "base_c": workspace_lib.relative_to_root(base_c),
        "target_s": workspace_lib.relative_to_root(target_s),
        "target_o": workspace_lib.relative_to_root(target_o),
        "compile_sh": workspace_lib.relative_to_root(compile_sh),
        "settings_toml": workspace_lib.relative_to_root(settings_toml),
        "setup_inputs": setup_inputs,
    }

    if can_reuse_permuter_dir(
        existing_payload=existing_payload,
        setup_inputs=setup_inputs,
        perm_dir=perm_dir,
    ):
        return payload

    write_base_source(
        source_file,
        variant=variant_name,
        func_name=func_name,
        compile_commands=compile_commands,
        output_path=base_c,
    )
    shutil.copyfile(expected_s_source, target_s)
    shutil.copyfile(expected_o_source, target_o)
    compile_sh.write_text(
        compile_script_text(
            compile_commands=compile_commands, source_file=compile_source_file
        ),
        encoding="utf-8",
    )
    compile_sh.chmod(0o755)
    settings_toml.write_text(
        settings_toml_text(func_name=func_name, objdump_command=objdump_command),
        encoding="utf-8",
    )
    write_json_output(perm_dir / "workspace.json", payload)
    return payload


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_permuter")
    if not args.compile_commands.exists():
        logger.error(f"compile_commands not found: {args.compile_commands}")
        return 1
    if not DECOMP_PERMUTER_SCRIPT.exists():
        logger.error(f"decomp-permuter not found: {DECOMP_PERMUTER_SCRIPT}")
        return 1
    if args.timeout_seconds is not None and args.timeout_seconds <= 0:
        logger.error("--timeout-seconds must be greater than zero")
        return 1

    resolved = pipeline_ready.resolve_workspace(args, logger)
    if resolved is None:
        return 1
    workspace_json, workspace_payload = resolved
    state = pipeline_ready.refresh_expected_baseline(
        pipeline_ready.build_workspace_state(workspace_json, workspace_payload)
    )
    status, next_steps = pipeline_ready.diff_status(state)
    if status != "ready_for_backend_diff":
        logger.error(f"workspace is not ready for permuter setup: {status}")
        for step in next_steps:
            logger.item(f"- {step}")
        return 1

    try:
        prepared = prepare_permuter_dir(
            state.workspace_json,
            state.workspace_payload,
            compile_commands=args.compile_commands,
            variant=args.variant,
        )
    except (
        LookupError,
        FileNotFoundError,
        ValueError,
        pipeline_backend.BackendFailure,
    ) as exc:
        logger.error(str(exc))
        return 1

    if args.setup_only:
        logger.summary(
            f"permuter_dir={prepared['permuter_dir']} variant={prepared['variant']}"
        )
        return 0

    extra_args = list(args.permuter_args)
    if extra_args[:1] == ["--"]:
        extra_args = extra_args[1:]
    command = [
        "python3",
        str(DECOMP_PERMUTER_SCRIPT),
        str(ROOT / str(prepared["permuter_dir"])),
        "-j",
        str(max(args.threads, 1)),
        *extra_args,
    ]
    perm_dir = ROOT / str(prepared["permuter_dir"])
    permuter_log_path = perm_dir / "permuter.log"
    result, timed_out = run_permuter(
        command,
        timeout_seconds=args.timeout_seconds,
        log_path=None if args.stdout else permuter_log_path,
        stream_stdout=bool(args.stdout),
    )
    history_lib.append_entry(
        state.workspace_dir,
        {
            "event": "permuter",
            "program_path": state.workspace_payload.get("program_path"),
            "entry_hex": state.workspace_payload.get("entry_hex"),
            "variant": prepared["variant"],
            "threads": max(args.threads, 1),
            "timed_out": bool(timed_out),
            "timeout_seconds": args.timeout_seconds,
            "returncode": int(result.returncode),
            "succeeded": result.returncode == 0 and not timed_out,
            "permuter_dir": prepared["permuter_dir"],
            "log_path": None
            if args.stdout
            else workspace_lib.relative_to_root(permuter_log_path),
            "stream_stdout": bool(args.stdout),
            "command": command,
        },
    )
    if timed_out:
        logger.summary(
            "permuter_dir="
            f"{prepared['permuter_dir']} threads={max(args.threads, 1)} "
            f"variant={prepared['variant']} timed_out=1 "
            f"timeout_seconds={args.timeout_seconds}"
            + (" output=stdout" if args.stdout else "")
        )
        return 0
    if result.returncode != 0:
        if args.stdout:
            logger.error("permuter failed; output was streamed to stdout/stderr")
        else:
            logger.error(
                f"permuter failed; see {workspace_lib.relative_to_root(permuter_log_path)}"
            )
        return int(result.returncode)
    logger.summary(
        f"permuter_dir={prepared['permuter_dir']} threads={max(args.threads, 1)} variant={prepared['variant']}"
        + (" output=stdout" if args.stdout else "")
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
