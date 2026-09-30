"""Declared Lua modules and read-only mission input staging."""

from __future__ import annotations

import re
from pathlib import Path

from harness.io import file_sha256


def collect_assets(modules: list[str], files: list[str]) -> dict[str, dict]:
    assets = {}
    total = 0
    for role, values in (("module", modules), ("file", files)):
        for item in values:
            name, separator, value = item.partition("=")
            pattern = (
                r"[a-z][a-z0-9_]{0,63}"
                if role == "module"
                else r"[A-Za-z][A-Za-z0-9_]{0,63}"
            )
            if not separator or not re.fullmatch(pattern, name) or not value:
                raise ValueError(f"runtime {role} requires NAME=PATH")
            name = name if role == "module" else name.upper()
            key = f"{role}:{name}"
            if key in assets:
                raise ValueError(f"duplicate runtime asset: {key}")
            path = Path(value).resolve(strict=True)
            if not path.is_file():
                raise ValueError(f"runtime asset must be a regular file: {path}")
            size = path.stat().st_size
            limit = 1024 * 1024 if role == "module" else 64 * 1024 * 1024
            total += size
            if size > limit or total > 128 * 1024 * 1024 or len(assets) >= 64:
                raise ValueError("runtime assets exceed file/count/total bounds")
            assets[key] = {
                "path": str(path),
                "bytes": size,
                "sha256": file_sha256(path),
            }
    return assets


def stage_assets(output: Path, assets: dict[str, dict]) -> dict[str, Path]:
    staged = {}
    for key, identity in assets.items():
        role, name = key.split(":", 1)
        parent = output / ("lua" if role == "module" else "inputs")
        parent.mkdir(exist_ok=True)
        path = parent / (f"{name}.lua" if role == "module" else name)
        with Path(identity["path"]).open("rb") as source:
            data = source.read(identity["bytes"] + 1)
        if len(data) != identity["bytes"]:
            raise ValueError(f"runtime asset size changed while staging: {key}")
        path.write_bytes(data)
        if file_sha256(path) != identity["sha256"]:
            raise ValueError(f"runtime asset changed while staging: {key}")
        staged[key] = path
    return staged
