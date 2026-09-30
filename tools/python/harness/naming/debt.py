"""Repository naming-debt inventory and regression gate."""

from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re

from harness.domain.symbols import map_path
from harness.domain.manifests import TargetManifest
from harness.domain.splat_evidence import boundary_kind_evidence

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

    def to_rows(self) -> dict[str, list[str]]:
        return {
            "raw_function_files": sorted(self.raw_function_files),
            "invalid_semantic_files": sorted(self.invalid_semantic_files),
            "raw_functions": sorted(self.raw_functions),
            "raw_data": sorted(self.raw_data),
        }


def classify_raw_symbol(name: str) -> str | None:
    """Return the naming-debt kind implied by the spelling alone.

    This is only the fallback: a spelling is not a proven storage or function
    boundary.  ``collect_symbol_debt`` overrides it with the target's own Splat
    listing whenever that evidence exists (see ``domain/splat_evidence.py``).
    """
    if _RAW_FUNCTION.fullmatch(name):
        return "function"
    if _RAW_DATA.fullmatch(name):
        return "data"
    return None


def collect_naming_debt(root: Path, manifests: dict[str, TargetManifest]) -> NamingDebt:
    raw_function_files: set[str] = set()
    invalid_semantic_files: set[str] = set()

    for path in (root / "src" / "bof3").rglob("*.c"):
        relative = path.relative_to(root).as_posix()
        if _RAW_FUNCTION.fullmatch(path.stem):
            raw_function_files.add(relative)
        elif (
            path.parent != root / "src" / "bof3" / "support"
            and not _SEMANTIC_FILE.fullmatch(path.name)
        ):
            invalid_semantic_files.add(relative)

    raw_functions, raw_data = collect_symbol_debt(root, manifests)
    return NamingDebt(
        frozenset(raw_function_files),
        frozenset(invalid_semantic_files),
        raw_functions,
        raw_data,
    )


def collect_symbol_debt(
    root: Path, manifests: dict[str, TargetManifest]
) -> tuple[frozenset[str], frozenset[str]]:
    """Collect raw map spellings without scanning authored source filenames."""
    raw_functions: set[str] = set()
    raw_data: set[str] = set()
    for target in sorted(manifests):
        path = map_path(root, target)
        if not path.is_file():
            continue
        for match in _MAP_ROW.finditer(path.read_text(encoding="utf-8")):
            name = match.group("name")
            row = f"{target}:{name}"
            kind = classify_raw_symbol(name)
            if kind is None:
                continue
            # Evidence beats spelling.  A raw row whose own Splat listing proves
            # the other kind is booked under the proven kind, so data ranges stop
            # being routed and counted as function naming debt (and vice versa).
            # The row's wrong spelling is a separate defect corrected by the
            # naming/data owners; this only stops the kind from being inherited
            # from a name that the target's own artifact contradicts.
            proven = boundary_kind_evidence(root, target, name)
            if proven == "data" and kind == "function":
                kind = "data"
            elif proven == "code" and kind == "data":
                kind = "function"
            if kind == "function":
                raw_functions.add(row)
            else:
                raw_data.add(row)

    return frozenset(raw_functions), frozenset(raw_data)


def load_naming_baseline(root: Path) -> dict[str, set[str]]:
    path = root / _BASELINE
    if not path.is_file():
        raise ValueError(f"missing naming baseline: {_BASELINE}")
    data = json.loads(path.read_text(encoding="utf-8"))
    return {key: set(values) for key, values in data.items()}


def merge_naming_baseline(root: Path, debt: NamingDebt) -> dict[str, list[str]]:
    """Record current raw-spelling debt in the baseline; return the rows added.

    The baseline is reviewed truth, so this only appends rows, never deletes,
    and keeps every category sorted. Run it after an accepted transaction that
    intentionally retains raw `func_`/`D_` spellings; `bin/harness source symbols check` then
    reports only debt introduced by later unreviewed edits.
    """
    path = root / _BASELINE
    baseline = load_naming_baseline(root)
    added: dict[str, list[str]] = {}
    for category, rows in debt.to_rows().items():
        known = baseline.get(category, set())
        new_rows = sorted(set(rows) - known)
        if new_rows:
            added[category] = new_rows
        baseline[category] = known | set(rows)
    payload = {key: sorted(values) for key, values in baseline.items()}
    path.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return added


def naming_debt_regressions(
    debt: NamingDebt, baseline: dict[str, set[str]]
) -> list[str]:
    current = debt.to_rows()
    errors: list[str] = []
    for category, rows in current.items():
        for row in sorted(set(rows) - baseline.get(category, set())):
            errors.append(f"new naming debt ({category}): {row}")
    return errors


def address_of(name: str) -> int:
    """Extract the embedded address from one raw inventory symbol name."""
    match = re.search(r"([0-9A-Fa-f]{8})$", name)
    if match is None:
        raise ValueError(f"raw inventory name lacks address: {name}")
    return int(match.group(1), 16)
