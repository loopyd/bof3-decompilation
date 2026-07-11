from __future__ import annotations

import difflib
import json
import os
import re
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..jsonio import read_json, write_json
from ..paths import RepoLayout, repo_layout


PSX_EXE_MAGIC = b"PS-X EXE"
PSX_EXE_HEADER_SIZE = 0x800
SOURCE_ADDRESS_RE = re.compile(r"@source\s+(0x[0-9a-fA-F]+|[0-9a-fA-F]{8})")
FUNC_NAME_RE = re.compile(r"func_([0-9a-fA-F]{8})")
FUNC_SYMBOL_RE = re.compile(r"^func_([0-9a-fA-F]{8})$")
SYMBOL_ADDRESS_RE = re.compile(r"(?:^|_)(?P<address>[0-9a-fA-F]{8})(?:$|_)")
INSTRUCTION_RE = re.compile(
    r"^\s*(?P<address>[0-9a-fA-F]+):\s+[0-9a-fA-F]{8}\s+(?P<instruction>.+?)\s*$"
)
RELOCATION_RE = re.compile(
    r"^\s*[0-9a-fA-F]+:\s+R_MIPS_(?P<kind>\S+)\s+(?P<symbol>\S+)"
)
SYMBOL_SIZE_RE = re.compile(
    r"^(?P<address>[0-9a-fA-F]+)\s+(?P<size>[0-9a-fA-F]+)\s+[A-Za-z]\s+(?P<name>\S+)$"
)
IMPLAUSIBLE_SIBLING_FUNCTION_SIZE = 0x1000
MIPS_JR_RA = 0x03E00008


@dataclass(frozen=True)
class PsxExeInfo:
    load_address: int
    payload_size: int
    payload_offset: int = PSX_EXE_HEADER_SIZE


@dataclass(frozen=True)
class AsmDiffRequest:
    source_path: Path
    address: int | None = None
    size: int | None = None
    binary_path: Path | None = None
    load_address: int | None = None
    output_root: Path | None = None


def parse_int(value: str | int) -> int:
    if isinstance(value, int):
        return value
    return int(value, 0)


def format_hex(value: int) -> str:
    return f"0x{value:08x}"


def read_u32le(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], byteorder="little")


def read_psx_exe_info(path: Path) -> PsxExeInfo | None:
    header = path.read_bytes()[:PSX_EXE_HEADER_SIZE]
    if not header.startswith(PSX_EXE_MAGIC):
        return None
    return PsxExeInfo(
        load_address=read_u32le(header, 0x18),
        payload_size=read_u32le(header, 0x1C),
    )


def parse_source_address(source_path: Path) -> int:
    text = source_path.read_text(encoding="utf-8")
    match = SOURCE_ADDRESS_RE.search(text)
    if match is not None:
        return parse_int(match.group(1))
    match = FUNC_NAME_RE.search(source_path.stem)
    if match is not None:
        return int(match.group(1), 16)
    raise ValueError(
        f"cannot infer original address from {source_path}; add @source or pass --address"
    )


def collect_source_addresses(source_dir: Path) -> list[tuple[Path, int]]:
    rows: list[tuple[Path, int]] = []
    for source_path in sorted(source_dir.glob("*.c")):
        try:
            rows.append((source_path, parse_source_address(source_path)))
        except ValueError:
            continue
    return sorted(rows, key=lambda row: (row[1], row[0].name))


def infer_size_from_sibling_sources(source_path: Path, address: int) -> int | None:
    for candidate_path, candidate_address in collect_source_addresses(
        source_path.parent
    ):
        if candidate_path == source_path:
            continue
        if candidate_address > address:
            size = candidate_address - address
            if size <= IMPLAUSIBLE_SIBLING_FUNCTION_SIZE:
                return size
            break
    return None


def infer_size_from_binary_return(
    binary_path: Path,
    *,
    address: int,
    load_address: int | None,
) -> int:
    binary_data = binary_path.read_bytes()
    psx_info = read_psx_exe_info(binary_path)
    if psx_info is None:
        if load_address is None:
            raise ValueError(
                f"{binary_path} is not a PS-X EXE; pass --load-address for raw binaries"
            )
        payload_offset = 0
        payload_load_address = load_address
        payload_size = len(binary_data)
    else:
        payload_offset = psx_info.payload_offset
        payload_load_address = psx_info.load_address
        payload_size = psx_info.payload_size

    relative_offset = address - payload_load_address
    if relative_offset < 0 or relative_offset >= payload_size:
        raise ValueError(
            f"{format_hex(address)} is outside {binary_path} loaded at {format_hex(payload_load_address)}"
        )

    for offset in range(
        payload_offset + relative_offset, payload_offset + payload_size, 4
    ):
        if read_u32le(binary_data, offset) == MIPS_JR_RA:
            return offset - payload_offset - relative_offset + 8
    raise ValueError(
        f"cannot infer original size for {format_hex(address)} from {binary_path}; pass --size"
    )


