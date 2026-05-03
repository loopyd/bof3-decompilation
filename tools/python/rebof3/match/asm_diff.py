from __future__ import annotations

import difflib
import os
import re
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..jsonio import write_json
from ..paths import RepoLayout, repo_layout


PSX_EXE_MAGIC = b"PS-X EXE"
PSX_EXE_HEADER_SIZE = 0x800
SOURCE_ADDRESS_RE = re.compile(r"@source:\s*(0x[0-9a-fA-F]+|[0-9a-fA-F]{8})")
FUNC_NAME_RE = re.compile(r"func_([0-9a-fA-F]{8})")
INSTRUCTION_RE = re.compile(r"^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]{8}\s+(.+?)\s*$")
RELOCATION_RE = re.compile(
    r"^\s*[0-9a-fA-F]+:\s+R_MIPS_(?P<kind>\S+)\s+(?P<symbol>\S+)"
)
SYMBOL_SIZE_RE = re.compile(
    r"^(?P<address>[0-9a-fA-F]+)\s+(?P<size>[0-9a-fA-F]+)\s+[A-Za-z]\s+(?P<name>\S+)$"
)


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
        f"cannot infer original address from {source_path}; add an @source line or pass --address"
    )


def collect_source_addresses(source_dir: Path) -> list[tuple[Path, int]]:
    rows: list[tuple[Path, int]] = []
    for source_path in sorted(source_dir.glob("*.c")):
        try:
            rows.append((source_path, parse_source_address(source_path)))
        except ValueError:
            continue
    return sorted(rows, key=lambda row: (row[1], row[0].name))


def infer_size_from_sibling_sources(source_path: Path, address: int) -> int:
    for candidate_path, candidate_address in collect_source_addresses(
        source_path.parent
    ):
        if candidate_path == source_path:
            continue
        if candidate_address > address:
            return candidate_address - address
    raise ValueError(
        f"cannot infer original size for {source_path}; pass --size or add the next source in the same directory"
    )


def source_function_name(source_path: Path, address: int) -> str:
    match = FUNC_NAME_RE.search(source_path.stem)
    if match is not None:
        return f"func_{match.group(1).lower()}"
    return f"func_{address:08x}"


def object_path_for_source(layout: RepoLayout, source_path: Path) -> Path:
    resolved_source = source_path.expanduser().resolve()
    try:
        source_relative_to_project = resolved_source.relative_to(layout.bof3_dir)
    except ValueError as exc:
        raise ValueError(
            f"source must live under {layout.bof3_dir}: {source_path}"
        ) from exc
    return (
        layout.build_dir
        / "default"
        / "bof3"
        / "CMakeFiles"
        / "bof3.dir"
        / source_relative_to_project
    ).with_suffix(source_relative_to_project.suffix + ".obj")


def compiler_asm_path_for_object(object_path: Path) -> Path:
    return object_path.with_name(object_path.name + ".s")


def build_target_for_source(layout: RepoLayout, source_path: Path) -> str:
    resolved_source = source_path.expanduser().resolve()
    source_relative_to_project = resolved_source.relative_to(layout.bof3_dir)
    return f"bof3/CMakeFiles/bof3.dir/{source_relative_to_project.as_posix()}.obj"


def default_binary_for_source(layout: RepoLayout, source_path: Path) -> Path:
    resolved_source = source_path.expanduser().resolve()
    source_relative_to_project = resolved_source.relative_to(layout.bof3_dir)
    parts = source_relative_to_project.parts
    if parts[:2] == ("src", "core") or parts[:2] == ("src", "boot"):
        return layout.slus_path
    if parts[:3] == ("src", "modules", "logo"):
        return layout.logo_path
    raise ValueError(
        "overlay sources need an explicit --binary and --load-address until the overlay map is wired into asm diff"
    )


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
    build_ninja = layout.build_dir / "default" / "build.ninja"
    if not build_ninja.is_file():
        raise FileNotFoundError(f"missing {build_ninja}; run `bin/configure` first")
    target = build_target_for_source(layout, source_path)
    result = run_command(
        ["cmake", "--build", "--preset", "default", "--target", target],
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


def normalize_disassembly(disassembly: str) -> list[str]:
    lines: list[str] = []
    for raw_line in disassembly.splitlines():
        relocation = RELOCATION_RE.match(raw_line)
        if relocation is not None and lines:
            symbol = relocation.group("symbol")
            kind = relocation.group("kind")
            if kind == "26":
                mnemonic = lines[-1].split(" ", 1)[0]
                lines[-1] = f"{mnemonic} {symbol}"
            elif kind == "HI16":
                lines[-1] = f"{lines[-1]} # %hi({symbol})"
            elif kind == "LO16":
                lines[-1] = f"{lines[-1]} # %lo({symbol})"
            continue
        match = INSTRUCTION_RE.match(raw_line)
        if match is None:
            continue
        instruction = re.sub(r"\s+", " ", match.group(1).strip())
        lines.append(instruction)
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
    return {
        "schema": "rebof3-simple.asm-diff-one/v1",
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
        },
        "outputs": {
            "directory": str(output_dir),
            "summary_json": str(output_dir / "summary.json"),
            "diff": str(output_dir / "diff.patch"),
            "original_asm": str(output_dir / "original.normalized.s"),
            "current_asm": str(output_dir / "current.normalized.s"),
            "original_extracted_asm": str(output_dir / "original.objdump.s"),
            "current_compiler_asm": str(output_dir / "current.compiler.s"),
            "original_objdump": str(output_dir / "original.objdump.s"),
            "current_objdump": str(output_dir / "current.objdump.s"),
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
    original_size = (
        request.size
        if request.size is not None
        else infer_size_from_sibling_sources(source_path, address)
    )
    function_name = source_function_name(source_path, address)
    binary_path = (
        request.binary_path.expanduser().resolve()
        if request.binary_path is not None
        else default_binary_for_source(repo, source_path)
    )
    object_path = object_path_for_source(repo, source_path)
    output_root = request.output_root or repo.out_dir / "asm-diff"
    output_dir = output_root / function_name
    output_dir.mkdir(parents=True, exist_ok=True)

    run_build_object(repo, source_path, output_dir / "build.log")
    if not object_path.is_file():
        raise FileNotFoundError(f"expected object was not built: {object_path}")
    if not binary_path.is_file():
        raise FileNotFoundError(f"original binary not found: {binary_path}")

    objdump_path = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-objdump"
    nm_path = repo.psn00b_toolchain_root / "bin" / "mipsel-none-elf-nm"
    if not os.access(objdump_path, os.X_OK):
        raise FileNotFoundError(
            f"missing executable {objdump_path}; run `bin/setup-open`"
        )
    if not os.access(nm_path, os.X_OK):
        raise FileNotFoundError(f"missing executable {nm_path}; run `bin/setup-open`")
    original_bytes_path = output_dir / "original.bin"
    original_bytes_path.write_bytes(
        extract_original_bytes(
            binary_path,
            address=address,
            size=original_size,
            load_address=request.load_address,
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

    (output_dir / "original.objdump.s").write_text(original_objdump, encoding="utf-8")
    (output_dir / "current.objdump.s").write_text(current_objdump, encoding="utf-8")
    shutil.copyfile(current_compiler_asm, output_dir / "current.compiler.s")
    (output_dir / "original.normalized.s").write_text(
        render_normalized(original_lines), encoding="utf-8"
    )
    (output_dir / "current.normalized.s").write_text(
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
