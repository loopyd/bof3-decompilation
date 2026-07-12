from __future__ import annotations

import re
import subprocess
from pathlib import Path

INSTRUCTION_RE = re.compile(
    r"^\s*(?P<address>[0-9a-fA-F]+):\s+[0-9a-fA-F]{8}\s+(?P<instruction>.+?)\s*$"
)
SYMBOL_SIZE_RE = re.compile(
    r"^(?P<address>[0-9a-fA-F]+)\s+(?P<size>[0-9a-fA-F]+)\s+[A-Za-z]\s+(?P<name>\S+)$"
)


def format_hex(value: int) -> str:
    return f"0x{value:08x}"


def run_command(
    argv: list[str], *, cwd: Path | None = None
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        argv,
        cwd=cwd,
        check=False,
        capture_output=True,
        text=True,
    )


def disassemble_original(
    *,
    objdump_path: Path,
    original_bytes_path: Path,
    address: int,
) -> str:
    result = run_command(
        [
            str(objdump_path),
            "-D",
            "-b",
            "binary",
            "-m",
            "mips:3000",
            "-EL",
            f"--adjust-vma={format_hex(address)}",
            str(original_bytes_path),
        ]
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr or result.stdout)
    return result.stdout


def disassemble_linked(*, objdump_path: Path, linked_path: Path) -> str:
    result = run_command([str(objdump_path), "-d", str(linked_path)])
    if result.returncode != 0:
        raise RuntimeError(result.stderr or result.stdout)
    return result.stdout


def current_symbol_size(
    nm_path: Path, object_path: Path, function_name: str
) -> int | None:
    result = run_command([str(nm_path), "-S", str(object_path)])
    if result.returncode != 0:
        return None
    for raw_line in result.stdout.splitlines():
        match = SYMBOL_SIZE_RE.match(raw_line.strip())
        if match is None or match.group("name") != function_name:
            continue
        return int(match.group("size"), 16)
    return None


_COMMENT_RE = re.compile(r"\s*<[^>]*>")


def extract_instructions(disassembly: str) -> list[str]:
    lines: list[str] = []
    for raw_line in disassembly.splitlines():
        if raw_line.strip().startswith("R_MIPS_"):
            continue
        match = INSTRUCTION_RE.match(raw_line)
        if match is None:
            continue
        instr = re.sub(r"\s+", " ", match.group("instruction").strip())
        instr = _COMMENT_RE.sub("", instr)
        instr = _normalize_hex(instr)
        lines.append(instr)
    return lines


def _normalize_hex(instruction: str) -> str:
    parts = instruction.split()
    normalized: list[str] = []
    for p in parts:
        m = re.fullmatch(r"([-+]?)([0-9a-fA-F]+)(.*)", p)
        if m is not None:
            sign, digits, suffix = m.group(1), m.group(2), m.group(3)
            if re.fullmatch(r"[0-9a-fA-F]{5,}", digits) and not suffix.startswith("0x"):
                normalized.append(f"{sign}0x{digits}{suffix}")
                continue
        normalized.append(p)
    return " ".join(normalized)


def render_normalized(lines: list[str]) -> str:
    return "\n".join(lines) + ("\n" if lines else "")