def _infer_size_from_function_index(source_path: Path, address: int) -> int:
    """Fallback: look up function size from the Ghidra function index."""
    layout = repo_layout()
    func_index = layout.out_dir / "inventory" / "ghidra_function_index.json"
    if func_index.is_file():
        payload = read_json(func_index)
        rows = payload.get("rows", [])
        entry_hex = f"0x{address:08x}"
        for row in rows:
            if row.get("entry_hex") != entry_hex:
                continue
            body_min = row.get("body_min", "")
            body_max = row.get("body_max", "")
            if body_min and body_max:
                return int(body_max, 16) - int(body_min, 16) + 1
    raise ValueError(
        f"cannot infer original size for {source_path}; pass --size or add the next source in the same directory"
    )


def infer_original_size(
    source_path: Path,
    *,
    address: int,
    binary_path: Path,
    load_address: int | None,
) -> int:
    sibling_size = infer_size_from_sibling_sources(source_path, address)
    if sibling_size is not None:
        return sibling_size
    try:
        return _infer_size_from_function_index(source_path, address)
    except ValueError:
        return infer_size_from_binary_return(
            binary_path, address=address, load_address=load_address
        )


def source_function_name(source_path: Path, address: int) -> str:
    match = FUNC_NAME_RE.search(source_path.stem)
    if match is not None:
        return f"func_{match.group(1).lower()}"
    return f"func_{address:08x}"


def object_path_for_source(layout: RepoLayout, source_path: Path) -> Path:
    command = compile_command_for_source(layout, source_path)
    return Path(command["directory"]) / command["output"]


def compiler_asm_path_for_object(object_path: Path) -> Path:
    return object_path.with_name(object_path.name + ".s")


def build_target_for_source(layout: RepoLayout, source_path: Path) -> str:
    source_rel = source_path.expanduser().resolve().relative_to(layout.root.resolve())
    return source_rel.with_suffix(".obj").as_posix()


def compile_command_for_source(layout: RepoLayout, source_path: Path) -> dict[str, str]:
    commands_path = layout.build_dir / "default" / "compile_commands.json"
    if not commands_path.is_file():
        raise FileNotFoundError(f"missing {commands_path}; run `just build` first")
    payload = json.loads(commands_path.read_text(encoding="utf-8"))
    if not isinstance(payload, list):
        raise ValueError(f"expected a JSON array in {commands_path}")
    resolved_source = source_path.expanduser().resolve()
    matches = [
        row
        for row in payload
        if isinstance(row, dict)
        and Path(str(row.get("file", ""))).resolve() == resolved_source
        and isinstance(row.get("directory"), str)
        and isinstance(row.get("output"), str)
    ]
    if len(matches) != 1:
        raise ValueError(
            f"expected one CMake compile command for {source_path}, found {len(matches)}"
        )
    return {
        "directory": str(matches[0]["directory"]),
        "output": str(matches[0]["output"]),
    }


CMAKE_TARGET_RE = re.compile(r"(?:^|/)CMakeFiles/(?P<target>[^/]+)\.dir/")


def default_binary_for_source(layout: RepoLayout, source_path: Path) -> Path:
    """Resolve the original binary for a source file."""
    resolved_source = source_path.expanduser().resolve()
    try:
        source_rel = resolved_source.relative_to(layout.bof3_dir).as_posix()
    except ValueError:
        source_rel = ""
    parts = source_rel.split("/")

    if parts[:2] == ["src", "core"] or parts[:2] == ["src", "boot"]:
        return layout.slus_path

    if parts[:3] == ["src", "modules", "logo"]:
        return layout.logo_path

    artifact = _artifact_overlay_for_source(layout, source_path)
    if artifact is not None:
        return artifact[0]
    raise ValueError(f"cannot resolve original binary for overlay source: {source_rel}")


def overlay_load_address_for_source(
    layout: RepoLayout, source_path: Path
) -> int | None:
    """Return the load address for an overlay source file."""
    resolved_source = source_path.expanduser().resolve()
    try:
        source_rel = resolved_source.relative_to(layout.bof3_dir).as_posix()
    except ValueError:
        return None

    if source_rel.startswith("src/modules/logo"):
        return 0x801CE000

    artifact = _artifact_overlay_for_source(layout, source_path)
    return artifact[1] if artifact is not None else None


