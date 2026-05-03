from __future__ import annotations

from pathlib import Path
from typing import Any

from .config import HarnessConfig
from .workspace import safe_name


def common_context_dir(config: HarnessConfig) -> Path:
    return config.root / "bof3" / "context" / "common"


def ensure_common_context(config: HarnessConfig) -> Path:
    path = common_context_dir(config) / "common.h"
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists():
        path.write_text(
            "#ifndef BOF3_CONTEXT_COMMON_H\n"
            "#define BOF3_CONTEXT_COMMON_H\n\n"
            '#include "bof3/defines.h"\n\n'
            "#endif\n",
            encoding="utf-8",
        )
    return path


def build_context_header(config: HarnessConfig, target: dict[str, Any]) -> Path:
    ensure_common_context(config)
    context_root = config.context_dir / safe_name(str(target["id"]))
    context_root.mkdir(parents=True, exist_ok=True)
    for name, description in (
        ("symbols.h", "function and label names"),
        ("structs.h", "local struct definitions"),
        ("globals.h", "global data declarations"),
        ("prototypes.h", "function prototypes"),
    ):
        stub = context_root / name
        if not stub.exists():
            guard = f"REBOF3_HARNESS_{safe_name(str(target['id'])).upper()}_{name.replace('.', '_').upper()}"
            stub.write_text(
                f"#ifndef {guard}\n"
                f"#define {guard}\n\n"
                f"/* {description} for {target['id']} */\n\n"
                "#endif\n",
                encoding="utf-8",
            )
    path = context_root / "context.h"
    guard = f"REBOF3_HARNESS_CONTEXT_{safe_name(str(target['id'])).upper()}_H"
    source_hint = target.get("source_hint") or ""
    program_path = target.get("program_path") or ""
    entry_hex = target.get("entry_hex") or ""
    path.write_text(
        f"#ifndef {guard}\n"
        f"#define {guard}\n\n"
        '#include "bof3/context/common/common.h"\n\n'
        f"/* target: {target['id']} */\n"
        f"/* source: {source_hint} */\n"
        f"/* program: {program_path} */\n"
        f"/* entry: {entry_hex} */\n\n"
        '#include "symbols.h"\n'
        '#include "structs.h"\n'
        '#include "globals.h"\n'
        '#include "prototypes.h"\n\n'
        "#endif\n",
        encoding="utf-8",
    )
    return path
