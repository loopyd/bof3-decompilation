from __future__ import annotations

import json
import os
import subprocess
import hashlib
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

from .config import ROOT
from .logger import Rebof3Logger
from .models.core import BinMetadata, SourceSpec


def parse_hexish(text: str | None) -> int:
    if text is None:
        raise ValueError("missing hex value")
    normalized = str(text).strip()
    if not normalized:
        raise ValueError("empty hex value")
    return (
        int(normalized, 16)
        if not normalized.startswith(("0x", "0X"))
        else int(normalized, 16)
    )


def format_hex(value: int) -> str:
    return f"0x{value:08x}"


def relative_to_root(path: Path) -> str:
    try:
        return path.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return str(path.resolve())


def normalize_repo_path(path_text: str | None) -> Path | None:
    if not path_text:
        return None
    path = Path(path_text)
    return path if path.is_absolute() else ROOT / path


def parse_source_spec(source_text: str) -> SourceSpec:
    if "#" in source_text:
        path_text, entry_text = source_text.rsplit("#", 1)
        return SourceSpec(Path(path_text), int(entry_text, 0))
    return SourceSpec(Path(source_text), None)


def default_artifacts_dir(
    output_root: Path,
    source_path: Path,
    requested_address: int,
    entry_index: int | None,
) -> Path:
    try:
        relative_source = source_path.resolve().relative_to(ROOT)
        base_dir = output_root / relative_source
    except ValueError:
        base_dir = output_root / "external" / source_path.name
    if entry_index is not None:
        base_dir = base_dir / f"entry_{entry_index}"
    return base_dir / format_hex(requested_address)


def write_text_output(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def write_json_output(path: Path, payload: Any) -> None:
    write_text_output(path, json.dumps(payload, indent=2, sort_keys=True) + "\n")


def ensure_output_parents(*paths: Path | None) -> None:
    for path in paths:
        if path is None:
            continue
        path.parent.mkdir(parents=True, exist_ok=True)


def write_markdown_output(path: Path, text: str) -> None:
    write_text_output(path, text if text.endswith("\n") else text + "\n")


def hash_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def emit_output_summary(
    logger: Rebof3Logger,
    *,
    summary: str,
    json_path: Path | None = None,
    md_path: Path | None = None,
) -> None:
    parts = [summary]
    if json_path is not None:
        parts.append(f"json={json_path}")
    if md_path is not None:
        parts.append(f"md={md_path}")
    logger.summary(" ".join(parts))


def run_command(
    command: list[str],
    *,
    cwd: Path | None = None,
    input_text: str | None = None,
    env: dict[str, str] | None = None,
    stream_output: bool = False,
    timeout: float | None = None,
) -> subprocess.CompletedProcess[str]:
    kwargs: dict[str, Any] = {
        "input": input_text,
        "text": True,
        "errors": "replace",
        "check": False,
        "cwd": ROOT if cwd is None else cwd,
        "env": env,
    }
    if not stream_output:
        kwargs["capture_output"] = True
    if timeout is not None:
        kwargs["timeout"] = timeout
    return subprocess.run(command, **kwargs)


def prepend_pythonpath(path: Path, env: dict[str, str] | None = None) -> dict[str, str]:
    merged = dict(os.environ if env is None else env)
    existing = merged.get("PYTHONPATH")
    merged["PYTHONPATH"] = (
        str(path) if not existing else f"{path}{os.pathsep}{existing}"
    )
    return merged


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def infer_bin_metadata(path: Path) -> dict[str, Any] | None:
    manifest_path = path.with_name("emi.json")
    if not manifest_path.exists():
        return None

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    for entry in manifest.get("entries", []):
        if entry.get("name") == path.name:
            return BinMetadata(
                manifest_path=relative_to_root(manifest_path),
                entry_name=entry.get("name"),
                entry_index=entry.get("index"),
                entry_type=entry.get("type"),
                load_address=int(entry["ram_ptr"]),
            ).as_dict()
    return None


def source_is_executable(path: Path) -> bool:
    return path.name.upper().startswith("SLUS_") or path.suffix.lower() == ".exe"
