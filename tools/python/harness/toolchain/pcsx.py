from __future__ import annotations

from enum import Enum
import hashlib
import json
import re
import stat
import os
import shutil
from pathlib import Path
import subprocess

from ..io import file_sha256, write_json
from .base import ExecutableToolchain
from ..patches.git import run_git, git_text
from ..common.process import run_bounded
from ..common.deadlines import check_deadline, use_deadline
from ..runtime.origin import decode_object
from .sdl import SdlToolchain, build_command

PACKAGES = (
    "libcapstone-dev",
    "libcapstone4",
    "libavcodec-dev",
    "libavformat-dev",
    "libavutil-dev",
    "libswresample-dev",
    "libcurl4-openssl-dev",
    "libuv1-dev",
)


class _SourceUse(Enum):
    OBSERVE = "observe"
    BUILD = "build"
    RUNTIME = "runtime"


class PcsxReduxToolchain(ExecutableToolchain):
    label = "PCSX-Redux"
    submodule = "third_party/pcsx-redux-agentic"

    @property
    def executable(self) -> Path:
        return self.layout.root / self.submodule / "pcsx-redux"

    @property
    def library(self) -> Path:
        return SdlToolchain(self.layout).library

    @property
    def environment(self) -> dict[str, str]:
        environment = os.environ.copy()
        environment["LD_LIBRARY_PATH"] = str(self.library.parent)
        return environment

    def source_identity(self) -> dict:
        return _inspect_sources(self.layout, self.submodule, use=_SourceUse.RUNTIME)

    def _runtime_identity(self, source: dict) -> dict:
        if not self.executable.is_file() or not os.access(self.executable, os.X_OK):
            raise FileNotFoundError(f"PCSX-Redux binary not built: {self.executable}")
        if not self.library.is_file():
            raise FileNotFoundError(f"missing approved SDL3 build: {self.library}")
        binary = file_sha256(self.executable)
        resources = self.catalogue(source=source, binary_sha256=binary)
        return {
            **source,
            "binary": str(self.executable),
            "binary_sha256": binary,
            "sdl_library": str(self.library.resolve()),
            "sdl_sha256": file_sha256(self.library),
            "resources": resources,
        }

    def runtime_identity(self) -> dict:
        return self._runtime_identity(self.source_identity())

    def catalogue(self, *, source=None, binary_sha256=None, deadline=None) -> dict:
        with use_deadline(deadline):
            check_deadline()
            source = self.source_identity() if source is None else source
            binary_sha256 = (
                file_sha256(self.executable) if binary_sha256 is None else binary_sha256
            )
            result = run_bounded(
                self.layout.root,
                [str(self.executable), "-mission-catalog"],
                timeout=15,
                output_limit=2 * 1024 * 1024,
                deadline=deadline,
                env=self.environment,
            )
            if result["exit_code"] or result["failure"] or result["stderr"]:
                raise ValueError("Native mission catalogue query failed")
            report = _qualify_catalogue(
                self.layout.root / self.submodule, result["stdout"]
            )
            if (
                file_sha256(self.executable) != binary_sha256
                or _inspect_sources(self.layout, self.submodule, use=_SourceUse.OBSERVE)
                != source
            ):
                raise ValueError("Native catalogue binary/source identity changed")
            return report

    def prerequisites(self) -> dict:
        tools = {}
        for name in (
            "git",
            "cmake",
            "make",
            "gcc",
            "g++",
            "pkg-config",
            "dpkg-query",
            "ldd",
        ):
            path = shutil.which(name)
            if path is None:
                raise FileNotFoundError(
                    f"PCSX-Redux build prerequisite missing: {name}"
                )
            tools[name] = path
        result = subprocess.run(
            [
                "dpkg-query",
                "-W",
                "-f=${Package}\t${db:Status-Status}\n",
                *PACKAGES,
            ],
            capture_output=True,
            text=True,
            check=False,
            timeout=30,
        )
        rows = [row.split("\t") for row in result.stdout.splitlines()]
        installed = sorted(
            row[0] for row in rows if len(row) == 2 and row[1] == "installed"
        )
        if result.returncode or installed != sorted(PACKAGES):
            raise ValueError(
                "PCSX-Redux build packages missing; see docs/reference/audio-runtime-setup.md"
            )
        result = subprocess.run(
            [
                "pkg-config",
                "--exists",
                "capstone",
                "freetype2",
                "libavcodec",
                "libavformat",
                "libavutil",
                "libswresample",
                "libcurl",
                "libuv",
                "zlib",
                "gl",
                "x11",
                "xcb",
            ],
            capture_output=True,
            text=True,
            check=False,
            timeout=30,
        )
        if result.returncode:
            raise ValueError(
                f"PCSX-Redux development metadata missing: {result.stderr.strip()}"
            )
        return {
            "packages": installed,
            "tools": tools,
        }

    def build(self) -> str:
        dependencies = self.prerequisites()
        before = _inspect_sources(self.layout, self.submodule, use=_SourceUse.BUILD)
        sdl = SdlToolchain(self.layout)
        sdl.run()
        environment = self.environment
        environment["PKG_CONFIG_PATH"] = str(sdl.prefix / "lib/pkgconfig")
        command = [
            "make",
            "-C",
            str(self.layout.root / self.submodule),
            "-j6",
            "BUILD=Release",
        ]
        for name in ("pcsx-redux-build.json", "pcsx-redux.json"):
            receipt = self.layout.out_dir / "setup" / name
            if receipt.exists():
                destination = receipt.with_name(
                    name + "." + file_sha256(receipt) + ".historical"
                )
                if (
                    destination.exists()
                    and destination.read_bytes() != receipt.read_bytes()
                ):
                    raise ValueError("PCSX build receipt archive identity differs")
                if not destination.exists():
                    shutil.copyfile(receipt, destination)
        build_command(
            self.layout.root,
            command,
            self.layout.out_dir / "setup/pcsx-redux-build.json",
            env=environment,
        )
        after = _inspect_sources(self.layout, self.submodule, use=_SourceUse.OBSERVE)
        if {k: v for k, v in after.items() if k != "generated_sources"} != {
            k: v for k, v in before.items() if k != "generated_sources"
        } or self.prerequisites() != dependencies:
            raise ValueError("PCSX-Redux build inputs changed")
        identity = self._runtime_identity(after)
        write_json(
            self.layout.out_dir / "setup/pcsx-redux.json",
            {
                "schema": "bof3.redux-build/v2",
                "identity": identity,
                "dependencies": dependencies,
                "command": command,
            },
        )
        return str(self.executable)

    def _git(self, *arguments: str) -> str:
        result = subprocess.run(
            ["git", *arguments],
            cwd=self.layout.root,
            check=False,
            capture_output=True,
            text=True,
        )
        if result.returncode:
            raise RuntimeError(
                f"PCSX-Redux git command failed: {result.stderr.strip()}"
            )
        return result.stdout.strip()

    def _revision(self) -> str:
        fields = self._git("ls-files", "--stage", "--", self.submodule).split()
        if len(fields) != 4 or fields[0] != "160000" or fields[2] != "0":
            raise ValueError(f"{self.submodule} must be a registered Git submodule")
        return fields[1]

    def install(self, *, force: bool = False) -> str:
        revision = self._revision()
        key = f"submodule.{self.submodule}"
        if (
            self._git("config", "--file", ".gitmodules", "--get", key + ".path")
            != self.submodule
            or self._git("config", "--file", ".gitmodules", "--get", key + ".url")
            != "https://github.com/loopyd/pcsx-redux-agentic.git"
        ):
            raise ValueError("Registered PCSX-Redux Agentic source path/URL differs")
        configured = subprocess.run(
            ["git", "config", "--get", key + ".url"],
            cwd=self.layout.root,
            capture_output=True,
            text=True,
            check=False,
            timeout=30,
        )
        if configured.returncode not in {0, 1} or (
            configured.returncode == 0
            and configured.stdout.strip()
            != "https://github.com/loopyd/pcsx-redux-agentic.git"
        ):
            raise ValueError("Local PCSX-Redux Agentic fetch URL differs")
        for _ in range(64):
            observed = _inspect_sources(
                self.layout,
                self.submodule,
                use=_SourceUse.OBSERVE,
                collect_missing=True,
            )
            missing = observed["missing_sources"]
            if not missing:
                _inspect_sources(self.layout, self.submodule, use=_SourceUse.BUILD)
                return revision
            for relative, expected, owner, name in missing:
                parent = self.layout.root / owner if owner else self.layout.root
                run_git(
                    parent, "submodule", "update", "--init", "--checkout", "--", name
                )
                if (
                    git_text(self.layout.root / relative, "rev-parse", "HEAD")
                    != expected
                ):
                    raise ValueError(
                        "Initialized recursive source differs from its registered pin"
                    )
        raise ValueError("Recursive source initialization exceeds depth/count bound")

    def verify(self) -> str:
        SdlToolchain(self.layout).verify()
        identity = self.runtime_identity()
        receipt = json.loads(
            (self.layout.out_dir / "setup/pcsx-redux.json").read_text()
        )
        if (
            receipt.get("schema") != "bof3.redux-build/v2"
            or receipt.get("identity") != identity
        ):
            raise ValueError("PCSX-Redux build receipt is missing or stale; run setup")
        if receipt.get("dependencies") != self.prerequisites():
            raise ValueError("PCSX-Redux build dependencies changed; run setup")
        linked = subprocess.run(
            ["ldd", str(self.executable)],
            env=self.environment,
            capture_output=True,
            text=True,
            check=False,
            timeout=30,
        )
        if (
            linked.returncode
            or "not found" in linked.stdout
            or str(self.library.parent) not in linked.stdout
        ):
            raise ValueError(
                "PCSX-Redux linkage is missing the approved SDL3 or another library"
            )
        return f"{identity['revision']}; built executable, dependencies, SDL3 and linkage verified"


