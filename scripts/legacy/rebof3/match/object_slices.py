from __future__ import annotations

from pathlib import Path
import re
from typing import NamedTuple

from ..common import ROOT, run_command


OBJDUMP = ROOT / "deps" / "psn00b_toolchain" / "bin" / "mipsel-none-elf-objdump"
AS = ROOT / "deps" / "psn00b_toolchain" / "bin" / "mipsel-none-elf-as"
READELF = ROOT / "deps" / "psn00b_toolchain" / "bin" / "mipsel-none-elf-readelf"


class FunctionSlice(NamedTuple):
    symbol_name: str
    start_offset: int
    size: int
    asm_text: str


class AddressSymbolResolver(NamedTuple):
    function_symbols: dict[int, str]
    data_symbols: dict[int, str]

    def function_symbol(self, address: int) -> str | None:
        return self.function_symbols.get(address)

    def data_symbol(self, address: int) -> str | None:
        return self.data_symbols.get(address)


def normalize_function_symbol_name(name: str, address: int) -> str:
    if name == f"FUN_{address:08x}" or name == f"fun_{address:08x}":
        return f"func_{address:08x}"
    return name


def normalize_asm_symbol_name(name: str, fallback_prefix: str, address: int) -> str:
    normalized = re.sub(r"[^A-Za-z0-9_]", "_", name)
    if not normalized:
        return f"{fallback_prefix}_{address:08x}"
    if not re.match(r"[A-Za-z_]", normalized[0]):
        normalized = f"_{normalized}"
    return normalized


DISASM_WORD_RE = re.compile(r"^\s*(?P<offset>[0-9a-f]+):\s+(?P<word>[0-9a-f]{8})\b")
SYMBOL_HEADER_RE = re.compile(r"^\s*(?P<offset>[0-9a-fA-F]+)\s+<(?P<name>[^>]+)>:$")
REGISTER_RE = re.compile(
    r"(?<![$A-Za-z0-9_])(?P<name>zero|at|v0|v1|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)(?![A-Za-z0-9_])"
)
COMMENTED_ASM_RE = re.compile(
    r"^/\*\s*(?P<addr>[0-9a-fA-F]{8})\s*\*/\s*(?P<opcode>.+)$"
)
HEX_TARGET_RE = re.compile(r"0x[0-9a-fA-F]+")
BREAK_IMMEDIATE_RE = re.compile(r"^break\s+(0x[0-9a-fA-F]+|[0-9]+)$")
LUI_RE = re.compile(
    r"^lui\s+(?P<reg>\$[A-Za-z0-9]+),\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|[0-9]+))$"
)
LOADSTORE_RE = re.compile(
    r"^(?P<mnemonic>[A-Za-z0-9]+)\s+(?P<dst>\$[A-Za-z0-9]+),\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|[0-9]+))\((?P<base>\$[A-Za-z0-9]+)\)$"
)
THREE_ARG_IMM_RE = re.compile(
    r"^(?P<mnemonic>addiu|ori)\s+(?P<dst>\$[A-Za-z0-9]+),\s*(?P<src>\$[A-Za-z0-9]+),\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|[0-9]+))$"
)
JAL_NUMERIC_RE = re.compile(r"^jal\s+(0x[0-9a-fA-F]+)$")
LOCAL_BRANCH_MNEMONICS = {
    "b",
    "beq",
    "beqz",
    "bgez",
    "bgtz",
    "blez",
    "bltz",
    "bne",
    "bnez",
    "j",
}


def parse_symbol_table(text: str, symbol_name: str) -> tuple[int, int] | None:
    for line in text.splitlines():
        fields = line.split()
        if len(fields) < 6:
            continue
        if fields[-1] != symbol_name:
            continue
        if "F" not in fields:
            continue
        return int(fields[0], 16), int(fields[-2], 16)
    return None


def symbol_bounds(object_path: Path, symbol_name: str) -> tuple[int, int]:
    result = run_command([str(OBJDUMP), "-t", str(object_path)])
    if result.returncode != 0:
        raise RuntimeError(
            result.stderr.strip() or f"objdump -t failed for {object_path}"
        )
    parsed = parse_symbol_table(result.stdout, symbol_name)
    if parsed is None:
        raise LookupError(f"symbol not found in object: {symbol_name}")
    return parsed


