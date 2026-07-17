#!/usr/bin/env python3
"""Shared helpers for invoking Rizin in bundled scripts."""

from __future__ import annotations

import json
import shutil
import subprocess
from pathlib import Path
from typing import Any


class RizinError(RuntimeError):
    pass


def require_tool(name: str) -> str:
    resolved = shutil.which(name)
    if resolved is None:
        raise RizinError(f"required tool not found in PATH: {name}")
    return resolved


def run_rizin(
    binary: Path,
    base: int,
    command: str,
    *,
    rizin: str = "rizin",
    analyze: bool = True,
    timeout: int = 300,
) -> tuple[str, str]:
    executable = require_tool(rizin)
    analysis = "aa;aar;aaf;aac;aad;" if analyze else ""
    full_command = f"{analysis}{command};q"
    argv = [
        executable,
        "-q",
        "-a",
        "mips",
        "-b",
        "32",
        "-e",
        "cfg.bigendian=false",
        "-m",
        f"0x{base:x}",
        "-c",
        full_command,
        str(binary),
    ]
    completed = subprocess.run(
        argv,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout,
        check=False,
    )
    if completed.returncode != 0:
        raise RizinError(
            f"Rizin command failed ({completed.returncode}): {command}\n{completed.stderr.strip()}"
        )
    return completed.stdout, completed.stderr


def parse_json_output(output: str) -> Any:
    text = output.strip()
    if not text:
        raise RizinError("Rizin produced no JSON output")
    decoder = json.JSONDecoder()
    for index, char in enumerate(text):
        if char not in "[{":
            continue
        try:
            value, _ = decoder.raw_decode(text[index:])
            return value
        except json.JSONDecodeError:
            continue
    raise RizinError(f"could not locate JSON in Rizin output: {text[:500]!r}")
