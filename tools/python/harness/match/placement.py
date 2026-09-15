"""Native placement and bounded extraction for grouped function sections."""

from __future__ import annotations

import hashlib
import re
import shutil
import tempfile
from collections.abc import Mapping
from dataclasses import dataclass
from pathlib import Path

from harness.build.sections import function_section
from harness.common.deadlines import check_deadline, use_deadline
from harness.common.files import atomic_write
from harness.common.paths import require_absent
from harness.io import RepoLayout
from harness.match.execution import NativeExecution
from harness.match.extraction import read_function_image
from harness.match.flow import validate_function_flow

_SECTION = re.compile(r"[.A-Za-z_][.A-Za-z0-9_]*")
_HEADER = re.compile(
    r"^\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+\S+\s+\S+\s+2\*\*(\d+)\s*$"
)
_METADATA = {".reginfo", ".MIPS.abiflags"}


@dataclass(frozen=True)
class ObjectSection:
    size: int
    address: int
    alignment: int
    executable: bool


def read_allocated_sections(
    object_path: Path, objdump: Path, *, execution: NativeExecution
) -> dict[str, ObjectSection]:
    """Read every nonempty allocated input section from the configured GNU tool."""

    with object_path.open("rb") as stream:
        header = stream.read(20)
    if (
        len(header) != 20
        or header[:7] != b"\x7fELF\x01\x01\x01"
        or header[18:20] != b"\x08\x00"
    ):
        raise ValueError("grouped placement requires ELF32 little-endian MIPS")
    text = execution.run_text([objdump, "-h", object_path])
    sections = {}
    lines = text.splitlines()
    for position, line in enumerate(lines):
        match = _HEADER.match(line)
        if match is None:
            if re.match(r"\s*\d+\s+", line):
                raise ValueError("unsupported object section header")
            continue
        name, size_text, address, alignment = match.groups()
        size = int(size_text, 16)
        if position + 1 >= len(lines):
            raise ValueError("missing object section flags")
        flags = {flag.strip() for flag in lines[position + 1].split(",")}
        if size and "ALLOC" in flags:
            if name in sections:
                raise ValueError(f"duplicate object section: {name}")
            sections[name] = ObjectSection(
                size, int(address, 16), 1 << int(alignment), "CODE" in flags
            )
    return sections


def _placements(
    functions: Mapping[str, int], data: Mapping[str, int]
) -> dict[str, int]:
    if len(functions) < 2 or len(set(functions.values())) != len(functions):
        raise ValueError("grouped placement requires distinct function addresses")
    placements = {
        function_section(name): address for name, address in functions.items()
    }
    for name, address in data.items():
        if (
            not _SECTION.fullmatch(name)
            or name.startswith(".bof3.")
            or name in _METADATA
        ):
            raise ValueError(f"unsupported data section placement: {name!r}")
        placements[name] = address
    for address in placements.values():
        if type(address) is not int or not 0 <= address <= 0xFFFFFFFF or address % 4:
            raise ValueError("section addresses must be aligned 32-bit integers")
    return placements


def _validate_sections(
    sections: Mapping[str, ObjectSection],
    placements: Mapping[str, int],
    functions: Mapping[str, int],
    *,
    linked: bool,
) -> None:
    required = {function_section(name) for name in functions}
    if not required <= sections.keys():
        raise ValueError("object omits required compiled function sections")
    ranges = []
    for name, section in sections.items():
        if name in _METADATA:
            continue
        if name not in placements or section.executable != (name in required):
            raise ValueError(
                f"runtime section has no compatible reviewed placement: {name}"
            )
        address = placements[name]
        if address % section.alignment or address + section.size > 0x100000000:
            raise ValueError(f"unaligned or overflowing section placement: {name}")
        if linked and section.address != address:
            raise ValueError(f"linked section moved from its reviewed address: {name}")
        ranges.append((address, address + section.size, name))
    ranges.sort()
    for previous, following in zip(ranges, ranges[1:]):
        if previous[1] > following[0]:
            raise ValueError(
                f"overlapping section placements: {previous[2]}, {following[2]}"
            )


