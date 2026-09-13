"""Spelling-only data identities and their exact, directly owned consumers."""

from __future__ import annotations

import hashlib
import re
import stat
from pathlib import Path
from typing import Any

from harness.common.lexicon import iter_c_lexemes
from harness.domain.claims import manifest_header_paths, manifest_source_paths
from harness.domain.functions import parse_function_records
from harness.domain.layout import parse_splat_layout
from harness.domain.registry import resolve_function
from harness.naming.context import TargetContext
from harness.naming.debt import address_of
from harness.naming.readiness import canonical_storage


def _digest(content: bytes) -> str:
    return hashlib.sha256(content).hexdigest()


def _compile_name(name: str) -> re.Pattern[str]:
    return re.compile(r"\b" + re.escape(name) + r"\b")


def _mask_code(text: str, old: str) -> str:
    spliced = re.sub(r"\\\r?\n", "", text)
    renamed_then_spliced = re.sub(r"\\\r?\n", "", _compile_name(old).sub("0", text))
    if renamed_then_spliced != _compile_name(old).sub("0", spliced):
        raise ValueError("continued data inputs require preprocessing review")
    text = spliced
    masked = list(text)
    for match in iter_c_lexemes(text):
        if not match.group().startswith(("/*", "//")) and _compile_name(old).search(
            match.group()
        ):
            raise ValueError("data spelling in a literal requires separate review")
        masked[match.start() : match.end()] = [
            "\n" if character == "\n" else " " for character in match.group()
        ]
    result = "".join(masked)
    if re.search(
        r"^[ \t]*#\s*define\s+(?:if|while|for|switch|sizeof|return)\b",
        result,
        re.MULTILINE,
    ):
        raise ValueError("keyword macros require preprocessing review")
    if re.search(r"^[ \t]*#.*\b" + re.escape(old) + r"\b", result, re.MULTILINE):
        raise ValueError("macro-mediated data identity is unsupported")
    return result


def _validate_direct_uses(code: str, old: str) -> None:
    frames: list[bool] = []
    previous = ""
    for token in re.findall(r"[A-Za-z_][A-Za-z_0-9]*|[^\s]", code):
        if token == "(":
            frames.append(
                previous in {")", "]"}
                or (
                    bool(re.fullmatch(r"[A-Za-z_]\w*", previous))
                    and previous
                    not in {"if", "while", "for", "switch", "sizeof", "return"}
                )
            )
        elif token == ")" and frames:
            frames.pop()
        elif token == old and any(frames):
            raise ValueError("data in call arguments requires macro-expansion review")
        previous = token


def _collect_consumers(
    context: TargetContext, images: dict[str, bytes], old: str
) -> list[dict]:
    result = []
    layout = parse_splat_layout(
        context.root / context.manifest.splat, context.manifest.load_address
    )
    support = set(context.manifest.support_sources)
    headers = set(context.manifest.headers)
    for relative in sorted(set(context.manifest.sources) | support | headers):
        text = images[relative].decode("utf-8")
        code = _mask_code(text, old)
        uses = list(_compile_name(old).finditer(code))
        if not uses:
            continue
        if relative in headers:
            _validate_direct_uses(code, old)
            declarations = list(
                re.finditer(
                    r"^[ \t]*extern\s+(?:[A-Za-z_]\w*\s+|\*\s*)+"
                    + re.escape(old)
                    + r"\s*(?:\[[^\[\]();={}]*\]\s*)*;",
                    code,
                    re.MULTILINE,
                )
            )
            declarations = [
                item
                for item in declarations
                if len(_compile_name(old).findall(item.group())) == 1
            ]
            if any(
                not any(
                    item.start() <= use.start() < item.end() for item in declarations
                )
                for use in uses
            ):
                raise ValueError("data header uses must be extern declarations")
            continue
        if relative in support:
            bindings = list(
                re.finditer(
                    r"^[ \t]*WEAK_SYMBOL_AT\s*\(\s*"
                    + re.escape(old)
                    + r"\s*,\s*(0[xX][0-9a-fA-F]+|[0-9]+)\s*\)\s*;?[ \t]*$",
                    code,
                    re.MULTILINE,
                )
            )
            bindings = [
                item for item in bindings if int(item.group(1), 0) == address_of(old)
            ]
            if any(
                not any(item.start() <= use.start() < item.end() for item in bindings)
                for use in uses
            ):
                raise ValueError("data support uses must be literal symbol bindings")
            continue
        records = parse_function_records(re.sub(r"\\\r?\n", "", text))
        if any(
            not any(
                record.implementation_start <= use.start() < record.implementation_end
                for record in records
            )
            for use in uses
        ):
            raise ValueError("data uses outside owned implementations are unsupported")
        for record in records:
            body = code[record.implementation_start : record.implementation_end]
            if not _compile_name(old).search(body):
                continue
            _validate_direct_uses(body, old)
            if record.status != "exact":
                raise ValueError("data proposals require exact consumer sources")
            selector = f"{context.target}@0x{record.address:08X}"
            resolved = resolve_function(context.root, selector)
            if (
                resolved.source != context.root / relative
                or resolved.compiled_symbol != record.spelling
            ):
                raise ValueError("data consumer ownership is ambiguous")
            boundary = layout.find_boundary_at(record.address)
            if (
                boundary is None
                or not boundary.is_function
                or not boundary.virtual_size
            ):
                raise ValueError("data consumer requires a reviewed function range")
            result.append(
                {
                    "selector": selector,
                    "name": record.spelling,
                    "address": f"0x{record.address:08X}",
                    "source": relative,
                    "size": boundary.virtual_size,
                }
            )
    if not result:
        raise ValueError("data proposals require at least one exact direct consumer")
    return result


