"""Validate native build receipts against both forks and every retained input."""

from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import zipfile

from harness.common.digests import file_record
from harness.common.artifacts import active_reader
from harness.common.git import read_head

SCHEMA = "bof3.rz-psxdec-build/v1"
PREREQUISITES = {
    "LLVM": (
        "toolchains/downloads/retdec-psx/llvm.zip",
        "b5879b30768135e5fce84ccd8be356d2c55c940ab32ceb22d278b228e88c4c60",
        "llvm-a776c2a976ef64d9cd84d7ee71d0e4a04aa117a1",
    ),
    "CAPSTONE": (
        "toolchains/downloads/retdec-psx/capstone.zip",
        "c47acdabb9ba4922a6d68b96eb7e14a431bfef7d7c57cea1e5881f87776228b2",
        "capstone-5.0-rc2",
    ),
    "YARA": (
        "toolchains/downloads/retdec-psx/yara.zip",
        "ae1adad2ae33106f4c296cef32ddba2c93867010ef853028d30cad42548d0474",
        "yara-4.2.0-rc1",
    ),
}
TOOL_ROLES = frozenset(
    {
        "cc",
        "cxx",
        "cmake",
        "make",
        "ar",
        "ranlib",
        "ld",
        "cc1",
        "cc1plus",
        "collect2",
        "autoconf",
        "automake",
        "aclocal",
        "libtoolize",
        "pkg-config",
        "git",
        "python",
        "rizin",
        "ldd",
        "driver",
        "receipt_reader",
    }
)


@dataclass(frozen=True)
class FileIdentity:
    path: Path
    bytes: int
    sha256: str


@dataclass(frozen=True)
class ForkSource:
    root: Path
    commit: str
    before: tuple[dict, ...]
    after: tuple[dict, ...]


@dataclass(frozen=True)
class BuildExecution:
    argv: tuple[str, ...]
    cwd: Path
    raw_exit: int
    failure: None
    log: FileIdentity
    streams: tuple[FileIdentity, ...]


@dataclass(frozen=True)
class Prerequisite:
    role: str
    root: Path
    archive: FileIdentity
    preparation: dict
    before: tuple[dict, ...]
    after: tuple[dict, ...]


@dataclass(frozen=True)
class NativeBuildReceipt:
    build_root: Path
    plugin: ForkSource
    retdec: ForkSource
    configure: BuildExecution
    build: BuildExecution
    probe: BuildExecution
    prerequisites: tuple[Prerequisite, ...]
    tools: tuple[FileIdentity, ...]
    external_inputs: tuple[FileIdentity, ...]
    compiler_inputs: tuple[FileIdentity, ...]
    link_inputs: tuple[FileIdentity, ...]
    build_files: tuple[dict, ...]
    dso: FileIdentity


def _require(condition: object, message: str) -> None:
    if not condition:
        raise ValueError(message)


def tree_manifest(root: Path) -> list[dict]:
    """Hash every source file while excluding Git administration and bytecode."""
    _require(root.is_dir(), f"missing native tree: {root}")
    rows = []
    for directory, folders, files in os.walk(root, followlinks=False):
        folders[:] = sorted(
            name for name in folders if name not in {".git", "__pycache__"}
        )
        for name in sorted(files):
            if name == ".git":
                continue
            path = Path(directory) / name
            if path.is_file():
                rows.append(
                    {**file_record(path), "path": path.relative_to(root).as_posix()}
                )
    return sorted(rows, key=lambda row: row["path"])


def git_commit(root: Path) -> str:
    reader = active_reader()
    if reader is not None:
        return read_head(root, reader=reader)
    return subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=root, text=True
    ).strip()


def fork_record(root: Path) -> dict:
    files = tree_manifest(root)
    _require(bool(files), f"empty native fork: {root}")
    return {"root": str(root), "commit": git_commit(root), "files": files}


def archive_manifest(path: Path, directory: str) -> list[dict]:
    """Re-derive the pristine prerequisite manifest from its pinned ZIP."""
    rows = []
    with zipfile.ZipFile(path) as archive:
        for member in archive.infolist():
            if member.is_dir():
                continue
            relative = Path(member.filename)
            _require(
                relative.parts[0] == directory and len(relative.parts) > 1,
                "prerequisite archive root differs",
            )
            _require(
                not relative.is_absolute() and ".." not in relative.parts,
                "prerequisite archive path escapes its source",
            )
            data = archive.read(member)
            rows.append(
                {
                    "path": Path(*relative.parts[1:]).as_posix(),
                    "bytes": len(data),
                    "sha256": hashlib.sha256(data).hexdigest(),
                }
            )
    _require(bool(rows), "empty prerequisite archive")
    _require(len({row["path"] for row in rows}) == len(rows), "duplicate archive path")
    return sorted(rows, key=lambda row: row["path"])


def source_changes(before: list[dict], after: list[dict]) -> list[dict]:
    old, new = ({row["path"]: row for row in rows} for rows in (before, after))
    return [
        {"path": path, "before": old.get(path), "after": new.get(path)}
        for path in sorted(old.keys() | new.keys())
        if old.get(path) != new.get(path)
    ]


