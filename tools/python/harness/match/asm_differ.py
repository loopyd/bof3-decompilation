"""Adapter artifact for the untouched asm-differ dependency."""

from __future__ import annotations

import html
import json
from pathlib import Path
import shutil
from typing import Any


def write_bundle(
    root: Path,
    payload: dict[str, Any],
    *,
    target: str | None = None,
    html_output: bool = False,
) -> Path:
    """Materialize the normalized diff and adapter settings under ``out``."""

    root = root.resolve()
    function = str(payload["function"])
    address = str(payload["address"]).removeprefix("0x")
    if target is None:
        source_path = Path(str(payload["source"]))
        try:
            source_parent = source_path.parent.relative_to(root).as_posix()
        except ValueError:
            source_parent = source_path.parent.name
        owner = source_parent.removeprefix("src/")
    else:
        owner = target
    bundle = root / "out" / "matching" / owner / function / "asm-differ"
    bundle.mkdir(parents=True, exist_ok=True)
    outputs = payload["outputs"]
    for source_name, destination_name in (
        ("original", "original.s"),
        ("current", "current.s"),
        ("diff", "diff.txt"),
    ):
        source = Path(outputs[source_name])
        if source.is_file():
            shutil.copyfile(source, bundle / destination_name)
    settings = {
        "schema": "harness.asm-differ/v1",
        "architecture": "mipsel",
        "function": function,
        "address": f"0x{address}",
        "expected": str((bundle / "original.s").relative_to(root)),
        "current": str((bundle / "current.s").relative_to(root)),
        "build_command": "harness diff",
        "watched_sources": [payload["source"]],
    }
    (bundle / "diff_settings.py").write_text(
        "# Generated adapter settings for third_party/asm-differ.\n"
        + "SETTINGS = "
        + repr(settings)
        + "\n",
        encoding="utf-8",
    )
    (bundle / "asm-differ.json").write_text(
        json.dumps(settings, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    summary = dict(payload)
    summary["adapter"] = "asm-differ"
    summary["adapter_bundle"] = str(bundle.relative_to(root))
    (bundle / "summary.json").write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    if html_output:
        diff_path = bundle / "diff.txt"
        content = (
            html.escape(diff_path.read_text(encoding="utf-8"))
            if diff_path.is_file()
            else ""
        )
        (bundle / "diff.html").write_text(f"<pre>{content}</pre>\n", encoding="utf-8")
    return bundle