def function_disassembly(
    object_path: Path,
    symbol_name: str,
    *,
    start_offset: int | None = None,
    size: int | None = None,
) -> str:
    result = run_command([str(OBJDUMP), "-dr", str(object_path)])
    if result.returncode != 0:
        raise RuntimeError(
            result.stderr.strip() or f"objdump -dr failed for {object_path}"
        )

    lines = result.stdout.splitlines()
    header = f"<{symbol_name}>:"
    end_offset = None if start_offset is None or size is None else start_offset + size
    capture = False
    block: list[str] = []
    for line in lines:
        if line.endswith(header):
            capture = True
            block.append(line)
            continue
        if capture and line.startswith("Disassembly of section "):
            break
        if capture and end_offset is not None:
            symbol_match = SYMBOL_HEADER_RE.match(line)
            if symbol_match is not None and not line.endswith(header):
                if int(symbol_match.group("offset"), 16) >= end_offset:
                    break
            word_match = DISASM_WORD_RE.match(line)
            if word_match is not None:
                if int(word_match.group("offset"), 16) >= end_offset:
                    break
        if (
            capture
            and end_offset is None
            and line.startswith("000")
            and line.endswith(":")
            and not line.endswith(header)
        ):
            break
        if capture:
            block.append(line)
    if not block:
        raise LookupError(f"disassembly not found for symbol: {symbol_name}")
    return "\n".join(block).rstrip() + "\n"


def function_words_from_disassembly(disassembly_text: str) -> list[int]:
    words: list[int] = []
    for line in disassembly_text.splitlines()[1:]:
        match = DISASM_WORD_RE.match(line)
        if match is None:
            continue
        words.append(int(match.group("word"), 16))
    if not words:
        raise ValueError("no instruction words found in disassembly")
    return words


def render_word_assembly(symbol_name: str, words: list[int]) -> str:
    lines = [
        ".set noreorder",
        '.section .text, "ax"',
        f".globl {symbol_name}",
        f".type {symbol_name}, @function",
        f"{symbol_name}:",
    ]
    lines.extend(f".word 0x{word:08x}" for word in words)
    lines.append(f".size {symbol_name}, .-{symbol_name}")
    return "\n".join(lines) + "\n"


def rename_top_level_symbol(asm_text: str, old_symbol: str, new_symbol: str) -> str:
    result_lines: list[str] = []
    for line in asm_text.splitlines():
        stripped = line.strip()
        if stripped == f".globl {old_symbol}":
            result_lines.append(line.replace(old_symbol, new_symbol, 1))
            continue
        if stripped == f"{old_symbol}:":
            result_lines.append(line.replace(old_symbol, new_symbol, 1))
            continue
        if stripped == f".ent {old_symbol}":
            result_lines.append(line.replace(old_symbol, new_symbol, 1))
            continue
        if stripped == f".end {old_symbol}":
            result_lines.append(line.replace(old_symbol, new_symbol, 1))
            continue
        result_lines.append(line)
    rewritten = "\n".join(result_lines).strip() + "\n"
    if not rewritten.startswith(".set noreorder\n"):
        rewritten = ".set noreorder\n" + rewritten
    return rewritten


def normalize_expected_asm(asm_text: str) -> str:
    lines: list[str] = []
    for line in asm_text.splitlines():
        if line.lstrip().startswith("/*") and "*/" in line:
            _, remainder = line.split("*/", 1)
            line = remainder.strip()
            if not line:
                continue
        line = normalize_expected_opcode(line)
        lines.append(line)
    rewritten = "\n".join(lines).strip() + "\n"
    if not rewritten.startswith(".set noreorder\n"):
        rewritten = ".set noreorder\n" + rewritten
    return rewritten


