"""decomp.me scratch publishing from one target-qualified BOF3 lift."""

from __future__ import annotations

import json
import re
import subprocess
import urllib.error
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..c_context import public_declaration_context
from ..domain import FunctionId, TargetManifest, load_target_manifests
from ..io import RepoLayout
from ..layout import parse_splat_layout
from ..match._asm_resolve import (
    extract_original_bytes,
    infer_original_size,
    source_function_name,
)

DEFAULT_API_URL = "https://decomp.me/api"
DEFAULT_SITE_URL = "https://decomp.me"
_USER_AGENT = "Mozilla/5.0 (compatible; rebof3-scratchpad/1.0)"
_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
_LINE_MARKER = re.compile(r'^#\s+\d+\s+"(?P<path>[^"]+)"')
_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.DOTALL)
_STRING = re.compile(r'"(?:\\.|[^"\\])*"')
_LOCAL_DECLARATION = re.compile(
    r"\b[A-Za-z_][A-Za-z0-9_]*\s*\*?\s+([A-Za-z_][A-Za-z0-9_]*)"
)
_PIN_DECLARATION = re.compile(
    r"\bREGISTER_PIN\s*\(\s*[^,]+,\s*([A-Za-z_][A-Za-z0-9_]*)"
)
_C_KEYWORDS = frozenset(
    "auto break case char const continue default do double else enum extern float for "
    "goto if int long register return short signed sizeof static struct switch "
    "typedef union unsigned void volatile while".split()
)
_BASE_CONTEXT = """typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
"""


@dataclass(frozen=True)
class ScratchpadPayload:
    name: str
    platform: str
    compiler: str
    compiler_flags: str
    diff_label: str
    target_asm: str
    context: str
    source_code: str

    def as_api_data(self) -> dict[str, Any]:
        return {
            "name": self.name,
            "platform": self.platform,
            "compiler": self.compiler,
            "compiler_flags": self.compiler_flags,
            "diff_label": self.diff_label,
            "target_asm": self.target_asm,
            "context": self.context,
            "source_code": self.source_code,
        }


def _remote_compiler_id(local_id: str) -> str:
    """Translate this repository's catalog spelling to decomp.me's PS1 ID."""
    if not re.fullmatch(r"gcc-\d+(?:\.\d+)+-psx", local_id):
        raise ValueError(f"unsupported decomp.me compiler ID: {local_id}")
    return local_id.replace("gcc-", "gcc", 1)


def _source_command(layout: RepoLayout, source: Path) -> list[str]:
    database = layout.root / "compile_commands.json"
    if not database.is_file():
        raise FileNotFoundError(f"missing {database}; run `just build` first")
    rows = json.loads(database.read_text(encoding="utf-8"))
    matches = [
        row for row in rows if Path(row.get("file", "")).resolve() == source.resolve()
    ]
    if len(matches) != 1:
        raise ValueError(
            f"expected 1 compile command for {source}, found {len(matches)}"
        )
    arguments = list(matches[0].get("arguments", []))
    if not arguments:
        raise ValueError(f"compile command has no arguments for {source}")

    command: list[str] = []
    skip = False
    for argument in arguments:
        if skip:
            skip = False
            continue
        if argument == "-c":
            continue
        if argument == "-o":
            skip = True
            continue
        if argument.startswith("-o") and len(argument) > 2:
            continue
        if argument.startswith("-Wa,"):
            continue
        command.append(argument)
    return command


