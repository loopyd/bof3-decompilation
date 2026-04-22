from __future__ import annotations

import shutil
import subprocess
import zipfile
from pathlib import Path


DEFAULT_PROJECT_NAME = "bof3_main"
DEFAULT_PROJECT_ROOT = Path("out") / "ghidra-project"


def resolve_ghidra_home(ghidra_home: Path | None) -> Path:
    candidate = ghidra_home
    if candidate is None:
        raise ValueError("pass --ghidra-home")
    resolved = candidate.expanduser().resolve()
    if not resolved.is_dir():
        raise FileNotFoundError(f"ghidra home not found: {resolved}")
    return resolved


def resolve_ghidra_run(ghidra_home: Path | None) -> Path:
    ghidra_root = resolve_ghidra_home(ghidra_home)
    ghidra_run = ghidra_root / "ghidraRun"
    if not ghidra_run.is_file():
        raise FileNotFoundError(f"ghidraRun not found under {ghidra_root}")
    return ghidra_run


def resolve_user_settings_dir(user_dir: Path | None) -> Path:
    if user_dir is not None:
        return user_dir.expanduser().resolve()

    root = Path.home() / ".ghidra"
    version_dirs = sorted(path for path in root.glob(".ghidra_*") if path.is_dir())
    if version_dirs:
        return version_dirs[-1].resolve()
    return (root / ".ghidra_local").resolve()


def resolve_extensions_dir(user_dir: Path | None) -> Path:
    return resolve_user_settings_dir(user_dir) / "Extensions"


def extension_properties_in_zip(path: Path) -> bool:
    with zipfile.ZipFile(path) as archive:
        return any(
            Path(name).name == "extension.properties" for name in archive.namelist()
        )


def validate_extension_source(path: Path) -> Path:
    resolved = path.expanduser().resolve()
    if not resolved.exists():
        raise FileNotFoundError(f"extension source not found: {resolved}")

    if resolved.is_dir():
        if not any(resolved.rglob("extension.properties")):
            raise ValueError(
                f"extension directory missing extension.properties: {resolved}"
            )
        return resolved

    if resolved.suffix.lower() != ".zip":
        raise ValueError(f"extension source must be a directory or zip: {resolved}")
    if not extension_properties_in_zip(resolved):
        raise ValueError(f"extension archive missing extension.properties: {resolved}")
    return resolved


def install_extension(source: Path, extensions_dir: Path) -> Path:
    if source.is_dir():
        destination = extensions_dir / source.name
        if destination.exists():
            shutil.rmtree(destination)
        shutil.copytree(source, destination)
        return destination

    with zipfile.ZipFile(source) as archive:
        archive.extractall(extensions_dir)

    directory_entries = [
        path
        for path in extensions_dir.iterdir()
        if path.is_dir() and any(path.rglob("extension.properties"))
    ]
    if not directory_entries:
        raise ValueError(
            f"installed archive did not create an extension directory: {source}"
        )
    directory_entries.sort(key=lambda path: path.stat().st_mtime_ns, reverse=True)
    return directory_entries[0]


def install_extensions(
    sources: list[Path],
    *,
    user_dir: Path | None,
) -> tuple[Path, list[Path]]:
    if not sources:
        raise ValueError("at least one extension source is required")

    extensions_dir = resolve_extensions_dir(user_dir)
    extensions_dir.mkdir(parents=True, exist_ok=True)
    installed_paths = [
        install_extension(validate_extension_source(source), extensions_dir)
        for source in sources
    ]
    return extensions_dir, installed_paths


def launch_ui(
    *,
    ghidra_home: Path | None,
    project_dir: Path | None,
    project_name: str | None,
    extra_args: list[str],
) -> int:
    command = [str(resolve_ghidra_run(ghidra_home))]
    if project_dir is not None and project_name:
        project_file = project_dir.expanduser().resolve() / f"{project_name}.gpr"
        if project_file.exists():
            command.append(str(project_file))
    command.extend(extra_args)
    result = subprocess.run(command, check=False)
    return int(result.returncode)
