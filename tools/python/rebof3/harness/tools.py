from __future__ import annotations

from dataclasses import dataclass
import os
from pathlib import Path
import shlex
import shutil

from ..core.process import ProcessResult, run_process
from .config import HarnessConfig


@dataclass(frozen=True)
class ToolHealth:
    name: str
    status: str
    detail: str

    def to_dict(self) -> dict[str, str]:
        return {"detail": self.detail, "name": self.name, "status": self.status}


def command_argv(config: HarnessConfig, name: str) -> list[str]:
    command = config.commands.get(name)
    if not command:
        raise KeyError(f"no harness command configured for {name!r}")
    argv = shlex.split(command)
    if not argv:
        raise ValueError(f"empty harness command configured for {name!r}")
    return argv


def run_configured_command(config: HarnessConfig, name: str) -> ProcessResult:
    return run_process(command_argv(config, name), cwd=config.root, stream=True)


def ghidra_analyze_headless() -> Path | None:
    candidates: list[Path] = []
    if ghidra_home := os.environ.get("GHIDRA_HOME"):
        candidates.append(Path(ghidra_home) / "support" / "analyzeHeadless")
    if path := shutil.which("analyzeHeadless"):
        candidates.append(Path(path))
    candidates.append(Path("/opt/ghidra/support/analyzeHeadless"))
    return next((path for path in candidates if path.exists()), None)


def tool_health(config: HarnessConfig) -> list[ToolHealth]:
    paths: list[tuple[str, Path]] = [
        ("emi_catalog", config.emi_catalog),
        ("function_index", config.function_index),
        ("artifact_manifest", config.artifact_manifest),
        ("emi_root", config.emi_root),
    ]
    health: list[ToolHealth] = []
    for name, path in paths:
        if path.exists():
            health.append(ToolHealth(name=name, status="ok", detail=str(path)))
        else:
            health.append(ToolHealth(name=name, status="missing", detail=str(path)))
    executable_paths: list[tuple[str, Path]] = [
        (
            "psn00b-gcc",
            config.root / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-gcc",
        ),
        (
            "psn00b-objdump",
            config.root / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-objdump",
        ),
        (
            "psn00b-nm",
            config.root / "toolchains/psn00b_toolchain/bin/mipsel-none-elf-nm",
        ),
        ("gcc-2.7.2-psx", config.root / "toolchains/gcc-2.7.2-psx/gcc"),
        ("maspsx", config.root / "bin/maspsx-cc"),
        ("m2c", config.root / "third_party/m2c/m2c.py"),
        ("objdiff-cli", config.root / "build/third_party/objdiff/release/objdiff-cli"),
    ]
    for name, path in executable_paths:
        status = "ok" if path.exists() else "missing"
        health.append(ToolHealth(name=name, status=status, detail=str(path)))
    for name in ("rizin", "rz-asm"):
        path = shutil.which(name)
        health.append(
            ToolHealth(
                name=name,
                status="ok" if path else "missing",
                detail=path or "not found on PATH",
            )
        )
    if analyze := ghidra_analyze_headless():
        health.append(ToolHealth(name="ghidra", status="ok", detail=str(analyze)))
    else:
        detail = (
            "set GHIDRA_HOME for headless import/export"
            if not os.environ.get("GHIDRA_HOME")
            else str(Path(os.environ["GHIDRA_HOME"]) / "support" / "analyzeHeadless")
        )
        health.append(ToolHealth(name="ghidra", status="missing", detail=detail))
    for name, command in sorted(config.commands.items()):
        health.append(
            ToolHealth(name=f"command/{name}", status="configured", detail=command)
        )
    return health
