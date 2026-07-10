"""Generate m2c context stubs (symbols.h/structs.h/globals.h/prototypes.h)
from a module's internal.h for the overlay-level context pipeline.

Usage:
    bin/gen-context-stubs <module>/<entry>   e.g. bin/gen-context-stubs game/00
    bin/gen-context-stubs --all
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from ._common import run_main


PROJECT_ROOT = Path(__file__).resolve().parents[4]
STUB_NAMES = ["symbols.h", "structs.h", "globals.h", "prototypes.h"]

ALL_OVERLAYS: list[tuple[str, str]] = [
    ("battle", "03"),
    ("battle", "15"),
    ("game", "00"),
    ("game", "01"),
    ("sce10eff", "00"),
    ("logo", "logo"),
]


def _find_internal_h(module: str, entry: str) -> Path:
    if module == "logo":
        p = PROJECT_ROOT / "src/modules/logo/internal.h"
    else:
        p = PROJECT_ROOT / f"src/modules/{module}/{entry}/internal.h"
    if not p.is_file():
        raise FileNotFoundError(f"no internal.h at {p}")
    return p


def _context_dir(module: str, entry: str) -> Path:
    target = "logo" if module == "logo" else f"{module}/{entry}"
    return PROJECT_ROOT / "out/harness/context" / target


def _strip_c_comments(text: str) -> str:
    text = re.sub(r"//.*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return text


def _join_continuation_lines(lines: list[str]) -> list[str]:
    """Join backslash-continuation lines into single logical lines."""
    joined: list[str] = []
    buf: list[str] = []
    for line in lines:
        stripped = line.rstrip()
        if buf:
            buf.append(
                stripped.rstrip("\\").strip()
                if stripped.endswith("\\")
                else stripped.strip()
            )
            if not stripped.endswith("\\"):
                joined.append(" ".join(buf))
                buf = []
        elif stripped.endswith("\\"):
            buf = [stripped.rstrip("\\").strip()]
        else:
            joined.append(stripped)
    if buf:
        joined.append(" ".join(buf))
    return joined


def _join_prototype_lines(text: str) -> str:
    """Join multi-line function declarations onto single lines."""
    lines = text.split("\n")
    result: list[str] = []
    buf: list[str] = []
    in_proto = False

    PROTO_START = re.compile(
        r"^\s*(?:extern\s+)?"
        r"(void|u8|u16|u32|u64|s8|s16|s32|s64|f32|f64"
        r"|bool|char|short|int|long|unsigned|signed|const|volatile"
        r")\s+\w+\s*\("
    )

    for line in lines:
        stripped = line.strip()
        if in_proto:
            buf.append(stripped)
            if ";" in stripped:
                result.append(" ".join(buf))
                buf = []
                in_proto = False
            continue

        if PROTO_START.match(stripped) and ";" not in stripped:
            buf = [stripped]
            in_proto = True
        else:
            result.append(line)

    if buf:
        result.append(" ".join(buf))

    return "\n".join(result)


def _categorize(
    text: str,
) -> dict[str, list[str]]:
    """Categorize internal.h lines into structs, symbols, globals, prototypes."""
    # Remove C comments but keep line structure
    cleaned = _strip_c_comments(text)

    # Remove include guard wrapper
    cleaned = re.sub(r"#ifndef\s+\S+\s*\n#define\s+\S+\s*\n", "", cleaned, count=1)
    cleaned = re.sub(r"\n#endif\s*$", "", cleaned)

    # Join multi-line function prototypes onto single lines
    cleaned = _join_prototype_lines(cleaned)

    # Join backslash-continuation lines into single logical lines
    raw_lines = cleaned.split("\n")
    lines = _join_continuation_lines(raw_lines)

    out: dict[str, list[str]] = {
        "structs": [],
        "symbols": [],
        "globals": [],
        "prototypes": [],
        "skipped": [],
    }

    i = 0
    n = len(lines)

    while i < n:
        raw = lines[i].strip()

        # Blank lines
        if not raw:
            i += 1
            continue

        # #define
        if raw.startswith("#define"):
            _categorize_define(raw, out)
            i += 1
            continue

        # Include lines
        if raw.startswith("#include"):
            i += 1
            continue

        # Preprocessor conditionals — skip the directive line, keep content
        if raw.startswith("#if") or raw.startswith("#else") or raw.startswith("#elif"):
            i += 1
            continue
        if raw.startswith("#endif"):
            i += 1
            continue

        # Struct/union/enum block
        if re.match(r"^(typedef\s+)?(struct|union|enum)\s", raw):
            block_lines = [raw]
            brace_depth = raw.count("{") - raw.count("}")
            j = i + 1
            while j < n and brace_depth > 0:
                block_lines.append(lines[j])
                brace_depth += lines[j].count("{") - lines[j].count("}")
                j += 1
            # block now includes closing brace + optional typedef name
            block = "\n".join(block_lines)
            out["structs"].append(block)
            i = j
            continue

        # Standalone typedef (not struct/union/enum — e.g. function pointer typedef)
        if raw.startswith("typedef"):
            # Semicolon already on this line — single-line typedef, no gathering
            if ";" in raw:
                out["structs"].append(raw)
                i += 1
                continue
            # Multi-line typedef — gather until semicolon
            block_lines = [raw]
            j = i + 1
            while j < n and ";" not in lines[j]:
                block_lines.append(lines[j])
                j += 1
            if j < n:
                block_lines.append(lines[j])
                j += 1
            out["structs"].append("\n".join(block_lines))
            i = j
            continue

        # static inline — skip (code body, not useful type info for m2c)
        if raw.startswith("static inline"):
            # Find the closing brace
            j = i + 1
            brace_depth = raw.count("{") - raw.count("}")
            while j < n and (brace_depth > 0 or not lines[j].strip()):
                brace_depth += lines[j].count("{") - lines[j].count("}")
                j += 1
            i = j
            continue

        # extern declaration
        if raw.startswith("extern"):
            if "(" in raw and raw.rstrip().endswith(";"):
                out["prototypes"].append(raw)
            i += 1
            continue

        # Function prototype — ends with ; and contains (
        if ";" in raw and "(" in raw:
            # Check that this is indeed a prototype (return type + name + params)
            # Could be a macro invocation like BOF3_BATTLE_LOCAL_STATE_TABLE(...)
            # which is a function-like macro, not a prototype.
            # Heuristic: starts with known return type or a type keyword
            proto_prefixes = (
                "void",
                "u8",
                "u16",
                "u32",
                "u64",
                "s8",
                "s16",
                "s32",
                "s64",
                "f32",
                "f64",
                "bool",
                "char",
                "short",
                "int",
                "long",
                "unsigned",
                "signed",
                "size_t",
                "uintptr_t",
                "const",
                "volatile",
            )
            first_word = raw.split()[0] if raw.split() else ""
            if first_word in proto_prefixes or first_word.endswith("*"):
                out["prototypes"].append(raw)
                i += 1
                continue

        # Everything else — skip (comments, etc.)
        i += 1

    return out


def _categorize_define(raw: str, out: dict[str, list[str]]) -> None:
    """Categorize a single #define directive."""
    parts = raw.split(None, 2)
    if len(parts) < 2:
        return
    name = parts[1]
    value = parts[2] if len(parts) > 2 else ""

    value = value.strip()

    if not value:
        return

    # Dereference macro: (*(type*)addr)
    if re.match(r"^\(\s*\*\s*\(", value):
        out["globals"].append(raw)
        return

    # Dereference with volatile: (*(volatile type*)addr) or (*(const volatile ...)
    if re.match(r"^\(\s*\*\s*\(?(volatile|const)", value):
        out["globals"].append(raw)
        return

    # Pointer cast tableau: ((type*)addr) or ((type const volatile*)addr)
    if re.match(r"^\(\s*\(", value) and re.search(r"\*\)\s*0x", value):
        out["symbols"].append(raw)
        return

    # Plain address: (0x...)
    if re.match(r"^\(0x", value):
        out["symbols"].append(raw)
        return

    # Clang-format / compiler directives
    if name in ("BOF3_NO_SIBLING_CALLS",) or raw.startswith("/* clang-format"):
        return

    # Other non-deref/cast #defines — attribute macros, inline-function defs
    # Could be useful as constants; put in symbols as a safe default
    out["symbols"].append(raw)


