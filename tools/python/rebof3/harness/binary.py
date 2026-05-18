from __future__ import annotations

import difflib
import hashlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..core.process import run_process
from ..jsonio import write_json
from .config import HarnessConfig
from .workspace import safe_name


@dataclass(frozen=True)
class BinaryPair:
    original: Path | None
    compiled: Path | None
    load_address: int


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_hint_key(value: str | None) -> str:
    return str(value or "").replace(".EMI#", ".EMI#")


def compiled_bin_path_from_program_path(
    config: HarnessConfig, program_path: str
) -> Path:
    normalized = program_path.strip("/")
    if normalized.startswith("bins/"):
        normalized = normalized[len("bins/") :]
    return config.root / "build/default/artifacts/raw" / normalized


def artifact_compiled_paths(config: HarnessConfig) -> dict[str, Path]:
    from .catalog import artifact_records

    paths: dict[str, Path] = {}
    for record in artifact_records(config):
        source_hint = source_hint_key(record.get("source_hint"))
        program_path = record.get("program_path")
        if source_hint and program_path:
            paths[source_hint] = compiled_bin_path_from_program_path(
                config, str(program_path)
            )
    return paths


def resolve_binary_pair(config: HarnessConfig, target: dict[str, Any]) -> BinaryPair:
    payload = target.get("payload") if isinstance(target.get("payload"), dict) else {}
    original_raw = payload.get("payload_path")
    original = Path(str(original_raw)) if original_raw else None
    if original is not None and not original.is_absolute():
        original = config.root / original

    source_hint = source_hint_key(target.get("source_hint"))
    compiled = artifact_compiled_paths(config).get(source_hint)
    if compiled is None and target.get("program_path"):
        compiled = compiled_bin_path_from_program_path(
            config, str(target["program_path"])
        )

    return BinaryPair(
        original=original,
        compiled=compiled,
        load_address=int(payload.get("ram_ptr") or 0),
    )


def objdump_path(config: HarnessConfig) -> Path:
    return config.root / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-objdump"


def disassemble_binary(config: HarnessConfig, path: Path, *, load_address: int) -> str:
    tool = objdump_path(config)
    if not tool.is_file():
        raise FileNotFoundError(f"objdump not found: {tool}")
    result = run_process(
        [
            tool,
            "-b",
            "binary",
            "-m",
            "mips:3000",
            "-EL",
            "-D",
            f"--adjust-vma=0x{load_address:08x}",
            path,
        ],
        capture=True,
        check=True,
    )
    return result.stdout


def hex_lines(path: Path) -> list[str]:
    data = path.read_bytes()
    lines = []
    for offset in range(0, len(data), 16):
        chunk = data[offset : offset + 16]
        text = " ".join(f"{byte:02x}" for byte in chunk)
        lines.append(f"{offset:08x}: {text}")
    return lines


def build_binary_diff(
    config: HarnessConfig,
    target: dict[str, Any],
    *,
    output_root: Path,
) -> tuple[dict[str, Any], Path]:
    pair = resolve_binary_pair(config, target)
    output_dir = output_root / safe_name(str(target["id"]))
    output_dir.mkdir(parents=True, exist_ok=True)
    json_path = output_dir / "binary-diff.json"

    original_exists = bool(pair.original and pair.original.is_file())
    compiled_exists = bool(pair.compiled and pair.compiled.is_file())
    status = "ready"
    next_action = "inspect binary asm diff"
    if not original_exists:
        status = "missing_original_bin"
        next_action = "run bin/harness refresh after EMI unpack"
    elif not compiled_exists:
        status = "missing_compiled_bin"
        next_action = "register a compiled raw .bin output path for this target"

    payload: dict[str, Any] = {
        "schema": "rebof3-simple.harness-binary-diff/v1",
        "target_id": target["id"],
        "status": status,
        "next_action": next_action,
        "original_bin": None if pair.original is None else str(pair.original),
        "compiled_bin": None if pair.compiled is None else str(pair.compiled),
        "original_exists": original_exists,
        "compiled_exists": compiled_exists,
        "load_address": f"0x{pair.load_address:08x}",
    }

    if original_exists:
        payload["original_size"] = pair.original.stat().st_size
        payload["original_sha256"] = sha256(pair.original)
    if compiled_exists:
        payload["compiled_size"] = pair.compiled.stat().st_size
        payload["compiled_sha256"] = sha256(pair.compiled)
    if original_exists and compiled_exists:
        payload["exact_match"] = (
            payload["original_sha256"] == payload["compiled_sha256"]
        )
        payload["status"] = "exact_match" if payload["exact_match"] else "different"
        try:
            original_asm = disassemble_binary(
                config, pair.original, load_address=pair.load_address
            )
            compiled_asm = disassemble_binary(
                config, pair.compiled, load_address=pair.load_address
            )
            (output_dir / "original.s").write_text(original_asm, encoding="utf-8")
            (output_dir / "compiled.s").write_text(compiled_asm, encoding="utf-8")
            diff_lines = list(
                difflib.unified_diff(
                    original_asm.splitlines(),
                    compiled_asm.splitlines(),
                    fromfile=str(pair.original),
                    tofile=str(pair.compiled),
                    lineterm="",
                )
            )
            diff_path = output_dir / "asm.diff"
        except Exception:
            diff_lines = list(
                difflib.unified_diff(
                    hex_lines(pair.original),
                    hex_lines(pair.compiled),
                    fromfile=str(pair.original),
                    tofile=str(pair.compiled),
                    lineterm="",
                )
            )
            diff_path = output_dir / "hex.diff"
        diff_path.write_text("\n".join(diff_lines[:1000]) + "\n", encoding="utf-8")
        payload["diff_path"] = str(diff_path)

    write_json(json_path, payload)
    return payload, json_path


def binary_diff_exit_code(statuses: list[str], *, allow_different: bool) -> int:
    if allow_different:
        return (
            0
            if all(status in {"exact_match", "different"} for status in statuses)
            else 1
        )
    return 0 if all(status == "exact_match" for status in statuses) else 1
