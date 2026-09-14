"""Conservative native build input closure shared by type, macro and naming gates."""

from __future__ import annotations

import hashlib
import json
import re
import stat
from pathlib import Path

from harness.io import unique_object


def load(path: Path):
    return json.loads(path.read_bytes(), object_pairs_hook=unique_object)


def file_state(path: Path):
    for part in (path, *path.parents):
        if part.is_symlink():
            raise ValueError(f"symlink input: {path}")
    if not path.exists():
        return None
    before = path.stat()
    mode = before.st_mode
    if not stat.S_ISREG(mode):
        raise ValueError(f"not a regular input: {path}")
    content = path.read_bytes()
    after = path.stat()
    # Reading may update atime; identity and mutation metadata must stay stable.
    fields = (
        "st_dev",
        "st_ino",
        "st_mode",
        "st_nlink",
        "st_uid",
        "st_gid",
        "st_size",
        "st_mtime_ns",
        "st_ctime_ns",
    )
    if any(getattr(before, field) != getattr(after, field) for field in fields):
        raise ValueError(f"moving input: {path}")
    return {
        "sha256": hashlib.sha256(content).hexdigest(),
        "mode": stat.S_IMODE(mode),
    }


def relative(value: str) -> str:
    if (
        not isinstance(value, str)
        or not value
        or value.startswith("/")
        or "\\" in value
        or any(p in {"", ".", ".."} for p in value.split("/"))
    ):
        raise ValueError("noncanonical repository input path")
    return value


def input_state(root: Path, target: str, transaction_paths: set[str]) -> dict:
    """Re-derive the complete conservative CMake/target closure, never caller scope."""
    from harness.domain.claims import manifest_source_paths
    from harness.domain.includes import local_include_files
    from harness.domain.manifests import load_target_manifests

    manifest = load_target_manifests(root)[target]
    if not manifest.has_explicit_sources or manifest.companions:
        raise ValueError(
            "native postapply requires explicit sources and no companion overlays"
        )
    paths = transaction_paths | {
        manifest.binary,
        manifest.splat,
        f"config/targets/{target}/reviewed.rz",
        "CMakeLists.txt",
        "out/catalog/emi.json",
        "config/splat.yaml",
        "config/compiler/object-flags.cmake",
        "config/compiler/variants.json",
        manifest.psyq_source,
    }
    # CMake enumerates all source claims, sources, shared headers and compiler
    # inputs. Capturing those sets also detects new/deleted glob dependencies.
    for directory, suffixes in (
        ("src", {".c", ".s", ".S", ".h", ".inc"}),
        ("include", {".h", ".inc"}),
        ("config/targets", {".toml", ".txt"}),
        ("config/sdk", {".txt"}),
        ("config/compiler", {".json", ".cmake"}),
        ("toolchains/psyq/4.7/include", {".h", ".inc"}),
        ("tools/python/harness", {".py"}),
        ("third_party/splat", {".py"}),
        ("third_party/spimdisasm", {".py"}),
        ("config/toolchains", {".toml", ".json"}),
    ):
        paths.update(
            p.relative_to(root).as_posix()
            for p in (root / directory).rglob("*")
            if p.suffix in suffixes
        )
    paths.update(
        p.relative_to(root).as_posix() for p in (root / "bin").iterdir() if p.is_file()
    )
    for directory in ("toolchains/gcc-2.7.2-psx", "toolchains/psn00b_toolchain/bin"):
        paths.update(
            p.relative_to(root).as_posix()
            for p in (root / directory).iterdir()
            if p.is_file()
        )
    paths.update(
        p.relative_to(root).as_posix()
        for p in (root / "third_party/maspsx").rglob("*.py")
    )
    # CMake resolves every configured variant, even outside the selected target.
    # Refuse missing installs before build can auto-install an opted-in variant.
    variants = load(root / "config/compiler/variants.json")["candidates"]
    # ponytail: literal, single-line sets and line comments only; extend this
    # declarative ceiling explicitly, never silently ignore executable CMake.
    text = (root / "config/compiler/object-flags.cmake").read_text()
    if re.search(r"#\[=*\[", text):
        raise ValueError("unsupported object-flags CMake bracket comment")
    configured = []
    for line in text.split("\n"):
        code = line.partition("#")[0].strip(" \t")
        if not code:
            continue
        assignment = re.fullmatch(
            r"set\(BOF3_OBJ(FLAGS|COMPILER)_[A-Za-z0-9_]+[ \t]+"
            r"([^()]+)\)",
            code,
        )
        if assignment is None:
            raise ValueError("unsupported object-flags CMake assignment")
        kind, value = assignment.groups()
        pattern = (
            r"[A-Za-z0-9][A-Za-z0-9._-]*"
            if kind == "COMPILER"
            else r"-[A-Za-z0-9_,=.+-]+(?:[ \t]+-[A-Za-z0-9_,=.+-]+)*"
        )
        value = value.strip(" \t")
        if re.fullmatch(pattern, value) is None:
            raise ValueError("unsupported object-flags CMake literal")
        if kind == "COMPILER":
            configured.append(value)
    for identifier in configured:
        entries = [v for v in variants if v["id"] == identifier]
        if len(entries) != 1:
            raise ValueError("unsupported configured compiler variant")
        directory = root / "toolchains/gcc-variants" / relative(identifier)
        executable = directory / relative(entries[0]["executable_relpath"])
        if file_state(executable) is None:
            raise ValueError("configured compiler variant must already be installed")
        paths.update(
            p.relative_to(root).as_posix() for p in directory.rglob("*") if p.is_file()
        )
    import yaml

    options = yaml.safe_load((root / manifest.splat).read_text())["options"]
    maps = options.get("symbol_addrs_path", [])
    paths.update([maps] if isinstance(maps, str) else maps)
    if options.get("target_path") != manifest.binary:
        raise ValueError("Splat target binary differs from manifest")
    seeds = [root / relative(p) for p in paths if p]
    paths.update(
        p.relative_to(root).as_posix() for p in local_include_files(root, seeds)
    )
    paths.discard("")
    for p in manifest_source_paths(root, manifest):
        if file_state(p) is None:
            raise ValueError(f"missing claimed source: {p}")
    return {p: file_state(root / relative(p)) for p in sorted(paths)}
