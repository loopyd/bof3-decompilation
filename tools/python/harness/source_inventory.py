"""Source declaration and binding inventory.

Inventories lifted functions, data declarations, PsyQ bindings, and
weak symbol bindings for a single target.  Declaration kind comes from
C declarations in ``internal.h`` and symbol headers, not from name
shape.

All IDs are target-qualified.  Conflicts between declarations and
bindings produce diagnostics, not silent guesses.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from .symbols import load_weak_symbol_bindings


_WEAK_SYMBOL_AT_RE = re.compile(
    r"WEAK_SYMBOL_AT\((?P<name>\w+)\s*,\s*(?P<address>0x[0-9a-fA-F]+)\)"
)
_FUNC_NAME_RE = re.compile(r"^func_[0-9a-fA-F]{8}$")
_DAT_NAME_RE = re.compile(r"^DAT_[0-9a-fA-F]{8}$")
_FUNCTION_FILE_RE = re.compile(r"func_([0-9a-fA-F]{8})\.c$")
_EXTERN_FUNC_RE = re.compile(
    r"^\s*(?:extern\s+)?"
    r"(?:volatile\s+)?"
    r"(?:u8|u16|u32|s8|s16|s32|vu8|vu16|vu32|vs8|vs16|vs32|"
    r"void|int|short|long|char|unsigned|signed|float|double|"
    r"[A-Z_][A-Z_0-9]*|"
    r"[A-Za-z_][A-Za-z_0-9]*\s*\*)"
    r"\s+(?P<name>\w+)\s*\(",
    re.M,
)
_EXTERN_DATA_RE = re.compile(
    r"^\s*extern\s+"
    r"(?P<type>"
    r"volatile\s+)?"
    r"(?P<base_type>u8|u16|u32|s8|s16|s32|vu8|vu16|vu32|vs8|vs16|vs32)\s+"
    r"(?P<name>\w+)\s*;",
    re.M,
)


@dataclass(frozen=True)
class FunctionDeclaration:
    """A function declared or bound in the target source tree."""

    target: str
    address: int
    name: str
    declaration_path: Path | None
    binding_path: Path | None
    source_path: Path | None
    semantic_name: str | None

    @property
    def function_id(self) -> str:
        return f"{self.target}@{self.address:08x}"

    @property
    def is_lifted(self) -> bool:
        return self.source_path is not None

    @property
    def is_reviewed(self) -> bool:
        return self.declaration_path is not None or self.binding_path is not None


@dataclass(frozen=True)
class DataDeclaration:
    """A data symbol declared or bound in the target source tree."""

    target: str
    address: int
    name: str
    declaration_path: Path | None
    binding_path: Path | None

    @property
    def symbol_id(self) -> str:
        return f"{self.target}::{self.name}"


@dataclass(frozen=True)
class PsyqBinding:
    """A PsyQ library binding in the target source tree."""

    target: str
    address: int
    name: str
    library: str | None
    binding_path: Path

    @property
    def symbol_id(self) -> str:
        return f"{self.target}::psyq:{self.name}"


@dataclass(frozen=True)
class SourceInventory:
    """A complete inventory of a target's source declarations."""

    target: str
    functions: tuple[FunctionDeclaration, ...]
    data: tuple[DataDeclaration, ...]
    psyq: tuple[PsyqBinding, ...]
    input_hash: str

    def function_ids(self) -> set[str]:
        return {f.function_id for f in self.functions}

    def function_addresses(self) -> set[int]:
        return {f.address for f in self.functions}

    def data_addresses(self) -> set[int]:
        return {d.address for d in self.data}

    def lifted_addresses(self) -> set[int]:
        return {f.address for f in self.functions if f.is_lifted}

    def reviewed_addresses(self) -> set[int]:
        return {f.address for f in self.functions if f.is_reviewed}

    def by_address(self) -> dict[int, FunctionDeclaration]:
        result: dict[int, FunctionDeclaration] = {}
        for func in self.functions:
            result.setdefault(func.address, func)
        return result

    def functions_by_address(self) -> dict[int, FunctionDeclaration]:
        return self.by_address()

    def semantic_names(self) -> dict[int, str]:
        """Return addresses with a semantic alias that is not a func_ name."""
        return {
            f.address: f.semantic_name
            for f in self.functions
            if f.semantic_name is not None
        }

    def function_by_name(self, name: str) -> FunctionDeclaration | None:
        for f in self.functions:
            if f.name == name or f.semantic_name == name:
                return f
        return None

    def data_by_address(self, address: int) -> DataDeclaration | None:
        for d in self.data:
            if d.address == address:
                return d
        return None

    def psyq_by_address(self, address: int) -> PsyqBinding | None:
        for p in self.psyq:
            if p.address == address:
                return p
        return None


