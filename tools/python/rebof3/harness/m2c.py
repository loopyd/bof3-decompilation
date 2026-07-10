from __future__ import annotations

import subprocess
from dataclasses import dataclass
from pathlib import Path
import re
from typing import Any

from ..jsonio import read_json, write_json
from ..match.asm_diff import (
    disassemble_original,
    extract_original_bytes,
    format_hex,
    parse_int,
)
from .config import HarnessConfig
from .context import build_context_header
from .tasks import source_function_payload
from .workspace import workspace_dir


STAGED_EMI_PROGRAM_RE = re.compile(
    r"^(?P<archive>.+)_e(?P<entry>[0-9]+)_[0-9a-fA-F]{8}\.bin$"
)
OBJDUMP_INSTRUCTION_RE = re.compile(
    r"^\s*(?P<address>[0-9a-fA-F]+):\s+[0-9a-fA-F]{8}\s+(?P<instruction>.+?)\s*$"
)
LOCAL_BRANCH_MNEMONICS = {
    "b",
    "beq",
    "beql",
    "beqz",
    "bge",
    "bgez",
    "bgezal",
    "bgtz",
    "blez",
    "bltz",
    "bltzal",
    "bne",
    "bnel",
    "bnez",
    "j",
}


@dataclass(frozen=True)
class FunctionInput:
    address: int
    binary_path: Path
    function_name: str
    load_address: int | None
    size: int


def _hex_without_prefix(value: Any) -> str:
    return str(value or "").lower().removeprefix("0x")


def _canonical_function_name(name: str, address: int) -> str:
    if name.startswith("FUN_") and len(name) == 12:
        suffix = name.removeprefix("FUN_").lower()
        if all(char in "0123456789abcdef" for char in suffix):
            return f"func_{suffix}"
    return name or f"func_{address:08x}"


def _canonical_signature(signature: str, address: int) -> str:
    rendered = signature.strip().rstrip(";")
    if not rendered:
        return ""
    rendered = rendered.replace("undefined FUN_", "void func_", 1)
    rendered = rendered.replace(f"FUN_{address:08x}", f"func_{address:08x}")
    return rendered


def _function_size_from_target(target: dict[str, Any], payload: dict[str, Any]) -> int:
    if payload.get("size") is not None:
        return int(payload["size"])
    body_min = _hex_without_prefix(payload.get("body_min"))
    body_max = _hex_without_prefix(payload.get("body_max"))
    if body_min and body_max:
        return int(body_max, 16) - int(body_min, 16) + 1
    raise ValueError(f"cannot infer function size for {target['id']}")


def _function_name(
    target: dict[str, Any], payload: dict[str, Any], address: int
) -> str:
    source_path = payload.get("source_path")
    if source_path:
        return Path(str(source_path)).stem
    name = str(payload.get("name") or "")
    if name and name not in {"None", "<None>"}:
        return _canonical_function_name(name, address)
    return f"func_{address:08x}"


def _path_from_program(config: HarnessConfig, program_path: str) -> Path | None:
    if program_path == "/boot/SLUS_004.22":
        return config.root / "out/extracted/SLUS_004.22"
    if program_path == "/boot/LOGO.EXE":
        return config.root / "out/extracted/LOGO/LOGO.EXE"
    if not program_path.startswith("/bins/"):
        return None
    parts = list(Path(program_path.removeprefix("/bins/")).parts)
    if parts and parts[0] == "BIN":
        parts = parts[1:]
    raw_path = config.root / "out/extracted/BIN" / Path(*parts)
    if raw_path.is_file() or not parts:
        return raw_path
    match = STAGED_EMI_PROGRAM_RE.match(parts[-1])
    if not match:
        return raw_path
    raw_dir = config.root / "out/extracted/BIN" / Path(*parts[:-1])
    decimal_entry = int(match.group("entry"), 10)
    decimal_path = raw_dir / f"{decimal_entry}.bin"
    if decimal_path.is_file():
        return decimal_path
    hex_entry = int(match.group("entry"), 16)
    hex_path = raw_dir / f"{hex_entry}.bin"
    if hex_path.is_file():
        return hex_path
    return decimal_path