def _read_function_symbols(
    path: Path, objdump: Path, execution: NativeExecution
) -> dict[str, tuple[int, str, int]]:
    rows = {}
    for line in execution.run_text([objdump, "-t", path]).splitlines():
        fields = line.split()
        if len(fields) < 6 or "F" not in fields[1:-3] or fields[-3] == "*UND*":
            continue
        name = fields[-1]
        if name in rows:
            raise ValueError(f"duplicate linked function: {name}")
        rows[name] = (int(fields[0], 16), fields[-3], int(fields[-2], 16))
    return rows


def _validate_symbols(
    path: Path,
    objdump: Path,
    functions: Mapping[str, int],
    sections: Mapping[str, ObjectSection],
    execution: NativeExecution,
) -> None:
    rows = _read_function_symbols(path, objdump, execution)
    if set(rows) != set(functions):
        raise ValueError("linked symbols do not cover the complete function inventory")
    for name, (address, section, size) in rows.items():
        expected = function_section(name)
        if (
            address != functions[name]
            or section != expected
            or size != sections[expected].size
        ):
            raise ValueError(
                f"linked function has unaccounted address, prefix or tail bytes: {name}"
            )


def link_grouped_object(
    object_path: Path,
    output: Path,
    functions: Mapping[str, int],
    *,
    layout: RepoLayout,
    execution: NativeExecution,
    bindings: Mapping[str, int] | None = None,
    data_sections: Mapping[str, int] | None = None,
) -> Path:
    """Return the linked path after guarded complete-group placement."""
    result = place_grouped_object(
        object_path,
        output,
        functions,
        layout=layout,
        execution=execution,
        bindings=bindings,
        data_sections=data_sections,
    )
    return Path(result["path"])


def place_grouped_object(
    object_path: Path,
    output: Path,
    functions: Mapping[str, int],
    *,
    layout: RepoLayout,
    execution: NativeExecution,
    bindings: Mapping[str, int] | None = None,
    data_sections: Mapping[str, int] | None = None,
) -> dict:
    """Validate privately, then publish confined new artifacts without replacement.

    Failure retains scratch and any partial publication for inspection. Publication
    of the script and ELF is not an atomic pair; neither existing leaf is replaced.
    """

    if execution.root != layout.root:
        raise ValueError("native execution root differs from placement root")
    execution.check_deadline()
    output = output if output.is_absolute() else layout.root / output
    script = output.with_suffix(output.suffix + ".ld")
    output_name = output.relative_to(layout.root).as_posix()
    script_name = script.relative_to(layout.root).as_posix()
    require_absent(layout.root, output_name)
    require_absent(layout.root, script_name)
    if (
        output.resolve() == object_path.resolve()
        or script.resolve() == object_path.resolve()
    ):
        raise ValueError("grouped link output aliases its input")
    scratch = Path(tempfile.mkdtemp(prefix="bof3-grouped-link-"))
    try:
        private_input = scratch / "input.o"
        private_input.write_bytes(object_path.read_bytes())
        private_output = scratch / "linked.elf"
        private_script = private_output.with_suffix(".elf.ld")
        _link_object(
            private_input,
            private_output,
            functions,
            layout=layout,
            execution=execution,
            bindings=bindings,
            data_sections=data_sections,
        )
        execution.check_deadline()
        script_content = private_script.read_bytes()
        linked_content = private_output.read_bytes()
        result = {
            "path": str(output),
            "sha256": hashlib.sha256(linked_content).hexdigest(),
            "script": str(script),
            "script_sha256": hashlib.sha256(script_content).hexdigest(),
        }
        atomic_write(layout.root, script_name, script_content, exclusive=True)
        execution.check_deadline()
        atomic_write(layout.root, output_name, linked_content, exclusive=True)
    except BaseException as error:
        error.add_note(
            f"grouped native scratch retained at {scratch}; inspect partial outputs"
        )
        raise
    shutil.rmtree(scratch)
    execution.check_deadline()
    check_deadline()
    return result


