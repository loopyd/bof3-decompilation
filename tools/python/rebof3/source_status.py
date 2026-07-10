from __future__ import annotations

import json
import csv
import re
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

from .paths import RepoLayout


SOURCE_ADDRESS_RE = re.compile(r"@source:\s*(0x[0-9a-fA-F]+|[0-9a-fA-F]{8})")
FUNCTION_DEF_RE = re.compile(
    r"\b(?:void|s32|u32|s16|u16|s8|u8|int|long|const\s+\w+\s*\*?|\w+\s*\*?)\s+"
    r"(?P<name>func_[0-9a-fA-F]{8}|[A-Za-z_][A-Za-z0-9_]*)\s*\("
)
VARIABLE_RE = re.compile(r"\b(?:DAT|PTR|s)_[0-9A-Za-z_]+|\bDAT_[0-9a-fA-F]{8}\b")
ADDRESS_MACRO_RE = re.compile(
    r"^\s*#define\s+(?P<name>[A-Z0-9_]+)\b.*0x[0-9a-fA-F]{8}", re.M
)
STRUCT_RE = re.compile(
    r"\btypedef\s+struct\s+(?P<typedef>[A-Za-z0-9_]*)|"
    r"\bstruct\s+(?P<struct>[A-Za-z][A-Za-z0-9_]*)\s*\{"
)
COMPLEXITY_RE = re.compile(r"\b(if|else if|for|while|switch|case|goto)\b|&&|\|\||\?")
CALL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")
NON_CALL_NAMES = {"if", "for", "while", "switch", "return", "sizeof"}


@dataclass(frozen=True)
class FunctionStatus:
    name: str
    source: str
    address: str | None
    has_asm_summary: bool
    exact_match: bool
    match_percent: float | None
    original_size: int | None
    current_size: int | None
    size_delta: int | None
    loc: int
    branches: int
    calls: int
    complexity_score: int


@dataclass(frozen=True)
class GhidraFunctionStatus:
    program_path: str
    program_name: str
    source_hint: str
    entry: str
    name: str
    signature: str
    body_min: str
    body_max: str
    is_thunk: bool


@dataclass(frozen=True)
class GhidraProgramStatus:
    program: str
    program_path: str
    source_hint: str
    ghidra_functions: int
    lifted_functions: int
    unlifted_functions: int
    thunks: int
    unlifted_samples: list[str]


@dataclass(frozen=True)
class MergedFunctionStatus:
    module: str
    address: str
    status: str
    ghidra_name: str
    source_name: str
    source: str
    match_percent: float | None
    exact_match: bool
    program: str
    source_hint: str


@dataclass(frozen=True)
class ModuleStatus:
    module: str
    c_files: int
    h_files: int
    functions: int
    source_tagged: int
    asm_summaries: int
    exact_matches: int
    avg_match_percent: float | None
    ghidra_functions: int
    ghidra_lifted: int
    ghidra_unlifted: int
    variables: list[str]
    structs: list[str]
    address_min: str | None
    address_max: str | None
    function_statuses: list[FunctionStatus]
    merged_function_statuses: list[MergedFunctionStatus]
    ghidra_programs: list[str]
    ghidra_unlifted_samples: list[str]


def parse_source_address(text: str) -> str | None:
    match = SOURCE_ADDRESS_RE.search(text)
    if match is None:
        return None
    return f"0x{int(match.group(1), 16):08x}"