GENERATED_LUAJIT = {
    "src/host/buildvm_arch.h",
    "src/jit/vmdef.lua",
    "src/lj_bcdef.h",
    "src/lj_ffdef.h",
    "src/lj_folddef.h",
    "src/lj_libdef.h",
    "src/lj_recdef.h",
    "src/lj_vm.S",
    "src/luajit.h",
}


def is_redux_artifact(submodule: str, relative: str, path: Path) -> bool:
    if relative == submodule:
        if str(path) in {"pcsx-redux", "bins/Release/pcsx-redux"}:
            return True
        return (
            len(path.parts) >= 3
            and path.parts[1] == "Release"
            and (
                (path.parts[0] == "objs" and path.suffix == ".o")
                or (path.parts[0] == "deps" and path.suffix == ".dep")
            )
        )
    if relative == f"{submodule}/third_party/luajit":
        return str(path) in {
            "src/host/buildvm",
            "src/host/buildvm.o",
            "src/host/buildvm_asm.o",
            "src/host/buildvm_fold.o",
            "src/host/buildvm_lib.o",
            "src/host/buildvm_peobj.o",
            "src/host/minilua",
            "src/host/minilua.o",
            "src/libluajit.a",
            "src/lj_vm.o",
            "src/ljamalg.o",
            "src/luajit",
            "src/luajit.o",
            "src/luajit_relver.txt",
        }
    return False


