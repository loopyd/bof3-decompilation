"""Validate hash-bound plan coverage and recoverable publication transactions."""

from __future__ import annotations

import hashlib
import json
import os
import re
import stat
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

STATES = ("open", "in-progress", "blocked", "done")
NAME = re.compile(r"[a-z0-9]+(?:-[a-z0-9]+)*\.md\Z")
ITEM = re.compile(r"(## |)([1-9][0-9]*)\. \[([A-Z][A-Z0-9.-]*)\] \(([^)]+)\) (.+)")
FIELDS = ("Owner", "Depends", "Blocker", "Evidence", "Acceptance")


@dataclass
class Item:
    id: str
    state: str
    title: str
    phase: bool
    parent: str | None
    fields: dict[str, str] = field(default_factory=dict)


@dataclass
class Plan:
    items: list[Item]
    anchors: set[str]


def parse_plan(text: str) -> Plan:
    """Read only bof3.plan/v1, excluding fenced examples from task data."""
    if not text.startswith("<!-- bof3.plan/v1 -->\n"):
        raise ValueError("missing bof3.plan/v1 header")
    items: list[Item] = []
    anchors: set[str] = set()
    fence: tuple[str, int] | None = None
    parent = None
    current = None
    for line in text.splitlines()[1:]:
        marker = re.match(r"^ {0,3}(`{3,}|~{3,})(.*)$", line)
        if marker:
            run, suffix = marker.groups()
            if fence is None:
                fence = (run[0], len(run))
            elif run[0] == fence[0] and len(run) >= fence[1] and not suffix.strip():
                fence = None
            continue
        if fence:
            continue
        anchor = re.fullmatch(r'<a id="([a-zA-Z0-9.-]+)"></a>', line)
        if anchor:
            if anchor[1] in anchors:
                raise ValueError(f"duplicate anchor: {anchor[1]}")
            anchors.add(anchor[1])
        match = ITEM.fullmatch(line)
        if match:
            heading, _, id_, state, title = match.groups()
            if state not in STATES:
                raise ValueError(f"invalid state for {id_}: {state}")
            phase = bool(heading)
            if not phase and parent is None:
                raise ValueError("step before phase")
            current = Item(id_, state, title, phase, None if phase else parent)
            items.append(current)
            if phase:
                parent = id_
            continue
        if re.match(r"^(?:## )?[0-9]+\. \[", line):
            raise ValueError(f"malformed task: {line}")
        detail = re.fullmatch(
            r"- (Owner|Depends|Blocker|Evidence|Acceptance): (.+)", line
        )
        if detail:
            if current is None or detail[1] in current.fields:
                raise ValueError("orphan or duplicate task field")
            current.fields[detail[1]] = detail[2].strip()
        elif re.match(r"^- (?:Owner|Depends|Blocker|Evidence|Acceptance):", line):
            raise ValueError("empty or malformed task field")
    if fence or not items:
        raise ValueError("unclosed fence or empty plan")
    by_id = {item.id: item for item in items}
    if len(by_id) != len(items):
        raise ValueError("duplicate task ID")
    dependencies: dict[str, list[str]] = {}
    for item in items:
        if set(item.fields) != set(FIELDS) or not all(item.fields.values()):
            raise ValueError(f"missing fields: {item.id}")
        deps = item.fields["Depends"]
        dependencies[item.id] = (
            []
            if deps == "none"
            else [dep.removesuffix("@unit") for dep in deps.split(", ")]
        )
        if item.parent:
            inherited = by_id[item.parent].fields["Depends"]
            dependencies[item.id] += (
                []
                if inherited == "none"
                else [dep.removesuffix("@unit") for dep in inherited.split(", ")]
            )
        if any(dep not in by_id for dep in dependencies[item.id]):
            raise ValueError(f"unknown dependency: {item.id}")
        if item.state == "blocked" and item.fields["Blocker"] == "none":
            raise ValueError(f"blocked task without blocker: {item.id}")
        if item.state == "done":
            if item.fields["Evidence"] == "none" or item.fields["Blocker"] != "none":
                raise ValueError(f"done task without acceptance evidence: {item.id}")
            if any(
                child.parent == item.id and child.state != "done" for child in items
            ):
                raise ValueError(f"done parent with incomplete children: {item.id}")
    visiting: set[str] = set()
    visited: set[str] = set()

    def visit(id_: str) -> None:
        if id_ in visiting:
            raise ValueError(f"dependency cycle: {id_}")
        if id_ in visited:
            return
        visiting.add(id_)
        for dep in dependencies[id_]:
            visit(dep)
        visiting.remove(id_)
        visited.add(id_)

    for id_ in by_id:
        visit(id_)
    return Plan(items, anchors)


def plan_name(value: str) -> str:
    if not isinstance(value, str) or not NAME.fullmatch(value):
        raise ValueError("plan must be an exact direct-child kebab-case .md filename")
    return value


