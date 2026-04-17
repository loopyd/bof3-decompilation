from __future__ import annotations

import re
from typing import NamedTuple


class AddressSymbolResolver(NamedTuple):
    function_symbols: dict[int, str]
    data_symbols: dict[int, str]

    def function_symbol(self, address: int) -> str | None:
        return self.function_symbols.get(address)

    def data_symbol(self, address: int) -> str | None:
        return self.data_symbols.get(address)


ASM_LINE_RE = re.compile(r"^\s*/\*\s*(?P<addr>[0-9a-fA-F]+)\s*\*/\s*(?P<body>.*)$")
HEX_TARGET_RE = re.compile(r"0x[0-9a-fA-F]+")
LUI_RE = re.compile(
    r"^lui\s+(?P<reg>[A-Za-z0-9$_]+),\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|[0-9]+))$"
)
LOADSTORE_RE = re.compile(
    r"^(?P<mnemonic>[A-Za-z0-9]+)\s+(?P<dst>[A-Za-z0-9$_]+),\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|[0-9]+))\((?P<base>[A-Za-z0-9$_]+)\)$"
)
THREE_ARG_IMM_RE = re.compile(
    r"^(?P<mnemonic>addiu|ori)\s+(?P<dst>[A-Za-z0-9$_]+),\s*(?P<src>[A-Za-z0-9$_]+),\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|[0-9]+))$"
)
ABSOLUTE_CALL_RE = re.compile(r"^(?P<mnemonic>jal|j)\s+(?P<target>0x[0-9a-fA-F]+)$")
LOCAL_BRANCH_MNEMONICS = {
    "b",
    "beq",
    "beqz",
    "bgez",
    "bgezal",
    "bgtz",
    "blez",
    "bltz",
    "bltzal",
    "bne",
    "bnez",
    "beql",
    "bnel",
    "bc1f",
    "bc1t",
    "bc1fl",
    "bc1tl",
    "j",
}


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


def parse_int(text: str) -> int:
    return int(text, 0)


def sign_extend_16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def combine_hi_lo(hi_value: int, low_value: int) -> int:
    return ((hi_value & 0xFFFF) << 16) + sign_extend_16(low_value)


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


def rewrite_absolute_call_or_jump(
    opcode: str, resolver: AddressSymbolResolver | None
) -> str:
    if resolver is None:
        return opcode
    match = ABSOLUTE_CALL_RE.match(opcode)
    if match is None:
        return opcode
    target = parse_int(match.group("target"))
    symbol = resolver.function_symbol(target)
    if symbol is None:
        return opcode
    return f"{match.group('mnemonic')} {normalize_asm_symbol_name(symbol, 'func', target)}"


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
        if pending is None:
            return opcode
        full_address = combine_hi_lo(pending[0], parse_int(loadstore.group("imm")))
        symbol = resolver.data_symbol(full_address)
        if symbol is None:
            return opcode
        symbol = normalize_asm_symbol_name(symbol, "DAT", full_address)
        body_lines[pending[1]] = f"lui {base}, %hi({symbol})"
        return (
            f"{loadstore.group('mnemonic')} {loadstore.group('dst')}, "
            f"%lo({symbol})({base})"
        )
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
    return f"{three_arg.group('mnemonic')} {three_arg.group('dst')}, {src}, %lo({symbol})"


def normalize_commented_asm(
    text: str, *, resolver: AddressSymbolResolver | None = None
) -> str:
    addresses: set[int] = set()
    commented_lines: list[tuple[int, str, str]] = []
    for line in text.splitlines():
        match = ASM_LINE_RE.match(line)
        if match is None:
            continue
        address = int(match.group("addr"), 16)
        addresses.add(address)
        commented_lines.append((address, match.group("body").strip(), line))

    branch_targets: set[int] = set()
    for _, body, _ in commented_lines:
        parts = body.split(None, 1)
        mnemonic = parts[0] if parts else ""
        if mnemonic not in LOCAL_BRANCH_MNEMONICS:
            continue
        match = HEX_TARGET_RE.search(body)
        if match is None:
            continue
        target = int(match.group(0), 16)
        if target in addresses:
            branch_targets.add(target)

    rewritten_lines: list[str] = []
    pending_hi: dict[str, tuple[int, int]] = {}
    for line in text.splitlines():
        match = ASM_LINE_RE.match(line)
        if match is None:
            rewritten_lines.append(line)
            continue

        address = int(match.group("addr"), 16)
        opcode = match.group("body").strip()
        if address in branch_targets:
            rewritten_lines.append(f"{local_label_for_address(address)}:")

        opcode = rewrite_absolute_call_or_jump(opcode, resolver)
        opcode = rewrite_hi_lo_pair(
            body_lines=rewritten_lines,
            opcode=opcode,
            pending_hi=pending_hi,
            resolver=resolver,
        )
        opcode = rewrite_branch_target(opcode, addresses)
        rewritten_lines.append(line[: match.start("body")] + opcode)

        lui_match = LUI_RE.match(opcode)
        if lui_match is not None:
            pending_hi[lui_match.group("reg")] = (
                parse_int(lui_match.group("imm")),
                len(rewritten_lines) - 1,
            )

    return "\n".join(rewritten_lines) + ("\n" if text.endswith("\n") else "")


__all__ = [
    "AddressSymbolResolver",
    "normalize_commented_asm",
    "normalize_function_symbol_name",
    "normalize_asm_symbol_name",
]