def _load_address_from_manifest(binary_path: Path) -> int | None:
    manifest = binary_path.parent / "emi.json"
    if not manifest.is_file():
        return None
    payload = read_json(manifest)
    entries = payload.get("entries", [])
    if not isinstance(entries, list):
        return None
    for entry in entries:
        if not isinstance(entry, dict):
            continue
        if (
            str(entry.get("name") or f"{entry.get('index', '')}.bin")
            == binary_path.name
        ):
            ram_ptr = entry.get("ram_ptr")
            return int(ram_ptr) if ram_ptr is not None else None
    return None


def resolve_function_input(
    config: HarnessConfig, target: dict[str, Any]
) -> FunctionInput:
    payload = target.get("payload") if isinstance(target.get("payload"), dict) else {}
    if str(target.get("id") or "").startswith("func-src:"):
        source_path = payload.get("source_path") or str(target["id"]).removeprefix(
            "func-src:"
        )
        source_payload = source_function_payload(config, Path(str(source_path)))
        payload = {**payload, **source_payload}
    entry = target.get("entry_hex") or payload.get("entry_hex") or payload.get("entry")
    if not entry:
        raise ValueError(f"target has no function entry: {target['id']}")
    address = parse_int(str(entry))
    binary_path = (
        Path(str(payload["binary_path"]))
        if payload.get("binary_path")
        else _path_from_program(config, str(target.get("program_path") or ""))
    )
    if binary_path is None:
        raise ValueError(f"cannot infer original binary for {target['id']}")
    if not binary_path.is_absolute():
        binary_path = config.root / binary_path
    load_address = (
        int(payload["load_address"])
        if payload.get("load_address") is not None
        else _load_address_from_manifest(binary_path)
    )
    return FunctionInput(
        address=address,
        binary_path=binary_path,
        function_name=_function_name(target, payload, address),
        load_address=load_address,
        size=_function_size_from_target(target, payload),
    )


def render_m2c_asm(function_name: str, normalized_lines: list[str]) -> str:
    body = "\n".join(f"    {line}" for line in normalized_lines)
    return f"{function_name}:\n{body}\n"


def render_m2c_asm_from_objdump(
    function_name: str, objdump_text: str, *, address: int, size: int
) -> str:
    instructions: list[tuple[int, str]] = []
    end = address + size
    for raw_line in objdump_text.splitlines():
        match = OBJDUMP_INSTRUCTION_RE.match(raw_line)
        if match is None:
            continue
        instruction_address = int(match.group("address"), 16)
        instruction = re.sub(r"\s+", " ", match.group("instruction").strip())
        instructions.append((instruction_address, instruction))

    labels: set[int] = set()
    branch_target_re = re.compile(r"(?:^|[\s,])(0x[0-9a-fA-F]{8})$")
    for _, instruction in instructions:
        mnemonic = instruction.split(" ", 1)[0]
        if mnemonic not in LOCAL_BRANCH_MNEMONICS:
            continue
        if (match := branch_target_re.search(instruction)) is None:
            continue
        target = int(match.group(1), 16)
        if address <= target < end:
            labels.add(target)

    lines = [f"{function_name}:"]
    for instruction_address, instruction in instructions:
        if instruction_address in labels:
            lines.append(f".L{instruction_address:08x}:")
        mnemonic = instruction.split(" ", 1)[0]
        if mnemonic in LOCAL_BRANCH_MNEMONICS:
            for target in labels:
                instruction = instruction.replace(
                    f"0x{target:08x}", f".L{target:08x}", 1
                )
        lines.append(f"    {instruction}")
    return "\n".join(lines) + "\n"


def _strip_preprocessor_directives(text: str) -> str:
    """Remove all preprocessor directive lines from flattened context.

    m2c's ``--context`` requires C that has already been run through the
    preprocessor.  After we flatten the include tree we must therefore strip
    every remaining ``#`` line.
    """
    return "\n".join(
        line for line in text.splitlines() if not line.strip().startswith("#")
    )