def check_path(path: Path, *, missing: bool = False) -> Path:
    """Reject symlink ancestors before callers resolve a path."""
    if ".." in path.parts:
        raise ValueError(f"path traversal: {path}")
    path = path.absolute()
    for component in (*reversed(path.parents), path):
        if component.is_symlink():
            raise ValueError(f"symlink path: {component}")
        if not component.exists():
            if missing:
                continue
            raise ValueError(f"missing path: {component}")
        if component != path and not component.is_dir():
            raise ValueError(f"non-directory ancestor: {component}")
    return path


def read_regular(path: Path) -> bytes:
    check_path(path)
    if not path.is_file() or path.stat().st_nlink != 1:
        raise ValueError(f"not a single-link regular file: {path}")
    return path.read_bytes()


def inventory(root: Path) -> dict[str, bytes]:
    directory = check_path(root / "docs" / "plans", missing=True)
    if not directory.exists():
        return {}
    if not directory.is_dir():
        raise ValueError("plans is not a directory")
    return {plan_name(p.name): read_regular(p) for p in sorted(directory.iterdir())}


def digest(raw: bytes) -> str:
    return hashlib.sha256(raw).hexdigest()


def closed(value, fields: str) -> None:
    if not isinstance(value, dict) or set(value) != set(fields.split()):
        raise ValueError(f"expected object fields: {fields}")


def nonempty(value) -> None:
    if not isinstance(value, str) or not value.strip():
        raise ValueError("expected nonempty string")


