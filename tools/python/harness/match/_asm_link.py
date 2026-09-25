"""Link selected function objects and compare complete symbol-qualified bytes."""

from __future__ import annotations

import re
import subprocess
from collections.abc import Mapping
from pathlib import Path

from ..domain.manifests import TargetManifest
from ..domain.symbols import load_target_symbols, load_weak_symbol_bindings
from ..io import RepoLayout, repo_layout
from .extraction import _HEADER, _SECTION, Section, read_function_image

# Raw address-encoding names are exactly `func_XXXXXXXX`/`D_XXXXXXXX`;
# overlay-prefixed variants (`SCENA16_D_*`) are banned — conflicts resolve by
# a different name or a suffix, never an overlay-name prefix.
_HEX_SUFFIX_RE = re.compile(r"^(?:func|D)_([0-9a-fA-F]{8})$")


def _target_map_bindings(
    repo: RepoLayout, symbols_c_path: Path, *, manifest: TargetManifest | None = None
) -> dict[str, int]:
    """Load the canonical map only for the source target owning this link."""

    if manifest is not None:
        target = manifest.id.value
        space = manifest.psyq_space
    else:
        try:
            target = symbols_c_path.parent.relative_to(repo.root / "src").as_posix()
        except ValueError:
            return {}
        space = None
    return {
        symbol.canonical_name: symbol.address
        for symbol in load_target_symbols(repo.root, target, psyq_space=space)
    }


def resolve_symbol_address(
    name: str,
    *,
    symbols_c_path: Path,
    canonical_bindings: Mapping[str, int] | None = None,
) -> int | None:
    m = _HEX_SUFFIX_RE.search(name)
    if m is not None:
        return int(m.group(1), 16)
    if canonical_bindings is not None and name in canonical_bindings:
        return canonical_bindings[name]
    return load_weak_symbol_bindings(symbols_c_path).get(name)


def _object_section_alignments(content: bytes) -> dict[str, int]:
    """Read a relocatable object's per-section required alignments."""
    header = _HEADER.unpack_from(content)
    section_offset, entry_size, count, names_index = (
        header[6],
        header[11],
        header[12],
        header[13],
    )
    if (
        entry_size != _SECTION.size
        or not 0 < count < 0xFF00
        or not 0 < names_index < count
        or section_offset + count * entry_size > len(content)
    ):
        raise ValueError("unsupported object section table layout")
    table = content[section_offset : section_offset + count * entry_size]
    sections = [Section(*values) for values in _SECTION.iter_unpack(table)]
    names = sections[names_index]
    strings = content[names.offset : names.offset + names.size]
    alignments: dict[str, int] = {}
    for section in sections:
        end = strings.find(b"\0", section.name_offset)
        if end < 0:
            continue
        name = strings[section.name_offset : end].decode("ascii", "replace")
        alignments[name] = section.alignment
    return alignments


def _exactly_placeable_object(
    object_path: Path,
    section_addresses: Mapping[str, int] | None,
    *,
    layout: RepoLayout,
) -> Path:
    """Return an object whose placed sections can honor the reviewed addresses.

    A compiler-emitted table can require 8-byte alignment while its original
    position is only 4-byte aligned (a 4-mod-8 jump table).  ``--section-start``
    cannot honor such an address: the linker front-pads the section and the
    table lands four bytes late, so the emitted ``lui``/``lw`` relocation no
    longer matches the original.  For each requested section whose address the
    object's current alignment cannot satisfy, lower that section's required
    alignment to the largest power of two dividing the reviewed address - the
    alignment the original object must have had - and link the adjusted copy.
    No adjustment is made when every address already satisfies its alignment.
    """
    if not section_addresses:
        return object_path
    current = _object_section_alignments(object_path.read_bytes())
    adjustments: dict[str, int] = {}
    for section, section_address in section_addresses.items():
        alignment = current.get(section, 0)
        if alignment == 0 or section_address % alignment == 0:
            continue
        reduced = section_address & -section_address
        while reduced > alignment:
            reduced //= 2
        if reduced:
            adjustments[section] = reduced
    if not adjustments:
        return object_path
    objcopy = layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objcopy"
    adjusted = object_path.with_suffix(".placed.o")
    command = [str(objcopy)]
    for section, alignment in sorted(adjustments.items()):
        command.append(f"--set-section-alignment={section}={alignment}")
    command.extend([str(object_path), str(adjusted)])
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"objcopy failed: {result.stderr}")
    return adjusted