def _blob(data: bytes) -> str:
    return hashlib.sha1(b"blob " + str(len(data)).encode() + b"\0" + data).hexdigest()


def _checkout_representations(directory: Path, mismatches: list) -> None:
    if not mismatches:
        return
    information = Path(
        git_text(directory, "rev-parse", "--git-path", "info/attributes")
    )
    if not information.is_absolute():
        information = directory / information
    if information.exists() and (not information.is_file() or information.read_bytes()):
        raise ValueError("Repository-local attribute overrides are unsupported")
    environment = {
        key: value for key, value in os.environ.items() if not key.startswith("GIT_")
    }
    environment["GIT_ATTR_NOSYSTEM"] = "1"
    names = [row[0] for row in mismatches]
    result = subprocess.run(
        [
            "git",
            "-C",
            str(directory),
            "-c",
            "core.attributesFile=/dev/null",
            "check-attr",
            "--cached",
            "-z",
            "--stdin",
            "text",
            "eol",
            "filter",
            "working-tree-encoding",
            "ident",
        ],
        input=b"\0".join(name.encode() for name in names) + b"\0",
        capture_output=True,
        check=False,
        timeout=60,
        env=environment,
    )
    values = result.stdout.split(b"\0")
    if result.returncode or values[-1] != b"" or len(values) != len(names) * 15 + 1:
        raise ValueError("Committed checkout attributes cannot be qualified")
    attributes = {}
    for index in range(0, len(values) - 1, 3):
        name, attribute, value = (part.decode() for part in values[index : index + 3])
        owner = attributes.setdefault(name, {})
        if attribute in owner:
            raise ValueError("Duplicate committed checkout attribute")
        owner[attribute] = value
    if set(attributes) != set(names):
        raise ValueError("Committed checkout attribute paths differ")
    for name, oid, data, row in mismatches:
        selected = attributes[name]
        if (
            set(selected) != {"text", "eol", "filter", "working-tree-encoding", "ident"}
            or selected["eol"] != "crlf"
            or selected["text"] not in {"set", "auto", "unspecified"}
            or any(
                selected[key] != "unspecified"
                for key in ("filter", "working-tree-encoding", "ident")
            )
        ):
            raise ValueError(f"Unsupported committed checkout conversion: {name}")
        canonical = run_git(directory, "cat-file", "blob", oid)
        if b"\r" in canonical or data != canonical.replace(b"\n", b"\r\n"):
            raise ValueError(
                f"Tracked bytes differ from committed CRLF checkout: {name}"
            )
        row["checkout_attributes"] = selected
        row["canonical_bytes"] = len(canonical)
        row["canonical_sha256"] = hashlib.sha256(canonical).hexdigest()


