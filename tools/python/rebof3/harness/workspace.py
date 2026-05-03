from __future__ import annotations

from pathlib import Path
from typing import Any
import re

from ..jsonio import write_json
from .config import HarnessConfig


SAFE_RE = re.compile(r"[^0-9A-Za-z_.-]+")


def safe_name(value: str) -> str:
    cleaned = SAFE_RE.sub("_", value).strip("_")
    return cleaned or "target"


def workspace_dir(config: HarnessConfig, target_id: str) -> Path:
    return config.workspace_dir / safe_name(target_id)


def initialize_target_workspace(config: HarnessConfig, target: dict[str, Any]) -> Path:
    root = workspace_dir(config, str(target["id"]))
    root.mkdir(parents=True, exist_ok=True)
    workspace_path = root / "workspace.json"
    payload = {
        "schema": "rebof3-simple.harness-workspace/v1",
        "target": {
            "entry_hex": target.get("entry_hex"),
            "id": target["id"],
            "program_path": target.get("program_path"),
            "source_hint": target.get("source_hint"),
            "summary": target.get("summary"),
            "type": target.get("type"),
        },
        "paths": {
            "context_h": str(
                (config.context_dir / safe_name(str(target["id"])) / "context.h")
            ),
            "log": str((root / "harness.log").resolve()),
            "workspace_dir": str(root.resolve()),
        },
    }
    write_json(workspace_path, payload)
    return workspace_path
