from __future__ import annotations

import re

PSEUDO_TYPES = {
    "undefined label",
    "undefined symbol",
}

CALLING_CONVENTION_TOKENS = (
    "__cdecl",
    "__fastcall",
    "__stdcall",
    "__thiscall",
    "__gtemacro",
)

C_TYPE_ALIAS_REWRITES: tuple[tuple[re.Pattern[str], str], ...] = (
    (re.compile(r"\buchar\b"), "unsigned char"),
    (re.compile(r"\bushort\b"), "unsigned short"),
    (re.compile(r"\buint\b"), "unsigned int"),
    (re.compile(r"\bulong\b"), "unsigned long"),
)

ARRAY_PREFIX_RE = re.compile(r"^(\[[^\]]+\])\s+(.+)$")
IDENTIFIER_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_:]*\b")
TRAILING_ARRAY_SUFFIX_RE = re.compile(r"(\s*(?:\[[^\]]*\]\s*)+)$")
FUNCTION_POINTER_PARAM_RE = re.compile(
    r"(?P<full>(?P<return_type>[^,()]+?)\s*\(\s*\*\s*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*\)\s*\((?P<params>[^()]*)\))"
)
IGNORED_TYPE_TOKENS = {
    "void",
    "undefined",
    "undefined1",
    "undefined2",
    "undefined4",
    "char",
    "short",
    "int",
    "long",
    "unsigned",
    "signed",
    "struct",
    "enum",
    "union",
    "const",
    "volatile",
    "pointer",
    "string",
    "bool",
}
