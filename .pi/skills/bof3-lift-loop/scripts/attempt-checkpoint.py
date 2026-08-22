#!/usr/bin/env python3
"""Record and restore the best owned-file state for one lift-loop lane."""

from __future__ import annotations

import argparse
import errno
import hashlib
import json
import os
from pathlib import Path, PurePath, PurePosixPath
import re
import secrets
import stat
import subprocess
import tomllib


ROOT = Path(__file__).resolve().parents[4]
CHECKPOINT_PARTS = ("out", "lift-loop", "checkpoints")
_DIRECTORY_FLAGS = os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW
_ATTEMPT_NAME = re.compile(r"attempt-(0|[1-9][0-9]*)(?:-([0-9a-f]{16}))?\Z")


def run_json(*args: str) -> dict:
    result = subprocess.run(args, cwd=ROOT, text=True, capture_output=True)
    if result.returncode not in (0, 1):
        raise subprocess.CalledProcessError(
            result.returncode, args, result.stdout, result.stderr
        )
    return json.loads(result.stdout)


def metric(selector: str, reported: float | None) -> dict:
    diff = run_json("bin/asm-diff", "--json", selector)
    first = diff.get("first_mismatch") or {}
    instruction_count = diff.get("instruction_count") or {}
    live_score = float(
        instruction_count.get(
            "match_percent", 100.0 if diff.get("exact_match") else 0.0
        )
    )
    return {
        "match_percent": live_score,
        "reported_match_percent": reported,
        "report_matches_live": reported is None or abs(live_score - reported) < 0.005,
        "exact": bool(diff.get("exact_match")),
        "current_size": diff.get("current_size"),
        "original_size": diff.get("original_size"),
        "size_delta": diff.get("size_delta"),
        "source": diff.get("source"),
        "first_mismatch": {
            "original_offset": first.get("original_offset"),
            "current_offset": first.get("current_offset"),
            "original": first.get("original"),
            "current": first.get("current"),
        },
    }


def checkpoint_dir(lane: str) -> str:
    if not lane or PurePosixPath(lane).name != lane or lane in {".", ".."}:
        raise ValueError(f"invalid checkpoint lane: {lane!r}")
    return lane


def attempt_name_for(attempt: int | str) -> str:
    name = f"attempt-{attempt}"
    if _ATTEMPT_NAME.fullmatch(name) is None:
        raise ValueError(f"invalid checkpoint attempt: {attempt!r}")
    return name


def unique_attempt_name(attempt: int) -> str:
    return f"{attempt_name_for(attempt)}-{secrets.token_hex(8)}"


def canonical_path(name: str) -> PurePosixPath:
    path = PurePosixPath(name)
    if (
        not name
        or "\\" in name
        or path.is_absolute()
        or path.as_posix() != name
        or any(part in {"", ".", ".."} for part in path.parts)
    ):
        raise ValueError(
            f"checkpoint path must be canonical and repository-relative: {name!r}"
        )
    return path


def _open_child(directory: int, name: str) -> int:
    return os.open(name, _DIRECTORY_FLAGS, dir_fd=directory)


def _open_chain(parts: tuple[str, ...], *, create: bool = False) -> int:
    descriptor = os.open(ROOT, os.O_RDONLY | os.O_DIRECTORY)
    try:
        for part in parts:
            if create:
                try:
                    os.mkdir(part, dir_fd=descriptor)
                except FileExistsError:
                    pass
            child = _open_child(descriptor, part)
            os.close(descriptor)
            descriptor = child
        return descriptor
    except BaseException:
        os.close(descriptor)
        raise


def open_directory(path: PurePosixPath) -> int:
    return _open_chain(path.parts)


def _checkpoint_root(*, create: bool = False) -> int:
    try:
        return _open_chain(CHECKPOINT_PARTS, create=create)
    except OSError as error:
        if error.errno in {errno.ELOOP, errno.ENOTDIR}:
            raise ValueError(
                "checkpoint path contains a symlink or non-directory"
            ) from error
        raise


def _open_lane(lane: str, *, create: bool = False) -> int:
    lane = checkpoint_dir(lane)
    root = _checkpoint_root(create=create)
    try:
        if create:
            try:
                os.mkdir(lane, dir_fd=root)
            except FileExistsError:
                pass
        return _open_child(root, lane)
    except OSError as error:
        if error.errno in {errno.ELOOP, errno.ENOTDIR}:
            raise ValueError(
                "checkpoint path contains a symlink or non-directory"
            ) from error
        raise
    finally:
        os.close(root)