def _artifact_overlay_for_source(
    layout: RepoLayout, source_path: Path
) -> tuple[Path, int] | None:
    """Resolve overlay ownership through CMake's artifact source hint."""
    try:
        output = compile_command_for_source(layout, source_path)["output"]
    except (FileNotFoundError, ValueError):
        return None
    target_match = CMAKE_TARGET_RE.search(output)
    if target_match is None:
        return None
    manifest_path = (
        layout.build_dir / "default" / "artifacts" / "metadata" / "artifacts.json"
    )
    catalog_path = layout.root / "out" / "catalog" / "emi.json"
    if not manifest_path.is_file() or not catalog_path.is_file():
        return None
    target = target_match.group("target")
    manifest = read_json(manifest_path)
    rows = [row for row in manifest.get("artifacts", []) if row.get("target") == target]
    if len(rows) != 1:
        return None
    source_hint = str(rows[0].get("source_hint", ""))
    if ".EMI#" not in source_hint.upper():
        return None
    from ..binaries import resolve_entry

    normalized_hint = source_hint.replace("\\", "/")
    marker = "BIN/"
    marker_offset = normalized_hint.upper().find(marker)
    if marker_offset >= 0:
        normalized_hint = normalized_hint[marker_offset:]
    entry = resolve_entry(read_json(catalog_path), normalized_hint)
    return Path(entry["payload_path"]), int(entry["load_address"])


def extract_original_bytes(
    binary_path: Path,
    *,
    address: int,
    size: int,
    load_address: int | None,
) -> bytes:
    binary_data = binary_path.read_bytes()
    psx_info = read_psx_exe_info(binary_path)
    if psx_info is None:
        if load_address is None:
            raise ValueError(
                f"{binary_path} is not a PS-X EXE; pass --load-address for raw binaries"
            )
        payload_offset = 0
        payload_load_address = load_address
        payload_size = len(binary_data)
    else:
        payload_offset = psx_info.payload_offset
        payload_load_address = psx_info.load_address
        payload_size = psx_info.payload_size

    relative_offset = address - payload_load_address
    if relative_offset < 0 or relative_offset + size > payload_size:
        raise ValueError(
            f"{format_hex(address)}..{format_hex(address + size)} is outside {binary_path} loaded at {format_hex(payload_load_address)}"
        )
    file_offset = payload_offset + relative_offset
    return binary_data[file_offset : file_offset + size]


def run_command(
    argv: list[str], *, cwd: Path | None = None
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        argv,
        cwd=cwd,
        check=False,
        capture_output=True,
        text=True,
    )


def run_build_object(
    layout: RepoLayout, source_path: Path, build_log_path: Path
) -> None:
    build_dir = layout.build_dir / "default"
    if (
        not (build_dir / "build.ninja").is_file()
        and not (build_dir / "Makefile").is_file()
    ):
        raise FileNotFoundError(
            f"no build.ninja or Makefile in {build_dir}; run `just build` first"
        )
    target = build_target_for_source(layout, source_path)
    result = run_command(
        ["cmake", "--build", build_dir.as_posix(), "--target", target],
        cwd=layout.root,
    )
    log_text = result.stdout
    if result.stderr:
        if log_text:
            log_text += "\n"
        log_text += result.stderr
    build_log_path.write_text(log_text, encoding="utf-8")
    if result.returncode != 0:
        raise RuntimeError(f"object build failed for {target}; see {build_log_path}")
    object_path = object_path_for_source(layout, source_path)
    if (
        not object_path.is_file()
        or object_path.stat().st_mtime < source_path.stat().st_mtime
    ):
        raise RuntimeError(
            f"object build did not refresh {object_path}; see {build_log_path}"
        )


def disassemble_original(
    *,
    objdump_path: Path,
    original_bytes_path: Path,
    address: int,
) -> str:
    result = run_command(
        [
            str(objdump_path),
            "-D",
            "-b",
            "binary",
            "-m",
            "mips:3000",
            "-EL",
            f"--adjust-vma={format_hex(address)}",
            str(original_bytes_path),
        ]
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr or result.stdout)
    return result.stdout


def disassemble_current(*, objdump_path: Path, object_path: Path) -> str:
    result = run_command([str(objdump_path), "-dr", str(object_path)])
    if result.returncode != 0:
        raise RuntimeError(result.stderr or result.stdout)
    return result.stdout