def _preprocess_source(layout: RepoLayout, source: Path) -> tuple[str, str]:
    command = _source_command(layout, source)
    result = subprocess.run(
        [*command, "-E"],
        cwd=layout.root,
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode:
        raise RuntimeError(f"preprocessing {source} failed: {result.stderr.strip()}")

    source_marker = f'# 1 "{source}" 2'
    lines = result.stdout.splitlines()
    try:
        marker = max(index for index, line in enumerate(lines) if line == source_marker)
    except ValueError as exc:
        raise ValueError(f"cannot isolate preprocessed source for {source}") from exc

    def without_directives(rows: list[str]) -> str:
        return "\n".join(line for line in rows if not line.startswith("#")) + "\n"

    return "\n".join(lines[:marker]) + "\n", without_directives(lines[marker + 1 :])


def _private_identifiers(preprocessed: str) -> set[str]:
    """Return declarations supplied by ignored PsyQ headers only."""
    private = False
    identifiers: set[str] = set()
    for line in preprocessed.splitlines():
        marker = _LINE_MARKER.match(line)
        if marker is not None:
            private = "/toolchains/psyq/" in marker.group("path")
        elif private:
            identifiers.update(_IDENTIFIER.findall(line))
    return identifiers - _C_KEYWORDS


def _is_reviewed_name_at(root: Path, manifest, function: FunctionId) -> bool:
    """True when the target map binds a semantic name at the function address."""

    from ..canonical import load_target_symbols

    return any(
        symbol.address == function.address
        for symbol in load_target_symbols(root, manifest.id.value)
    )


def _require_reviewed_function_boundary(
    layout: RepoLayout, function: FunctionId, manifest: TargetManifest
) -> None:
    boundary = parse_splat_layout(
        layout.root / manifest.splat, manifest.load_address
    ).boundary_starting_at(function.address)
    if (
        boundary is None
        or not boundary.is_function
        or not (
            boundary.function_name == f"func_{function.address:08X}"
            or _is_reviewed_name_at(layout.root, manifest, function)
        )
    ):
        raise ValueError(
            f"not a reviewed function boundary: "
            f"{function.target.value}@0x{function.address:08X}"
        )


def _target_assembly(layout: RepoLayout, function: FunctionId, source: Path) -> str:
    manifest = load_target_manifests(layout.root).get(function.target.value)
    if manifest is None:
        raise ValueError(f"unknown target: {function.target.value}")
    _require_reviewed_function_boundary(layout, function, manifest)

    boundary = parse_splat_layout(
        layout.root / manifest.splat, manifest.load_address
    ).boundary_starting_at(function.address)
    asm_name = (
        boundary.name
        if boundary is not None and boundary.name is not None
        else f"func_{function.address:08X}"
    )
    path = layout.out_dir / "splat" / manifest.id.value / "asm" / f"{asm_name}.s"
    if path.is_file():
        lines = path.read_text(encoding="utf-8").splitlines()
        kept = [
            line
            for line in lines
            if not line.startswith((".include", ".set ", "/*", "nonmatching "))
        ]
        while kept and not kept[0].strip():
            kept.pop(0)
        return ".text\n" + "\n".join(kept) + "\n"

    binary = layout.root / manifest.binary
    size = infer_original_size(
        source,
        address=function.address,
        binary_path=binary,
        load_address=manifest.load_address,
        root=layout.root,
    )
    raw = extract_original_bytes(
        binary,
        address=function.address,
        size=size,
        load_address=manifest.load_address,
    )
    if len(raw) % 4:
        raise ValueError(f"unaligned original range for {function.target.value}@0x{function.address:08X}")
    words = "\n".join(
        f"    .word 0x{int.from_bytes(raw[offset:offset + 4], 'little'):08X}"
        for offset in range(0, len(raw), 4)
    )
    return f".text\nglabel func_{function.address:08X}\n{words}\n"


class DecompMeScratchpadToolchain:
    """Small API client; it does not participate in `just setup` lifecycle."""

    label = "decomp.me scratchpad"

    def __init__(self, layout: RepoLayout) -> None:
        self.layout = layout

    def payload(self, function: FunctionId, *, compiler: str) -> ScratchpadPayload:
        manifest = load_target_manifests(self.layout.root).get(function.target.value)
        if manifest is None:
            raise ValueError(f"unknown target: {function.target.value}")
        _require_reviewed_function_boundary(self.layout, function, manifest)
        from ..domain.claims import resolve_manifest_source_for_address

        source = resolve_manifest_source_for_address(
            self.layout.root, manifest, function.address
        )
        if source is None:
            raise FileNotFoundError(
                f"lifted source does not exist for {function.target.value}@0x{function.address:08X}; "
                "a lift source must carry '@source' and '@behavior' metadata"
            )
        # Macro-expanded source avoids target-local includes. Only retain
        # declarations it references, never the full (possibly ignored PsyQ)
        # preprocessor context.
        preprocessed_context, source_code = _preprocess_source(self.layout, source)
        private = _private_identifiers(preprocessed_context)
        local_text = _STRING.sub(
            "", _COMMENT.sub("", source.read_text(encoding="utf-8"))
        )
        local_identifiers = {
            *(_LOCAL_DECLARATION.findall(local_text)),
            *(_PIN_DECLARATION.findall(local_text)),
        }
        referenced_private = sorted(
            (
                (set(_IDENTIFIER.findall(_STRING.sub("", source_code))) & private)
                - local_identifiers
            )
            - _C_KEYWORDS
        )
        if referenced_private:
            raise ValueError(
                "cannot publish source that references ignored PsyQ declarations: "
                + ", ".join(referenced_private[:5])
            )
        context = public_declaration_context(
            preprocessed_context, source_code, base=_BASE_CONTEXT
        )
        context_private = set(_IDENTIFIER.findall(context)) & private
        if context_private:
            raise ValueError(
                "cannot publish context that references ignored PsyQ declarations: "
                + ", ".join(sorted(context_private)[:5])
            )
        name = source_function_name(source, function.address, self.layout.root)
        return ScratchpadPayload(
            name=name,
            platform="ps1",
            compiler=_remote_compiler_id(compiler),
            compiler_flags="-O2 -G0 -funsigned-char -msoft-float -gcoff",
            diff_label=name,
            target_asm=_target_assembly(self.layout, function, source),
            context=context,
            source_code=source_code,
        )

    def publish(self, payload: ScratchpadPayload) -> str:
        request = urllib.request.Request(
            f"{DEFAULT_API_URL}/scratch",
            data=json.dumps(payload.as_api_data()).encode("utf-8"),
            headers={
                "Accept": "application/json",
                "Content-Type": "application/json",
                "User-Agent": _USER_AGENT,
            },
            method="POST",
        )
        try:
            with urllib.request.urlopen(request, timeout=30) as response:
                body = response.read().decode("utf-8")
        except urllib.error.HTTPError as exc:
            detail = exc.read().decode("utf-8", errors="replace")[:1000]
            raise RuntimeError(f"decomp.me returned HTTP {exc.code}: {detail}") from exc
        except (OSError, UnicodeDecodeError) as exc:
            raise RuntimeError(f"cannot reach decomp.me: {exc}") from exc
        try:
            response_data = json.loads(body)
            slug = response_data["slug"]
        except (TypeError, KeyError, json.JSONDecodeError) as exc:
            raise RuntimeError("decomp.me returned no scratch slug") from exc
        if not isinstance(slug, str) or not re.fullmatch(r"[A-Za-z0-9]+", slug):
            raise RuntimeError("decomp.me returned an invalid scratch slug")
        return f"{DEFAULT_SITE_URL}/scratch/{slug}"