def _read_file(directory: int, name: str) -> bytes:
    descriptor = os.open(name, os.O_RDONLY | os.O_NOFOLLOW, dir_fd=directory)
    with os.fdopen(descriptor, "rb") as stream:
        return stream.read()


def _read_json(directory: int, name: str) -> dict:
    return json.loads(_read_file(directory, name))


def _canonical_json(value: object) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":")).encode()


def _checkpoint_manifest(record: dict, record_data: bytes, outcome_data: bytes) -> dict:
    files = record.get("files")
    if not isinstance(files, list):
        raise ValueError("incomplete or corrupt checkpoint file manifest")
    tree = []
    previous = None
    for state in files:
        if not isinstance(state, dict) or set(state) not in (
            {"path", "exists"},
            {"path", "exists", "sha256"},
            {"path", "exists", "sha256", "mode"},
        ):
            raise ValueError("incomplete or corrupt checkpoint file state")
        name = state.get("path")
        if not isinstance(name, str):
            raise ValueError("incomplete or corrupt checkpoint file state")
        path = canonical_path(name)
        exists = state.get("exists")
        digest = state.get("sha256")
        mode = state.get("mode")
        if (
            not isinstance(exists, bool)
            or (exists and (not isinstance(digest, str) or len(digest) != 64))
            or (exists and mode is not None and not isinstance(mode, int))
            or (not exists and (digest is not None or mode is not None))
            or previous is not None
            and path.as_posix() <= previous
        ):
            raise ValueError("incomplete or corrupt checkpoint file state")
        previous = path.as_posix()
        tree.append(state)
    return {
        "record.json": hashlib.sha256(record_data).hexdigest(),
        "outcome.json": hashlib.sha256(outcome_data).hexdigest(),
        "tree": tree,
    }


def scoped_paths_from_descriptor(directory: int) -> set[str]:
    paths: set[str] = set()

    def visit(current: int, prefix: PurePosixPath) -> None:
        with os.scandir(current) as entries:
            for entry in entries:
                path = prefix / entry.name
                if entry.is_dir(follow_symlinks=False):
                    child = _open_child(current, entry.name)
                    try:
                        visit(child, path)
                    finally:
                        os.close(child)
                elif entry.is_file(follow_symlinks=False):
                    paths.add(path.as_posix())
                else:
                    raise ValueError("incomplete or corrupt checkpoint snapshot tree")

    visit(directory, PurePosixPath())
    return paths


def verify_complete_checkpoint(directory: int) -> tuple[dict, dict, dict[str, bytes]]:
    """Verify all checkpoint metadata and snapshots before returning any content."""
    try:
        complete = _read_json(directory, "complete.json")
        record_data = _read_file(directory, "record.json")
        outcome_data = _read_file(directory, "outcome.json")
        record = json.loads(record_data)
        outcome = json.loads(outcome_data)
        manifest = _checkpoint_manifest(record, record_data, outcome_data)
        if (
            complete.get("schema") != "bof3.attempt-checkpoint-complete/v1"
            or complete.get("manifest") != manifest
            or complete.get("manifest_sha256")
            != hashlib.sha256(_canonical_json(manifest)).hexdigest()
        ):
            raise ValueError("incomplete or corrupt checkpoint manifest")
        files = _open_child(directory, "files")
        try:
            snapshots = {}
            actual = scoped_paths_from_descriptor(files)
            expected = {state["path"] for state in manifest["tree"] if state["exists"]}
            if actual != expected:
                raise ValueError("incomplete or corrupt checkpoint snapshot tree")
            for state in manifest["tree"]:
                if state["exists"]:
                    data = _snapshot_bytes(files, state["path"])
                    if hashlib.sha256(data).hexdigest() != state["sha256"]:
                        raise ValueError(
                            f"incomplete or corrupt checkpoint snapshot: {state['path']}"
                        )
                    snapshots[state["path"]] = data
        finally:
            os.close(files)
        return record, outcome, snapshots
    except (FileNotFoundError, json.JSONDecodeError, OSError, TypeError) as error:
        raise ValueError("incomplete or corrupt checkpoint") from error


def _verified_record(
    directory: int,
    lane: str,
    checkpoint: str,
    *,
    binding: dict | None = None,
) -> tuple[dict, dict, dict[str, bytes]]:
    record, outcome, snapshots = verify_complete_checkpoint(directory)
    return (
        _validate_record(record, lane, checkpoint, binding=binding),
        outcome,
        snapshots,
    )