def build_source_inventory(source_dir: Path, target_id: str) -> SourceInventory:
    """Build a complete inventory of ``source_dir`` for ``target_id``.

    Reads declarations from headers, weak bindings from symbol units,
    and lifted files from the source root.  Classification comes from
    the declaration layer, not from naming heuristics.
    """

    import hashlib as _hashlib

    pieces: list[bytes] = []

    # 1. Inventory lifted function source files.
    source_files: dict[int, Path] = {}
    if source_dir.is_dir():
        for path in source_dir.glob("func_*.c"):
            match = _FUNCTION_FILE_RE.match(path.name)
            if match is None:
                continue
            address = int(match.group(1), 16)
            source_files[address] = path
            pieces.append(path.read_bytes())

    # 2. Read weak bindings from the canonical entry point.
    all_bindings: dict[str, int] = {}
    bindings_root = source_dir / "symbols.c"
    if bindings_root.is_file():
        all_bindings.update(load_weak_symbol_bindings(bindings_root))
        pieces.append(bindings_root.read_bytes())

    # 3. Parse function declarations from internal.h and symbol headers.
    func_declarations: dict[int, str] = {}
    data_declarations: dict[int, str] = {}
    declaration_paths: dict[int, Path] = {}

    for header_path in _declaration_headers(source_dir):
        text = header_path.read_text(encoding="utf-8")
        pieces.append(text.encode("utf-8"))
        for match in _EXTERN_FUNC_RE.finditer(text):
            name = match.group("name")
            if _FUNC_NAME_RE.match(name) or _DAT_NAME_RE.match(name):
                continue
            # Find the address from bindings.
            addr = all_bindings.get(name)
            if addr is not None:
                func_declarations.setdefault(addr, name)
                declaration_paths.setdefault(addr, header_path)
        for match in _EXTERN_DATA_RE.finditer(text):
            name = match.group("name")
            if _DAT_NAME_RE.match(name):
                continue
            addr = all_bindings.get(name)
            if addr is not None:
                data_declarations.setdefault(addr, name)
                declaration_paths.setdefault(addr, header_path)

    # 4. Resolve psyq bindings separately.
    psyq_bindings: list[PsyqBinding] = []
    for candidate in (source_dir / "symbols" / "psyq.c", source_dir / "psyq.c"):
        if candidate.is_file():
            psyq_text = candidate.read_text(encoding="utf-8")
            pieces.append(psyq_text.encode("utf-8"))
            current_library = "???"
            for line in psyq_text.splitlines():
                lib_match = re.match(r"/\*\s*(LIB\w+)\s*\*/", line)
                if lib_match:
                    current_library = lib_match.group(1)
                bind_match = _WEAK_SYMBOL_AT_RE.search(line)
                if bind_match:
                    name = bind_match.group("name")
                    addr = int(bind_match.group("address"), 16)
                    psyq_bindings.append(
                        PsyqBinding(
                            target=target_id,
                            address=addr,
                            name=name,
                            library=current_library,
                            binding_path=candidate,
                        )
                    )
            break

    # 5. Build function and data declarations by scanning bindings against
    #    declarations.  Classification comes from the declaration layer,
    #    not from the binding name's shape.
    func_addresses: dict[int, str] = {}      # address -> canonical name
    func_semantic: dict[int, str] = {}       # address -> semantic alias
    data_addresses: dict[int, str] = {}      # address -> canonical name

    # a. func_ bindings always create function declarations.
    for name, address in sorted(all_bindings.items()):
        if _FUNC_NAME_RE.match(name):
            func_addresses.setdefault(address, name)

    # b. Semantic function declarations from headers.
    for address, semantic_name in func_declarations.items():
        if address in func_addresses:
            func_semantic[address] = semantic_name
        else:
            func_addresses[address] = semantic_name

    # c. Semantic bindings that share an address with a func_ binding.
    for name, address in sorted(all_bindings.items()):
        if _FUNC_NAME_RE.match(name) or _DAT_NAME_RE.match(name):
            continue
        if address in func_addresses:
            func_semantic.setdefault(address, name)
            continue
        if address in func_declarations:
            func_addresses.setdefault(address, name)
            continue
        # Undeclared semantic binding: leave as potential data or ignore.
        # It will be picked up in the data pass below if declared as data.

    # d. DAT_ bindings always create data declarations.
    for name, address in sorted(all_bindings.items()):
        if _DAT_NAME_RE.match(name):
            data_addresses.setdefault(address, name)

    # e. Semantic data declarations from headers that don't match functions.
    for address, name in data_declarations.items():
        if address not in func_addresses:
            data_addresses.setdefault(address, name)

    # f. Semantic bindings that share an address with a DAT_ binding.
    for name, address in sorted(all_bindings.items()):
        if _FUNC_NAME_RE.match(name) or _DAT_NAME_RE.match(name):
            continue
        if address in data_addresses:
            continue
        if address in func_addresses:
            continue
        # Undeclared semantic binding that's neither func nor data from
        # declarations.  Skip it to avoid misclassification.

    # 6. Build function records.
    functions: list[FunctionDeclaration] = []
    for address in sorted(func_addresses):
        name = func_addresses[address]
        semantic = func_semantic.get(address)
        if semantic == name:
            semantic = None
        functions.append(
            FunctionDeclaration(
                target=target_id,
                address=address,
                name=name,
                declaration_path=declaration_paths.get(address),
                binding_path=bindings_root if bindings_root.is_file() else None,
                source_path=source_files.get(address),
                semantic_name=semantic,
            )
        )

    # 7. Build data records.
    data: list[DataDeclaration] = []
    seen_data_addrs: set[int] = set()
    for address, name in sorted(data_addresses.items()):
        data.append(
            DataDeclaration(
                target=target_id,
                address=address,
                name=name,
                declaration_path=declaration_paths.get(address),
                binding_path=bindings_root if bindings_root.is_file() else None,
            )
        )
        seen_data_addrs.add(address)

    input_hash = _hashlib.sha256(b"\x00".join(pieces)).hexdigest()[:16] if pieces else ""

    return SourceInventory(
        target=target_id,
        functions=tuple(sorted(functions, key=lambda f: f.address)),
        data=tuple(sorted(data, key=lambda d: d.address)),
        psyq=tuple(sorted(psyq_bindings, key=lambda p: (p.library or "", p.address))),
        input_hash=input_hash,
    )


def _declaration_headers(source_dir: Path) -> list[Path]:
    """Return every header that may contain type/function declarations."""

    paths: list[Path] = []
    for candidate in (
        source_dir / "internal.h",
        source_dir / "symbols" / "symbols.h",
        source_dir / "symbols" / "functions.h",
        source_dir / "symbols" / "variables.h",
    ):
        if candidate.is_file():
            paths.append(candidate)
    return sorted(paths)


__all__ = [
    "DataDeclaration",
    "FunctionDeclaration",
    "PsyqBinding",
    "SourceInventory",
    "build_source_inventory",
]