def _inspect_sources(
    layout, submodule: str, *, use: _SourceUse, collect_missing: bool = False
) -> dict:
    root = layout.root
    fields = git_text(root, "ls-files", "--stage", "--", submodule).split()
    if (
        len(fields) != 4
        or fields[0] != "160000"
        or fields[2] != "0"
        or fields[3] != submodule
    ):
        raise ValueError("PCSX-Redux Agentic requires one stage-0 registered gitlink")
    revision = fields[1]
    pending = [(submodule, revision, "", submodule)]
    forests, generated, missing = [], {}, []
    while pending:
        check_deadline()
        relative, expected, owner, source_name = pending.pop(0)
        directory = root / relative
        if directory.resolve() != directory.absolute():
            raise ValueError("Fork source owner traverses a symlink")
        if not (directory / ".git").exists():
            if not collect_missing or (
                directory.exists()
                and (not directory.is_dir() or any(directory.iterdir()))
            ):
                raise ValueError(
                    "Fork source owner is missing or its uninitialized path is nonempty"
                )
            missing.append((relative, expected, owner, source_name))
            continue
        if git_text(directory, "rev-parse", "HEAD") != expected:
            raise ValueError(f"Fork recursive revision differs: {relative}")
        tree = run_git(directory, "ls-tree", "-r", "-z", "HEAD")
        index = run_git(directory, "ls-files", "--stage", "-z")
        committed, staged, rows, mismatches = {}, {}, [], []
        for entry in filter(None, tree.split(b"\0")):
            metadata, name = entry.split(b"\t", 1)
            mode, kind, oid = metadata.decode().split()
            committed[name.decode()] = (mode, oid)
        for entry in filter(None, index.split(b"\0")):
            metadata, name = entry.split(b"\t", 1)
            mode, oid, stage = metadata.decode().split()
            if stage != "0" or name.decode() in staged:
                raise ValueError("Fork source index has conflicts")
            staged[name.decode()] = (mode, oid)
        if committed != staged:
            raise ValueError(
                f"Fork source index differs from committed tree: {relative}"
            )
        for name, (mode, oid) in sorted(committed.items()):
            path = directory / name
            if not name or Path(name).is_absolute() or ".." in Path(name).parts:
                raise ValueError("Invalid committed source path")
            if path.parent.resolve() != path.parent.absolute():
                raise ValueError("Committed source parent traverses a symlink")
            if mode == "160000":
                pending.append((f"{relative}/{name}", oid, relative, name))
                rows.append({"path": name, "mode": mode, "oid": oid})
                continue
            observed = path.lstat()
            if mode == "120000":
                if not stat.S_ISLNK(observed.st_mode):
                    raise ValueError("Committed source symlink changed type")
                data = os.readlink(path).encode()
            elif mode in {"100644", "100755"}:
                if not stat.S_ISREG(observed.st_mode) or bool(
                    observed.st_mode & 0o111
                ) != (mode == "100755"):
                    raise ValueError(
                        "Committed source file changed type or executable mode"
                    )
                data = path.read_bytes()
            else:
                raise ValueError("Unsupported committed source mode")
            if _blob(data) != oid and mode == "120000":
                raise ValueError(f"Fork tracked source differs: {relative}/{name}")
            rows.append(
                {
                    "path": name,
                    "mode": mode,
                    "oid": oid,
                    "bytes": len(data),
                    "sha256": hashlib.sha256(data).hexdigest(),
                }
            )
            if _blob(data) != oid:
                mismatches.append((name, oid, data, rows[-1]))
        _checkout_representations(directory, mismatches)
        forest = hashlib.sha256(
            json.dumps(rows, sort_keys=True, separators=(",", ":")).encode()
        ).hexdigest()
        forests.append(
            {"path": relative, "revision": expected, "source_sha256": forest}
        )
        unknown = run_git(directory, "ls-files", "--others", "-z")
        for raw_name in filter(None, unknown.split(b"\0")):
            name = raw_name.decode()
            path = directory / name
            if (
                relative == f"{submodule}/third_party/luajit"
                and name in GENERATED_LUAJIT
            ):
                if path.resolve() != path.absolute() or not path.is_file():
                    raise ValueError(
                        "Generated LuaJIT input is not a canonical regular file"
                    )
                generated[f"{relative}/{name}"] = file_sha256(path)
            elif not is_redux_artifact(submodule, relative, Path(name)):
                raise ValueError(
                    f"Fork has an untracked or ignored build input: {relative}/{name}"
                )
    forests.sort(key=lambda row: row["path"])
    identity = {
        "source_owner": "pcsx-redux-agentic",
        "revision": revision,
        "source_forests": forests,
        "source_manifest_sha256": hashlib.sha256(
            json.dumps(forests, sort_keys=True, separators=(",", ":")).encode()
        ).hexdigest(),
        "generated_sources": dict(sorted(generated.items())),
    }
    expected_generated = {
        f"{submodule}/third_party/luajit/{name}" for name in GENERATED_LUAJIT
    }
    if generated and set(generated) != expected_generated:
        raise ValueError("Generated LuaJIT source map is incomplete")
    if collect_missing:
        identity["missing_sources"] = missing
    if generated and use is not _SourceUse.OBSERVE:
        try:
            previous = json.loads(
                (layout.out_dir / "setup/pcsx-redux.json").read_text(),
                object_pairs_hook=decode_object,
            )
            old = previous["identity"]
            cached = old["generated_sources"]
        except (OSError, ValueError, KeyError, TypeError) as error:
            raise ValueError(
                "Generated fork sources require the exact prior successful build receipt"
            ) from error
        if (
            previous.get("schema") != "bof3.redux-build/v2"
            or not isinstance(old, dict)
            or not isinstance(cached, dict)
            or set(cached) != expected_generated
            or any(
                not isinstance(value, str)
                or re.fullmatch(r"[0-9a-f]{64}", value) is None
                for value in cached.values()
            )
            or cached != generated
            or (
                use is _SourceUse.RUNTIME
                and any(old.get(key) != value for key, value in identity.items())
            )
        ):
            raise ValueError(
                "Generated fork sources or committed source identity changed"
            )
    return identity