def _verified_best(
    lane: int, lane_name: str, *, ignore_corrupt: bool = False
) -> tuple[dict, dict, dict[str, bytes]]:
    try:
        binding = _read_json(lane, "best.json")
        checkpoint = binding.get("checkpoint")
        if (
            not isinstance(checkpoint, str)
            or _ATTEMPT_NAME.fullmatch(checkpoint) is None
        ):
            raise ValueError(f"invalid checkpoint attempt leaf: {checkpoint!r}")
        attempt = _open_child(lane, checkpoint)
        try:
            record, _outcome, snapshots = _verified_record(
                attempt, lane_name, checkpoint, binding=binding
            )
        finally:
            os.close(attempt)
        return binding, record, snapshots
    except (FileNotFoundError, ValueError):
        if not ignore_corrupt:
            raise
    candidates = _attempt_records(lane, lane_name)
    candidates = [record for record in candidates if record.get("metric") is not None]
    if not candidates:
        raise FileNotFoundError("no complete checkpoint is eligible for best")
    record = max(candidates, key=lambda item: item["metric"]["match_percent"])
    checkpoint = record.pop("_checkpoint")
    attempt = _open_child(lane, checkpoint)
    try:
        record, _outcome, snapshots = _verified_record(attempt, lane_name, checkpoint)
    finally:
        os.close(attempt)
    return record | {"checkpoint": checkpoint, "worktree_paths": []}, record, snapshots


def _write_atomic(directory: int, name: str, data: bytes) -> None:
    temporary = f".{name}.{secrets.token_hex(8)}.tmp"
    descriptor = os.open(
        temporary,
        os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW,
        0o600,
        dir_fd=directory,
    )
    try:
        with os.fdopen(descriptor, "wb", closefd=False) as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        os.close(descriptor)
        descriptor = -1
        os.replace(temporary, name, src_dir_fd=directory, dst_dir_fd=directory)
        os.fsync(directory)
    finally:
        if descriptor >= 0:
            os.close(descriptor)
        try:
            os.unlink(temporary, dir_fd=directory)
        except FileNotFoundError:
            pass


def _write_json(directory: int, name: str, value: dict) -> None:
    _write_atomic(directory, name, (json.dumps(value, indent=2) + "\n").encode())


def _mkdir_chain(directory: int, parts: tuple[str, ...]) -> int:
    descriptor = os.dup(directory)
    try:
        for part in parts:
            try:
                os.mkdir(part, dir_fd=descriptor)
            except FileExistsError:
                pass
            child = _open_child(descriptor, part)
            os.close(descriptor)
            descriptor = child
        return descriptor
    except BaseException:
        os.close(descriptor)
        raise


def _open_relative(directory: int, path: PurePosixPath) -> int:
    descriptor = os.dup(directory)
    try:
        for part in path.parts:
            child = _open_child(descriptor, part)
            os.close(descriptor)
            descriptor = child
        return descriptor
    except BaseException:
        os.close(descriptor)
        raise


def _validate_record(
    record: dict, lane: str, checkpoint: str, *, binding: dict | None = None
) -> dict:
    match = _ATTEMPT_NAME.fullmatch(checkpoint)
    if match is None:
        raise ValueError(f"invalid checkpoint attempt leaf: {checkpoint!r}")
    expected = {
        "schema": "bof3.attempt-checkpoint/v1",
        "attempt": int(match.group(1)),
        "run": lane,
        "root": str(ROOT.resolve()),
    }
    if any(record.get(key) != value for key, value in expected.items()):
        raise ValueError("checkpoint record attempt/run/root binding mismatch")
    if not isinstance(record.get("selector"), str) or not record["selector"]:
        raise ValueError("checkpoint record selector binding mismatch")
    if binding is not None and any(
        binding.get(key) != record.get(key)
        for key in ("attempt", "run", "selector", "root")
    ):
        raise ValueError("best checkpoint record binding mismatch")
    return record


def _attempt_records(lane: int, lane_name: str) -> list[dict]:
    records: list[dict] = []
    with os.scandir(lane) as entries:
        names = sorted(
            entry.name
            for entry in entries
            if _ATTEMPT_NAME.fullmatch(entry.name)
            and entry.is_dir(follow_symlinks=False)
        )
    for name in names:
        attempt = _open_child(lane, name)
        try:
            record, _outcome, _snapshots = _verified_record(attempt, lane_name, name)
            records.append(record | {"_checkpoint": name})
        except (FileNotFoundError, ValueError):
            pass
        finally:
            os.close(attempt)
    return records


