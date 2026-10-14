"""verify rejects changes to selected file records or the compiler recipe digest."""

from __future__ import annotations

import hashlib
import subprocess
from collections.abc import Callable
from pathlib import Path

from harness.build import recipe
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
MASPSX_BOOTSTRAP = b"""import sys

ENTRY = sys.argv[1]
ROOT = sys.argv[2]
sys.path.insert(0, ROOT)
sys.argv = [ENTRY, *sys.argv[3:]]
with open(ENTRY, "rb") as stream:
    SOURCE = stream.read()
exec(compile(SOURCE, ENTRY, "exec"), {"__name__": "__main__", "__file__": ENTRY})
"""


class CatalogDrift(ValueError):
    selection: dict | None = None


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
            row["mode"] = (
                path.lstat().st_mode if path.exists() or path.is_symlink() else None
            )
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
        "bootstrap": {
            "bytes": len(MASPSX_BOOTSTRAP),
            "sha256": hashlib.sha256(MASPSX_BOOTSTRAP).hexdigest(),
        },
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
    paths = {
        name: str(
            directory / (entry["executable_relpath"] if name == "driver" else name)
        )
        for name in ("driver", "cc1", "cpp")
    }
    paths["directory"] = str(directory)
    observed = _observe(paths)
    profile = {
        "profile_id": recipe.CANONICAL_PROFILE_ID,
        "ordinal": 0,
        "entry": entry,
        "original": observed,
        "availability": "refused",
        "refusal": None,
        "staged": {},
    }
    try:
        gcc_variants.check_host_compatible(entry["host"])
        if entry["assembler"] != f"maspsx --aspsx-version={recipe.MASPSX_VERSION}":
            raise ValueError("assembler is incompatible with the canonical recipe")
        if any(row["symlink"] or not row["exists"] for row in observed.values()):
            raise ValueError("compiler component or directory is absent or symlinked")
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
        staged_shared={},
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


def stage(
    root: Path,
    destination: Path,
    *,
    with_profiles=False,
    retained=None,
    events: list[FailureEvent] | None = None,
) -> tuple[dict, dict]:
    selection = select(
        root,
        with_profiles=with_profiles,
        retained=retained,
        events=events,
    )
    try:
        if selection.get("preparation_failure"):
            raise CatalogDrift(selection["preparation_failure"])
        paths = {
            "driver": "gcc/gcc",
            "cc1": "gcc/cc1",
            "cpp": "gcc/cpp",
            "specs": "gcc/specs",
            "assembler": "bin/as",
            "linker": "bin/ld",
            "maspsx_entry": "maspsx/maspsx.py",
            "maspsx_package": "maspsx/maspsx/__init__.py",
            "maspsx_bootstrap": "maspsx_bootstrap.py",
        }
        canonical = {
            name: str(destination / paths[name])
            for name in ("driver", "cc1", "cpp", "specs", "assembler", "maspsx_entry")
        }
        canonical.update(
            maspsx_root=str(destination / "maspsx"),
            maspsx_bootstrap=str(destination / "maspsx_bootstrap.py"),
            maspsx_interpreter=selection["files"]["maspsx_interpreter"]["path"],
        )
        copies = []
        specs = {"bytes": 0, "sha256": hashlib.sha256(b"").hexdigest()}
        for name, relative in paths.items():
            if with_profiles and name in {"driver", "cc1", "cpp", "specs"}:
                continue
            record = (
                selection["bootstrap"]
                if name == "maspsx_bootstrap"
                else specs
                if name == "specs"
                else selection["files"][name]
            )
            copies.append((name, record, destination / relative))
        for profile in selection.get("profiles", []):
            if profile["availability"] != "available":
                continue
            directory = destination / (
                "gcc"
                if profile["ordinal"] == 0
                else f"variants/{profile['ordinal']:03d}"
            )
            profile["tools"] = {
                **canonical,
                **{
                    name: str(directory / ("gcc" if name == "driver" else name))
                    for name in ("driver", "cc1", "cpp", "specs")
                },
            }
            copies += [
                (name, profile["original"][name]["file"], Path(profile["tools"][name]))
                for name in ("driver", "cc1", "cpp")
            ]
            copies.append(("specs", specs, Path(profile["tools"]["specs"])))
        for name, record, target in copies:
            source = (
                None if name in {"maspsx_bootstrap", "specs"} else Path(record["path"])
            )
            local, stream = [], None
            try:
                data = (
                    MASPSX_BOOTSTRAP
                    if name == "maspsx_bootstrap"
                    else b""
                    if name == "specs"
                    else source.read_bytes()
                )
                target.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
                stream = target.open("xb")
                if stream.write(data) != len(data):
                    raise ValueError("file write was incomplete")
            except Exception as error:
                retain(local, error, phase="tool-copy", path=target, origin="ordinary")
            if stream is not None:
                for phase in ("close", "chmod"):
                    try:
                        if phase == "close":
                            stream.close()
                        else:
                            target.chmod(
                                0o400
                                if name.startswith("maspsx_") or name == "specs"
                                else 0o500
                            )
                    except Exception as error:
                        retain(
                            local, error, phase=phase, path=target, origin="ordinary"
                        )
            if events is not None:
                events.extend(local)
            if local:
                raise local[0][3]
            try:
                copied = digests.file_record(target)
            except Exception as error:
                if events is not None:
                    retain(
                        events, error, phase="tool-read", path=target, origin="ordinary"
                    )
                raise
            if (copied["sha256"], copied["bytes"]) != (
                record["sha256"],
                record["bytes"],
            ):
                raise PolicyFailure("copied compiler tool differs")
        selection["staged_shared"] = {
            name: digests.file_record(destination / relative)
            for name, relative in paths.items()
            if not with_profiles or name not in {"driver", "cc1", "cpp", "specs"}
        }
        for profile in selection.get("profiles", []):
            profile["staged"] = (
                {
                    name: digests.file_record(Path(profile["tools"][name]))
                    for name in ("driver", "cc1", "cpp", "specs")
                }
                if "tools" in profile
                else {}
            )
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
                phase="catalog-stage",
                path=destination,
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
    for group in ("files", "staged_shared"):
        try:
            values = selection[group] if group == "files" else selection.get(group, {})
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
                    for field in ("original", "identity", "staged"):
                        try:
                            if field == "original":
                                original = profile["original"]
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
                            else:
                                records.extend(profile["staged"].values())
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
                raise CatalogDrift("selected or staged compiler tool changed")
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