def load_asm_summaries(asm_root: Path) -> dict[str, dict[str, Any]]:
    summaries: dict[str, dict[str, Any]] = {}
    if not asm_root.is_dir():
        return summaries
    for summary_path in sorted(asm_root.glob("*/summary.json")):
        try:
            payload = json.loads(summary_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            continue
        function_name = payload.get("function")
        if isinstance(function_name, str):
            summaries[function_name] = payload
    return summaries


def normalize_hex_address(value: str | None) -> str | None:
    if not value:
        return None
    try:
        return f"0x{int(value, 16):08x}"
    except ValueError:
        return None


def load_ghidra_functions(function_index_tsv: Path) -> list[GhidraFunctionStatus]:
    if not function_index_tsv.is_file():
        return []

    rows: list[GhidraFunctionStatus] = []
    seen: set[tuple[str, str]] = set()
    with function_index_tsv.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        for row in reader:
            entry = normalize_hex_address(row.get("entry_hex") or row.get("entry"))
            if entry is None:
                continue
            source_hint = row.get("source_hint") or ""
            program_path = row.get("program_path") or ""
            dedupe_key = (source_hint or program_path, entry)
            if dedupe_key in seen:
                continue
            seen.add(dedupe_key)
            rows.append(
                GhidraFunctionStatus(
                    program_path=program_path,
                    program_name=row.get("program_name") or "",
                    source_hint=source_hint,
                    entry=entry,
                    name=row.get("name") or "",
                    signature=row.get("signature") or "",
                    body_min=row.get("body_min") or "",
                    body_max=row.get("body_max") or "",
                    is_thunk=(row.get("is_thunk") or "").lower() == "true",
                )
            )
    return rows


def source_group_for_ghidra(row: GhidraFunctionStatus) -> str:
    return row.source_hint or row.program_path or row.program_name


def decimal_suffix(part: str) -> str:
    return str(int(part, 10))


def inferred_source_hint_for_module(module_name: str) -> str | None:
    parts = module_name.split("/")
    if parts[:1] == ["core"] or parts[:1] == ["boot"]:
        return "out/extracted/SLUS_004.22"
    if parts == ["modules", "logo"]:
        return "out/extracted/LOGO/LOGO.EXE"
    if len(parts) == 3 and parts[:2] == ["modules", "game"]:
        return f"out/extracted/ETC/GAME.EMI#{decimal_suffix(parts[2])}"
    if len(parts) == 3 and parts[:2] == ["modules", "commu00"]:
        return f"out/extracted/ETC/COMMU00.EMI#{decimal_suffix(parts[2])}"
    if len(parts) == 3 and parts[:2] == ["modules", "bate"]:
        return f"out/extracted/ETC/BATE.EMI#{decimal_suffix(parts[2])}"
    if len(parts) == 3 and parts[:2] == ["modules", "batl_re2"]:
        return f"out/extracted/BATTLE/BATL_RE2.EMI#{decimal_suffix(parts[2])}"
    if len(parts) == 3 and parts[:2] == ["modules", "battle"]:
        return f"out/extracted/BATTLE/BATTLE.EMI#{decimal_suffix(parts[2])}"
    if len(parts) == 3 and parts[:2] == ["modules", "sce10eff"]:
        return f"out/extracted/SCENARIO/SCE10EFF.EMI#{decimal_suffix(parts[2])}"
    if len(parts) == 3 and parts[:2] == ["modules", "scena16"]:
        return f"out/extracted/SCENARIO/SCENA16.EMI#{decimal_suffix(parts[2])}"
    if len(parts) == 4 and parts[:2] == ["modules", "world00"]:
        area = parts[2].upper()
        return f"out/extracted/WORLD00/{area}.EMI#{decimal_suffix(parts[3])}"
    return None


def group_ghidra_by_source(
    rows: list[GhidraFunctionStatus],
) -> dict[str, list[GhidraFunctionStatus]]:
    grouped: dict[str, list[GhidraFunctionStatus]] = {}
    for row in rows:
        grouped.setdefault(source_group_for_ghidra(row), []).append(row)
    return grouped


def group_ghidra_by_entry(
    rows: list[GhidraFunctionStatus],
) -> dict[str, list[GhidraFunctionStatus]]:
    grouped: dict[str, list[GhidraFunctionStatus]] = {}
    for row in rows:
        grouped.setdefault(row.entry, []).append(row)
    return grouped


def strip_comment_lines(text: str) -> str:
    return "\n".join(
        line
        for line in text.splitlines()
        if not line.strip().startswith(("/*", "*", "//"))
    )


def source_loc(code: str) -> int:
    return sum(1 for line in code.splitlines() if line.strip())


def call_count(code: str) -> int:
    return sum(
        1 for match in CALL_RE.finditer(code) if match.group(1) not in NON_CALL_NAMES
    )


def branch_count(code: str) -> int:
    return len(COMPLEXITY_RE.findall(code))


def complexity_score(*, loc: int, branches: int, calls: int) -> int:
    return loc + (branches * 8) + (calls * 2)


def function_names_for_source(path: Path, text: str) -> list[str]:
    if path.stem.startswith("func_"):
        return [path.stem]
    names: list[str] = []
    for match in FUNCTION_DEF_RE.finditer(text):
        tail = text[match.end() : match.end() + 120]
        if "{" in tail.split(";", 1)[0]:
            names.append(match.group("name"))
    return names


def scan_names(paths: list[Path]) -> tuple[list[str], list[str]]:
    variables: set[str] = set()
    structs: set[str] = set()
    for path in paths:
        text = path.read_text(encoding="utf-8", errors="ignore")
        variables.update(match.group(0) for match in VARIABLE_RE.finditer(text))
        variables.update(
            match.group("name") for match in ADDRESS_MACRO_RE.finditer(text)
        )
        for match in STRUCT_RE.finditer(text):
            name = match.group("typedef") or match.group("struct")
            if name:
                structs.add(name)
    return sorted(variables), sorted(structs)


def analyze_source_module(
    source_root: Path,
    module_dir: Path,
    asm_summaries: dict[str, dict[str, Any]],
    ghidra_by_entry: dict[str, list[GhidraFunctionStatus]],
    ghidra_by_source: dict[str, list[GhidraFunctionStatus]],
) -> ModuleStatus:
    c_files = sorted(module_dir.glob("*.c"))
    h_files = sorted(module_dir.glob("*.h"))
    variables, structs = scan_names([*c_files, *h_files])
    function_statuses: list[FunctionStatus] = []
    addresses: list[int] = []
    address_strings: set[str] = set()
    ghidra_sources: set[str] = set()

    for source_path in c_files:
        text = source_path.read_text(encoding="utf-8", errors="ignore")
        code = strip_comment_lines(text)
        address = parse_source_address(text)
        if address is not None:
            addresses.append(int(address, 16))
            address_strings.add(address)
            for ghidra_row in ghidra_by_entry.get(address, []):
                ghidra_sources.add(source_group_for_ghidra(ghidra_row))
        loc = source_loc(code)
        branches = branch_count(code)
        calls = call_count(code)
        score = complexity_score(loc=loc, branches=branches, calls=calls)
        relative_source = source_path.relative_to(source_root.parent).as_posix()

        for function_name in function_names_for_source(source_path, text):
            summary = asm_summaries.get(function_name, {})
            instruction_count = summary.get("instruction_count", {})
            match_percent = instruction_count.get("match_percent")
            function_statuses.append(
                FunctionStatus(
                    name=function_name,
                    source=relative_source,
                    address=address,
                    has_asm_summary=bool(summary),
                    exact_match=bool(summary.get("exact_match")),
                    match_percent=(
                        float(match_percent)
                        if isinstance(match_percent, int | float)
                        else None
                    ),
                    original_size=summary.get("original_size")
                    if isinstance(summary.get("original_size"), int)
                    else None,
                    current_size=summary.get("current_size")
                    if isinstance(summary.get("current_size"), int)
                    else None,
                    size_delta=summary.get("size_delta")
                    if isinstance(summary.get("size_delta"), int)
                    else None,
                    loc=loc,
                    branches=branches,
                    calls=calls,
                    complexity_score=score,
                )
            )

    match_values = [
        status.match_percent
        for status in function_statuses
        if status.match_percent is not None
    ]
    module_name = module_dir.relative_to(source_root).as_posix()
    inferred_source = inferred_source_hint_for_module(module_name)
    if inferred_source is not None:
        ghidra_sources = {inferred_source}

    ghidra_rows: list[GhidraFunctionStatus] = []
    for source in sorted(ghidra_sources):
        rows = ghidra_by_source.get(source, [])
        # SLUS contains many unrelated core systems; keep core rows scoped to
        # addresses already represented by the source module. Overlay and LOGO
        # programs are module-sized, so include all functions from that program.
        if source.endswith("SLUS_004.22"):
            rows = [row for row in rows if row.entry in address_strings]
        ghidra_rows.extend(rows)
    if not ghidra_rows:
        ghidra_rows = [
            row
            for address in sorted(address_strings)
            for row in ghidra_by_entry.get(address, [])
        ]
    ghidra_rows = sorted(
        {
            (row.entry, source_group_for_ghidra(row)): row for row in ghidra_rows
        }.values(),
        key=lambda row: row.entry,
    )
    ghidra_lifted = sum(1 for row in ghidra_rows if row.entry in address_strings)
    ghidra_unlifted_samples = [
        f"{row.entry}:{row.name}"
        for row in ghidra_rows
        if row.entry not in address_strings
    ][:8]
    source_by_address = {
        status.address: status for status in function_statuses if status.address
    }
    ghidra_entries = {row.entry for row in ghidra_rows}
    merged_function_statuses: list[MergedFunctionStatus] = []
    for row in ghidra_rows:
        source_status = source_by_address.get(row.entry)
        if source_status is None:
            status = "unlifted"
            source_name = ""
            source_path = ""
            match_percent = None
            exact_match = False
        else:
            status = "exact" if source_status.exact_match else "lifted"
            source_name = source_status.name
            source_path = source_status.source
            match_percent = source_status.match_percent
            exact_match = source_status.exact_match
        merged_function_statuses.append(
            MergedFunctionStatus(
                module=module_name,
                address=row.entry,
                status=status,
                ghidra_name=row.name,
                source_name=source_name,
                source=source_path,
                match_percent=match_percent,
                exact_match=exact_match,
                program=row.program_path or row.program_name,
                source_hint=source_group_for_ghidra(row),
            )
        )
    for source_status in function_statuses:
        if source_status.address is None or source_status.address in ghidra_entries:
            continue
        merged_function_statuses.append(
            MergedFunctionStatus(
                module=module_name,
                address=source_status.address,
                status="source-only",
                ghidra_name="",
                source_name=source_status.name,
                source=source_status.source,
                match_percent=source_status.match_percent,
                exact_match=source_status.exact_match,
                program="",
                source_hint="",
            )
        )

    return ModuleStatus(
        module=module_name,
        c_files=len(c_files),
        h_files=len(h_files),
        functions=len({status.name for status in function_statuses}),
        source_tagged=sum(1 for status in function_statuses if status.address),
        asm_summaries=sum(1 for status in function_statuses if status.has_asm_summary),
        exact_matches=sum(1 for status in function_statuses if status.exact_match),
        avg_match_percent=(
            round(sum(match_values) / len(match_values), 2) if match_values else None
        ),
        ghidra_functions=len(ghidra_rows),
        ghidra_lifted=ghidra_lifted,
        ghidra_unlifted=max(len(ghidra_rows) - ghidra_lifted, 0),
        variables=variables,
        structs=structs,
        address_min=f"0x{min(addresses):08x}" if addresses else None,
        address_max=f"0x{max(addresses):08x}" if addresses else None,
        function_statuses=sorted(
            function_statuses, key=lambda status: (status.address or "", status.name)
        ),
        merged_function_statuses=sorted(
            merged_function_statuses,
            key=lambda status: (status.address, status.ghidra_name, status.source_name),
        ),
        ghidra_programs=sorted(ghidra_sources),
        ghidra_unlifted_samples=ghidra_unlifted_samples,
    )


def analyze_source_status(
    layout: RepoLayout,
    *,
    module_filter: str | None = None,
    asm_root: Path | None = None,
    ghidra_function_index_tsv: Path | None = None,
) -> list[ModuleStatus]:
    source_root = layout.bof3_dir / "src"
    summaries = load_asm_summaries(asm_root or (layout.out_dir / "asm-diff"))
    ghidra_rows = load_ghidra_functions(
        ghidra_function_index_tsv or layout.inventory_ghidra_function_index_tsv_path
    )
    ghidra_by_entry = group_ghidra_by_entry(ghidra_rows)
    ghidra_by_source = group_ghidra_by_source(ghidra_rows)
    modules: list[ModuleStatus] = []
    for module_dir in sorted(path for path in source_root.rglob("*") if path.is_dir()):
        if (
            module_filter
            and module_filter not in module_dir.relative_to(source_root).as_posix()
        ):
            continue
        if not list(module_dir.glob("*.c")) and not list(module_dir.glob("*.h")):
            continue
        status = analyze_source_module(
            source_root,
            module_dir,
            summaries,
            ghidra_by_entry,
            ghidra_by_source,
        )
        if status.functions or status.h_files:
            modules.append(status)
    return modules


def analyze_ghidra_programs(
    layout: RepoLayout,
    *,
    module_filter: str | None = None,
    ghidra_function_index_tsv: Path | None = None,
) -> list[GhidraProgramStatus]:
    ghidra_rows = load_ghidra_functions(
        ghidra_function_index_tsv or layout.inventory_ghidra_function_index_tsv_path
    )
    modules = analyze_source_status(
        layout,
        module_filter=module_filter,
        ghidra_function_index_tsv=ghidra_function_index_tsv,
    )
    lifted_addresses = {
        status.address
        for module in modules
        for status in module.function_statuses
        if status.address
    }
    grouped = group_ghidra_by_source(ghidra_rows)
    program_statuses: list[GhidraProgramStatus] = []
    for source, rows in sorted(grouped.items()):
        if module_filter and module_filter not in source:
            matching_module_sources = {
                program for module in modules for program in module.ghidra_programs
            }
            if source not in matching_module_sources:
                continue
        lifted = sum(1 for row in rows if row.entry in lifted_addresses)
        unlifted_samples = [
            f"{row.entry}:{row.name}"
            for row in rows
            if row.entry not in lifted_addresses
        ][:8]
        first = rows[0]
        program_statuses.append(
            GhidraProgramStatus(
                program=first.program_name,
                program_path=first.program_path,
                source_hint=source,
                ghidra_functions=len(rows),
                lifted_functions=lifted,
                unlifted_functions=max(len(rows) - lifted, 0),
                thunks=sum(1 for row in rows if row.is_thunk),
                unlifted_samples=unlifted_samples,
            )
        )
    return program_statuses


def analyze_all_ghidra_function_statuses(
    layout: RepoLayout,
    modules: list[ModuleStatus],
    *,
    ghidra_function_index_tsv: Path | None = None,
) -> list[MergedFunctionStatus]:
    module_rows = {
        (status.source_hint, status.address): status
        for module in modules
        for status in module.merged_function_statuses
        if status.source_hint
    }
    rows = load_ghidra_functions(
        ghidra_function_index_tsv or layout.inventory_ghidra_function_index_tsv_path
    )
    merged: list[MergedFunctionStatus] = []
    seen: set[tuple[str, str]] = set()
    for row in rows:
        source_hint = source_group_for_ghidra(row)
        key = (source_hint, row.entry)
        if key in seen:
            continue
        seen.add(key)
        if key in module_rows:
            merged.append(module_rows[key])
            continue
        merged.append(
            MergedFunctionStatus(
                module=f"bin:{source_hint}",
                address=row.entry,
                status="unlifted",
                ghidra_name=row.name,
                source_name="",
                source="",
                match_percent=None,
                exact_match=False,
                program=row.program_path or row.program_name,
                source_hint=source_hint,
            )
        )
    return sorted(
        merged,
        key=lambda status: (status.source_hint, status.address, status.ghidra_name),
    )


def top_complex_functions(
    modules: list[ModuleStatus], *, limit: int
) -> list[FunctionStatus]:
    functions = [status for module in modules for status in module.function_statuses]
    return sorted(
        functions,
        key=lambda status: (
            status.complexity_score,
            status.loc,
            status.branches,
            status.calls,
        ),
        reverse=True,
    )[:limit]


def render_percent(value: float | None) -> str:
    return "" if value is None else f"{value:.2f}%"


def render_module_table(modules: list[ModuleStatus]) -> str:
    lines = [
        "module\tc_files\tsource_funcs\t@source\tghidra_funcs\tghidra_lifted\tghidra_unlifted\tasm\texact\tavg_match\tvars/macros\tstructs\taddr_range"
    ]
    for module in modules:
        address_range = (
            ""
            if module.address_min is None
            else f"{module.address_min}-{module.address_max}"
        )
        lines.append(
            "\t".join(
                [
                    module.module,
                    str(module.c_files),
                    str(module.functions),
                    str(module.source_tagged),
                    str(module.ghidra_functions),
                    str(module.ghidra_lifted),
                    str(module.ghidra_unlifted),
                    str(module.asm_summaries),
                    str(module.exact_matches),
                    render_percent(module.avg_match_percent),
                    str(len(module.variables)),
                    str(len(module.structs)),
                    address_range,
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_ghidra_program_table(programs: list[GhidraProgramStatus]) -> str:
    lines = [
        "source_hint\tprogram\tghidra_funcs\tlifted\tunlifted\tthunks\tunlifted_samples"
    ]
    for program in programs:
        lines.append(
            "\t".join(
                [
                    program.source_hint,
                    program.program_path or program.program,
                    str(program.ghidra_functions),
                    str(program.lifted_functions),
                    str(program.unlifted_functions),
                    str(program.thunks),
                    ", ".join(program.unlifted_samples),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_function_table(modules: list[ModuleStatus]) -> str:
    return render_function_rows(
        [status for module in modules for status in module.merged_function_statuses]
    )


def render_function_rows(functions: list[MergedFunctionStatus]) -> str:
    lines = [
        "module\taddress\tstatus\tghidra_name\tsource_name\tmatch\texact\tsource\tprogram"
    ]
    for status in functions:
        lines.append(
            "\t".join(
                [
                    status.module,
                    status.address,
                    status.status,
                    status.ghidra_name,
                    status.source_name,
                    render_percent(status.match_percent),
                    "yes" if status.exact_match else "no",
                    status.source,
                    status.source_hint or status.program,
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_complex_table(functions: list[FunctionStatus]) -> str:
    lines = ["function\tsource\taddress\tscore\tloc\tbranches\tcalls\tmatch\texact"]
    for status in functions:
        lines.append(
            "\t".join(
                [
                    status.name,
                    status.source,
                    status.address or "",
                    str(status.complexity_score),
                    str(status.loc),
                    str(status.branches),
                    str(status.calls),
                    render_percent(status.match_percent),
                    "yes" if status.exact_match else "no",
                ]
            )
        )
    return "\n".join(lines) + "\n"


def render_details(modules: list[ModuleStatus]) -> str:
    lines: list[str] = []
    for module in modules:
        lines.append(f"[{module.module}]")
        if module.variables:
            lines.append("variables/macros: " + ", ".join(module.variables))
        if module.structs:
            lines.append("structs: " + ", ".join(module.structs))
        if module.ghidra_programs:
            lines.append("ghidra: " + ", ".join(module.ghidra_programs))
        if module.ghidra_unlifted_samples:
            lines.append(
                "ghidra-unlifted-samples: " + ", ".join(module.ghidra_unlifted_samples)
            )
        for status in module.merged_function_statuses:
            match = render_percent(status.match_percent) or "unmeasured"
            exact = " exact" if status.exact_match else ""
            lines.append(
                f"{status.address} {status.status}: "
                f"{status.ghidra_name or status.source_name}; {match}{exact}"
            )
        lines.append("")
    return "\n".join(lines)


def render_json(
    modules: list[ModuleStatus],
    complex_functions: list[FunctionStatus],
    ghidra_programs: list[GhidraProgramStatus],
) -> str:
    payload = {
        "modules": [asdict(module) for module in modules],
        "ghidra_programs": [asdict(program) for program in ghidra_programs],
        "complex_functions": [asdict(status) for status in complex_functions],
    }
    return json.dumps(payload, indent=2, sort_keys=True) + "\n"