def path_state(name: str) -> bool:
    """Return regular-file existence without following any path component."""
    path = canonical_path(name)
    try:
        descriptor = open_directory(PurePosixPath(*path.parts[:-1]))
    except FileNotFoundError:
        return False
    try:
        try:
            mode = os.stat(path.name, dir_fd=descriptor, follow_symlinks=False).st_mode
        except FileNotFoundError:
            return False
        if not stat.S_ISREG(mode):
            raise ValueError(f"checkpoint baseline path is not a regular file: {name}")
        return True
    finally:
        os.close(descriptor)


def path_mode(name: str) -> int:
    path = canonical_path(name)
    descriptor = open_directory(PurePosixPath(*path.parts[:-1]))
    try:
        mode = os.stat(path.name, dir_fd=descriptor, follow_symlinks=False).st_mode
        if not stat.S_ISREG(mode):
            raise ValueError(f"checkpoint baseline path is not a regular file: {name}")
        return stat.S_IMODE(mode)
    finally:
        os.close(descriptor)


def read_path(name: str) -> bytes:
    path = canonical_path(name)
    descriptor = open_directory(PurePosixPath(*path.parts[:-1]))
    try:
        return _read_file(descriptor, path.name)
    finally:
        os.close(descriptor)


def scoped_paths(root: str, *, baseline: bool) -> set[str]:
    root_path = canonical_path(root)
    try:
        descriptor = open_directory(root_path)
    except FileNotFoundError:
        return set()
    except OSError as error:
        raise ValueError(f"checkpoint scope root is not a directory: {root}") from error
    paths: set[str] = set()

    def visit(directory: int, prefix: PurePosixPath) -> None:
        with os.scandir(directory) as entries:
            for entry in entries:
                path = prefix / entry.name
                name = path.as_posix()
                if entry.is_dir(follow_symlinks=False):
                    child = _open_child(directory, entry.name)
                    try:
                        visit(child, path)
                    finally:
                        os.close(child)
                elif entry.is_file(follow_symlinks=False):
                    paths.add(name)
                elif baseline:
                    raise ValueError(
                        f"checkpoint baseline path is not a regular file: {name}"
                    )
                else:
                    paths.add(name)

    try:
        visit(descriptor, root_path)
    finally:
        os.close(descriptor)
    return paths


def target_scope(target: str) -> tuple[set[str], set[str]]:
    config = Path("config/targets") / target
    manifest_path = config / "target.toml"
    manifest = tomllib.loads(read_path(manifest_path.as_posix()).decode())
    paths = {
        manifest_path.as_posix(),
        (config / "symbols.txt").as_posix(),
        str(manifest["splat"]),
        *(
            str(value)
            for key in ("sources", "support_sources", "headers")
            for value in manifest.get(key, [])
        ),
    }
    if manifest.get("psyq_source"):
        paths.add(str(manifest["psyq_source"]))
    roots = {
        str(manifest["source_dir"]),
        config.as_posix(),
        *(PurePath(path).parent.as_posix() for path in paths),
    }
    for root in roots:
        paths.update(scoped_paths(root, baseline=True))
    return paths, roots


def _lane_scope(lane: int, lane_name: str, selector: str) -> dict:
    try:
        scope = _read_json(lane, "scope.json")
    except (FileNotFoundError, json.JSONDecodeError, OSError, TypeError) as error:
        raise ValueError("missing or corrupt lane scope manifest") from error
    if (
        scope.get("schema") != "bof3.attempt-checkpoint-scope/v1"
        or scope.get("run") != lane_name
        or scope.get("selector") != selector
        or not isinstance(scope.get("roots"), list)
        or not isinstance(scope.get("paths"), list)
    ):
        raise ValueError("invalid lane scope manifest binding")
    roots = [canonical_path(name).as_posix() for name in scope["roots"]]
    paths = [canonical_path(name).as_posix() for name in scope["paths"]]
    if roots != sorted(set(roots)) or paths != sorted(set(paths)):
        raise ValueError("invalid lane scope manifest paths")
    return scope


def ensure_lane_scope(
    lane: int,
    lane_name: str,
    selector: str,
    roots: set[str],
    paths: set[str],
) -> dict:
    manifest = {
        "schema": "bof3.attempt-checkpoint-scope/v1",
        "run": lane_name,
        "selector": selector,
        "roots": sorted(canonical_path(name).as_posix() for name in roots),
        "paths": sorted(canonical_path(name).as_posix() for name in paths),
    }
    try:
        existing = _lane_scope(lane, lane_name, selector)
    except ValueError as error:
        try:
            os.stat("scope.json", dir_fd=lane, follow_symlinks=False)
        except FileNotFoundError:
            _write_json(lane, "scope.json", manifest)
            return manifest
        raise error
    if existing != manifest:
        if not existing["roots"] and all(
            path_in_scope(name, manifest) for name in existing["paths"]
        ):
            _write_json(lane, "scope.json", manifest)
            return manifest
        raise ValueError("attempt would widen or change persisted lane scope")
    return existing


