"""Read-only cleanup selection and allocated-storage accounting."""

from __future__ import annotations

import os
import re
import stat
import subprocess
import time
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path

DAY = 86400
DUMP_SUFFIXES = {
    "combine",
    "cse",
    "cse2",
    "dbr",
    "flow",
    "greg",
    "jump",
    "jump2",
    "loop",
    "lreg",
    "rtl",
    "sched",
    "sched2",
}
EVIDENCE_ROOTS = (
    "out/source-refinement",
    "out/matching",
    "out/asm-diff",
    "out/lift-batches",
)
HISTORY_ROOTS = (
    ".pi/sessions",
    ".pi-subagents/sessions",
    ".pi-subagents/chain-runs",
    ".pi-subagents/artifacts/outputs",
    ".pi-subagents/artifacts/progress",
    ".pi-subagents/artifacts/structured-output",
    ".pi-subagents/artifacts/handoffs",
)
HASHED_BINARY = re.compile(r"(.+)-[0-9a-f]{16}")


@dataclass(frozen=True)
class Options:
    worktrees: bool = False
    session_history: bool = False
    evidence: bool = False


@dataclass(frozen=True)
class Activity:
    paths: tuple[Path, ...] = ()
    rust_busy: bool = False
    unreadable_processes: tuple[int, ...] = ()


@dataclass(frozen=True)
class Entry:
    path: str
    device: int
    inode: int
    kind: int
    size: int
    modified_ns: int
    links: int
    blocks: int


@dataclass(frozen=True)
class Operation:
    path: str
    category: str
    reason: str
    entries: tuple[Entry, ...]
    worktree_head: str = ""


@dataclass(frozen=True)
class Skip:
    path: str
    reason: str


@dataclass(frozen=True)
class Plan:
    root: Path
    options: Options
    head: str
    operations: tuple[Operation, ...]
    skipped: tuple[Skip, ...]
    cargo_roots: tuple[Path, ...]
    unreadable_processes: tuple[int, ...] = ()

    @property
    def estimated_bytes(self) -> int:
        return estimate_bytes(self.operations)


def git(root: Path, *args: str, check: bool = True) -> subprocess.CompletedProcess:
    result = subprocess.run(
        ["git", "-C", str(root), *args],
        capture_output=True,
        text=True,
        errors="surrogateescape",
        env={**os.environ, "GIT_OPTIONAL_LOCKS": "0"},
    )
    if check and result.returncode:
        raise ValueError(f"Git {' '.join(args[:2])}: {result.stderr.strip()}")
    return result


def tracked_paths(root: Path) -> frozenset[str]:
    indexed = git(root, "ls-files", "-z").stdout
    committed = git(root, "ls-tree", "-rz", "--name-only", "HEAD").stdout
    return frozenset((indexed + committed).split("\0")) - {""}


def read_activity(root: Path) -> Activity:
    paths: set[Path] = set()
    rust_busy = False
    unreadable = []
    for proc in Path("/proc").iterdir():
        if not proc.name.isdigit():
            continue
        try:
            if proc.stat().st_uid != os.geteuid():
                continue
            comm = (proc / "comm").read_text().strip()
            cwd = Path(os.readlink(proc / "cwd"))
            rust_busy |= cwd.is_relative_to(root) and comm in {
                "cargo",
                "rustc",
                "rustdoc",
            }
            links = [proc / "cwd", proc / "exe", *(proc / "fd").iterdir()]
            for link in links:
                try:
                    name = os.readlink(link)
                except FileNotFoundError:
                    continue
                if name.startswith("/"):
                    path = Path(name.removesuffix(" (deleted)"))
                    if path.is_relative_to(root):
                        paths.add(path)
        except (FileNotFoundError, ProcessLookupError):
            continue
        except PermissionError:
            unreadable.append(int(proc.name))
    return Activity(tuple(sorted(paths)), rust_busy, tuple(sorted(unreadable)))


def has_recovery_material(path: str) -> bool:
    return any(
        part.lower() in {"pre", "post", "restore"}
        or "recovery" in part.lower()
        or "rollback" in part.lower()
        for part in Path(path).parts
    )