def normalize_expected_opcode(opcode: str) -> str:
    rewritten = REGISTER_RE.sub(lambda match: "$" + match.group("name"), opcode)
    match = BREAK_IMMEDIATE_RE.fullmatch(rewritten.strip())
    if match is None:
        return rewritten
    immediate = int(match.group(1), 0)
    encoded = ((immediate & 0xFFFFF) << 6) | 0x0D
    return f".word 0x{encoded:08x}"


def parse_int(text: str) -> int:
    return int(text, 0)


def sign_extend_16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def combine_hi_lo(hi_value: int, low_value: int) -> int:
    return ((hi_value & 0xFFFF) << 16) + sign_extend_16(low_value)


def rewrite_absolute_jal(opcode: str, resolver: AddressSymbolResolver | None) -> str:
    if resolver is None:
        return opcode
    match = JAL_NUMERIC_RE.match(opcode)
    if match is None:
        return opcode
    symbol = resolver.function_symbol(parse_int(match.group(1)))
    if symbol is None:
        return opcode
    return f"jal {normalize_asm_symbol_name(symbol, 'func', parse_int(match.group(1)))}"


def rewrite_hi_lo_pair(
    *,
    body_lines: list[str],
    opcode: str,
    pending_hi: dict[str, tuple[int, int]],
    resolver: AddressSymbolResolver | None,
) -> str:
    if resolver is None:
        return opcode
    loadstore = LOADSTORE_RE.match(opcode)
    if loadstore is not None:
        base = loadstore.group("base")
        pending = pending_hi.get(base)
        if pending is not None:
            full_address = combine_hi_lo(pending[0], parse_int(loadstore.group("imm")))
            symbol = resolver.data_symbol(full_address)
            if symbol is not None:
                symbol = normalize_asm_symbol_name(symbol, "DAT", full_address)
                body_lines[pending[1]] = f"lui {base}, %hi({symbol})"
                return (
                    f"{loadstore.group('mnemonic')} {loadstore.group('dst')}, "
                    f"%lo({symbol})({base})"
                )
        return opcode
    three_arg = THREE_ARG_IMM_RE.match(opcode)
    if three_arg is None:
        return opcode
    src = three_arg.group("src")
    pending = pending_hi.get(src)
    if pending is None:
        return opcode
    full_address = combine_hi_lo(pending[0], parse_int(three_arg.group("imm")))
    symbol = resolver.data_symbol(full_address)
    if symbol is None:
        return opcode
    symbol = normalize_asm_symbol_name(symbol, "DAT", full_address)
    body_lines[pending[1]] = f"lui {src}, %hi({symbol})"
    return (
        f"{three_arg.group('mnemonic')} {three_arg.group('dst')}, {src}, %lo({symbol})"
    )


def local_label_for_address(address: int) -> str:
    return f".L{address:08x}"


def rewrite_branch_target(opcode: str, known_addresses: set[int]) -> str:
    parts = opcode.split(None, 1)
    mnemonic = parts[0] if parts else ""
    if mnemonic not in LOCAL_BRANCH_MNEMONICS:
        return opcode
    match = HEX_TARGET_RE.search(opcode)
    if match is None:
        return opcode
    target = int(match.group(0), 16)
    if target not in known_addresses:
        return opcode
    return (
        opcode[: match.start()]
        + local_label_for_address(target)
        + opcode[match.end() :]
    )


