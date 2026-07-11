from __future__ import annotations

import json
import os
import shutil
import subprocess
import zipfile
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from pathlib import Path


DEFAULT_PROJECT_NAME = "bof3_main"
DEFAULT_PROJECT_ROOT = Path("output") / "ghidra-project"
DEFAULT_IMPORT_MANIFEST = Path("output") / "ghidra-bof3" / "ghidra_import_manifest.json"
DEFAULT_IMPORT_STAGING = Path("output") / "ghidra-import-staging"
DEFAULT_SYMBOL_EXPORT = Path("output") / "inventory" / "raw_ghidra_export.json"
DEFAULT_SYMBOL_EXPORT_SCRIPT = (
    Path(__file__).resolve().parents[4]
    / "tools"
    / "ghidra"
    / "scripts"
    / "ExportAnalysisJson.java"
)

DEFAULT_ANALYSIS_EXPORT = Path("output") / "inventory" / "analysis.json"
DEFAULT_ANALYSIS_EXPORT_SCRIPT = (
    Path(__file__).resolve().parents[4]
    / "tools"
    / "ghidra"
    / "scripts"
    / "ExportAnalysisJson.java"
)

HeadlessRunner = Callable[[Sequence[str]], object]


@dataclass(frozen=True)
class GhidraProjectImportResult:
    imported_count: int
    commands: list[tuple[str, ...]]


@dataclass(frozen=True)
class GhidraSymbolExportResult:
    output_path: Path
    command: tuple[str, ...]


def resolve_ghidra_home(ghidra_home: Path | None) -> Path:
    env_home = os.environ.get("GHIDRA_HOME")
    candidate = ghidra_home or (Path(env_home) if env_home else None)
    if candidate is None:
        raise ValueError("pass --ghidra-home or set GHIDRA_HOME")
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


