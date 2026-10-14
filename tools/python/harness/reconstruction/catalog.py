"""verify rejects changes to selected file records or the compiler recipe digest."""

from __future__ import annotations

import hashlib
import subprocess
from collections.abc import Callable
from pathlib import Path

from harness.build import recipe, runtime
from harness.build.compiler import DEFAULT_COMPILER_ID
from harness.common import digests
from harness.common.failures import FailureEvent, PolicyFailure, primary, retain
from harness.io import repo_layout
from harness.toolchain import gcc, gcc_variants

CATALOG_ID = "retdec118-psx"
TOOL_PATHS = {
    "driver": "toolchains/gcc-2.7.2-psx/gcc",
    "cc1": "toolchains/gcc-2.7.2-psx/cc1",
    "cpp": "toolchains/gcc-2.7.2-psx/cpp",
    "assembler": "toolchains/psyqo/bin/mipsel-none-elf-as",
    "linker": "toolchains/psyqo/bin/mipsel-none-elf-ld",
    "maspsx_entry": "third_party/maspsx/maspsx.py",
    "maspsx_package": "third_party/maspsx/maspsx/__init__.py",
}


class CatalogDrift(ValueError):
    selection: dict | None = None


def _profile_paths(directory: Path, entry: dict) -> dict:
    paths = {
        name: str(
            directory / (entry["executable_relpath"] if name == "driver" else name)
        )
        for name in ("driver", "cc1", "cpp")
    }
    paths["directory"] = str(directory)
    paths.update(
        (f"specs_{ordinal:02d}", path)
        for ordinal, path in enumerate(runtime.gcc_specs_paths(str(directory) + "/"))
    )
    return paths


def _observe(
    paths: dict,
    *,
    follow_symlinks=False,
    expected=None,
    events: list[FailureEvent] | None = None,
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
) -> dict:
    observed, local = {}, []
    try:
        blocked = "directory" in paths and Path(paths["directory"]).is_symlink()
    except OSError as error:
        if expected is not None and events is not None:
            retain(
                events,
                error,
                phase="catalog-observation",
                path=paths["directory"],
                origin="ordinary",
            )
        raise
    for name, value in paths.items():
        path = Path(value)
        if expected is None:
            row = {
                "path": str(path),
                "exists": path.exists(),
                "symlink": path.is_symlink(),
            }
        try:
            if expected is not None:
                if expected[name]["exists"]:
                    path.lstat()
                row = {
                    "path": str(path),
                    "exists": path.exists(),
                    "symlink": path.is_symlink(),
                }
            if (
                path.is_file()
                and not blocked
                and (follow_symlinks or not row["symlink"])
            ):
                if reader is None:
                    row["file"] = digests.file_record(path)
                else:
                    known = expected[name]["file"] if expected is not None else None
                    bound = known["bytes"] if type(known) is dict else 64 << 20
                    start = len(local)
                    try:
                        raw = reader(path, bound, local)
                    finally:
                        local[start:] = [
                            (
                                origin,
                                "catalog-observation"
                                if phase in {"read", "close"}
                                else phase,
                                source,
                                error,
                            )
                            for origin, phase, source, error in local[start:]
                        ]
                    row["file"] = dict(
                        path=str(path.absolute()),
                        bytes=len(raw),
                        sha256=hashlib.sha256(raw).hexdigest(),
                    )
            else:
                row["file"] = None
        except OSError as error:
            if expected is not None:
                if not any(row[3] is error for row in local):
                    retain(
                        local,
                        error,
                        phase="catalog-observation",
                        path=path,
                        origin="ordinary",
                    )
                continue
            row.update(file=None, failure=f"{type(error).__name__}: {error}")
        observed[name] = row
    if events is not None:
        events.extend(local)
    if primary(local) is not None:
        raise primary(local)
    return observed


