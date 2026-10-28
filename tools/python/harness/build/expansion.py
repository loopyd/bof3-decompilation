"""Bind configured preprocessor output to metadata-owned function invocations."""

from __future__ import annotations

import ast
import re
import shutil
import tempfile
from dataclasses import dataclass
from pathlib import Path

from harness.build.arguments import read_source
from harness.build.compilation import configured_command
from harness.common.files import read_file
from harness.common.lexicon import mask_c_text
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.functions import FunctionRecord, parse_function_records
from harness.domain.sources import compiled_symbol_name, owning_manifest

_LIMIT = 64 * 1024 * 1024
_MARKER = re.compile(r'^\s*#\s*(?:line\s+)?(\d+)\s+("(?:\\.|[^"\\])*")(?:\s+\d+)*\s*$')


@dataclass(frozen=True)
class ExpandedFunction:
    """One configured expansion attributed to its authored implementation span."""

    address: int
    symbol: str
    implementation: str
    source: str
    source_line: int
    invocation: str


def _source_lines(root: Path, source: Path, expanded: str) -> dict[int, list[str]]:
    lines: dict[int, list[str]] = {}
    current = None
    number = 0
    for line in expanded.splitlines(keepends=True):
        marker = _MARKER.fullmatch(line.rstrip("\r\n"))
        if marker:
            try:
                spelling = ast.literal_eval(marker[2])
            except (SyntaxError, ValueError) as error:
                raise ValueError(
                    "invalid filename in configured source marker"
                ) from error
            current = Path(spelling)
            if not current.is_absolute():
                current = root / current
            current = current.resolve()
            number = int(marker[1])
            continue
        if line.lstrip().startswith("#"):
            raise ValueError("unsupported directive in configured preprocessing output")
        if current == source:
            lines.setdefault(number, []).append(line)
        elif current is None and line.strip():
            raise ValueError("configured preprocessing output lacks source markers")
        number += 1
    return lines


def _expanded_function(fragment: str, record: FunctionRecord) -> tuple[str, str]:
    prefix = f"/* @source 0x{record.address:08X}\n * @behavior Expanded native member.\n */\n"
    text = prefix + fragment
    records = parse_function_records(text, validate_progress=False)
    if len(records) != 1 or records[0].kind != "function":
        raise ValueError("each function invocation must emit exactly one function")
    expanded = records[0]
    if mask_c_text(text[expanded.implementation_end :]).strip().strip(";").strip():
        raise ValueError(
            "function invocation emits extra implementation or declarations"
        )
    return (
        expanded.spelling,
        text[expanded.implementation_start : expanded.implementation_end],
    )


def bind_expanded_members(
    root: Path, source: Path, preprocessed_text: str
) -> dict[int, ExpandedFunction]:
    """Require one reviewed emitted function for each authored metadata record."""
    root, source = root.resolve(), source.resolve()
    if len(preprocessed_text) > _LIMIT:
        raise ValueError("configured preprocessing output exceeds the size limit")
    text = read_source(source)
    records = parse_function_records(text, validate_progress=False)
    manifest = owning_manifest(root, source)
    if not records or manifest is None:
        raise ValueError(
            "expanded functions require metadata and explicit target ownership"
        )
    lines = _source_lines(root, source, preprocessed_text)
    members = {}
    for record in records:
        last_line = text.count("\n", 0, record.implementation_end - 1) + 1
        fragment = "".join(
            line
            for number in range(record.line, last_line + 1)
            for line in lines.get(number, ())
        )
        try:
            symbol, implementation = _expanded_function(fragment, record)
            expected = compiled_symbol_name(
                root, source, record.address, manifest=manifest
            )
            if symbol != expected:
                raise ValueError(
                    f"emitted function {symbol!r} differs from {expected!r}"
                )
        except ValueError as error:
            raise ValueError(
                f"invalid expansion at {source.relative_to(root)}:{record.line}: {error}"
            ) from error
        members[record.address] = ExpandedFunction(
            record.address,
            symbol,
            implementation,
            source.relative_to(root).as_posix(),
            record.line,
            text[record.implementation_start : record.implementation_end],
        )
    return members


def preprocess_source(root: Path, source: Path) -> str:
    """Preprocess with the current configured driver and its normal input guards."""
    root, source = root.resolve(), source.resolve()
    directory = root / "out"
    directory.mkdir(exist_ok=True)
    temporary = Path(tempfile.mkdtemp(prefix="harness-expand-", dir=directory))
    cleanup = True
    try:
        output = temporary / "source.i"
        arguments, environment = configured_command(
            root, source, output, allow_grouped=True, preprocess=True
        )
        result = run_bounded(root, arguments, env=environment, output_limit=_LIMIT)
        if result["failure"] or result["exit_code"] != 0:
            raise RuntimeError(
                f"configured preprocessing failed: {result['failure'] or result['stderr']}"
            )
        return read_file(output.parent, output.name, max_bytes=_LIMIT).decode("utf-8")
    except ProcessCleanupError as error:
        cleanup = False
        error.add_note(f"Retained configured preprocessing output: {temporary}")
        raise
    finally:
        if cleanup:
            shutil.rmtree(temporary)


def expand_function_members(root: Path, source: Path) -> dict[int, ExpandedFunction]:
    """Return fresh configured implementations for naming or native validation."""
    root, source = root.resolve(), source.resolve()
    before = read_source(source)
    expanded = preprocess_source(root, source)
    members = bind_expanded_members(root, source, expanded)
    if read_source(source) != before:
        raise ValueError("source changed during configured expansion")
    return members