def _generate_context_h(module: str, entry: str, guard: str) -> str:
    if module == "logo":
        inc_main = '#include "bof3/defines.h"'
        inc_stubs = "\n".join(f'#include "{s}"' for s in STUB_NAMES)
    else:
        inc_main = '#include "bof3/defines.h"'
        inc_stubs = "\n".join(f'#include "{s}"' for s in STUB_NAMES)

    return f"#ifndef {guard}\n#define {guard}\n\n{inc_main}\n\n{inc_stubs}\n\n#endif\n"


def _write_stub(
    context_dir: Path, name: str, lines: list[str], description: str
) -> Path:
    path = context_dir / name
    guard = f"BOF3_CONTEXT_{context_dir.name.upper()}_{name.replace('.', '_').upper()}"
    if lines:
        body = "\n".join(lines) + "\n"
    else:
        body = f"/* {description} */\n"
    path.write_text(
        f"#ifndef {guard}\n#define {guard}\n\n/* {description} */\n\n{body}#endif\n",
        encoding="utf-8",
    )
    return path


def _run_gen(module: str, entry: str) -> int:
    internal_h = _find_internal_h(module, entry)
    ctx_dir = _context_dir(module, entry)
    ctx_dir.mkdir(parents=True, exist_ok=True)

    text = internal_h.read_text(encoding="utf-8")
    categorized = _categorize(text)

    configs = [
        ("symbols.h", categorized["symbols"], "address and table pointer definitions"),
        ("structs.h", categorized["structs"], "struct, typedef, and type definitions"),
        ("globals.h", categorized["globals"], "global data register/memory macros"),
        ("prototypes.h", categorized["prototypes"], "function prototypes"),
    ]

    for name, lines, desc in configs:
        _write_stub(ctx_dir, name, lines, desc)
        print(f"  wrote {ctx_dir / name} ({len(lines)} items)")

    guard = f"BOF3_CONTEXT_{module.upper()}_{entry.upper()}_H"
    context_h_text = _generate_context_h(module, entry, guard)
    (ctx_dir / "context.h").write_text(context_h_text, encoding="utf-8")
    print(f"  wrote {ctx_dir / 'context.h'}")

    skipped = categorized.get("skipped", [])
    if skipped:
        for s in skipped:
            print(f"  [skip] {s}")

    return 0