def symbol_address(symbol: str) -> int | None:
    match = SYMBOL_ADDRESS_RE.search(symbol)
    if match is None:
        return None
    return int(match.group("address"), 16)


def mips_hi(value: int) -> int:
    return ((value + 0x8000) >> 16) & 0xFFFF


def mips_lo(value: int) -> int:
    lo = value & 0xFFFF
    if lo >= 0x8000:
        return lo - 0x10000
    return lo


def replace_final_immediate(instruction: str, value: str) -> str:
    replaced = re.sub(
        r"[-+]?(?:0x[0-9a-fA-F]+|\d+)(?=\([^)]*\)$)",
        value,
        instruction,
        count=1,
    )
    if replaced != instruction:
        return replaced
    return re.sub(
        r",[-+]?(?:0x[0-9a-fA-F]+|\d+)$",
        f",{value}",
        instruction,
        count=1,
    )


def apply_relocation(instruction: str, kind: str, symbol: str) -> str:
    if kind == "26":
        if symbol_match := FUNC_SYMBOL_RE.match(symbol):
            symbol = f"0x{symbol_match.group(1).lower()}"
        mnemonic = instruction.split(" ", 1)[0]
        return f"{mnemonic} {symbol}"

    address = symbol_address(symbol)
    if address is None:
        if kind == "HI16":
            return f"{instruction} # %hi({symbol})"
        if kind == "LO16":
            return f"{instruction} # %lo({symbol})"
        return instruction

    mnemonic = instruction.split(" ", 1)[0]
    if kind == "HI16":
        return replace_final_immediate(instruction, f"0x{mips_hi(address):x}")
    if kind == "LO16":
        lo = mips_lo(address)
        if mnemonic == "ori":
            return replace_final_immediate(instruction, f"0x{lo & 0xFFFF:x}")
        return replace_final_immediate(instruction, str(lo))
    return instruction


def normalize_branch_target(instruction: str, address: int) -> str:
    if " " not in instruction:
        return instruction
    mnemonic, operands_text = instruction.split(" ", 1)
    if mnemonic not in {"beq", "bne", "beqz", "bnez", "bgtz", "blez", "bgez", "bltz"}:
        return instruction

    operands = [operand.strip() for operand in operands_text.split(",")]
    if not operands:
        return instruction

    target_text = operands[-1].split(" ", 1)[0]
    try:
        target = int(target_text, 0 if target_text.startswith("0x") else 16)
    except ValueError:
        return instruction

    operands[-1] = str(target - address)
    return f"{mnemonic} {','.join(operands)}"


def normalize_disassembly(disassembly: str) -> list[str]:
    lines: list[str] = []
    for raw_line in disassembly.splitlines():
        relocation = RELOCATION_RE.match(raw_line)
        if relocation is not None and lines:
            lines[-1] = apply_relocation(
                lines[-1], relocation.group("kind"), relocation.group("symbol")
            )
            continue
        match = INSTRUCTION_RE.match(raw_line)
        if match is None:
            continue
        address = int(match.group("address"), 16)
        instruction = re.sub(r"\s+", " ", match.group("instruction").strip())
        lines.append(normalize_branch_target(instruction, address))
    return lines


def render_normalized(lines: list[str]) -> str:
    return "\n".join(lines) + ("\n" if lines else "")


def render_diff(original_lines: list[str], current_lines: list[str]) -> str:
    diff_lines = difflib.unified_diff(
        original_lines,
        current_lines,
        fromfile="original",
        tofile="current",
        lineterm="",
    )
    return "\n".join(diff_lines) + "\n"


def matching_instruction_count(
    original_lines: list[str], current_lines: list[str]
) -> int:
    matcher = difflib.SequenceMatcher(a=original_lines, b=current_lines, autojunk=False)
    return sum(block.size for block in matcher.get_matching_blocks())


def current_symbol_size(
    nm_path: Path, object_path: Path, function_name: str
) -> int | None:
    result = run_command([str(nm_path), "-S", str(object_path)])
    if result.returncode != 0:
        return None
    for raw_line in result.stdout.splitlines():
        match = SYMBOL_SIZE_RE.match(raw_line.strip())
        if match is None or match.group("name") != function_name:
            continue
        return int(match.group("size"), 16)
    return None