def _collect_paths(context: TargetContext, scope: dict) -> set[Path]:
    paths = set(manifest_source_paths(context.root, context.manifest))
    paths.update(manifest_header_paths(context.root, context.manifest))
    paths.update(context.root / scope[key] for key in ("map", "manifest"))
    paths.add(context.root / context.manifest.splat)
    return paths


def capture_data(context: TargetContext, row: dict, scope: dict) -> dict[str, Any]:
    old, new = row["name"], row["new_name"]
    if scope["cross_target_locations"]:
        raise ValueError("data proposals require exclusive target ownership")
    paths = _collect_paths(context, scope)
    renamed = set(scope["binding_locations"]) | set(scope["source_locations"])
    if scope["definition"] is not None or not renamed <= {
        path.relative_to(context.root).as_posix() for path in paths
    }:
        raise ValueError("data scope must be target-owned and cannot move a definition")
    images, files = {}, {}
    for path in sorted(paths):
        if (
            path.is_symlink()
            or not path.resolve().is_relative_to(context.root.resolve())
            or not path.is_file()
        ):
            raise ValueError("data inputs must be regular owned files")
        relative = path.relative_to(context.root).as_posix()
        content = path.read_bytes()
        text = content.decode("utf-8")
        if _compile_name(new).search(re.sub(r"\\\r?\n", "", text)):
            raise ValueError(
                "data destination spelling already occurs in target inputs"
            )
        replacement = (
            _compile_name(old).sub(new, text).encode("utf-8")
            if relative in renamed
            else content
        )
        images[relative] = content
        files[relative] = {
            "before": _digest(content),
            "after": _digest(replacement),
            "mode": stat.S_IMODE(path.stat().st_mode),
        }
    return {"files": files, "consumers": _collect_consumers(context, images, old)}


def validate_data_shape(data: object) -> dict[str, Any]:
    if not isinstance(data, dict) or set(data) != {"files", "consumers"}:
        raise ValueError("data transaction facts have unsupported shape")
    if (
        not isinstance(data["files"], dict)
        or not data["files"]
        or not isinstance(data["consumers"], list)
        or not data["consumers"]
    ):
        raise ValueError("data transaction needs files and consumers")
    for relative, item in data["files"].items():
        path = Path(relative)
        if (
            path.is_absolute()
            or path.as_posix() != relative
            or ".." in path.parts
            or not isinstance(item, dict)
            or set(item) != {"before", "after", "mode"}
        ):
            raise ValueError("invalid data file record")
        if (
            any(
                not isinstance(item[key], str)
                or not re.fullmatch(r"[0-9a-f]{64}", item[key])
                for key in ("before", "after")
            )
            or type(item["mode"]) is not int
            or not 0 <= item["mode"] <= 0o7777
        ):
            raise ValueError("invalid data file hash or mode")
    selectors = []
    for consumer in data["consumers"]:
        if not isinstance(consumer, dict) or set(consumer) != {
            "selector",
            "name",
            "address",
            "source",
            "size",
        }:
            raise ValueError("invalid data consumer record")
        if (
            any(
                not isinstance(consumer[key], str) or not consumer[key]
                for key in ("selector", "name", "address", "source")
            )
            or consumer["source"] not in data["files"]
            or type(consumer["size"]) is not int
            or consumer["size"] <= 0
        ):
            raise ValueError("invalid data consumer identity")
        selectors.append(consumer["selector"])
    if len(set(selectors)) != len(selectors):
        raise ValueError("duplicate data consumer")
    return data


def validate_data_preservation(
    context: TargetContext, row: dict, facts: dict, *, post_apply: bool
) -> None:
    data = validate_data_shape(facts.get("data"))
    if set(data["files"]) != {
        path.relative_to(context.root).as_posix()
        for path in _collect_paths(context, facts["scope"])
    }:
        raise ValueError("data inputs do not cover complete target ownership")
    images = {}
    for relative, item in data["files"].items():
        path = context.root / relative
        if (
            path.is_symlink()
            or not path.resolve().is_relative_to(context.root.resolve())
            or not path.is_file()
        ):
            raise ValueError("data input disappeared or escaped ownership")
        content = path.read_bytes()
        if (
            _digest(content) != item["after" if post_apply else "before"]
            or stat.S_IMODE(path.stat().st_mode) != item["mode"]
        ):
            raise ValueError(f"data representation or scope drift: {relative}")
        if post_apply and item["before"] != item["after"]:
            content = (
                _compile_name(row["new_name"])
                .sub(row["name"], content.decode("utf-8"))
                .encode("utf-8")
            )
        if _digest(content) != item["before"]:
            raise ValueError("data postimage is not a reversible spelling-only rename")
        images[relative] = content
    if _collect_consumers(context, images, row["name"]) != data["consumers"]:
        raise ValueError("data consumer set or identity changed")
    if (
        canonical_storage(context.root, context.target, int(facts["address"], 16))
        != facts["storage"]
    ):
        raise ValueError("data storage changed")
