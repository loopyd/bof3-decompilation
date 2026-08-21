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

from ..analysis.type_context import type_context
from ..domain.c_context import public_declaration_context
from ..domain import FunctionId, TargetManifest, resolve_function
from ..domain.sources import CompiledSymbolError
from ..io import RepoLayout
from ..domain.layout import parse_splat_layout
from ..match._asm_resolve import extract_original_bytes, infer_original_size

DEFAULT_API_URL = "https://decomp.me/api"
DEFAULT_SITE_URL = "https://decomp.me"
_USER_AGENT = "Mozilla/5.0 (compatible; rebof3-scratchpad/1.0)"
_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


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


def _source_arguments(layout: RepoLayout, source: Path) -> list[str]:
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
    return arguments


def _decompme_compiler_flags(arguments: list[str]) -> str:
    assembler_flags = [flag for flag in arguments if flag == "-Wa,--expand-div"]
    return " ".join(
        ["-O2", "-G0", "-funsigned-char", "-msoft-float", "-gcoff", *assembler_flags]
    )


def _source_command(layout: RepoLayout, source: Path) -> list[str]:
    command: list[str] = []
    arguments = _source_arguments(layout, source)
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


def _target_assembly(
    layout: RepoLayout,
    function: FunctionId,
    manifest: TargetManifest,
    source: Path,
    name: str,
) -> str:
    boundary = parse_splat_layout(
        layout.root / manifest.splat, manifest.load_address
    ).find_boundary_at(function.address)
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
        raise ValueError(
            f"unaligned original range for {function.target.value}@0x{function.address:08X}"
        )
    words = "\n".join(
        f"    .word 0x{int.from_bytes(raw[offset : offset + 4], 'little'):08X}"
        for offset in range(0, len(raw), 4)
    )
    return f".text\nglabel {name}\n{words}\n"


class DecompMeScratchpadToolchain:
    """Small API client; it does not participate in `just setup` lifecycle."""

    label = "decomp.me scratchpad"

    def __init__(self, layout: RepoLayout) -> None:
        self.layout = layout

    def payload(self, function: FunctionId, *, compiler: str) -> ScratchpadPayload:
        resolved = resolve_function(self.layout.root, function)
        manifest = resolved.manifest
        source = resolved.source
        if resolved.compiled_symbol is None:
            raise CompiledSymbolError(
                None, function.address, "no target-local reviewed function symbol"
            )
        name = resolved.compiled_symbol
        if source is None:
            raise FileNotFoundError(
                f"lifted source does not exist for {function.target.value}@0x{function.address:08X}; "
                "a lift source must carry '@source' and '@behavior' metadata"
            )
        # Macro-expanded source avoids includes. Resolve every referenced
        # declaration and its type dependencies from the complete preprocessor
        # stream, including SDK headers required to compile the scratch.
        preprocessed_context, source_code = _preprocess_source(self.layout, source)
        registry_context = type_context(
            self.layout.root,
            manifest.id.value,
            source_code,
        )
        context = public_declaration_context(
            preprocessed_context,
            source_code,
            base=registry_context,
        )
        compiler_flags = _decompme_compiler_flags(
            _source_arguments(self.layout, source)
        )
        return ScratchpadPayload(
            name=name,
            platform="ps1",
            compiler=_remote_compiler_id(compiler),
            compiler_flags=compiler_flags,
            diff_label=name,
            target_asm=_target_assembly(self.layout, function, manifest, source, name),
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