def path_in_scope(name: str, scope: dict) -> bool:
    return name in scope["paths"] or any(
        name == root or name.startswith(root + "/") for root in scope["roots"]
    )


def lane_scope_paths(scope: dict) -> set[str]:
    paths = set(scope["paths"])
    for root in scope["roots"]:
        paths.update(scoped_paths(root, baseline=False))
    return paths


def dirty_paths() -> set[str]:
    status = (
        subprocess.run(
            ("git", "status", "--porcelain", "-z"),
            cwd=ROOT,
            capture_output=True,
            check=True,
        )
        .stdout.decode(errors="surrogateescape")
        .split("\0")
    )
    return {
        entry[3:].split(" -> ")[-1]
        for entry in status
        if entry
        and entry[3:]
        .split(" -> ")[-1]
        .startswith(
            ("src/", "include/", "config/targets/", "docs/specs/", "docs/agents/")
        )
    }


def _snapshot_file(files: int, name: str, data: bytes) -> None:
    path = canonical_path(name)
    parent = _mkdir_chain(files, path.parts[:-1])
    try:
        descriptor = os.open(
            path.name,
            os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW,
            0o600,
            dir_fd=parent,
        )
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(data)
    finally:
        os.close(parent)


def capture(args: argparse.Namespace) -> int:
    lane_name = checkpoint_dir(args.lane)
    lane = _open_lane(lane_name, create=True)
    requested_name = attempt_name_for(args.attempt)
    unique_leaf = args.unique or args.replace
    attempt_name = unique_attempt_name(args.attempt) if unique_leaf else requested_name
    improved = False
    best = None
    try:
        if args.scan_worktree:
            best, _record, _snapshots = _verified_best(
                lane, lane_name, ignore_corrupt=True
            )
            baseline = set(best.get("worktree_paths", []))
            args.files.extend(sorted(dirty_paths() - baseline))
        try:
            existing = _open_child(lane, requested_name)
        except FileNotFoundError:
            existing = None
        except OSError as error:
            if error.errno in {errno.ELOOP, errno.ENOTDIR}:
                raise ValueError(
                    "checkpoint path contains a symlink or non-directory"
                ) from error
            raise
        if existing is not None:
            try:
                if not unique_leaf:
                    _record, outcome, _snapshots = _verified_record(
                        existing, lane_name, requested_name
                    )
                    print(json.dumps(outcome))
                    code = int(outcome["exit_code"])
                    return 0 if args.soft_no_improvement and code == 1 else code
            finally:
                os.close(existing)

        evidence = None if args.paths_only else metric(args.selector, args.match)
        paths = set(args.files)
        scope_roots: set[str] = set()
        if args.target_scope:
            owned_paths, scope_roots = target_scope(args.target_scope)
            paths.update(owned_paths)
        if evidence is not None:
            paths.add(Path(evidence["source"]).relative_to(ROOT).as_posix())
        canonical_paths = {canonical_path(name).as_posix() for name in paths}
        if args.target_scope:
            scope_paths = {
                name
                for name in canonical_paths
                if not any(
                    name == root or name.startswith(root + "/") for root in scope_roots
                )
            }
            scope = ensure_lane_scope(
                lane, lane_name, args.selector, scope_roots, scope_paths
            )
        else:
            try:
                scope = _lane_scope(lane, lane_name, args.selector)
            except ValueError:
                scope = ensure_lane_scope(
                    lane, lane_name, args.selector, set(), canonical_paths
                )
        if any(not path_in_scope(name, scope) for name in canonical_paths):
            raise ValueError("attempt path is outside persisted lane scope")
        paths.update(scope["paths"])
        for root in scope["roots"]:
            paths.update(scoped_paths(root, baseline=True))
        for record in _attempt_records(lane, lane_name):
            paths.update(state["path"] for state in record["files"])
        paths = sorted(canonical_path(name).as_posix() for name in paths)
        states = [
            {
                "path": name,
                "exists": path_state(name),
                **(
                    {
                        "sha256": hashlib.sha256(read_path(name)).hexdigest(),
                        "mode": path_mode(name),
                    }
                    if path_state(name)
                    else {}
                ),
            }
            for name in paths
        ]

        os.mkdir(attempt_name, dir_fd=lane)
        attempt = _open_child(lane, attempt_name)
        try:
            os.mkdir("files", dir_fd=attempt)
            files = _open_child(attempt, "files")
            try:
                for state in states:
                    if state["exists"]:
                        _snapshot_file(files, state["path"], read_path(state["path"]))
            finally:
                os.close(files)
            record = {
                "schema": "bof3.attempt-checkpoint/v1",
                "selector": args.selector,
                "attempt": args.attempt,
                "run": lane_name,
                "root": str(ROOT.resolve()),
                "files": states,
                "scope_roots": list(scope["roots"]),
                "metric": evidence,
            }
            _write_json(attempt, "record.json", record)
            if args.paths_only:
                outcome = {"paths_recorded": paths, "exit_code": 0}
            else:
                try:
                    best, _best_record, _snapshots = _verified_best(
                        lane, lane_name, ignore_corrupt=True
                    )
                except FileNotFoundError:
                    best = None
                best_score = best["metric"]["match_percent"] if best else None
                assert evidence is not None
                live_score = evidence["match_percent"]
                improved = best is None or live_score > best_score
                observable = best is None or {
                    key: value
                    for key, value in evidence.items()
                    if key != "reported_match_percent"
                } != {
                    key: value
                    for key, value in best["metric"].items()
                    if key != "reported_match_percent"
                }
                if improved and not args.no_promote:
                    best = record | {
                        "checkpoint": attempt_name,
                        "worktree_paths": sorted(dirty_paths()),
                    }
                below_floor = (
                    args.require_at_least is not None
                    and live_score < args.require_at_least
                )
                exit_code = (
                    2
                    if not evidence["report_matches_live"]
                    else 1
                    if (args.require_improvement and not improved) or below_floor
                    else 0
                )
                outcome = {
                    "accepted": exit_code == 0,
                    "improved": improved,
                    "observable_change": observable,
                    "current": record,
                    "best": best,
                    "exit_code": exit_code,
                }
            outcome["checkpoint"] = attempt_name
            _write_json(attempt, "outcome.json", outcome)
            record_data = _read_file(attempt, "record.json")
            outcome_data = _read_file(attempt, "outcome.json")
            manifest = _checkpoint_manifest(record, record_data, outcome_data)
            _write_json(
                attempt,
                "complete.json",
                {
                    "schema": "bof3.attempt-checkpoint-complete/v1",
                    "manifest": manifest,
                    "manifest_sha256": hashlib.sha256(
                        _canonical_json(manifest)
                    ).hexdigest(),
                },
            )
        finally:
            os.close(attempt)
        if not args.paths_only and improved and not args.no_promote:
            assert best is not None
            best["checkpoint"] = attempt_name
            _write_json(lane, "best.json", best)
        print(json.dumps(outcome))
        code = int(outcome["exit_code"])
        return 0 if args.soft_no_improvement and code == 1 else code
    finally:
        os.close(lane)


