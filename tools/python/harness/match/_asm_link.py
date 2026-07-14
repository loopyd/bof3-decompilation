from __future__ import annotations

import re
import subprocess
from pathlib import Path

from ..paths import RepoLayout, repo_layout
from ..symbols import load_weak_symbol_bindings

_HEX_SUFFIX_RE = re.compile(r"(?:func|DAT)_([0-9a-fA-F]{8})$")


def resolve_symbol_address(name: str, *, symbols_c_path: Path) -> int | None:
    m = _HEX_SUFFIX_RE.search(name)
    if m is not None:
        return int(m.group(1), 16)
    return load_weak_symbol_bindings(symbols_c_path).get(name)


def link_object_at_address(
    *,
    object_path: Path,
    address: int,
    undefined_symbols: list[str],
    symbols_c_path: Path | None = None,
    layout: RepoLayout | None = None,
    output_path: Path | None = None,
    section_addresses: dict[str, int] | None = None,
) -> Path:
    repo = layout or repo_layout()
    ld = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-ld"
    symbols_c = symbols_c_path or (repo.root / "src" / "boot" / "symbols.c")
    defsym_args: list[str] = []
    for sym in undefined_symbols:
        addr = resolve_symbol_address(sym, symbols_c_path=symbols_c)
        if addr is not None:
            defsym_args.extend([f"--defsym={sym}={addr}"])
    out = output_path or object_path.with_suffix(".linked.o")
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
            str(object_path),
            "-o",
            str(out),
        ],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"link failed: {result.stderr}")
    return out


def extract_function_bytes(
    linked_path: Path,
    *,
    size: int,
    layout: RepoLayout | None = None,
) -> bytes:
    repo = layout or repo_layout()
    objcopy = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objcopy"
    flat = subprocess.run(
        [str(objcopy), "-O", "binary", "-j", ".text", str(linked_path), "/dev/stdout"],
        capture_output=True,
    )
    if flat.returncode != 0:
        raise RuntimeError(f"objcopy failed: {flat.stderr.decode()}")
    return flat.stdout[:size]


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
    address: int,
    size: int,
    original_bytes: bytes,
    symbols_c_path: Path | None = None,
    layout: RepoLayout | None = None,
    section_addresses: dict[str, int] | None = None,
) -> tuple[bool, bytes]:
    repo = layout or repo_layout()
    nm = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-nm"
    nm_result = subprocess.run(
        [str(nm), "-u", str(object_path)], capture_output=True, text=True
    )
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
        layout=repo,
        section_addresses=section_addresses,
    )
    compiled = extract_function_bytes(linked, size=size, layout=repo)
    return compiled == original_bytes, compiled