def link_paths(build: Path, compiler: Path) -> set[Path]:
    """Resolve the actual rz-psxdec link command's object and library inputs."""
    command = build / "src/rz-plugin/CMakeFiles/rz-psxdec.dir/link.txt"
    directory = command.parent.parent.parent
    arguments = shlex.split(command.read_text())
    expanded = []
    for argument in arguments:
        if argument.startswith("@"):
            expanded += shlex.split((directory / argument[1:]).read_text())
        else:
            expanded.append(argument)
    directories = [
        Path(argument[2:]) for argument in expanded if argument.startswith("-L")
    ]
    paths = set()
    for argument in expanded:
        if argument.startswith("-l"):
            name = argument[2:]
            candidates = [
                folder / f"lib{name}{suffix}"
                for folder in directories
                for suffix in (".so", ".a")
            ]
            candidates += [
                Path(
                    subprocess.check_output(
                        [str(compiler), f"-print-file-name=lib{name}{suffix}"],
                        text=True,
                    ).strip()
                )
                for suffix in (".so", ".a")
            ]
            selected = next((path for path in candidates if path.is_file()), None)
            _require(
                selected is not None, f"cannot identify native link library: {name}"
            )
            paths.add(selected.resolve())
            continue
        path = Path(argument)
        if not path.is_absolute():
            path = directory / path
        if path.is_file() and (path.suffix in {".o", ".a"} or ".so" in path.name):
            paths.add(path.resolve())
    return paths


def compiler_paths(build: Path, source_roots: tuple[Path, ...]) -> set[Path]:
    """Read actual CMake and YARA depfiles after compilation."""
    paths = set()
    depfiles = list(build.rglob("*.o.d"))
    for root in source_roots:
        depfiles += list(root.rglob("*.Po")) + list(root.rglob("*.Plo"))
    for depfile in depfiles:
        content = depfile.read_text().replace("\\\n", " ")
        for line in content.splitlines():
            _, separator, dependencies = line.partition(":")
            if not separator:
                continue
            for token in shlex.split(dependencies):
                path = Path(token)
                if not path.is_absolute() or not path.is_file():
                    continue
                path = path.resolve()
                if not any(
                    path.is_relative_to(root) for root in (build, *source_roots)
                ):
                    paths.add(path)
    return paths


def _identity(row: dict) -> FileIdentity:
    _require(
        type(row) is dict and set(row) == {"path", "bytes", "sha256"},
        "malformed native file identity",
    )
    path = Path(row["path"])
    _require(
        path.is_absolute()
        and type(row["bytes"]) is int
        and row["bytes"] >= 0
        and type(row["sha256"]) is str
        and re.fullmatch("[a-f0-9]{64}", row["sha256"]),
        "invalid native file identity",
    )
    _require(file_record(path) == row, f"native input changed: {path}")
    return FileIdentity(path, row["bytes"], row["sha256"])


def _identities(rows: list[dict]) -> tuple[FileIdentity, ...]:
    _require(type(rows) is list and bool(rows), "empty native input identities")
    _require(len({row["path"] for row in rows}) == len(rows), "duplicate native input")
    return tuple(_identity(row) for row in rows)


def _execution(row: dict) -> BuildExecution:
    log = _identity(row["log"])
    actual = json.loads(log.path.read_bytes())
    _require(
        type(row["argv"]) is list
        and all(type(arg) is str for arg in row["argv"])
        and row["argv"] == actual["argv"]
        and row["cwd"] == actual["cwd"]
        and type(row["raw_exit"]) is int
        and row["raw_exit"] == 0
        and actual["raw_exit"] == 0
        and row["failure"] is None
        and actual["failure"] is None
        and actual["capture_complete"] is True,
        "native execution did not complete successfully",
    )
    streams = tuple(_identity(actual["streams"][name]) for name in ("stdout", "stderr"))
    return BuildExecution(tuple(row["argv"]), Path(row["cwd"]), 0, None, log, streams)


def _fork(row: dict) -> ForkSource:
    root = Path(row["root"])
    _require(
        root.is_absolute() and re.fullmatch("[a-f0-9]{40}", row["commit"]),
        "invalid native fork identity",
    )
    _require(
        row["commit"] == row["commit_after"] == git_commit(root),
        "native fork commit changed",
    )
    _require(
        bool(row["before"]) and row["before"] == row["after"] == tree_manifest(root),
        "complete native fork content changed",
    )
    return ForkSource(root, row["commit"], tuple(row["before"]), tuple(row["after"]))