def _qualify_catalogue(source: Path, raw: str) -> dict:
    if len(raw.encode()) > 2 * 1024 * 1024:
        raise ValueError("Mission catalogue exceeds 2 MiB")
    value = json.loads(raw, object_pairs_hook=decode_object)
    if (
        not isinstance(value, dict)
        or set(value) != {"schema", "resources", "state"}
        or value["schema"] != "psx.mission-catalog/v1"
    ):
        raise ValueError("Invalid mission catalogue schema")
    resources = value["resources"]
    if not isinstance(resources, list) or not 1 <= len(resources) <= 64:
        raise ValueError("Invalid mission resource count")
    names, rows = set(), []
    for row in resources:
        if not isinstance(row, dict) or set(row) != {"name", "role", "path", "chunk"}:
            raise ValueError("Invalid mission resource row")
        name, role, path, chunk = (
            row[key] for key in ("name", "role", "path", "chunk")
        )
        if (
            not isinstance(name, str)
            or re.fullmatch(r"[a-z][a-z0-9_]{0,63}", name) is None
            or name in names
            or role not in {"mission", "module"}
            or path != f"src/lua/missions/{name}.lua"
            or not isinstance(chunk, str)
        ):
            raise ValueError("Invalid or duplicate mission resource identity")
        names.add(name)
        target = source / path
        if target.resolve() != target.absolute() or not target.is_file():
            raise ValueError("Mission resource is not a canonical regular source")
        physical = target.read_bytes()
        opening, closing = b'-- lualoader, R"EOF(--\n', b'-- )EOF"\n'
        if (
            not physical.startswith(opening)
            or not physical.endswith(closing)
            or b')EOF"' in physical[len(opening) : -len(closing)]
        ):
            raise ValueError("Mission resource loader boundaries differ")
        compiled = b"--\n" + physical[len(opening) : -len(closing)] + b"-- "
        if len(compiled) > 1024 * 1024 or compiled != chunk.encode("utf-8"):
            raise ValueError(
                "Native compiled resource differs from owning source bytes"
            )
        rows.append(
            {
                "name": name,
                "role": role,
                "path": path,
                "source_sha256": hashlib.sha256(physical).hexdigest(),
                "source_bytes": len(physical),
                "chunk_sha256": hashlib.sha256(compiled).hexdigest(),
                "chunk_bytes": len(compiled),
            }
        )
    rows.sort(key=lambda row: row["name"])
    if [row["name"] for row in resources] != sorted(names):
        raise ValueError("Native catalogue resources must be sorted")
    if len(rows) != 53 or sum(row["role"] == "mission" for row in rows) != 20:
        raise ValueError(
            "Current native resource census differs from 53 resources / 20 missions"
        )
    state = value["state"]
    if (
        not isinstance(state, dict)
        or set(state) != {"version", "version_string", "schema"}
        or type(state["version"]) is not int
        or state["version"] != 6
        or state["version_string"] != "PCSX-Redux Agentic SaveState v6"
        or not isinstance(state["schema"], str)
        or not 1 <= len(state["schema"].encode()) <= 1024 * 1024
    ):
        raise ValueError("Native state catalogue identity differs")
    bundle = b"".join(
        (
            "\0".join(
                row[key]
                for key in ("name", "role", "path", "source_sha256", "chunk_sha256")
            )
            + "\n"
        ).encode()
        for row in rows
    )
    return {
        "schema": "psx.mission-bundle/v1",
        "resources": rows,
        "bundle_sha256": hashlib.sha256(bundle).hexdigest(),
        "state": {
            "version": state["version"],
            "version_string": state["version_string"],
            "schema_sha256": hashlib.sha256(state["schema"].encode()).hexdigest(),
            "schema_bytes": len(state["schema"].encode()),
        },
    }