def build_result_payload(
    *,
    source_path: Path,
    function_name: str,
    address: int,
    original_size: int,
    current_size: int | None,
    binary_path: Path,
    object_path: Path,
    output_dir: Path,
    original_lines: list[str],
    current_lines: list[str],
) -> dict[str, Any]:
    status = "exact_match" if original_lines == current_lines else "different"
    matching_count = matching_instruction_count(original_lines, current_lines)
    denominator = max(len(original_lines), len(current_lines), 1)
    match_percent = round((matching_count / denominator) * 100, 2)
    return {
        "schema": "rebof3-simple.asm-diff-one/v2",
        "status": status,
        "exact_match": status == "exact_match",
        "source": str(source_path),
        "function": function_name,
        "address": format_hex(address),
        "original_size": original_size,
        "current_size": current_size,
        "size_delta": None if current_size is None else current_size - original_size,
        "original_binary": str(binary_path),
        "current_object": str(object_path),
        "instruction_count": {
            "original": len(original_lines),
            "current": len(current_lines),
            "matching": matching_count,
            "match_percent": match_percent,
        },
        "outputs": {
            "directory": str(output_dir),
            "summary": str(output_dir / "summary.json"),
            "diff": str(output_dir / "diff.patch"),
            "original": str(output_dir / "original.s"),
            "current": str(output_dir / "current.s"),
            "compiler": str(output_dir / "compiler.s"),
            "original_bytes": str(output_dir / "original.bin"),
            "build_log": str(output_dir / "build.log"),
        },
    }


def run_asm_diff_one(
    request: AsmDiffRequest,
    *,
    layout: RepoLayout | None = None,
) -> dict[str, Any]:
    repo = layout or repo_layout()
    source_path = request.source_path.expanduser().resolve()
    address = (
        request.address
        if request.address is not None
        else parse_source_address(source_path)
    )
    function_name = source_function_name(source_path, address)
    binary_path = (
        request.binary_path.expanduser().resolve()
        if request.binary_path is not None
        else default_binary_for_source(repo, source_path)
    )
    load_address = request.load_address or overlay_load_address_for_source(
        repo, source_path
    )
    original_size = (
        request.size
        if request.size is not None
        else infer_original_size(
            source_path,
            address=address,
            binary_path=binary_path,
            load_address=load_address,
        )
    )
    object_path = object_path_for_source(repo, source_path)
    output_root = request.output_root or repo.out_dir / "asm-diff"
    output_dir = output_root / function_name
    if output_dir.is_dir():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    run_build_object(repo, source_path, output_dir / "build.log")
    if not object_path.is_file():
        raise FileNotFoundError(f"expected object was not built: {object_path}")
    if not binary_path.is_file():
        raise FileNotFoundError(f"original binary not found: {binary_path}")

    objdump_path = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objdump"
    nm_path = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-nm"
    if not os.access(objdump_path, os.X_OK):
        raise FileNotFoundError(f"missing executable {objdump_path}; run `just setup`")
    if not os.access(nm_path, os.X_OK):
        raise FileNotFoundError(f"missing executable {nm_path}; run `just setup`")
    original_bytes_path = output_dir / "original.bin"
    original_bytes_path.write_bytes(
        extract_original_bytes(
            binary_path,
            address=address,
            size=original_size,
            load_address=load_address,
        )
    )

    original_objdump = disassemble_original(
        objdump_path=objdump_path,
        original_bytes_path=original_bytes_path,
        address=address,
    )
    current_objdump = disassemble_current(
        objdump_path=objdump_path, object_path=object_path
    )
    current_compiler_asm = compiler_asm_path_for_object(object_path)
    if not current_compiler_asm.is_file():
        raise FileNotFoundError(
            f"expected compiler assembly was not written: {current_compiler_asm}"
        )
    original_lines = normalize_disassembly(original_objdump)
    current_lines = normalize_disassembly(current_objdump)
    current_size = current_symbol_size(nm_path, object_path, function_name)

    shutil.copyfile(current_compiler_asm, output_dir / "compiler.s")
    (output_dir / "original.s").write_text(
        render_normalized(original_lines), encoding="utf-8"
    )
    (output_dir / "current.s").write_text(
        render_normalized(current_lines), encoding="utf-8"
    )
    (output_dir / "diff.patch").write_text(
        render_diff(original_lines, current_lines), encoding="utf-8"
    )

    payload = build_result_payload(
        source_path=source_path,
        function_name=function_name,
        address=address,
        original_size=original_size,
        current_size=current_size,
        binary_path=binary_path,
        object_path=object_path,
        output_dir=output_dir,
        original_lines=original_lines,
        current_lines=current_lines,
    )
    write_json(output_dir / "summary.json", payload)
    return payload