def _link_object(
    object_path: Path,
    output: Path,
    functions: Mapping[str, int],
    *,
    layout: RepoLayout,
    execution: NativeExecution,
    bindings: Mapping[str, int] | None,
    data_sections: Mapping[str, int] | None,
) -> None:
    script = output.with_suffix(output.suffix + ".ld")
    tools = layout.psn00b_toolchain_root / "bin"
    objdump = tools / "mipsel-none-elf-objdump"
    placements = _placements(functions, data_sections or {})
    sections = read_allocated_sections(object_path, objdump, execution=execution)
    _validate_sections(sections, placements, functions, linked=False)
    definitions = []
    for line in execution.run_text(
        [str(tools / "mipsel-none-elf-nm"), "-u", str(object_path)]
    ).splitlines():
        fields = line.split()
        if len(fields) != 2 or fields[0] != "U" or fields[1] not in (bindings or {}):
            raise ValueError(
                f"undefined symbol has no explicit target binding: {line.strip()}"
            )
        name = fields[1]
        function_section(name)
        address = (bindings or {})[name]
        if type(address) is not int or not 0 <= address <= 0xFFFFFFFF:
            raise ValueError(f"invalid target binding: {name}")
        definitions.append(f"--defsym={name}=0x{address:08X}")
    lines = ["SECTIONS", "{"]
    for name, address in sorted(
        placements.items(), key=lambda item: (item[1], item[0])
    ):
        common = " *(COMMON) *(.scommon)" if name == ".bss" else ""
        lines.append(f"  {name} 0x{address:08X} : {{ KEEP(*({name})){common} }}")
    lines.extend(["  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) }", "}"])
    with script.open("x", encoding="utf-8") as stream:
        stream.write("\n".join(lines) + "\n")
    execution.run_text(
        [
            str(tools / "mipsel-none-elf-ld"),
            "-EL",
            "-T",
            str(script),
            *definitions,
            str(object_path),
            "-o",
            str(output),
        ]
    )
    linked = read_allocated_sections(output, objdump, execution=execution)
    _validate_sections(linked, placements, functions, linked=True)
    _validate_symbols(output, objdump, functions, linked, execution)


def extract_grouped_function(
    linked: Path,
    name: str,
    *,
    address: int,
    size: int,
    layout: RepoLayout,
    execution: NativeExecution,
) -> bytes:
    """Extract the complete named range, rejecting prefix/truncation-based success."""

    if execution.root != layout.root:
        raise ValueError("native execution root differs from extraction root")
    execution.check_deadline()
    check_deadline()
    scratch = Path(tempfile.mkdtemp(prefix="bof3-grouped-extract-"))
    try:
        private_input = scratch / "linked.elf"
        private_input.write_bytes(linked.read_bytes())
        content = _extract_function(
            private_input,
            name,
            address=address,
            size=size,
            layout=layout,
            execution=execution,
        )
    except BaseException as error:
        error.add_note(f"grouped native scratch retained at {scratch}")
        raise
    shutil.rmtree(scratch)
    execution.check_deadline()
    check_deadline()
    return content


def _extract_function(
    linked: Path,
    name: str,
    *,
    address: int,
    size: int,
    layout: RepoLayout,
    execution: NativeExecution,
) -> bytes:
    if type(size) is not int or size < 8 or size % 4:
        raise ValueError(
            "reviewed function size must be a positive instruction multiple"
        )
    if type(address) is not int or not 0 <= address <= 0xFFFFFFFF or address % 4:
        raise ValueError("reviewed function address must be an aligned 32-bit integer")
    if size > execution.output_limit:
        raise ValueError("reviewed function exceeds native extraction output limit")
    tools = layout.psn00b_toolchain_root / "bin"
    section = function_section(name)
    sections = read_allocated_sections(
        linked, tools / "mipsel-none-elf-objdump", execution=execution
    )
    actual = sections.get(section)
    symbol = _read_function_symbols(
        linked, tools / "mipsel-none-elf-objdump", execution
    ).get(name)
    if (
        actual is None
        or actual.address != address
        or actual.size != size
        or not actual.executable
        or symbol != (address, section, size)
    ):
        raise ValueError("linked function does not occupy exactly the reviewed range")
    with use_deadline(execution.deadline):
        image = read_function_image(linked, function_name=name, address=address)
        check_deadline()
        if (
            image.section != section
            or image.section_address != address
            or image.section_size != size
            or len(image.content) != size
        ):
            raise RuntimeError(
                "grouped function extraction failed or returned a short range"
            )
        content = image.content
        check_deadline()
        validate_function_flow(content, address)
        check_deadline()
        execution.check_deadline()
        return content