def extract_expected_body_lines(
    asm_text: str,
    symbol_name: str,
    *,
    resolver: AddressSymbolResolver | None = None,
) -> list[str]:
    normalized_lines = asm_text.splitlines()
    commented_ops: list[tuple[int, str]] = []
    plain_lines: list[str] = []
    for line in normalized_lines:
        stripped = line.strip()
        if not stripped:
            continue
        if (
            stripped.startswith(".include ")
            or stripped.startswith(".section ")
            or stripped.startswith(".align ")
            or stripped.startswith(".type ")
            or stripped.startswith(".size ")
            or stripped.startswith(".set ")
            or stripped.startswith("nonmatching ")
            or stripped.startswith("glabel ")
            or stripped.startswith("endlabel ")
        ):
            continue
        if stripped in {
            ".set noreorder",
            ".text",
            f".globl {symbol_name}",
            f".ent {symbol_name}",
            f".end {symbol_name}",
        }:
            continue
        if stripped == f"{symbol_name}:":
            continue
        comment_match = COMMENTED_ASM_RE.match(stripped)
        if comment_match is not None:
            commented_ops.append(
                (
                    int(comment_match.group("addr"), 16),
                    normalize_expected_opcode(comment_match.group("opcode")),
                )
            )
            continue
        stripped = normalize_expected_opcode(stripped)
        plain_lines.append(stripped)

    known_addresses = {address for address, _ in commented_ops}
    body_lines: list[str] = []
    pending_hi: dict[str, tuple[int, int]] = {}
    for address, opcode in commented_ops:
        opcode = rewrite_absolute_jal(opcode, resolver)
        if address in known_addresses and address != commented_ops[0][0]:
            branch_targets = {
                int(match.group(0), 16)
                for _, candidate_opcode in commented_ops
                for match in [HEX_TARGET_RE.search(candidate_opcode)]
                if match is not None
                and candidate_opcode.split(None, 1)[0] in LOCAL_BRANCH_MNEMONICS
                and int(match.group(0), 16) in known_addresses
            }
            if address in branch_targets:
                body_lines.append(local_label_for_address(address) + ":")
        opcode = rewrite_hi_lo_pair(
            body_lines=body_lines,
            opcode=opcode,
            pending_hi=pending_hi,
            resolver=resolver,
        )
        body_lines.append(rewrite_branch_target(opcode, known_addresses))
        lui_match = LUI_RE.match(opcode)
        if lui_match is not None:
            pending_hi[lui_match.group("reg")] = (
                parse_int(lui_match.group("imm")),
                len(body_lines) - 1,
            )
    body_lines.extend(plain_lines)
    return body_lines


def render_expected_assembly(symbol_name: str, body_lines: list[str]) -> str:
    lines = [
        ".set noreorder",
        '.section .text, "ax"',
        f".globl {symbol_name}",
        f".type {symbol_name}, @function",
        f"{symbol_name}:",
        *body_lines,
        f".size {symbol_name}, .-{symbol_name}",
    ]
    return "\n".join(lines) + "\n"


def patch_expected_asm_text(
    asm_text: str,
    *,
    original_symbol_name: str,
    target_symbol_name: str,
    resolver: AddressSymbolResolver | None = None,
) -> str:
    rewritten = rename_top_level_symbol(
        asm_text,
        old_symbol=original_symbol_name,
        new_symbol=target_symbol_name,
    )
    return render_expected_assembly(
        target_symbol_name,
        extract_expected_body_lines(
            rewritten,
            target_symbol_name,
            resolver=resolver,
        ),
    )


def slice_from_object(object_path: Path, symbol_name: str) -> FunctionSlice:
    start_offset, size = symbol_bounds(object_path, symbol_name)
    return FunctionSlice(
        symbol_name=symbol_name,
        start_offset=start_offset,
        size=size,
        asm_text=function_disassembly(
            object_path,
            symbol_name,
            start_offset=start_offset,
            size=size,
        ),
    )


def assemble_text(asm_path: Path, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    result = run_command(
        [str(AS), "-march=r3000", "-mabi=32", "-o", str(output_path), str(asm_path)]
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or f"assembler failed for {asm_path}")


def write_current_slice_asm(slice_data: FunctionSlice, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    words = function_words_from_disassembly(slice_data.asm_text)
    output_path.write_text(
        render_word_assembly(slice_data.symbol_name, words),
        encoding="utf-8",
    )


def write_expected_slice_asm(
    baseline_asm_path: Path,
    *,
    original_symbol_name: str,
    target_symbol_name: str,
    output_path: Path,
    resolver: AddressSymbolResolver | None = None,
) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        patch_expected_asm_text(
            baseline_asm_path.read_text(encoding="utf-8"),
            original_symbol_name=original_symbol_name,
            target_symbol_name=target_symbol_name,
            resolver=resolver,
        ),
        encoding="utf-8",
    )