def link_object_at_address(
    *,
    object_path: Path,
    address: int,
    undefined_symbols: list[str],
    symbols_c_path: Path | None = None,
    canonical_bindings: Mapping[str, int] | None = None,
    layout: RepoLayout | None = None,
    output_path: Path | None = None,
    section_addresses: dict[str, int] | None = None,
) -> Path:
    repo = layout or repo_layout()
    ld = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-ld"
    symbols_c = symbols_c_path or (repo.root / "src" / "boot" / "symbols.c")
    bindings = (
        dict(canonical_bindings)
        if canonical_bindings is not None
        else _target_map_bindings(repo, symbols_c)
    )
    defsym_args: list[str] = []
    for sym in undefined_symbols:
        addr = resolve_symbol_address(
            sym, symbols_c_path=symbols_c, canonical_bindings=bindings
        )
        if addr is not None:
            defsym_args.extend([f"--defsym={sym}={addr}"])
    out = output_path or object_path.with_suffix(".linked.o")
    linked_input = _exactly_placeable_object(
        object_path, section_addresses, layout=repo
    )
    result = subprocess.run(
        [
            str(ld),
            "-EL",
            f"-Ttext={address:#x}",
            *[
                f"--section-start={section}={section_address:#x}"
                for section, section_address in sorted(
                    (section_addresses or {}).items()
                )
            ],
            *defsym_args,
            str(linked_input),
            "-o",
            str(out),
        ],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"link failed: {result.stderr}")
    return out


def extract_section_bytes(
    linked_path: Path,
    *,
    section: str,
    layout: RepoLayout | None = None,
) -> bytes:
    repo = layout or repo_layout()
    objcopy = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objcopy"
    flat = subprocess.run(
        [str(objcopy), "-O", "binary", "-j", section, str(linked_path), "/dev/stdout"],
        capture_output=True,
    )
    if flat.returncode != 0:
        raise RuntimeError(f"objcopy failed: {flat.stderr.decode()}")
    return flat.stdout


def function_bytes_match(
    object_path: Path,
    *,
    function_name: str,
    address: int,
    size: int,
    original_bytes: bytes,
    symbols_c_path: Path | None = None,
    canonical_bindings: Mapping[str, int] | None = None,
    layout: RepoLayout | None = None,
    section_addresses: dict[str, int] | None = None,
) -> tuple[bool, bytes]:
    if type(size) is not int or size <= 0 or size % 4 or len(original_bytes) != size:
        raise ValueError(
            "byte comparison requires the complete aligned original extent"
        )
    repo = layout or repo_layout()
    nm = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-nm"
    nm_result = subprocess.run(
        [str(nm), "-u", str(object_path)], capture_output=True, text=True
    )
    if nm_result.returncode != 0:
        raise RuntimeError(f"nm failed: {nm_result.stderr}")
    undefined: list[str] = []
    for line in nm_result.stdout.splitlines():
        stripped = line.strip()
        if stripped.startswith("U "):
            undefined.append(stripped[2:].strip())
    linked = link_object_at_address(
        object_path=object_path,
        address=address,
        undefined_symbols=undefined,
        symbols_c_path=symbols_c_path,
        canonical_bindings=canonical_bindings,
        layout=repo,
        section_addresses=section_addresses,
    )
    compiled = read_function_image(
        linked, function_name=function_name, address=address
    ).content
    return compiled == original_bytes, compiled