def ensure_parent(
    path: PurePosixPath,
    *,
    create: bool,
    created: list[PurePosixPath] | None = None,
) -> int | None:
    """Open a safe parent chain, optionally creating only missing directories."""
    descriptor = os.open(ROOT, os.O_RDONLY | os.O_DIRECTORY)
    prefix = PurePosixPath()
    try:
        for part in path.parts[:-1]:
            prefix /= part
            try:
                child = _open_child(descriptor, part)
            except FileNotFoundError:
                if not create:
                    os.close(descriptor)
                    return None
                os.mkdir(part, dir_fd=descriptor)
                # Record before the parent fsync: a durable-marker failure must
                # still leave the new directory on the caller's rollback list.
                if created is not None:
                    created.append(prefix)
                os.fsync(descriptor)
                child = _open_child(descriptor, part)
            except OSError as error:
                if error.errno in {errno.ELOOP, errno.ENOTDIR}:
                    raise ValueError(
                        f"checkpoint parent is not a real directory: {prefix}"
                    ) from error
                raise
            os.close(descriptor)
            descriptor = child
        return descriptor
    except BaseException:
        os.close(descriptor)
        raise


def _leaf_state(path: PurePosixPath) -> tuple[str, bytes | str | None, int | None]:
    descriptor = ensure_parent(path, create=False)
    if descriptor is None:
        return "absent", None, None
    try:
        try:
            mode = os.stat(path.name, dir_fd=descriptor, follow_symlinks=False).st_mode
        except FileNotFoundError:
            return "absent", None, None
        if stat.S_ISREG(mode):
            return "file", _read_file(descriptor, path.name), stat.S_IMODE(mode)
        if stat.S_ISLNK(mode):
            return "symlink", os.readlink(path.name, dir_fd=descriptor), None
        raise ValueError(f"checkpoint destination is not removable: {path}")
    finally:
        os.close(descriptor)