def unique(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def validate(root: Path, review: Path):
    raw = read_regular(review)
    data = json.loads(raw, object_pairs_hook=unique)
    closed(
        data,
        "schema canonical candidate reviewer evidence sources mappings reconciliations",
    )
    if data["schema"] != "bof3.plan-review/v1":
        raise ValueError("unsupported review schema")
    canonical = plan_name(data["canonical"])
    for key in ("reviewer", "evidence"):
        nonempty(data[key])
    closed(data["candidate"], "path sha256")
    candidate = data["candidate"]
    nonempty(candidate["path"])
    path = Path(candidate["path"])
    if not path.is_absolute() or path.is_relative_to(root):
        raise ValueError("candidate must be outside repository, absolute")
    nonempty(candidate["sha256"])
    if not re.fullmatch(r"[0-9a-f]{64}", candidate["sha256"]):
        raise ValueError("malformed candidate hash")
    content = read_regular(path)
    if digest(content) != candidate["sha256"]:
        raise ValueError("candidate hash mismatch")
    plan = parse_plan(content.decode("utf-8"))
    destinations = plan.anchors | {item.id for item in plan.items}
    sources = data["sources"]
    if not isinstance(sources, list) or not sources:
        raise ValueError("empty source inventory")
    expected = {}
    for source in sources:
        closed(source, "name sha256")
        name = plan_name(source["name"])
        if (
            name in expected
            or not isinstance(source["sha256"], str)
            or not re.fullmatch(r"[0-9a-f]{64}", source["sha256"])
        ):
            raise ValueError("duplicate source or malformed hash")
        expected[name] = source["sha256"]
    if canonical not in expected:
        raise ValueError("canonical must be a reviewed source")
    live = inventory(root)
    replay = set(live) == {canonical} and digest(live[canonical]) == digest(content)
    if not replay and {name: digest(value) for name, value in live.items()} != expected:
        raise ValueError("stale/partial source inventory")
    mappings = data["mappings"]
    if not isinstance(mappings, list) or not mappings:
        raise ValueError("empty mappings")
    covered: dict[str, list[tuple[int, int]]] = {name: [] for name in expected}
    source_lines = {
        name: raw.decode("utf-8").splitlines() for name, raw in live.items()
    }
    for mapping in mappings:
        closed(mapping, "source start end label destination disposition evidence")
        name = mapping["source"]
        start, end = mapping["start"], mapping["end"]
        if (
            not isinstance(name, str)
            or name not in expected
            or type(start) is not int
            or type(end) is not int
            or start < 1
            or end < start
        ):
            raise ValueError("invalid coverage range")
        for key in ("label", "destination", "evidence"):
            nonempty(mapping[key])
        if mapping["destination"] not in destinations or mapping["disposition"] not in (
            "retained",
            "completed-with-evidence",
        ):
            raise ValueError("invalid mapping destination/disposition")
        if not replay and end > len(source_lines[name]):
            raise ValueError(f"coverage gap/out-of-range: {name}")
        if any(start <= last and first <= end for first, last in covered[name]):
            raise ValueError("overlapping coverage")
        covered[name].append((start, end))
    reconciliations = data["reconciliations"]
    if not isinstance(reconciliations, list):
        raise ValueError("reconciliations must be a list")
    reconciled = {}
    for entry in reconciliations:
        closed(entry, "source id destination evidence")
        for value in entry.values():
            nonempty(value)
        key = (entry["source"], entry["id"])
        if (
            key in reconciled
            or entry["source"] not in expected
            or entry["destination"] not in destinations
        ):
            raise ValueError("invalid reconciliation")
        reconciled[key] = entry
    if not replay:
        for name, source in live.items():
            lines = source_lines[name]
            if any(
                line.strip()
                and not any(first <= i <= last for first, last in covered[name])
                for i, line in enumerate(lines, 1)
            ):
                raise ValueError(f"coverage gap/out-of-range: {name}")
            if source.startswith(b"<!-- bof3.plan/v1 -->\n"):
                new = {item.id: item for item in plan.items}
                for item in parse_plan(source.decode("utf-8")).items:
                    if item.state != "done" and (
                        item.id not in new or new[item.id].state == "done"
                    ):
                        if (name, item.id) not in reconciled:
                            raise ValueError(f"unreconciled incomplete ID: {item.id}")
    return data, raw, content, live, replay


def sync_directory(path: Path) -> None:
    fd = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


def write_copy(path: Path, raw: bytes, mode: int) -> None:
    with path.open("xb") as stream:
        stream.write(raw)
        stream.flush()
        os.fchmod(stream.fileno(), mode)
        os.fsync(stream.fileno())
    if read_regular(path) != raw:
        raise ValueError(f"copy verification failed: {path}")


def publish(path: Path, raw: bytes, mode: int) -> None:
    fd, temporary = tempfile.mkstemp(prefix=".plans-", dir=path.parent)
    temp = Path(temporary)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(raw)
            stream.flush()
            os.fchmod(stream.fileno(), mode)
            os.fsync(stream.fileno())
        os.replace(temp, path)
        sync_directory(path.parent)
        if read_regular(path) != raw:
            raise ValueError("installed candidate verification failed")
    finally:
        if temp.exists():
            temp.unlink()


def consolidate(root: Path, review: Path, apply: bool, backup: Path | None) -> int:
    # ponytail: one trusted writer, recoverable updates, not hostile-swap safety;
    # parallel writers require a separately reviewed locking/ownership protocol.
    data, review_raw, candidate, sources, replay = validate(root, review)
    canonical = data["canonical"]
    if backup is not None and not apply:
        raise ValueError("--backup-dir requires --apply")
    if apply and backup is None:
        raise ValueError("--apply requires a fresh external --backup-dir")
    if backup is not None:
        if not backup.is_absolute():
            raise ValueError("backup directory must be absolute")
        backup = check_path(backup, missing=True)
        if backup.is_relative_to(root) or root.is_relative_to(backup):
            raise ValueError("backup must be outside repository")
        if backup.exists():
            raise ValueError("backup directory already exists")
        check_path(backup.parent)
    if replay:
        print(
            f"Already consolidated: {canonical} {digest(candidate)} (read-only no-op)"
        )
        return 0
    print(f"replace {canonical} {digest(candidate)}")
    for name, raw in sources.items():
        print(f"{'source' if name == canonical else 'delete'} {name} {digest(raw)}")
    if not apply:
        print("Preview only; independent semantic review required before --apply.")
        return 0
    assert backup is not None
    directory = root / "docs" / "plans"
    modes = {name: stat.S_IMODE((directory / name).stat().st_mode) for name in sources}
    changed: dict[str, bytes | None] = {}
    try:
        backup.mkdir(mode=0o700)
        (backup / "sources").mkdir(mode=0o700)
        for name, raw in sources.items():
            write_copy(backup / "sources" / name, raw, modes[name])
        write_copy(backup / "candidate.md", candidate, 0o600)
        write_copy(backup / "review.json", review_raw, 0o600)
        manifest = {
            name: {"sha256": digest(raw), "mode": modes[name]}
            for name, raw in sources.items()
        }
        write_copy(
            backup / "manifest.json",
            (json.dumps(manifest, indent=2) + "\n").encode(),
            0o600,
        )
        sync_directory(backup / "sources")
        sync_directory(backup)
        sync_directory(backup.parent)
        if (
            inventory(root) != sources
            or read_regular(review) != review_raw
            or read_regular(Path(data["candidate"]["path"])) != candidate
        ):
            raise ValueError("stale input immediately before publication")
        # Record intent before replace: even a post-replace fsync failure is recoverable.
        changed[canonical] = candidate
        publish(directory / canonical, candidate, modes[canonical])
        for name, raw in sources.items():
            if name == canonical:
                continue
            if read_regular(directory / name) != raw:
                raise ValueError(f"concurrent source edit: {name}")
            changed[name] = None
            (directory / name).unlink()
            sync_directory(directory)
        if inventory(root) != {canonical: candidate}:
            raise ValueError("unexpected final inventory")
    except (OSError, ValueError) as exc:
        failures = []
        for name, expected in reversed(list(changed.items())):
            path = directory / name
            try:
                current = (
                    read_regular(path) if path.exists() or path.is_symlink() else None
                )
                if current == sources[name]:
                    continue
                if current != expected:
                    raise ValueError("concurrent edit preserved")
                original = read_regular(backup / "sources" / name)
                if original != sources[name]:
                    raise ValueError("recovery copy changed")
                publish(path, original, modes[name])
            except (OSError, ValueError) as restore_error:
                failures.append(f"{name}: {restore_error}")
        raise ValueError(
            f"consolidation failed: {exc}; recovery: {backup}; restoration failures: {failures}"
        ) from exc
    print(f"Applied; verified original recovery bytes: {backup}")
    return 0