def _strip_include_guard(text: str) -> str:
    """Remove the top-level include guard (#ifndef / #define / #endif) trio.

    Only strips when the *first* ``#ifndef`` is immediately followed by
    ``#define`` and a final ``#endif`` exists — i.e. the idiomatic guard shape.
    """
    lines = text.splitlines()
    ifndef_idx = -1
    define_idx = -1
    endif_idx = -1

    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith("#ifndef") and ifndef_idx == -1:
            ifndef_idx = i
        elif s.startswith("#define") and ifndef_idx != -1 and define_idx == -1:
            define_idx = i
        elif s.startswith("#endif"):
            endif_idx = i

    # Only strip if the #define IMMEDIATELY follows the #ifndef (typical guard)
    if ifndef_idx != -1 and define_idx == ifndef_idx + 1 and endif_idx > define_idx:
        stripped = "\n".join(
            line
            for i, line in enumerate(lines)
            if i not in (ifndef_idx, define_idx, endif_idx)
        )
        return _strip_preprocessor_directives(stripped)
    return _strip_preprocessor_directives(text)


def _collect_flat_context(
    start_path: Path, root: Path, *, seen: set[Path] | None = None
) -> str:
    """Recursively flatten a context header tree into a single C source string,

    resolving ``#include`` paths first relative to the including file's
    directory, then falling back to the project *root*, stripping include guards.
    """
    if seen is None:
        seen = set()
    resolved = start_path.resolve()
    if resolved in seen:
        return ""
    seen.add(resolved)

    text = start_path.read_text(encoding="utf-8")
    text = _strip_include_guard(text)

    parts: list[str] = []
    for line in text.splitlines():
        s = line.strip()
        if not s:
            continue
        if s.startswith("#include"):
            # System includes (#include <...>) cannot be resolved as local
            # files AND haven't been preprocessed, so drop them entirely.
            if s[9:10] == "<":
                continue
            inc_path = (
                s.split('"')[1]
                if '"' in s
                else (s.split()[1] if len(s.split()) > 1 else "")
            )
            if not inc_path:
                continue
            # Try relative to including file first, then project root,
            # then bof3/include/ (the -I path used by the real build)
            inc_candidate = (start_path.parent / inc_path).resolve()
            if not inc_candidate.is_file():
                inc_candidate = (root / inc_path).resolve()
            if not inc_candidate.is_file():
                inc_candidate = (root / "include" / inc_path).resolve()
            if inc_candidate.is_file():
                parts.append(f"/* from {inc_path} */")
                parts.append(_collect_flat_context(inc_candidate, root, seen=seen))
            continue
        parts.append(line)
    return "\n".join(parts)


def render_m2c_context(
    target: dict[str, Any],
    *,
    context_header_path: Path | None = None,
    root: Path | None = None,
) -> str:
    payload = target.get("payload") if isinstance(target.get("payload"), dict) else {}
    entry = target.get("entry_hex") or payload.get("entry_hex") or payload.get("entry")
    address = parse_int(str(entry)) if entry else 0
    signature = _canonical_signature(
        str(payload.get("signature") or payload.get("type_spec") or ""),
        address,
    )
    lines = [
        "typedef unsigned char u8;",
        "typedef unsigned short u16;",
        "typedef unsigned int u32;",
        "typedef unsigned long long u64;",
        "typedef signed char s8;",
        "typedef signed short s16;",
        "typedef signed int s32;",
        "typedef signed long long s64;",
        "typedef unsigned char u_char;",
        "typedef unsigned short u_short;",
        "typedef unsigned int u_int;",
        "typedef unsigned long u_long;",
        "",
    ]
    if context_header_path and context_header_path.is_file() and root is not None:
        flat = _collect_flat_context(context_header_path, root)
        if flat.strip():
            lines.append(flat)
            lines.append("")
    else:
        if signature:
            lines.append(signature + ";")
            lines.append("")
    return "\n".join(lines)