def remove_path(path: PurePosixPath) -> None:
    """Remove one regular file or symlink without creating missing parents."""
    descriptor = ensure_parent(path, create=False)
    if descriptor is None:
        return
    try:
        try:
            mode = os.stat(path.name, dir_fd=descriptor, follow_symlinks=False).st_mode
        except FileNotFoundError:
            return
        if not (stat.S_ISREG(mode) or stat.S_ISLNK(mode)):
            raise ValueError(f"checkpoint destination is not removable: {path}")
        os.unlink(path.name, dir_fd=descriptor)
        os.fsync(descriptor)
    finally:
        os.close(descriptor)


def restore_file(
    data: bytes,
    path: PurePosixPath,
    mode: int = 0o666,
    *,
    created: list[PurePosixPath] | None = None,
) -> None:
    descriptor = ensure_parent(path, create=True, created=created)
    assert descriptor is not None
    temporary = f".{path.name}.{secrets.token_hex(8)}.restore"
    try:
        target = os.open(
            temporary,
            os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW,
            mode,
            dir_fd=descriptor,
        )
        with os.fdopen(target, "wb") as output_stream:
            output_stream.write(data)
            output_stream.flush()
            os.fchmod(output_stream.fileno(), mode)
            os.fsync(output_stream.fileno())
        os.replace(temporary, path.name, src_dir_fd=descriptor, dst_dir_fd=descriptor)
        os.fsync(descriptor)
    finally:
        try:
            os.unlink(temporary, dir_fd=descriptor)
        except FileNotFoundError:
            pass
        os.close(descriptor)


def _restore_symlink(target: str, path: PurePosixPath) -> None:
    descriptor = ensure_parent(path, create=True)
    assert descriptor is not None
    temporary = f".{path.name}.{secrets.token_hex(8)}.restore"
    try:
        os.symlink(target, temporary, dir_fd=descriptor)
        os.replace(temporary, path.name, src_dir_fd=descriptor, dst_dir_fd=descriptor)
        os.fsync(descriptor)
    finally:
        try:
            os.unlink(temporary, dir_fd=descriptor)
        except FileNotFoundError:
            pass
        os.close(descriptor)


def _prune_created_directories(created: list[PurePosixPath]) -> None:
    for path in reversed(created):
        parent = ensure_parent(path, create=False)
        if parent is None:
            continue
        try:
            try:
                os.rmdir(path.name, dir_fd=parent)
            except (FileNotFoundError, OSError) as error:
                if isinstance(error, OSError) and error.errno not in {
                    errno.ENOENT,
                    errno.ENOTEMPTY,
                }:
                    raise
            else:
                os.fsync(parent)
        finally:
            os.close(parent)


def _rollback_restore(
    recovery: dict[str, tuple[str, bytes | str | None, int | None]],
    mutated: list[str],
    created: list[PurePosixPath],
) -> None:
    failure: BaseException | None = None
    for name in reversed(mutated):
        kind, content, mode = recovery[name]
        path = canonical_path(name)
        try:
            if kind == "file":
                assert isinstance(content, bytes) and mode is not None
                restore_file(content, path, mode)
            elif kind == "symlink":
                assert isinstance(content, str)
                _restore_symlink(content, path)
            else:
                remove_path(path)
        except BaseException as error:
            failure = failure or error
    try:
        _prune_created_directories(created)
    except BaseException as error:
        failure = failure or error
    if failure is not None:
        raise RuntimeError("checkpoint rollback failed") from failure


def _snapshot_bytes(files: int, name: str) -> bytes:
    path = canonical_path(name)
    parent = _open_relative(files, PurePosixPath(*path.parts[:-1]))
    try:
        return _read_file(parent, path.name)
    finally:
        os.close(parent)


def inspect_best(args: argparse.Namespace) -> int:
    lane_name = checkpoint_dir(args.lane)
    lane = _open_lane(lane_name)
    try:
        binding, _record, _snapshots = _verified_best(lane, lane_name)
    finally:
        os.close(lane)
    print(json.dumps({"best": binding}))
    return 0