def snapshot(root: Path, name: str, *, worktree: bool = False) -> tuple[Entry, ...]:
    path = root / name
    if path.resolve() != path or not path.is_relative_to(root) or path == root:
        raise ValueError("symlink or unconfined path")
    device = root.stat().st_dev
    pending = [path]
    entries = []
    while pending:
        current = pending.pop()
        info = current.lstat()
        relative = current.relative_to(root).as_posix()
        if info.st_dev != device:
            raise ValueError("nested filesystem")
        marker = current.relative_to(path).parts
        if ".git" in marker and not (
            worktree and marker == (".git",) and stat.S_ISREG(info.st_mode)
        ):
            raise ValueError("nested Git repository")
        if not (
            stat.S_ISREG(info.st_mode)
            or stat.S_ISDIR(info.st_mode)
            or stat.S_ISLNK(info.st_mode)
        ):
            raise ValueError("special file")
        entries.append(
            Entry(
                relative,
                info.st_dev,
                info.st_ino,
                stat.S_IFMT(info.st_mode),
                info.st_size,
                info.st_mtime_ns,
                info.st_nlink,
                info.st_blocks,
            )
        )
        if stat.S_ISDIR(info.st_mode):
            pending.extend(sorted(current.iterdir(), reverse=True))
    return tuple(sorted(entries, key=lambda entry: entry.path))


def estimate_bytes(operations: tuple[Operation, ...] | list[Operation]) -> int:
    inodes = {}
    references = Counter()
    for operation in operations:
        for entry in operation.entries:
            key = (entry.device, entry.inode)
            inodes[key] = entry
            references[key] += 1
    return sum(
        entry.blocks * 512
        for key, entry in inodes.items()
        if entry.kind == stat.S_IFDIR or references[key] == entry.links
    )


def registered_worktrees(root: Path) -> list[dict[str, str]]:
    records = []
    for block in git(root, "worktree", "list", "--porcelain", "-z").stdout.split(
        "\0\0"
    ):
        record = {}
        for row in block.split("\0"):
            if row:
                key, _, value = row.partition(" ")
                record[key] = value
        if record:
            records.append(record)
    return records


def check_worktree(root: Path, record: dict[str, str], activity: Activity) -> None:
    records = registered_worktrees(root)
    primary = records[0]
    path = Path(record["worktree"])
    current = next((row for row in records if row["worktree"] == str(path)), None)
    if path in {root, Path(primary["worktree"])}:
        raise ValueError("primary or invoking checkout")
    if not path.is_relative_to(root) or path.resolve() != path:
        raise ValueError("external or symlinked worktree")
    if current is None or current.get("HEAD") != record.get("HEAD"):
        raise ValueError("worktree registration changed")
    if "locked" in current or "prunable" in current:
        raise ValueError("locked or unavailable worktree")
    if any(used.is_relative_to(path) for used in activity.paths):
        raise ValueError("active worktree")
    if git(path, "status", "--porcelain=v1", "--untracked-files=all").stdout:
        raise ValueError("dirty worktree")
    if git(
        root,
        "merge-base",
        "--is-ancestor",
        record["HEAD"],
        primary["HEAD"],
        check=False,
    ).returncode:
        raise ValueError("unmerged worktree")


def cargo_roots(root: Path) -> tuple[Path, ...]:
    paths = [*root.glob("tools/rust/*/target"), *root.glob("build/tools/rust/*")]
    return tuple(sorted(p for p in paths if p.is_dir() and p.resolve() == p))