def select(
    root: Path,
    *,
    with_profiles=False,
    retained=None,
    events: list[FailureEvent] | None = None,
) -> dict:
    names = (
        TOOL_PATHS
        if not with_profiles
        else {
            name: path
            for name, path in TOOL_PATHS.items()
            if name not in {"driver", "cc1", "cpp"}
        }
    )
    shared_paths = {name: str(root / relative) for name, relative in names.items()}
    shared_paths.update(
        maspsx_interpreter="/usr/bin/python3.12", syntax_compiler="/usr/bin/cc"
    )
    shared = _observe(shared_paths, follow_symlinks=True) if with_profiles else {}
    files = (
        {name: row["file"] for name, row in shared.items() if row["file"] is not None}
        if with_profiles
        else {
            name: digests.file_record(Path(path)) for name, path in shared_paths.items()
        }
    )
    selection = {
        "catalog_id": CATALOG_ID,
        "pipeline_id": recipe.CANONICAL_PROFILE_ID,
        "pipeline_digest": recipe.pipeline_digest(),
        "files": files,
        "options": list(recipe.REQUIRED_OPTIONS),
        "scope": "Generic object-only pipeline; no source profiles or answer inputs.",
    }
    if retained is not None:
        retained.update(selection)
    if not with_profiles:
        return selection
    layout = repo_layout(root)
    entry = {
        "id": DEFAULT_COMPILER_ID,
        "identity": gcc.OLD_GCC_IDENTITY,
        "host": "linux-x86_64",
        "assembler": "maspsx --aspsx-version=2.56",
        "executable_relpath": "gcc",
    }
    directory = layout.gcc272_psx_root
    paths = _profile_paths(directory, entry)
    specs = [name for name in paths if name.startswith("specs_")]
    observed = _observe(paths)
    profile = {
        "profile_id": recipe.CANONICAL_PROFILE_ID,
        "ordinal": 0,
        "entry": entry,
        "original": observed,
        "availability": "refused",
        "refusal": None,
    }
    try:
        gcc_variants.check_host_compatible(entry["host"])
        if entry["assembler"] != f"maspsx --aspsx-version={recipe.MASPSX_VERSION}":
            raise ValueError("assembler is incompatible with the canonical recipe")
        if any(
            observed[name]["symlink"] or not observed[name]["exists"]
            for name in ("driver", "cc1", "cpp", "directory")
        ):
            raise ValueError("compiler component or directory is absent or symlinked")
        if any(observed[name]["exists"] or observed[name]["symlink"] for name in specs):
            raise ValueError("canonical compiler has unsupported external specs")
        if any(
            observed[name]["file"] is None
            or directory.resolve() not in Path(paths[name]).resolve().parents
            for name in ("driver", "cc1", "cpp")
        ):
            raise ValueError(
                "compiler components must be regular files within their installation"
            )
        gcc.GccToolchain(layout).verify()
        if _observe(paths) != observed:
            raise ValueError("compiler changed during identity validation")
        profile["availability"] = "available"
    except (OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        profile["availability"] = "refused"
        profile["refusal"] = f"{type(error).__name__}: {error}"
    profile["identity"] = digests.digest(
        {
            "entry": entry,
            "original": observed,
            "shared": shared,
            "options": selection["options"],
            "pipeline": selection["pipeline_digest"],
        }
    )
    profiles = [profile]
    selection.update(
        profiles=profiles,
        shared_observation=shared,
        profiles_digest=digests.digest(profiles),
        preparation_failure="shared compiler tools are absent or unreadable"
        if len(files) != len(shared_paths)
        else None,
        preparation_probe_limit={
            "maximum_probes": len(profiles),
        },
    )
    for name in ("driver", "cc1", "cpp"):
        record = profiles[0]["original"][name]["file"]
        if record is not None:
            files[name] = record
    if retained is not None:
        retained.update(selection)
    verify(selection, events=events)
    return selection


def _tool_paths(selection: dict, profile: dict | None = None) -> dict:
    if profile is None and selection.get("profiles"):
        profile = selection["profiles"][0]
    tools = {
        name: selection["files"][name]["path"]
        for name in recipe.TOOL_NAMES
        if name != "maspsx_root"
        and (profile is None or name not in {"driver", "cc1", "cpp"})
    }
    tools["maspsx_root"] = str(Path(tools["maspsx_entry"]).parent)
    if profile is not None:
        tools.update(
            (name, profile["original"][name]["path"])
            for name in ("driver", "cc1", "cpp")
        )
    return tools


def resolve(
    root: Path,
    *,
    with_profiles=False,
    retained=None,
    events: list[FailureEvent] | None = None,
) -> tuple[dict, dict]:
    selection = select(
        root, with_profiles=with_profiles, retained=retained, events=events
    )
    try:
        if selection.get("preparation_failure"):
            raise CatalogDrift(selection["preparation_failure"])
        canonical = _tool_paths(selection)
        for profile in selection.get("profiles", []):
            if profile["availability"] == "available":
                profile["tools"] = _tool_paths(selection, profile)
        if with_profiles:
            selection["profiles_digest"] = digests.digest(selection["profiles"])
        verify(selection, events=events)
        return selection, canonical
    except Exception as error:
        selection["preparation_failure"] = f"{type(error).__name__}: {error}"
        if retained is not None:
            retained.update(selection)
        if events is not None:
            retain(
                events,
                error,
                phase="catalog-resolution",
                path=root,
                origin="policy"
                if isinstance(error, (CatalogDrift, PolicyFailure))
                else "unknown",
            )
        raise


def verify(
    selection: dict,
    *,
    events: list[FailureEvent] | None = None,
    reader: Callable[[Path, int, list[FailureEvent] | None], bytes] | None = None,
) -> None:
    local, records = [], []
    for group in ("files",):
        try:
            values = selection[group]
            if type(values) is not dict:
                raise CatalogDrift("selected compiler file inventory differs")
            records.extend(values.values())
        except Exception as error:
            retain(
                local,
                error,
                phase=group,
                path="tools",
                origin="policy" if isinstance(error, CatalogDrift) else "unknown",
            )
    for phase in ("pipeline", "options", "shared", "profiles"):
        try:
            if (
                phase == "pipeline"
                and selection["pipeline_digest"] != recipe.pipeline_digest()
            ):
                raise CatalogDrift("selected compiler recipe changed")
            if phase == "options" and selection["options"] != list(
                recipe.REQUIRED_OPTIONS
            ):
                raise CatalogDrift("selected compiler options changed")
            if phase == "shared" and "profiles" in selection:
                if selection["profiles_digest"] != digests.digest(
                    selection["profiles"]
                ):
                    raise CatalogDrift("frozen compiler profiles or ordering changed")
                shared = selection["shared_observation"]
                if (
                    _observe(
                        {name: row["path"] for name, row in shared.items()},
                        follow_symlinks=True,
                        expected=shared,
                        events=local,
                        reader=reader,
                    )
                    != shared
                ):
                    raise CatalogDrift(
                        "shared compiler tool availability or identity changed"
                    )
            if phase == "profiles":
                for profile in selection.get("profiles", []):
                    for field in ("original", "identity", "tools"):
                        try:
                            if field == "original":
                                original = profile["original"]
                                paths = _profile_paths(
                                    Path(original["directory"]["path"]),
                                    profile["entry"],
                                )
                                if set(original) != set(paths) or any(
                                    original[name]["path"] != path
                                    for name, path in paths.items()
                                ):
                                    raise CatalogDrift(
                                        "native compiler lookup inventory differs"
                                    )
                                if profile["availability"] == "available" and any(
                                    original[name]["exists"]
                                    or original[name]["symlink"]
                                    for name in paths
                                    if name.startswith("specs_")
                                ):
                                    raise CatalogDrift(
                                        "canonical compiler has unsupported external specs"
                                    )
                                if (
                                    _observe(
                                        {
                                            name: row["path"]
                                            for name, row in original.items()
                                        },
                                        expected=original,
                                        events=local,
                                        reader=reader,
                                    )
                                    != original
                                ):
                                    raise CatalogDrift(
                                        "compiler availability or component identity changed"
                                    )
                            elif field == "identity":
                                identity = digests.digest(
                                    dict(
                                        entry=profile["entry"],
                                        original=profile["original"],
                                        shared=selection["shared_observation"],
                                        options=selection["options"],
                                        pipeline=selection["pipeline_digest"],
                                    )
                                )
                                if identity != profile["identity"]:
                                    raise CatalogDrift(
                                        "compiler profile identity changed"
                                    )
                            elif "tools" in profile and profile["tools"] != _tool_paths(
                                selection, profile
                            ):
                                raise CatalogDrift(
                                    "executed tools differ from native compiler inputs"
                                )
                        except Exception as error:
                            retain(
                                local,
                                error,
                                phase=field,
                                path="tools",
                                origin="policy"
                                if isinstance(error, CatalogDrift)
                                else "unknown",
                            )
        except Exception as error:
            retain(
                local,
                error,
                phase=phase,
                path="tools",
                origin="policy" if isinstance(error, CatalogDrift) else "unknown",
            )
    for record in records:
        try:
            if (
                type(record) is not dict
                or set(record) != {"path", "bytes", "sha256"}
                or type(record["path"]) is not str
            ):
                raise CatalogDrift("selected compiler file record differs")
            path = Path(record["path"])
            try:
                if reader is None:
                    observed = digests.file_record(path)
                else:
                    start = len(local)
                    try:
                        raw = reader(path, record["bytes"], local)
                    finally:
                        local[start:] = [
                            (
                                origin,
                                "catalog-file" if phase in {"read", "close"} else phase,
                                source,
                                error,
                            )
                            for origin, phase, source, error in local[start:]
                        ]
                    observed = dict(
                        path=str(path.absolute()),
                        bytes=len(raw),
                        sha256=hashlib.sha256(raw).hexdigest(),
                    )
            except Exception as error:
                if not any(row[3] is error for row in local):
                    retain(
                        local, error, phase="catalog-file", path=path, origin="ordinary"
                    )
                raise
            if observed != record:
                raise CatalogDrift("selected native compiler tool changed")
        except Exception as error:
            retain(
                local,
                error,
                phase="catalog-identity",
                path="tools",
                origin="policy" if isinstance(error, CatalogDrift) else "unknown",
            )
    if events is not None:
        events.extend(local)
    if primary(local) is not None:
        raise primary(local)
