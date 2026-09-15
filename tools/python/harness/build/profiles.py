"""Read-only, input-pinned configured compiler profiles for source movement."""

from __future__ import annotations

import hashlib
import json
import os
import re
import stat
from pathlib import Path

from harness.build.compiler import (
    build_compiler_arguments,
    parse_object_compilers,
    parse_object_flags,
    resolve_compiler_settings,
    validate_object_configuration,
)
from harness.common.deadlines import check_deadline
from harness.common.inputs import InputBatch, read_input
from harness.common.observation import observe_directory
from harness.domain.cache import collect_claim_paths, collect_manifest_paths
from harness.domain.manifests import TargetManifest, load_manifest_generation
from harness.io import repo_layout, unique_object
from harness.toolchain.gcc import OLD_GCC_IDENTITY, OLD_GCC_SHA256
from harness.toolchain.gcc_variants import (
    CompilerVariant,
    parse_variants,
    select_variant,
)

_ENVIRONMENT = (
    "PSX_CC_DRIVER",
    "PSX_GCC",
    "PSX_AS",
    "PSX_MASPSX",
    "ASPSX_VERSION",
    "MASPSX_PYTHON",
    "PSX_PYTHON",
    "CPATH",
    "C_INCLUDE_PATH",
    "CPLUS_INCLUDE_PATH",
    "OBJC_INCLUDE_PATH",
    "DEPENDENCIES_OUTPUT",
    "SUNPRO_DEPENDENCIES",
)
_FLAG_VARIABLES = ("BOF3_CPPFLAGS", "BOF3_CFLAGS", "BOF3_CC_ASFLAGS")
_SUPPORTED_CMAKE_SHA256 = (
    "bbfb050b3c1d00cc98a37dec8e47e2b0f804ebdfa8ad5325e253275a2b6ab905"
)
_SUPPORTED_GRAPH_SHA256 = (
    "af34402a13617ad348bed381faeca2e776418494ee21bfcc80e16e5fa3af5130"
)
_SUPPORTED_CC_SHA256 = (
    "71b741ddeee18d0d35dc6901549b1f25513bfa515e7af7cf8c7721035a26cf4a"
)


def _validate_cmake_configuration(root: Path, text: str) -> None:
    if hashlib.sha256(text.encode()).hexdigest() != _SUPPORTED_CMAKE_SHA256:
        raise ValueError("unsupported CMake recipe; compiler-profile review required")
    program = re.sub(r"#[^\n]*", "", text)
    configured = []
    for name in _FLAG_VARIABLES:
        matches = re.findall(rf"(?m)^\s*set\(\s*{name}\s+([^)]*)\)", program)
        if len(matches) != 1:
            raise ValueError(f"profile requires one literal {name} assignment")
        for token in matches[0].split():
            literal = token.replace("${CMAKE_SOURCE_DIR}", "ROOT")
            if not re.fullmatch(r"-[A-Za-z0-9_=+.,:/-]+", literal):
                raise ValueError(f"unsupported {name} syntax")
            configured.append(token.replace("${CMAKE_SOURCE_DIR}", str(root)))
    expected = build_compiler_arguments(root)[1:]
    if configured != expected:
        raise ValueError(
            "CMake defaults differ from the Python compiler argument owner"
        )


def _observe_input(path: Path) -> tuple:
    location = path
    while True:
        check_deadline()
        try:
            state = location.stat(follow_symlinks=False)
        except (FileNotFoundError, NotADirectoryError):
            state = None
        if state is not None and (not stat.S_ISLNK(state.st_mode) or location.exists()):
            break
        parent = location.parent
        if parent == location:
            raise ValueError(f"profile input has no existing ancestor: {path}")
        location = parent
    if stat.S_ISDIR(state.st_mode):
        return (str(location), *observe_directory(location))
    return (
        str(location),
        state.st_dev,
        state.st_ino,
        state.st_mode,
        state.st_nlink,
        state.st_uid,
        state.st_gid,
        state.st_size,
        state.st_mtime_ns,
        state.st_ctime_ns,
    )


def _has_same_observation(expected: tuple, current: tuple) -> bool:
    if expected == current:
        return True
    return (
        len(expected) == len(current) == 11
        and stat.S_ISDIR(expected[3])
        and expected[:7] == current[:7]
        and expected[8:] == current[8:]
    )


def collect_profile_sources(root: Path, destination: str) -> list[str]:
    paths = {path.relative_to(root).as_posix() for path in (root / "src").rglob("*.c")}
    paths.add(destination)
    return sorted(paths)