def resolve_analyze_headless(ghidra_home: Path | None) -> Path:
    ghidra_root = resolve_ghidra_home(ghidra_home)
    candidates = [
        ghidra_root / "support" / "analyzeHeadless",
        ghidra_root / "support" / "analyzeHeadless.bat",
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise FileNotFoundError(f"analyzeHeadless not found under {ghidra_root}")


def resolve_user_settings_dir(user_dir: Path | None) -> Path:
    if user_dir is None:
        raise ValueError("pass --user-dir")
    return user_dir.expanduser().resolve()


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


def default_headless_runner(command: Sequence[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(command),
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )


def _command_output(result: object) -> str:
    stdout = getattr(result, "stdout", None)
    stderr = getattr(result, "stderr", None)
    if isinstance(stdout, str) and stdout:
        return stdout
    if isinstance(stderr, str) and stderr:
        return stderr
    return ""


def _import_progress_label(command: Sequence[str]) -> str:
    try:
        project = command[2]
        import_path = command[command.index("-import") + 1]
    except (IndexError, ValueError):
        return "unknown"
    return f"{project}:{Path(import_path).name}"


def _returncode(result: object) -> int:
    if result is None:
        return 0
    code = getattr(result, "returncode", None)
    if isinstance(code, int):
        return code
    if isinstance(result, int):
        return result
    return 0


def _load_manifest(path: Path) -> dict[str, object]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"expected a JSON object in {path}")
    return payload


def _entry_path(entry: dict[str, object], *, manifest_path: Path) -> Path:
    raw = entry.get("path") or entry.get("payload_path") or entry.get("source")
    if not raw:
        raise ValueError("Ghidra import entry missing path/payload_path/source")
    path = Path(str(raw)).expanduser()
    if not path.is_absolute():
        path = manifest_path.parent / path
    return path.resolve()


def _entry_name(entry: dict[str, object], *, path: Path) -> str:
    raw = entry.get("name") or entry.get("program_name") or entry.get("display")
    return str(raw or path.name)


def _entry_project_folder(entry: dict[str, object]) -> str:
    raw = str(entry.get("project_folder_path") or "")
    return raw.strip("/")


def _stage_import_path(
    *,
    source: Path,
    entry: dict[str, object],
    staging_dir: Path | None,
) -> Path:
    if staging_dir is None:
        return source

    staged_name = Path(_entry_name(entry, path=source)).name
    if not staged_name or staged_name == source.name:
        return source

    folder = _entry_project_folder(entry)
    folder_parts = [part for part in folder.split("/") if part]
    staged_path = (
        staging_dir.expanduser().resolve().joinpath(*folder_parts, staged_name)
    )
    staged_path.parent.mkdir(parents=True, exist_ok=True)
    if staged_path.exists() or staged_path.is_symlink():
        try:
            if not staged_path.is_symlink() and staged_path.samefile(source):
                return staged_path
        except FileNotFoundError:
            pass
        if staged_path.is_dir():
            raise IsADirectoryError(f"staging path is a directory: {staged_path}")
        staged_path.unlink()
    try:
        os.link(source, staged_path)
    except OSError:
        shutil.copy2(source, staged_path)
    return staged_path


def _project_name_with_folder(project_name: str, folder: str) -> str:
    normalized = folder.strip()
    if not normalized or normalized == "/":
        return project_name
    if not normalized.startswith("/"):
        normalized = f"/{normalized}"
    return f"{project_name}{normalized}"


def build_analyze_headless_import_commands(
    *,
    ghidra_home: Path | None,
    manifest: Path,
    project_dir: Path,
    project_name: str,
    staging_dir: Path | None = None,
    script_path: Path | None = None,
    analyze: bool | None = None,
) -> list[tuple[str, ...]]:
    manifest_path = manifest.expanduser().resolve()
    payload = _load_manifest(manifest_path)
    imports = payload.get("imports", [])
    if not isinstance(imports, list):
        raise ValueError("manifest imports must be a list")

    effective_analyze = (
        bool(payload.get("analyze", True)) if analyze is None else analyze
    )
    project_location = project_dir.expanduser().resolve()
    analyze_headless = resolve_analyze_headless(ghidra_home)

    commands: list[tuple[str, ...]] = []
    for raw_entry in imports:
        if not isinstance(raw_entry, dict):
            raise ValueError("manifest import entries must be JSON objects")
        path = _entry_path(raw_entry, manifest_path=manifest_path)
        folder = str(raw_entry.get("project_folder_path") or "")
        import_path = _stage_import_path(
            source=path,
            entry=raw_entry,
            staging_dir=staging_dir,
        )
        command = [
            str(analyze_headless),
            str(project_location),
            _project_name_with_folder(project_name, folder),
            "-import",
            str(import_path),
            "-overwrite",
        ]
        loader = raw_entry.get("loader")
        if isinstance(loader, dict):
            processor = loader.get("processor")
            compiler = loader.get("compiler")
            loader_name = loader.get("loader_name")
            if processor:
                command.extend(["-processor", str(processor)])
            if compiler:
                command.extend(["-cspec", str(compiler)])
            if loader_name:
                command.extend(["-loader", str(loader_name)])
            loader_args = loader.get("loader_args", [])
            if isinstance(loader_args, list):
                for raw_arg in loader_args:
                    if not isinstance(raw_arg, dict):
                        continue
                    name = raw_arg.get("name")
                    value = raw_arg.get("value")
                    if name and value is not None:
                        command.extend([str(name), str(value)])
        if script_path is not None:
            command.extend(["-scriptPath", str(script_path.expanduser().resolve())])
        if not effective_analyze:
            command.append("-noanalysis")
        commands.append(tuple(command))
    return commands


def build_analyze_headless_symbol_export_command(
    *,
    ghidra_home: Path | None,
    project_dir: Path,
    project_name: str,
    output_path: Path,
    script_path: Path = DEFAULT_SYMBOL_EXPORT_SCRIPT,
    process: str = "/",
    recursive: bool = True,
) -> tuple[str, ...]:
    analyze_headless = resolve_analyze_headless(ghidra_home)
    resolved_script = script_path.expanduser().resolve()
    if not resolved_script.is_file():
        raise FileNotFoundError(f"Ghidra export script not found: {resolved_script}")

    project_name_arg = project_name
    process_arg = process
    if process and process != "/" and process.startswith("/"):
        process_path = Path(process.strip("/"))
        if process_path.parent != Path("."):
            project_name_arg = f"{project_name}/{process_path.parent.as_posix()}"
            process_arg = process_path.name
        else:
            process_arg = process_path.name
    command = [
        str(analyze_headless),
        str(project_dir.expanduser().resolve()),
        project_name_arg,
        "-process",
    ]
    if process_arg and process_arg != "/":
        command.append(process_arg)
    if recursive:
        command.append("-recursive")
    command.extend(
        [
            "-scriptPath",
            str(resolved_script.parent),
            "-postScript",
            resolved_script.name,
            str(output_path.expanduser().resolve()),
            process,
            "-noanalysis",
        ]
    )
    return tuple(command)


def export_ghidra_symbols(
    *,
    ghidra_home: Path | None,
    project_dir: Path = DEFAULT_PROJECT_ROOT,
    project_name: str = DEFAULT_PROJECT_NAME,
    output_path: Path = DEFAULT_SYMBOL_EXPORT,
    script_path: Path = DEFAULT_SYMBOL_EXPORT_SCRIPT,
    process: str = "/",
    recursive: bool = True,
    runner: HeadlessRunner = default_headless_runner,
) -> GhidraSymbolExportResult:
    command = build_analyze_headless_symbol_export_command(
        ghidra_home=ghidra_home,
        project_dir=project_dir,
        project_name=project_name,
        output_path=output_path,
        script_path=script_path,
        process=process,
        recursive=recursive,
    )
    output_path.expanduser().resolve().parent.mkdir(parents=True, exist_ok=True)
    result = runner(command)
    returncode = _returncode(result)
    if returncode != 0:
        output = _command_output(result)
        raise subprocess.CalledProcessError(returncode, command, output=output)
    return GhidraSymbolExportResult(
        output_path=output_path.expanduser().resolve(),
        command=command,
    )


@dataclass(frozen=True)
class GhidraAnalysisExportResult:
    output_path: Path
    command: tuple[str, ...]


def export_analysis(
    *,
    ghidra_home: Path | None,
    project_dir: Path = DEFAULT_PROJECT_ROOT,
    project_name: str = DEFAULT_PROJECT_NAME,
    output_path: Path = DEFAULT_ANALYSIS_EXPORT,
    script_path: Path = DEFAULT_ANALYSIS_EXPORT_SCRIPT,
    process: str = "/",
    recursive: bool = True,
    runner: HeadlessRunner = default_headless_runner,
) -> GhidraAnalysisExportResult:
    """Export all analysis data (functions, symbols, xrefs, call_edges,
    constants, duplicates) in a single Ghidra headless call."""
    command = build_analyze_headless_symbol_export_command(
        ghidra_home=ghidra_home,
        project_dir=project_dir,
        project_name=project_name,
        output_path=output_path,
        script_path=script_path,
        process=process,
        recursive=recursive,
    )
    output_path.expanduser().resolve().parent.mkdir(parents=True, exist_ok=True)
    result = runner(command)
    returncode = _returncode(result)
    if returncode != 0:
        output = _command_output(result)
        raise subprocess.CalledProcessError(returncode, command, output=output)
    return GhidraAnalysisExportResult(
        output_path=output_path.expanduser().resolve(),
        command=command,
    )


DEFAULT_DUPLICATE_EXPORT = Path("out/ghidra/duplicate_groups.json")
DEFAULT_DUPLICATE_EXPORT_SCRIPT = (
    Path(__file__).resolve().parents[4]
    / "tools"
    / "ghidra"
    / "scripts"
    / "ExportDuplicateGroups.java"
)


@dataclass(frozen=True)
class GhidraDuplicateGroupsResult:
    output_path: Path
    command: tuple[str, ...]


def export_duplicate_groups(
    *,
    ghidra_home: Path | None,
    project_dir: Path = DEFAULT_PROJECT_ROOT,
    project_name: str = DEFAULT_PROJECT_NAME,
    output_path: Path = DEFAULT_DUPLICATE_EXPORT,
    script_path: Path = DEFAULT_DUPLICATE_EXPORT_SCRIPT,
    process: str = "/",
    recursive: bool = True,
    runner: HeadlessRunner = default_headless_runner,
) -> GhidraDuplicateGroupsResult:
    command = build_analyze_headless_symbol_export_command(
        ghidra_home=ghidra_home,
        project_dir=project_dir,
        project_name=project_name,
        output_path=output_path,
        script_path=script_path,
        process=process,
        recursive=recursive,
    )
    output_path.expanduser().resolve().parent.mkdir(parents=True, exist_ok=True)
    result = runner(command)
    returncode = _returncode(result)
    if returncode != 0:
        output = _command_output(result)
        raise subprocess.CalledProcessError(returncode, command, output=output)
    return GhidraDuplicateGroupsResult(
        output_path=output_path.expanduser().resolve(),
        command=command,
    )


def import_ghidra_project(
    *,
    ghidra_home: Path | None,
    manifest: Path = DEFAULT_IMPORT_MANIFEST,
    project_dir: Path = DEFAULT_PROJECT_ROOT,
    project_name: str = DEFAULT_PROJECT_NAME,
    staging_dir: Path | None = None,
    script_path: Path | None = None,
    analyze: bool | None = None,
    runner: HeadlessRunner = default_headless_runner,
) -> GhidraProjectImportResult:
    commands = build_analyze_headless_import_commands(
        ghidra_home=ghidra_home,
        manifest=manifest,
        project_dir=project_dir,
        project_name=project_name,
        staging_dir=staging_dir,
        script_path=script_path,
        analyze=analyze,
    )
    project_dir.expanduser().resolve().mkdir(parents=True, exist_ok=True)
    total = len(commands)
    show_progress = runner is default_headless_runner
    for index, command in enumerate(commands, start=1):
        if show_progress:
            print(f"[{index}/{total}] importing {_import_progress_label(command)}")
        result = runner(command)
        returncode = _returncode(result)
        if returncode != 0:
            output = _command_output(result)
            if show_progress and output:
                print(output[-2000:])
            raise subprocess.CalledProcessError(returncode, command, output=output)
    return GhidraProjectImportResult(imported_count=len(commands), commands=commands)