def read_native_build(path: Path) -> NativeBuildReceipt:
    """Read a complete receipt and rehash both forks, prerequisites and DSO."""
    _require(path.stat().st_size <= 64 << 20, "native receipt exceeds 64 MiB")
    document = json.loads(path.read_bytes())
    _require(
        type(document) is dict and document["schema"] == SCHEMA,
        "native build receipt schema differs",
    )
    _require(
        set(document["sources"]) == {"plugin", "retdec"}, "native source roles differ"
    )
    plugin, retdec = (_fork(document["sources"][role]) for role in ("plugin", "retdec"))
    _require(
        plugin.root.name == "rz-psxdec"
        and retdec.root.name == "retdec-psx"
        and plugin.root.parent == retdec.root.parent,
        "native sources are not sibling forks",
    )
    build_root = Path(document["build_root"])
    _require(
        build_root.is_absolute()
        and tuple(tree_manifest(build_root)) == tuple(document["build_files"]),
        "native build tree changed",
    )
    configure, build, probe = (
        _execution(document[role]) for role in ("configure", "build", "probe")
    )
    _require(
        configure.cwd == build.cwd == path.parent
        and configure.argv[1:5] == ("-S", str(plugin.root), "-B", str(build_root))
        and f"-DRETDEC_SOURCE_DIR={retdec.root}" in configure.argv
        and f"-DRZ_PSXDEC_REVISION={plugin.commit}" in configure.argv
        and f"-DRETDEC_PSX_REVISION={retdec.commit}" in configure.argv
        and f"-DCMAKE_INSTALL_PREFIX={path.parent / 'prefix'}" in configure.argv
        and "-DBUILD_BUNDLED_RETDEC=ON" in configure.argv
        and len(build.argv) == 7
        and build.argv[:4]
        == (
            configure.argv[0],
            "--build",
            str(build_root),
            "--parallel",
        )
        and re.fullmatch("[1-9][0-9]*", build.argv[4])
        and build.argv[5:] == ("--target", "rz-psxdec"),
        "native build commands differ from the sibling target",
    )
    _require(
        set(document["prerequisites"]) == set(PREREQUISITES),
        "prerequisite roles differ",
    )
    prerequisites = []
    for role, row in document["prerequisites"].items():
        archive = _identity(row["archive"])
        _, expected_hash, directory = PREREQUISITES[role]
        root = Path(row["root"])
        _require(
            archive.sha256 == expected_hash
            and root.is_relative_to(path.parent / "prerequisites")
            and row["before"] == archive_manifest(archive.path, directory)
            and row["after"] == tree_manifest(root),
            "prerequisite source identity differs",
        )
        preparation = row["preparation"]
        _require(
            preparation
            == {
                "operation": "harness.toolchain.releases.extract_archive",
                "executable_modes": "ZIP external_attr",
            },
            "prerequisite preparation differs",
        )
        changes = source_changes(row["before"], row["after"])
        _require(
            row["mutations"] == changes and (role == "YARA" or not changes),
            "undeclared prerequisite source mutation",
        )
        _require(
            f"-D{role}_LOCAL_DIR={root}" in configure.argv,
            "configure does not consume the recorded prerequisite",
        )
        prerequisites.append(
            Prerequisite(
                role,
                root,
                archive,
                preparation,
                tuple(row["before"]),
                tuple(row["after"]),
            )
        )
    _require(
        set(document["tools_before"]) == TOOL_ROLES
        and document["tools_before"] == document["tools_after"],
        "native tools changed",
    )
    tools = tuple(_identity(row) for row in document["tools_after"].values())
    tool_paths = {role: row["path"] for role, row in document["tools_after"].items()}
    _require(
        configure.argv[0] == tool_paths["cmake"]
        and f"-DCMAKE_C_COMPILER={tool_paths['cc']}" in configure.argv
        and f"-DCMAKE_CXX_COMPILER={tool_paths['cxx']}" in configure.argv,
        "native command tools differ from their identities",
    )
    _require(
        document["external_inputs_before"] == document["external_inputs_after"],
        "external native build inputs changed",
    )
    external = _identities(document["external_inputs_after"])
    compiler = _identities(document["compiler_inputs"])
    roots = (plugin.root, retdec.root, *(item.root for item in prerequisites))
    _require(
        document["compiler_inputs_observation"] == "post-build CMake/YARA depfiles"
        and {item.path for item in compiler} == compiler_paths(build_root, roots),
        "consumed compiler input inventory differs",
    )
    links = _identities(document["link_inputs"])
    _require(
        {item.path for item in links}
        == link_paths(build_root, Path(tool_paths["cxx"])),
        "actual rz-psxdec link inventory differs",
    )
    _require(
        {item.path for item in links if not item.path.is_relative_to(build_root)}
        <= {item.path for item in external},
        "external link input lacks pre-build identity",
    )
    dso = _identity(document["dso"])
    _require(
        dso.path == build_root / "src/rz-plugin/rz-psxdec.so"
        and any(item.path.suffix == ".a" for item in links)
        and any(item.path.suffix == ".o" for item in links)
        and probe.argv
        == (tool_paths["rizin"], "-N", "-n", "-q", "-l", str(dso.path), "-c", "pdza?"),
        "native DSO or explicit load probe differs",
    )
    output = probe.streams[0].path.read_bytes() + probe.streams[1].path.read_bytes()
    _require(
        b"pdzar" in output, "explicit DSO probe did not expose original-range command"
    )
    return NativeBuildReceipt(
        build_root,
        plugin,
        retdec,
        configure,
        build,
        probe,
        tuple(prerequisites),
        tools,
        external,
        compiler,
        links,
        tuple(document["build_files"]),
        dso,
    )