def _cmd_gen(args: argparse.Namespace) -> int:
    if args.all:
        errors = 0
        for module, entry in ALL_OVERLAYS:
            print(f"\n--- {module}/{entry} ---")
            try:
                errors += _run_gen(module, entry)
            except FileNotFoundError as e:
                print(f"  SKIP: {e}", file=sys.stderr)
                errors += 1
        return errors

    if args.overlay:
        parts = args.overlay.split("/", 1)
        module = parts[0]
        entry = parts[1] if len(parts) > 1 else "00"
    elif args.module and args.entry:
        module = args.module
        entry = args.entry
    else:
        print("error: specify <module>/<entry> or --all", file=sys.stderr)
        return 1

    print(f"\n--- {module}/{entry} ---")
    return _run_gen(module, entry)


def _build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Generate m2c context stubs from module internal.h",
    )
    p.add_argument(
        "overlay",
        nargs="?",
        help="Overlay path, e.g. game/00, battle/03",
    )
    p.add_argument(
        "--all",
        action="store_true",
        help="Generate context for all known overlays",
    )
    p.add_argument(
        "--module",
        help="Module name (alternative to overlay positional)",
    )
    p.add_argument(
        "--entry",
        help="Entry name (alternative to overlay positional)",
    )
    p.set_defaults(handler=_cmd_gen)
    return p


def main() -> int:
    return run_main(_build_parser)


if __name__ == "__main__":
    sys.exit(main())