def assess(
    root: Path, options: Options = Options(), *, now: float | None = None
) -> Plan:
    if root.is_symlink():
        raise ValueError("Cleanup root must not be a symlink")
    root = root.absolute()
    if git(root, "rev-parse", "--show-toplevel").stdout.strip() != str(root):
        raise ValueError("Cleanup requires a Git checkout root")
    now = time.time() if now is None else now
    tracked = tracked_paths(root)
    activity = read_activity(root)
    operations: list[Operation] = []
    skipped: list[Skip] = []
    roots = cargo_roots(root)

    def add(
        path: Path, category: str, reason: str, age_days: int = 1, head: str = ""
    ) -> None:
        name = path.relative_to(root).as_posix()
        try:
            if any(p == name or p.startswith(name + "/") for p in tracked):
                raise ValueError("indexed or committed path")
            if (
                category not in {"dump", "worktree"}
                and git(root, "check-ignore", "-q", "--", name, check=False).returncode
            ):
                raise ValueError("not an ignored generated path")
            entries = snapshot(root, name, worktree=category == "worktree")
            if any(has_recovery_material(entry.path) for entry in entries):
                raise ValueError("retained recovery material")
            if any(
                used == path or used.is_relative_to(path) for used in activity.paths
            ):
                raise ValueError("active path")
            if category in {"cache", "binary"} and activity.rust_busy:
                raise ValueError("active Rust build")
            if (
                age_days
                and max(entry.modified_ns for entry in entries) / 1e9
                > now - age_days * DAY
            ):
                raise ValueError("recently modified")
            operations.append(Operation(name, category, reason, entries, head))
        except (ValueError, OSError) as error:
            skipped.append(Skip(name, str(error)))

    for base in roots:
        for profile in ("debug", "release"):
            incremental = base / profile / "incremental"
            if incremental.exists() or incremental.is_symlink():
                add(incremental, "cache", "stale Rust incremental cache")
        for folder in ("deps", "examples"):
            directory = base / "debug" / folder
            if not directory.is_dir() or directory.resolve() != directory:
                continue
            groups = defaultdict(list)
            for path in sorted(directory.iterdir()):
                match = HASHED_BINARY.fullmatch(path.name)
                info = path.lstat()
                if match and stat.S_ISREG(info.st_mode) and info.st_mode & 0o111:
                    with path.open("rb") as handle:
                        magic = handle.read(4)
                    if magic == b"\x7fELF":
                        groups[match[1]].append((info.st_mtime_ns, path, info.st_nlink))
            for family in groups.values():
                family.sort(reverse=True)
                skipped.append(
                    Skip(
                        family[0][1].relative_to(root).as_posix(),
                        "newest executable in family",
                    )
                )
                for _, path, links in family[1:]:
                    if links != 1:
                        skipped.append(
                            Skip(
                                path.relative_to(root).as_posix(),
                                "retained executable hardlinks",
                            )
                        )
                    else:
                        add(path, "binary", "superseded hashed executable", 7)
    for path in sorted(root.glob("*.c.*")):
        if (
            path.suffix.removeprefix(".") not in DUMP_SUFFIXES
            or not path.is_file()
            or path.is_symlink()
        ):
            continue
        with path.open("rb") as handle:
            prefix = handle.read(8192)
        if prefix.lstrip().startswith(b";;") and (
            b"(insn" in prefix or b"(note" in prefix
        ):
            add(path, "dump", "generated compiler pass dump")
    if (root / "ledger").exists() or (root / "ledger").is_symlink():
        add(root / "ledger", "ledger", "retired ledger", 0)
    for enabled, bases, category in (
        (options.evidence, EVIDENCE_ROOTS, "evidence"),
        (options.session_history, HISTORY_ROOTS, "session-history"),
    ):
        if not enabled:
            continue
        for relative in bases:
            base = root / relative
            if base.is_dir() and base.resolve() == base:
                for path in sorted(base.iterdir()):
                    add(path, category, "inactive historical " + category)
    if options.worktrees:
        for record in registered_worktrees(root):
            path = Path(record["worktree"])
            try:
                check_worktree(root, record, activity)
                add(
                    path,
                    "worktree",
                    "clean merged linked worktree",
                    head=record["HEAD"],
                )
            except (ValueError, OSError) as error:
                skipped.append(Skip(str(path), str(error)))
    unique = []
    for operation in sorted(
        operations, key=lambda op: (len(Path(op.path).parts), op.path)
    ):
        if not any(
            Path(operation.path).is_relative_to(parent.path) for parent in unique
        ):
            unique.append(operation)
    return Plan(
        root,
        options,
        git(root, "rev-parse", "HEAD").stdout.strip(),
        tuple(unique),
        tuple(sorted(skipped, key=lambda skip: (skip.path, skip.reason))),
        roots,
        activity.unreadable_processes,
    )