def select_compiler(root: Path) -> Path:
    configured = os.environ.get("PSX_GCC")
    return Path(configured) if configured else repo_layout(root).gcc272_psx_root / "gcc"


class ProfileContext:
    """Capture configured inputs without invoking a compiler or installer."""

    def __init__(self, root: Path, *, compiler: Path | None = None) -> None:
        self.root = root.resolve()
        if any(character in str(self.root) for character in ";\\\r\n"):
            raise ValueError("unsupported repository path for compiler profiles")
        self.inputs: dict[str, dict | None] = {}
        self.content: dict[str, bytes] = {}
        self.observations: dict[str, tuple] = {}
        self.ancestors: dict[Path, tuple] = {}
        self.manifest_paths: tuple[str, ...] | None = None
        self._variants: list[CompilerVariant] | None = None
        active = [name for name in _ENVIRONMENT if os.environ.get(name)]
        if compiler is not None:
            if not compiler.is_absolute() or compiler.resolve() != compiler:
                raise ValueError(
                    "dispatched compiler must be a canonical absolute path"
                )
            if os.environ.get("PSX_GCC") == str(compiler):
                active = [name for name in active if name != "PSX_GCC"]
        if active:
            raise ValueError(
                "unsupported compiler environment overrides: " + ", ".join(active)
            )
        cmake = self.read("CMakeLists.txt").decode("utf-8")
        graph = self.read("config/compiler/graph.cmake")
        if hashlib.sha256(graph).hexdigest() != _SUPPORTED_GRAPH_SHA256:
            raise ValueError(
                "unsupported build graph recipe; compiler-profile review required"
            )
        overrides = self.read(
            "config/compiler/object-flags.cmake", required=False
        ).decode("utf-8")
        catalog = self.read("config/compiler/variants.json", required=False)
        if catalog:
            json.loads(catalog, object_pairs_hook=unique_object)
        validate_object_configuration(overrides)
        _validate_cmake_configuration(self.root, cmake)
        self.flags = parse_object_flags(overrides)
        self.compilers = parse_object_compilers(overrides)
        self.driver = self.read("bin/cc").decode("utf-8")
        for relative in (
            "bin/as",
            "bin/python-env",
            "tools/python/harness/build/compiler.py",
            "tools/python/harness/build/profiles.py",
            "tools/python/harness/build/preservation.py",
            "tools/python/harness/build/dispatch.py",
            "tools/python/harness/build/routing.py",
            "tools/python/harness/build/driver.py",
            "tools/python/harness/build/receipts.py",
            "tools/python/harness/build/inventory.py",
            "tools/python/harness/build/cli.py",
            "tools/python/harness/build/sections.py",
            "tools/python/harness/build/translation.py",
            "tools/python/harness/common/deadlines.py",
            "tools/python/harness/common/files.py",
            "tools/python/harness/common/inputs.py",
            "tools/python/harness/common/lexicon.py",
            "tools/python/harness/common/observation.py",
            "tools/python/harness/common/paths.py",
            "tools/python/harness/common/directory.py",
            "tools/python/harness/common/root.py",
            "tools/python/harness/domain/cache.py",
            "tools/python/harness/domain/claims.py",
            "tools/python/harness/domain/functions.py",
            "tools/python/harness/domain/layout.py",
            "tools/python/harness/domain/manifests.py",
            "tools/python/harness/domain/ids.py",
            "tools/python/harness/domain/psx.py",
            "tools/python/harness/domain/sources.py",
            "tools/python/harness/domain/symbols.py",
            "tools/python/harness/domain/tags.py",
            "tools/python/harness/io.py",
            "tools/python/harness/toolchain/gcc.py",
            "tools/python/harness/toolchain/gcc_variants.py",
        ):
            self.read(relative)
        self.layout = repo_layout(self.root)
        if hashlib.sha256(self.driver.encode()).hexdigest() != _SUPPORTED_CC_SHA256:
            raise ValueError("unsupported compiler bootstrap; dispatch review required")

    def read(
        self, relative: str, *, required: bool = True, batch: InputBatch | None = None
    ) -> bytes:
        check_deadline()
        if relative in self.content:
            if required and self.inputs[relative] is None:
                raise ValueError(f"missing profile input: {relative}")
            return self.content[relative]
        path = self.root / relative
        if not path.is_relative_to(self.root):
            raise ValueError("profile input must be a canonical repository path")
        if batch is None:
            if path.resolve() != path:
                raise ValueError("profile input must be a canonical repository path")
        else:
            batch.validate_path(path)
        for ancestor in path.parents:
            if ancestor not in self.ancestors:
                self.ancestors[ancestor] = _observe_input(ancestor)
        observation = _observe_input(path)
        state, data = read_input(path) if batch is None else batch.read(path)
        if state is None:
            if required:
                raise ValueError(f"missing profile input: {relative}")
            data = b""
        else:
            if path.stat().st_nlink != 1:
                raise ValueError(f"multiply linked profile input: {relative}")
        if not _has_same_observation(observation, _observe_input(path)):
            raise ValueError(f"profile input changed while reading: {relative}")
        self.inputs[relative] = state
        self.content[relative] = data
        self.observations[relative] = observation
        return data

    def read_manifests(self) -> dict[str, TargetManifest]:
        """Derive manifest models from this context's captured immutable bytes."""
        if self.manifest_paths is None:
            self.manifest_paths = tuple(collect_manifest_paths(self.root))
        self.verify_manifest_inventory()
        expected = set(self.manifest_paths)
        consumed: set[str] = set()
        claimed: set[str] = set()

        def read_manifest(name: str) -> bytes:
            if name not in expected or name in consumed:
                raise ValueError("manifest read set differs from captured inventory")
            consumed.add(name)
            return self.read(name, batch=batch)

        def read_claim(name: str) -> bytes | None:
            if name in claimed:
                raise ValueError("duplicate manifest claim read")
            claimed.add(name)
            content = self.read(name, required=False, batch=batch)
            return content if self.inputs[name] is not None else None

        with InputBatch(self.root) as batch:
            generation = load_manifest_generation(
                self.root, read_manifest=read_manifest, read_claim=read_claim
            )
        if consumed != expected:
            raise ValueError("manifest read set differs from captured inventory")
        if claimed != set(collect_claim_paths(generation.manifests)):
            raise ValueError("manifest claim read set differs from captured models")
        self.verify_manifest_inventory()
        return generation.manifests

    def verify_manifest_inventory(self) -> None:
        """Reject changed discovery without refreshing the original captured set."""
        if (
            self.manifest_paths is not None
            and tuple(collect_manifest_paths(self.root)) != self.manifest_paths
        ):
            raise ValueError("manifest inventory changed during profile capture")

    def resolve(self, source: str, *, text: str | None = None) -> dict:
        """Resolve the current configured compiler and ordered driver options."""
        if text is None:
            text = self.read(source, required=False).decode("utf-8")
        settings = resolve_compiler_settings(source, text, self.flags, self.compilers)
        compiler_id = settings.compiler_id
        if compiler_id is None:
            executable = self.layout.gcc272_psx_root / "gcc"
            identity = OLD_GCC_IDENTITY
            checksum = OLD_GCC_SHA256
            compiler_id = f"gcc-{identity}-psx"
        else:
            if self._variants is None:
                catalog_path = "config/compiler/variants.json"
                self._variants = (
                    parse_variants(self.content[catalog_path].decode("utf-8"))
                    if self.inputs[catalog_path] is not None
                    else []
                )
            variant = select_variant(self._variants, compiler_id)
            executable = variant.install_path(self.layout) / variant.executable_relpath
            identity = variant.identity
            checksum = variant.checksum
        executable_name = executable.relative_to(self.root).as_posix()
        self.read(executable_name)
        return {
            "compiler": {
                "id": compiler_id,
                "catalog_identity": identity,
                "archive_checksum": checksum,
                "executable": executable_name,
                "sha256": self.inputs[executable_name]["sha256"],
            },
            "arguments": build_compiler_arguments(self.root, settings.flags),
        }

    def verify(self) -> None:
        """Reject moving files or changed configuration instead of publishing stale pins."""
        check_deadline()
        self.verify_manifest_inventory()
        self._verify_ancestors()
        with InputBatch(self.root) as batch:
            for relative, state in self.inputs.items():
                check_deadline()
                path = self.root / relative
                observation = self.observations[relative]
                if not _has_same_observation(observation, _observe_input(path)):
                    raise ValueError(f"profile input changed: {relative}")
                if batch.read(path)[0] != state or (
                    state is not None and path.stat().st_nlink != 1
                ):
                    raise ValueError(f"profile input changed: {relative}")
                if not _has_same_observation(observation, _observe_input(path)):
                    raise ValueError(f"profile input changed: {relative}")
        self._verify_ancestors()
        self.verify_manifest_inventory()

    def _verify_ancestors(self) -> None:
        for ancestor, observation in self.ancestors.items():
            check_deadline()
            current = _observe_input(ancestor)
            if not _has_same_observation(observation, current):
                raise ValueError(
                    f"profile input ancestor changed during capture: {ancestor}; "
                    f"expected={observation!r}; observed={current!r}"
                )