def _apply_restore(
    ordered: list[str],
    best_states: dict[str, dict],
    snapshots: dict[str, bytes],
    scope: dict,
) -> None:
    """Preflight, apply, verify, and atomically roll back one restore."""
    for index, name in enumerate(ordered[:-1]):
        if ordered[index + 1].startswith(name + "/"):
            raise ValueError(f"checkpoint paths conflict: {name}")
    recovery = {name: _leaf_state(canonical_path(name)) for name in ordered}
    created: list[PurePosixPath] = []
    mutated: list[str] = []
    try:
        for name in ordered:
            state = best_states.get(name)
            path = canonical_path(name)
            # Mark mutation-started before the call: an exception after the
            # rename/unlink (e.g. parent fsync) must still roll back this path.
            mutated.append(name)
            if state and state["exists"]:
                restore_file(
                    snapshots[name],
                    path,
                    state.get("mode", 0o666),
                    created=created,
                )
            else:
                remove_path(path)

        unequal = []
        for name, state in best_states.items():
            exists = path_state(name)
            if exists != state["exists"] or (
                exists
                and (
                    read_path(name) != snapshots[name]
                    or "mode" in state
                    and path_mode(name) != state["mode"]
                )
            ):
                unequal.append(name)
        for root in scope["roots"]:
            current = scoped_paths(root, baseline=False)
            expected = {
                name
                for name in best_states
                if (name == root or name.startswith(root + "/"))
                and best_states[name]["exists"]
            }
            unequal.extend(sorted(current ^ expected))
        if unequal:
            raise RuntimeError(
                f"checkpoint restore equality failed: {sorted(set(unequal))}"
            )
    except BaseException:
        _rollback_restore(recovery, mutated, created)
        raise


def restore(args: argparse.Namespace) -> int:
    lane_name = checkpoint_dir(args.lane)
    lane = _open_lane(lane_name)
    try:
        if args.checkpoint is not None and args.attempt is not None:
            raise ValueError("restore accepts either checkpoint or attempt")
        binding = (
            _read_json(lane, "best.json")
            if args.attempt is None and args.checkpoint is None
            else None
        )
        if binding is not None:
            checkpoint = binding.get("checkpoint")
        elif args.checkpoint is not None:
            checkpoint = args.checkpoint
        elif args.attempt is not None:
            requested = attempt_name_for(args.attempt)
            matches = [
                entry.name
                for entry in os.scandir(lane)
                if entry.is_dir(follow_symlinks=False)
                and (entry.name == requested or entry.name.startswith(requested + "-"))
            ]
            if len(matches) != 1:
                raise ValueError("attempt does not identify one exact checkpoint leaf")
            checkpoint = matches[0]
        else:
            raise ValueError("restore requires an attempt or best checkpoint")
        if (
            not isinstance(checkpoint, str)
            or _ATTEMPT_NAME.fullmatch(checkpoint) is None
        ):
            raise ValueError(f"invalid checkpoint attempt leaf: {checkpoint!r}")
        attempt = _open_child(lane, checkpoint)
        try:
            best, _outcome, snapshots = _verified_record(
                attempt, lane_name, checkpoint, binding=binding
            )
            scope = _lane_scope(lane, lane_name, best["selector"])
            if best.get("scope_roots", []) != scope["roots"]:
                raise ValueError("checkpoint scope mismatches lane scope manifest")
            best_states = {state["path"]: state for state in best["files"]}
            known_paths = set(best_states)
            if args.attempt is None:
                for record in _attempt_records(lane, lane_name):
                    known_paths.update(state["path"] for state in record["files"])
            known_paths.update(lane_scope_paths(scope))
            _apply_restore(sorted(known_paths), best_states, snapshots, scope)
        finally:
            os.close(attempt)
    finally:
        os.close(lane)
    print(json.dumps({"restored": best, "clean_equality": True}))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    save = sub.add_parser("capture")
    save.add_argument("--lane", required=True)
    save.add_argument("--selector", required=True)
    save.add_argument("--attempt", required=True, type=int)
    save.add_argument("--match", type=float)
    save.add_argument("--paths-only", action="store_true")
    save.add_argument("--scan-worktree", action="store_true")
    save.add_argument("--target-scope")
    save.add_argument("--replace", action="store_true")
    save.add_argument("--unique", action="store_true")
    save.add_argument("--no-promote", action="store_true")
    save.add_argument("--require-improvement", action="store_true")
    save.add_argument("--require-at-least", type=float)
    save.add_argument("--soft-no-improvement", action="store_true")
    save.add_argument("files", nargs="*")
    load = sub.add_parser("restore")
    load.add_argument("--lane", required=True)
    load.add_argument("--attempt", type=int)
    load.add_argument("--checkpoint")
    inspect = sub.add_parser("best")
    inspect.add_argument("--lane", required=True)
    args = parser.parse_args()
    return (
        capture(args)
        if args.command == "capture"
        else inspect_best(args)
        if args.command == "best"
        else restore(args)
    )


if __name__ == "__main__":
    raise SystemExit(main())
