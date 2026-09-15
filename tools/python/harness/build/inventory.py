"""Validate configured source topology and compiler metadata."""

from __future__ import annotations

import hashlib
import json
from contextlib import closing
from dataclasses import asdict
from pathlib import Path

from harness.build.compiler import (
    parse_object_compilers,
    parse_object_flags,
    resolve_compiler_settings,
    validate_object_configuration,
)
from harness.common.deadlines import check_deadline
from harness.common.files import read_file
from harness.common.observation import PathWatch
from harness.domain.cache import collect_manifest_paths
from harness.domain.tags import count_function_metadata
from harness.io import unique_object

_OWNERS = (
    "CMakeLists.txt",
    "config/compiler/graph.cmake",
    "tools/python/harness/build/inventory.py",
    "tools/python/harness/build/cli.py",
    "tools/python/harness/build/compiler.py",
    "tools/python/harness/domain/functions.py",
    "tools/python/harness/domain/tags.py",
    "tools/python/harness/common/lexicon.py",
    "tools/python/harness/domain/cache.py",
    "tools/python/harness/common/directory.py",
    "tools/python/harness/common/files.py",
    "tools/python/harness/toolchain/gcc.py",
)
_CONFIGURATION = (
    "config/compiler/object-flags.cmake",
    "config/compiler/variants.json",
)
_LIMIT = 4 * 1024 * 1024


def _list_sources(root: Path) -> list[str]:
    source_root = root / "src"
    if not source_root.is_dir():
        raise ValueError("build inventory requires the source directory")
    sources = sorted(
        path.relative_to(root).as_posix() for path in source_root.rglob("*.c")
    )
    if any(any(character in name for character in ";|\\\r\n") for name in sources):
        raise ValueError("unsupported CMake source inventory path")
    return sources


def capture_inventory(root: Path) -> dict:
    """Bind configured metadata and source classes without object reuse authority."""
    root = root.resolve()
    check_deadline()
    sources = _list_sources(root)
    manifests = collect_manifest_paths(root)
    paths = {root / name for name in [*sources, *manifests, *_OWNERS, *_CONFIGURATION]}
    paths.update((root / "src", root / "config/targets"))
    with closing(PathWatch(paths)) as watch:
        configuration = {}
        configuration_text = {}
        for name in _CONFIGURATION:
            check_deadline()
            content = read_file(root, name, missing_ok=True, max_bytes=_LIMIT)
            configuration[name] = (
                hashlib.sha256(content).hexdigest() if content is not None else None
            )
            configuration_text[name] = (
                content.decode("utf-8") if content is not None else ""
            )
        flags_text = configuration_text["config/compiler/object-flags.cmake"]
        validate_object_configuration(flags_text)
        flags = parse_object_flags(flags_text)
        compilers = parse_object_compilers(flags_text)
        classified = {}
        profiles = {}
        for name in sources:
            check_deadline()
            content = read_file(root, name, max_bytes=_LIMIT)
            text = content.decode("utf-8")
            classified[name] = count_function_metadata(text) >= 2
            profiles[name] = asdict(
                resolve_compiler_settings(name, text, flags, compilers)
            )
        owners = {}
        for name in _OWNERS:
            check_deadline()
            owners[name] = hashlib.sha256(
                read_file(root, name, max_bytes=_LIMIT)
            ).hexdigest()
        manifest_inputs = {}
        for name in manifests:
            check_deadline()
            manifest_inputs[name] = hashlib.sha256(
                read_file(root, name, max_bytes=_LIMIT)
            ).hexdigest()
        if _list_sources(root) != sources:
            raise ValueError("build source inventory changed during classification")
        if collect_manifest_paths(root) != manifests:
            raise ValueError("build manifest inventory changed during capture")
        watch.validate()
        check_deadline()
        return {
            "schema": "bof3.build-inventory/v4",
            "sources": classified,
            "profiles": profiles,
            "manifests": manifest_inputs,
            "configuration": configuration,
            "owners": owners,
        }


def check_inventory(root: Path, snapshot: Path, expected_sha256: str) -> None:
    """Reject stale generated topology, including restored-mtime transitions."""
    root = root.resolve()
    name = snapshot.relative_to(root).as_posix()
    check_deadline()
    with closing(PathWatch({snapshot})) as watch:
        content = read_file(root, name, max_bytes=_LIMIT)
        if hashlib.sha256(content).hexdigest() != expected_sha256:
            raise ValueError(
                "configured inventory differs from its build-graph pin; reconfigure"
            )
        try:
            document = json.loads(content, object_pairs_hook=unique_object)
        except (ValueError, RecursionError) as error:
            raise ValueError(
                "invalid configured build inventory; reconfigure"
            ) from error
        current = capture_inventory(root)
        if json.dumps(document, sort_keys=True, allow_nan=False) != json.dumps(
            current, sort_keys=True
        ):
            raise ValueError(
                "stale configured build inventory; reconfigure the build tree"
            )
        watch.validate()
        check_deadline()


def has_current_inventory(root: Path, snapshot: Path) -> bool:
    """Decide whether the frontend must regenerate its disposable configuration."""
    try:
        content = read_file(
            root, snapshot.relative_to(root).as_posix(), max_bytes=_LIMIT
        )
        check_inventory(root, snapshot, hashlib.sha256(content).hexdigest())
    except (FileNotFoundError, ValueError):
        return False
    return True
