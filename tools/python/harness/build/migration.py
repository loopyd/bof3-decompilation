"""Plan and validate scoped legacy compiler-key migration without writing files."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

from harness.build.compiler import (
    CONFIGURATION_PATH,
    DEFAULT_COMPILER_ID,
    OBJECT_FLAGS_RE,
    OBJCOMPILER_RE,
    CompilerSettings,
    parse_compiler_configuration,
    sanitize_identifier,
    validate_compiler_flags,
)
from harness.common.deadlines import check_deadline
from harness.common.inputs import relative


def read_migration_settings(value: object) -> CompilerSettings:
    """Require normalized compiler identity and literal ordered override tokens."""
    if not isinstance(value, dict) or set(value) != {"compiler_id", "flags"}:
        raise ValueError("migration requires exact compiler settings")
    compiler_id, flags = value["compiler_id"], value["flags"]
    if compiler_id is not None and (
        not isinstance(compiler_id, str)
        or not re.fullmatch(r"[A-Za-z0-9._-]+", compiler_id)
        or compiler_id == DEFAULT_COMPILER_ID
    ):
        raise ValueError("migration compiler identity is not normalized")
    if flags is not None and (
        not isinstance(flags, list)
        or not flags
        or any(
            not isinstance(flag, str)
            or not re.fullmatch(r"-[A-Za-z0-9_=+.,:/-]+", flag)
            for flag in flags
        )
    ):
        raise ValueError("migration flags must be ordered literal tokens")
    selected = tuple(flags) if flags is not None else None
    validate_compiler_flags(selected)
    return CompilerSettings(compiler_id, selected)


def _collect_source_keys(sources: list[str], destination: str) -> dict[str, str]:
    if not 2 <= len(sources) <= 32 or len(set(sources)) != len(sources):
        raise ValueError("migration requires two to 32 distinct sources")
    keys = {}
    for name in sorted({*sources, destination}):
        if (
            relative(name) != name
            or not name.startswith("src/bof3/")
            or not name.endswith(".c")
        ):
            raise ValueError("migration requires canonical C source paths")
        keys[name] = sanitize_identifier(Path(name).relative_to("src").as_posix())
    if len(set(keys.values())) != len(keys):
        raise ValueError("migration has ambiguous sanitized compiler keys")
    return keys


def migrate_configuration(
    text: str, sources: list[str], destination: str, settings: CompilerSettings
) -> str:
    """Render only selected legacy-key changes, preserving unrelated input bytes."""
    check_deadline()
    flags, compilers = parse_compiler_configuration(text)
    keys = _collect_source_keys(sources, destination)
    owned = set(keys.values())
    if not owned.intersection({*flags, *compilers}):
        return text
    desired = {
        "flags": " ".join(settings.flags) if settings.flags is not None else None,
        "compiler": settings.compiler_id,
    }
    settings = read_migration_settings(
        {
            "compiler_id": settings.compiler_id,
            "flags": list(settings.flags) if settings.flags is not None else None,
        }
    )
    destination_key = keys[destination]
    superseded = {keys[source] for source in sources if source != destination}
    prefixes = {"flags": "BOF3_OBJFLAGS_", "compiler": "BOF3_OBJCOMPILER_"}
    seen = set()
    result = []
    for line in text.splitlines(keepends=True):
        check_deadline()
        match = OBJECT_FLAGS_RE.fullmatch(line.strip())
        kind = "flags"
        if match is None:
            match = OBJCOMPILER_RE.fullmatch(line.strip())
            kind = "compiler"
        if match is None or match[1] not in owned:
            result.append(line)
            continue
        if match[1] in superseded:
            continue
        seen.add(kind)
        value = desired[kind]
        if value is None:
            continue
        actual = (
            " ".join(flags[destination_key])
            if kind == "flags"
            else compilers[destination_key]
        )
        if actual == value:
            result.append(line)
        else:
            ending = (
                "\r\n" if line.endswith("\r\n") else "\n" if line.endswith("\n") else ""
            )
            result.append(f"set({prefixes[kind]}{destination_key} {value}){ending}")
    updated = "".join(result)
    additions = [
        f"set({prefixes[kind]}{destination_key} {desired[kind]})\n"
        for kind in ("flags", "compiler")
        if kind not in seen and desired[kind] is not None
    ]
    if additions:
        if updated and not updated.endswith("\n"):
            updated += "\n"
        updated += "".join(additions)
    parse_compiler_configuration(updated)
    check_deadline()
    return updated


def plan_configuration(
    before: str | None,
    sources: list[str],
    destination: str,
    settings: CompilerSettings,
) -> dict | None:
    after = migrate_configuration(before or "", sources, destination, settings)
    if after == (before or ""):
        return None
    return {
        "before": before,
        "after": after,
        "settings": {
            "compiler_id": settings.compiler_id,
            "flags": list(settings.flags) if settings.flags is not None else None,
        },
    }


def validate_configuration_migration(
    value: object, sources: list[str], destination: str, inputs: dict, post: dict
) -> dict | None:
    """Bind the exact scoped edit to authenticated PRE and joint POST states."""
    check_deadline()
    if value is None:
        return None
    if not isinstance(value, dict) or set(value) != {"before", "after", "settings"}:
        raise ValueError("invalid compiler configuration migration")
    before, after = value["before"], value["after"]
    if (before is not None and not isinstance(before, str)) or not isinstance(
        after, str
    ):
        raise ValueError("configuration migration requires UTF-8 text")
    if CONFIGURATION_PATH not in inputs:
        raise ValueError("configuration migration lacks a PRE input")
    state = inputs[CONFIGURATION_PATH]
    if (before is None) != (state is None) or (
        before is not None
        and hashlib.sha256(before.encode("utf-8")).hexdigest() != state["sha256"]
    ):
        raise ValueError("PRE compiler configuration text differs from its input pin")
    settings = read_migration_settings(value["settings"])
    if plan_configuration(before, sources, destination, settings) != value:
        raise ValueError(
            "configuration migration changes more than selected compiler keys"
        )
    expected = post.get(CONFIGURATION_PATH)
    if (
        expected is None
        or expected["sha256"] != hashlib.sha256(after.encode("utf-8")).hexdigest()
    ):
        raise ValueError("POST must pin the exact compiler configuration migration")
    if state is not None and expected["mode"] != state["mode"]:
        raise ValueError("configuration migration must preserve the existing file mode")
    check_deadline()
    return value
