from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any
import tomllib

from ..paths import repo_layout


@dataclass(frozen=True)
class MigrationTarget:
    id: str
    label: str
    program_path: str
    source_dir: Path
    source_hint: str


@dataclass(frozen=True)
class HarnessConfig:
    root: Path
    path: Path
    schema: str
    out_dir: Path
    database: Path
    workspace_dir: Path
    context_dir: Path
    dashboard_dir: Path
    emi_catalog: Path
    function_index: Path
    raw_ghidra_export: Path
    artifact_manifest: Path
    emi_root: Path
    commands: dict[str, str]
    migration_targets: tuple[MigrationTarget, ...]


def _resolve(root: Path, value: str | Path) -> Path:
    path = Path(value).expanduser()
    if path.is_absolute():
        return path
    return root / path


def _table(payload: dict[str, Any], name: str) -> dict[str, Any]:
    value = payload.get(name, {})
    if not isinstance(value, dict):
        raise ValueError(f"harness config table [{name}] must be an object")
    return value


def _commands(payload: dict[str, Any]) -> dict[str, str]:
    value = _table(payload, "commands")
    return {str(key): str(command) for key, command in value.items()}


def _migration_targets(
    root: Path, payload: dict[str, Any]
) -> tuple[MigrationTarget, ...]:
    raw_targets = payload.get("migration_targets", [])
    if not isinstance(raw_targets, list):
        raise ValueError("harness config migration_targets must be a list")
    targets: list[MigrationTarget] = []
    for raw_target in raw_targets:
        if not isinstance(raw_target, dict):
            continue
        targets.append(
            MigrationTarget(
                id=str(raw_target["id"]),
                label=str(raw_target["label"]),
                program_path=str(raw_target["program_path"]),
                source_dir=_resolve(root, str(raw_target["source_dir"])),
                source_hint=str(raw_target["source_hint"]),
            )
        )
    return tuple(targets)


def load_harness_config(path: Path | None = None) -> HarnessConfig:
    layout = repo_layout()
    root = layout.root
    config_path = (path or root / "harness.toml").expanduser()
    if not config_path.is_absolute():
        config_path = root / config_path
    if not config_path.is_file():
        raise FileNotFoundError(f"harness config not found: {config_path}")

    payload = tomllib.loads(config_path.read_text(encoding="utf-8"))
    harness = _table(payload, "harness")
    paths = _table(payload, "paths")

    out_dir = _resolve(root, str(harness.get("out_dir", "out/harness")))
    return HarnessConfig(
        root=root,
        path=config_path,
        schema=str(harness.get("schema", "rebof3-simple.harness/v1")),
        out_dir=out_dir,
        database=_resolve(
            root, str(harness.get("database", out_dir / "harness.sqlite3"))
        ),
        workspace_dir=_resolve(
            root, str(harness.get("workspace_dir", out_dir / "workspaces"))
        ),
        context_dir=_resolve(
            root, str(harness.get("context_dir", out_dir / "context"))
        ),
        dashboard_dir=_resolve(
            root, str(harness.get("dashboard_dir", out_dir / "dashboard"))
        ),
        emi_catalog=_resolve(
            root, str(paths.get("emi_catalog", layout.inventory_emi_catalog_path))
        ),
        function_index=_resolve(
            root,
            str(
                paths.get("function_index", layout.inventory_ghidra_function_index_path)
            ),
        ),
        raw_ghidra_export=_resolve(
            root,
            str(
                paths.get(
                    "raw_ghidra_export",
                    root / "out/inventory/raw_ghidra_export.json",
                )
            ),
        ),
        artifact_manifest=_resolve(
            root,
            str(
                paths.get(
                    "artifact_manifest",
                    root / "build/default/artifacts/metadata/artifacts.json",
                )
            ),
        ),
        emi_root=_resolve(root, str(paths.get("emi_root", layout.emi_root))),
        commands=_commands(payload),
        migration_targets=_migration_targets(root, payload),
    )
