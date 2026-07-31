"""Small repository-path and atomic-I/O helpers."""

from __future__ import annotations

import json
import os
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any


DEFAULT_PSYQ_VERSION = "4.7"


def normalize_psyq_version(version: str | None = None) -> str:
    value = (version or DEFAULT_PSYQ_VERSION).removeprefix("psyq").strip()
    if not value:
        raise ValueError("PsyQ version must not be empty")
    return value


@dataclass(frozen=True)
class RepoLayout:
    root: Path
    build_dir: Path
    out_dir: Path
    toolchains_dir: Path
    third_party_dir: Path
    inputs_dir: Path
    downloads_dir: Path
    private_assets_dir: Path
    harness_disk_src: Path
    emi_ex_src: Path
    harness_disk_bin: Path
    emi_ex_bin: Path
    psn00b_toolchain_root: Path
    psn00b_sdk_root: Path
    gcc272_psx_root: Path
    gcc_variants_root: Path
    psyq_root: Path

    @property
    def gcc_archive_cache_dir(self) -> Path:
        """Digest-verified GCC archive cache root under private assets.

        Derived (not a dataclass field) so callers that construct RepoLayout
        positionally keep working. Canonical GCC and every catalog variant
        share this cache; unrelated toolchain downloads stay in
        ``toolchains/downloads/``.
        """
        return self.private_assets_dir / "toolchains" / "gcc"


def repo_layout(
    root: Path | None = None, *, psyq_version: str | None = None
) -> RepoLayout:
    resolved = (root or Path(__file__).resolve().parents[3]).resolve()
    build = resolved / "build"
    toolchains = resolved / "toolchains"
    inputs = resolved / "inputs"
    psyq = normalize_psyq_version(psyq_version)
    return RepoLayout(
        root=resolved,
        build_dir=build,
        out_dir=resolved / "out",
        toolchains_dir=toolchains,
        third_party_dir=resolved / "third_party",
        inputs_dir=inputs,
        downloads_dir=toolchains / "downloads",
        private_assets_dir=inputs / "external" / "private-assets",
        harness_disk_src=resolved / "tools" / "rust" / "bof3-disk",
        emi_ex_src=resolved / "tools" / "rust" / "emi-ex",
        harness_disk_bin=build
        / "tools"
        / "rust"
        / "bof3-disk"
        / "release"
        / "bof3-disk",
        emi_ex_bin=build / "tools" / "rust" / "emi-ex" / "release" / "emi-ex",
        psn00b_toolchain_root=toolchains / "psn00b_toolchain",
        psn00b_sdk_root=toolchains / "psn00bsdk",
        gcc272_psx_root=toolchains / "gcc-2.7.2-psx",
        gcc_variants_root=toolchains / "gcc-variants",
        psyq_root=toolchains / "psyq" / psyq,
    )


def read_json(path: Path) -> dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"expected a JSON object in {path}")
    return payload


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    content = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    descriptor, temporary_name = tempfile.mkstemp(
        dir=path.parent, prefix=f".{path.name}.", suffix=".tmp"
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            stream.write(content)
            stream.flush()
            os.fsync(stream.fileno())
        temporary.replace(path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def run_command(
    command: list[str], *, cwd: Path | None = None, env: dict[str, str] | None = None
) -> None:
    full_env = os.environ.copy()
    if env is not None:
        full_env.update(env)
    result = subprocess.run(command, cwd=cwd, env=full_env, check=False)
    if result.returncode != 0:
        raise RuntimeError(
            f"command failed with exit code {result.returncode}: {' '.join(command)}"
        )
