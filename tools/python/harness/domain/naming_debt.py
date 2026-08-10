"""Repository naming-debt inventory and regression gate."""

from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re

from ..canonical import map_path
from .manifests import TargetManifest

_RAW_FUNCTION = re.compile(r"func_[0-9A-F]{8}")
_RAW_DATA = re.compile(r"D_[0-9A-F]{8}(?:_[A-Za-z0-9_]+)?")
_MAP_ROW = re.compile(r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x[0-9A-Fa-f]+;")
_SEMANTIC_FILE = re.compile(
    r"[a-z][A-Za-z0-9]*(?:_(?:[a-z][a-z0-9]*_)?[0-9A-F]{8})?\.c"
)
_BASELINE = Path("config/symbol-naming-baseline.json")


@dataclass(frozen=True)
class NamingDebt:
    raw_function_files: frozenset[str]
    invalid_semantic_files: frozenset[str]
    raw_functions: frozenset[str]
    raw_data: frozenset[str]

    def rows(self) -> dict[str, list[str]]:
        return {
            "raw_function_files": sorted(self.raw_function_files),
            "invalid_semantic_files": sorted(self.invalid_semantic_files),
            "raw_functions": sorted(self.raw_functions),
            "raw_data": sorted(self.raw_data),
        }


def collect_naming_debt(
    root: Path, manifests: dict[str, TargetManifest]
) -> NamingDebt:
    raw_function_files: set[str] = set()
    invalid_semantic_files: set[str] = set()
    raw_functions: set[str] = set()
    raw_data: set[str] = set()

    for path in (root / "src" / "bof3").rglob("*.c"):
        relative = path.relative_to(root).as_posix()
        if _RAW_FUNCTION.fullmatch(path.stem):
            raw_function_files.add(relative)
        elif (
            path.parent != root / "src" / "bof3" / "support"
            and not _SEMANTIC_FILE.fullmatch(path.name)
        ):
            invalid_semantic_files.add(relative)

    for target in sorted(manifests):
        path = map_path(root, target)
        if not path.is_file():
            continue
        for match in _MAP_ROW.finditer(path.read_text(encoding="utf-8")):
            name = match.group("name")
            row = f"{target}:{name}"
            if _RAW_FUNCTION.fullmatch(name):
                raw_functions.add(row)
            elif _RAW_DATA.fullmatch(name):
                raw_data.add(row)

    return NamingDebt(
        frozenset(raw_function_files),
        frozenset(invalid_semantic_files),
        frozenset(raw_functions),
        frozenset(raw_data),
    )


def load_naming_baseline(root: Path) -> dict[str, set[str]]:
    path = root / _BASELINE
    if not path.is_file():
        raise ValueError(f"missing naming baseline: {_BASELINE}")
    data = json.loads(path.read_text(encoding="utf-8"))
    return {key: set(values) for key, values in data.items()}


def naming_debt_regressions(
    debt: NamingDebt, baseline: dict[str, set[str]]
) -> list[str]:
    current = debt.rows()
    errors: list[str] = []
    for category, rows in current.items():
        for row in sorted(set(rows) - baseline.get(category, set())):
            errors.append(f"new naming debt ({category}): {row}")
    return errors
