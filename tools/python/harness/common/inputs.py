"""Conservative native build input closure shared by type, macro and naming gates."""

from __future__ import annotations

import hashlib
import json
import os
import re
import stat
from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.io import unique_object

_INPUT_FIELDS = (
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
_DIRECTORY_FLAGS = os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC
_DIRECTORY_CACHE_SIZE = 256


class InputBoundaryError(ValueError):
    """A claimed input resolves outside the repository boundary."""

    def __init__(self, path: str):
        self.path = path
        super().__init__(f"claimed path escapes repository: {path}")


def _capture_metadata(status: os.stat_result) -> tuple[int, ...]:
    return tuple(getattr(status, field) for field in _INPUT_FIELDS)


def _capture_parent_metadata(status: os.stat_result) -> tuple[int, ...]:
    return tuple(
        getattr(status, field) for field in _INPUT_FIELDS if field != "st_size"
    )


def load(path: Path):
    return json.loads(path.read_bytes(), object_pairs_hook=unique_object)


def read_input(path: Path) -> tuple[dict | None, bytes | None]:
    """Bind input state to the exact bytes read between stable metadata samples."""
    check_deadline()
    before = None
    for part in (path, *path.parents):
        check_deadline()
        try:
            observed = part.stat(follow_symlinks=False)
        except (FileNotFoundError, NotADirectoryError):
            continue
        if stat.S_ISLNK(observed.st_mode):
            raise ValueError(f"symlink input: {path}")
        if part == path:
            before = observed
    if before is None:
        try:
            path.stat(follow_symlinks=False)
        except (FileNotFoundError, NotADirectoryError):
            check_deadline()
            return None, None
        raise ValueError(f"input appeared during observation: {path}")
    mode = before.st_mode
    if not stat.S_ISREG(mode):
        raise ValueError(f"not a regular input: {path}")
    content = path.read_bytes()
    check_deadline()
    after = path.stat(follow_symlinks=False)
    # Reading may update atime; identity and mutation metadata must stay stable.
    if _capture_metadata(before) != _capture_metadata(after):
        raise ValueError(f"moving input: {path}")
    return {
        "sha256": hashlib.sha256(content).hexdigest(),
        "mode": stat.S_IMODE(mode),
    }, content


class InputBatch:
    """Reuse confined directory traversal within one verified input-reading pass."""

    def __init__(self, root: Path):
        self.root = Path(root)
        if self.root.anchor != "/" or Path(os.path.abspath(root)) != self.root:
            raise ValueError("input batch root must be canonical and absolute")
        self._directories: dict[Path, int] = {}
        self._metadata: dict[Path, tuple[int, ...]] = {}
        self._chains: dict[Path, tuple[tuple[Path, Path], ...]] = {}
        self._missing: set[Path] = set()
        self._active = False
        self._closed = False
        self._failed = False

    def __enter__(self) -> InputBatch:
        if self._active or self._closed:
            raise ValueError("input batch cannot be reused")
        self._active = True
        try:
            if self._open_directory(self.root) is None:
                raise ValueError("input batch root is missing")
        except BaseException:
            self._failed = True
            self._close()
            raise
        return self

    def __exit__(self, error_type, error, traceback) -> None:
        try:
            if error_type is None:
                self._verify()
        finally:
            self._close()

    def _require_active(self) -> None:
        check_deadline()
        if not self._active or self._closed or self._failed:
            raise ValueError("input batch is inactive or failed")

    def _release_directory(self, protected: Path) -> None:
        if len(self._directories) < _DIRECTORY_CACHE_SIZE:
            return
        parents = {path.parent for path in self._directories}
        for path in self._directories:
            if path not in parents and not protected.is_relative_to(path):
                descriptor = self._directories.pop(path)
                os.close(descriptor)
                return

    def _open_directory(self, path: Path) -> int | None:
        self._require_active()
        if path in self._directories:
            descriptor = self._directories.pop(path)
            self._directories[path] = descriptor
            return descriptor
        parent = None
        if path != path.parent:
            parent = self._open_directory(path.parent)
            if parent is None:
                return None
        name = str(path) if parent is None else path.name
        try:
            before = os.stat(name, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            if path in self._metadata:
                raise ValueError(
                    f"previously observed input parent disappeared: {path}"
                )
            try:
                os.stat(name, dir_fd=parent, follow_symlinks=False)
            except FileNotFoundError:
                return None
            raise ValueError(f"input parent appeared during observation: {path}")
        if stat.S_ISLNK(before.st_mode):
            raise ValueError(f"symlink input parent: {path}")
        if not stat.S_ISDIR(before.st_mode):
            if path in self._metadata:
                raise ValueError(f"previously observed input parent changed: {path}")
            return None
        self._release_directory(path)
        descriptor = os.open(name, _DIRECTORY_FLAGS, dir_fd=parent)
        try:
            expected = _capture_parent_metadata(before)
            if (
                _capture_parent_metadata(os.fstat(descriptor)) != expected
                or _capture_parent_metadata(
                    os.stat(name, dir_fd=parent, follow_symlinks=False)
                )
                != expected
                or self._metadata.get(path, expected) != expected
            ):
                raise ValueError(f"input parent changed during observation: {path}")
            self._metadata[path] = expected
            self._directories[path] = descriptor
        except BaseException:
            os.close(descriptor)
            raise
        return descriptor

    def _verify_chain(self, path: Path) -> None:
        chain = self._chains.get(path)
        if chain is None:
            chain = tuple(
                (ancestor, ancestor.parent)
                for ancestor in reversed((path, *path.parents))
            )
            self._chains[path] = chain
        for ancestor, parent in chain:
            check_deadline()
            linked = (
                os.stat(ancestor, follow_symlinks=False)
                if ancestor == parent
                else os.stat(
                    ancestor.name,
                    dir_fd=self._directories[parent],
                    follow_symlinks=False,
                )
            )
            if _capture_parent_metadata(linked) != self._metadata[ancestor]:
                raise ValueError(f"input parent detached or changed: {ancestor}")
        if (
            _capture_parent_metadata(os.fstat(self._directories[path]))
            != self._metadata[path]
        ):
            raise ValueError(f"input parent descriptor changed: {path}")

    def _observe_missing(self, parent: int, path: Path) -> None:
        try:
            os.stat(path.name, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            check_deadline()
            self._missing.add(path)
            return
        raise ValueError(f"input appeared during observation: {path}")

    def _prepare_parent(self, path: Path) -> int | None:
        self._require_active()
        if (
            not path.is_absolute()
            or not path.is_relative_to(self.root)
            or Path(os.path.abspath(path)) != path
        ):
            raise ValueError("input batch path must be canonical and confined")
        parent = self._open_directory(path.parent)
        if parent is not None:
            self._verify_chain(path.parent)
        return parent

    def validate_path(self, path: Path) -> None:
        """Reject noncanonical components before a caller observes the input path."""
        try:
            parent = self._prepare_parent(path)
            if parent is not None:
                try:
                    status = os.stat(path.name, dir_fd=parent, follow_symlinks=False)
                except FileNotFoundError:
                    check_deadline()
                    return
                if stat.S_ISLNK(status.st_mode):
                    raise ValueError(f"symlink input: {path}")
            check_deadline()
        except BaseException:
            self._failed = True
            raise

    def _read(
        self,
        path: Path,
        *,
        max_bytes: int | None = None,
        include_metadata: bool = False,
    ) -> tuple[dict | None, bytes | None]:
        if max_bytes is not None and (type(max_bytes) is not int or max_bytes < 0):
            raise ValueError("invalid input batch byte limit")
        if type(include_metadata) is not bool:
            raise ValueError("invalid input batch metadata selection")
        parent = self._prepare_parent(path)
        if parent is None:
            self._missing.add(path)
            check_deadline()
            return None, None
        try:
            before = os.stat(path.name, dir_fd=parent, follow_symlinks=False)
        except FileNotFoundError:
            self._observe_missing(parent, path)
            return None, None
        if not stat.S_ISREG(before.st_mode):
            raise ValueError(f"not a regular input: {path}")
        if max_bytes is not None and before.st_size > max_bytes:
            raise ValueError(f"input exceeds batch byte limit: {path}")
        descriptor = os.open(
            path.name,
            os.O_RDONLY | os.O_NOFOLLOW | os.O_CLOEXEC | os.O_NONBLOCK,
            dir_fd=parent,
        )
        try:
            expected = _capture_metadata(before)
            if _capture_metadata(os.fstat(descriptor)) != expected:
                raise ValueError(f"moving input: {path}")
            with os.fdopen(descriptor, "rb", closefd=False) as stream:
                content = (
                    stream.read() if max_bytes is None else stream.read(max_bytes + 1)
                )
            if max_bytes is not None and len(content) > max_bytes:
                raise ValueError(f"input exceeds batch byte limit: {path}")
            check_deadline()
            if (
                _capture_metadata(os.fstat(descriptor)) != expected
                or _capture_metadata(
                    os.stat(path.name, dir_fd=parent, follow_symlinks=False)
                )
                != expected
            ):
                raise ValueError(f"moving input: {path}")
            state = {
                "sha256": hashlib.sha256(content).hexdigest(),
                "mode": stat.S_IMODE(before.st_mode),
            }
            if include_metadata:
                state["metadata"] = dict(zip(_INPUT_FIELDS, expected, strict=True))
            return state, content
        finally:
            os.close(descriptor)

    def read(
        self,
        path: Path,
        *,
        max_bytes: int | None = None,
        include_metadata: bool = False,
    ) -> tuple[dict | None, bytes | None]:
        """Read fresh bytes; never cache content, absence or validation success."""
        try:
            return self._read(
                path, max_bytes=max_bytes, include_metadata=include_metadata
            )
        except BaseException:
            self._failed = True
            raise

    def _verify(self) -> None:
        self._require_active()
        for path in self._missing:
            parent = self._open_directory(path.parent)
            if parent is not None:
                self._observe_missing(parent, path)
        for path, expected in self._metadata.items():
            check_deadline()
            descriptor = self._open_directory(path)
            if descriptor is None:
                raise ValueError(f"input parent disappeared: {path}")
            linked = (
                os.stat(path, follow_symlinks=False)
                if path == path.parent
                else os.stat(
                    path.name,
                    dir_fd=self._directories[path.parent],
                    follow_symlinks=False,
                )
            )
            if (
                _capture_parent_metadata(linked) != expected
                or _capture_parent_metadata(os.fstat(descriptor)) != expected
            ):
                raise ValueError(f"input parent detached or changed: {path}")
        check_deadline()

    def _close(self) -> None:
        descriptors = tuple(self._directories.values())
        self._directories.clear()
        self._chains.clear()
        self._active = False
        self._closed = True
        failure = None
        for descriptor in reversed(descriptors):
            try:
                os.close(descriptor)
            except OSError as error:
                failure = error
        if failure is not None:
            raise failure


def file_state(path: Path):
    return read_input(path)[0]


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