def run_m2c_for_target(
    config: HarnessConfig,
    target: dict[str, Any],
    *,
    extra_args: list[str] | None = None,
) -> tuple[dict[str, Any], Path]:
    function = resolve_function_input(config, target)
    root = workspace_dir(config, str(target["id"]))
    root.mkdir(parents=True, exist_ok=True)
    context_h = build_context_header(config, target)

    original_bytes = extract_original_bytes(
        function.binary_path,
        address=function.address,
        size=function.size,
        load_address=function.load_address,
    )
    original_bytes_path = root / "original.bin"
    original_bytes_path.write_bytes(original_bytes)

    objdump_path = (
        config.root / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-objdump"
    )
    original_objdump = disassemble_original(
        objdump_path=objdump_path,
        original_bytes_path=original_bytes_path,
        address=function.address,
    )
    original_asm = root / "original.s"
    original_asm.write_text(
        render_m2c_asm_from_objdump(
            function.function_name,
            original_objdump,
            address=function.address,
            size=function.size,
        ),
        encoding="utf-8",
    )

    m2c_context = root / "m2c_context.c"
    m2c_context.write_text(
        render_m2c_context(target, context_header_path=context_h, root=config.root),
        encoding="utf-8",
    )
    ghidra_json = root / "ghidra.json"
    write_json(
        ghidra_json,
        {
            "target": target,
            "function": {
                "address": format_hex(function.address),
                "binary_path": str(function.binary_path),
                "function_name": function.function_name,
                "load_address": None
                if function.load_address is None
                else format_hex(function.load_address),
                "size": function.size,
            },
        },
    )

    output_c = root / "func.m2c.c"
    log_path = root / "m2c.log"
    command = [
        str(config.root / "third_party/m2c/m2c.py"),
        "--target",
        "mipsel-gcc-c",
        "--stack-structs",
        "--unk-underscore",
        "--globals",
        "none",
        "--valid-syntax",
        "--no-cache",
        "--context",
        str(m2c_context),
        str(original_asm),
        "--function",
        function.function_name,
        *(extra_args or []),
    ]
    result = subprocess.run(
        command,
        cwd=config.root,
        check=False,
        capture_output=True,
        text=True,
    )
    output_c.write_text(result.stdout, encoding="utf-8")
    log_text = result.stderr
    if result.returncode != 0 and result.stdout:
        log_text = result.stdout + ("\n" if result.stderr else "") + result.stderr
    log_path.write_text(log_text, encoding="utf-8")

    notes = root / "notes.md"
    notes.write_text(
        "# m2c Draft\n\n"
        f"- target: `{target['id']}`\n"
        f"- function: `{function.function_name}` at `{format_hex(function.address)}`\n"
        f"- original: `{function.binary_path}`\n"
        f"- load address: `{'' if function.load_address is None else format_hex(function.load_address)}`\n"
        f"- context: `{context_h}`\n"
        "- proof command: `bin/harness verify function <source>`\n",
        encoding="utf-8",
    )
    payload = {
        "schema": "rebof3-simple.harness-m2c/v1",
        "status": "ok" if result.returncode == 0 else "failed",
        "returncode": result.returncode,
        "target_id": target["id"],
        "function": function.function_name,
        "address": format_hex(function.address),
        "original_binary": str(function.binary_path),
        "load_address": None
        if function.load_address is None
        else format_hex(function.load_address),
        "size": function.size,
        "command": command,
        "outputs": {
            "workspace": str(root),
            "original_asm": str(original_asm),
            "context": str(context_h),
            "m2c_context": str(m2c_context),
            "m2c_c": str(output_c),
            "ghidra_json": str(ghidra_json),
            "notes": str(notes),
            "log": str(log_path),
        },
        "next_action": "edit src/... then run bin/harness verify function <source>",
    }
    write_json(root / "m2c.json", payload)
    return payload, root / "m2c.json"
