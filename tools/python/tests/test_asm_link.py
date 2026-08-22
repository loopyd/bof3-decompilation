"""Fixture-local test for relocation-aware function comparison.

Uses the real PSn00b binutils (as/ld/objcopy/nm) available in the repo's
toolchains/psn00b_toolchain directory to verify that:
  1. A minimal MIPS object can be assembled.
  2. link_object_at_address links it at a chosen text address.
  3. extract_function_bytes extracts the correct bytes from the linked output.

This requires PSn00b toolchain binutils on the host at the standard repo path.
"""

from __future__ import annotations

import subprocess
from pathlib import Path

import pytest


_REPO_ROOT = Path(__file__).resolve().parents[3]


def _psn00b_toolchain_root() -> Path | None:
    """Return the PSn00b toolchain root if available, else None."""
    candidate = _REPO_ROOT / "toolchains" / "psn00b_toolchain"
    ld = candidate / "bin" / "mipsel-none-elf-ld"
    return candidate if ld.is_file() else None


_TOOLCHAIN = _psn00b_toolchain_root()

pytestmark = [
    pytest.mark.skipif(
        _TOOLCHAIN is None,
        reason="PSn00b toolchain not installed at toolchains/psn00b_toolchain",
    ),
]


def _assemble_minimal(as_path: Path, tmp: Path) -> Path:
    """Assemble a minimal MIPS stub returning from a function."""
    src = tmp / "minimal.s"
    src.write_text(
        ".text\n"
        ".globl func_80010000\n"
        ".ent func_80010000\n"
        "func_80010000:\n"
        "  jr $ra\n"
        "  nop\n"
        ".end func_80010000\n"
    )
    obj = tmp / "minimal.o"
    result = subprocess.run(
        [str(as_path), "-EL", "-o", str(obj), str(src)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"assembler failed: {result.stderr}")
    return obj


class TestLinkAtAddress:
    """Test relocation-aware linking without game inputs."""

    @pytest.fixture(autouse=True)
    def _setup(self, tmp_path: Path) -> None:
        self.tmp = tmp_path
        root = _psn00b_toolchain_root()
        assert root is not None  # skipif guard
        self.as_path = root / "bin" / "mipsel-none-elf-as"
        self.ld_path = root / "bin" / "mipsel-none-elf-ld"
        self.objcopy_path = root / "bin" / "mipsel-none-elf-objcopy"
        self.nm_path = root / "bin" / "mipsel-none-elf-nm"

    def test_assemble_minimal_object(self) -> None:
        """A trivial .s file assembles to a valid .o."""
        obj = _assemble_minimal(self.as_path, self.tmp)
        assert obj.stat().st_size > 0, "assembled object is empty"

    def test_link_at_address_text_section(self) -> None:
        """Linking at an address places .text at that address."""
        obj = _assemble_minimal(self.as_path, self.tmp)
        linked = self.tmp / "linked.elf"
        result = subprocess.run(
            [
                str(self.ld_path),
                "-EL",
                "-Ttext=0x80010000",
                str(obj),
                "-o",
                str(linked),
            ],
            capture_output=True,
            text=True,
        )
        assert result.returncode == 0, f"link failed: {result.stderr}"

        # objdump to verify the .text section header
        objdump_path = self.as_path.parent / "mipsel-none-elf-objdump"
        dump = subprocess.run(
            [str(objdump_path), "-h", str(linked)],
            capture_output=True,
            text=True,
        )
        assert ".text" in dump.stdout, f"no .text section: {dump.stdout}"

    def test_extract_function_bytes_matches_known_size(self) -> None:
        """Extracted .text bytes have the expected size (8 bytes = jr $ra + nop)."""
        obj = _assemble_minimal(self.as_path, self.tmp)
        linked = self.tmp / "linked.elf"
        result = subprocess.run(
            [
                str(self.ld_path),
                "-EL",
                "-Ttext=0x80010000",
                str(obj),
                "-o",
                str(linked),
            ],
            capture_output=True,
            text=True,
        )
        assert result.returncode == 0, f"link failed: {result.stderr}"

        flat = subprocess.run(
            [
                str(self.objcopy_path),
                "-O",
                "binary",
                "-j",
                ".text",
                str(linked),
                "/dev/stdout",
            ],
            capture_output=True,
        )
        assert flat.returncode == 0, f"objcopy failed: {flat.stderr.decode()}"
        # jr $ra = 0x03e00008, nop = 0x00000000 -> 8 bytes
        # With -EL (little-endian): 08 00 e0 03 00 00 00 00
        assert len(flat.stdout) >= 8, f"extracted too few bytes: {len(flat.stdout)}"
        assert flat.stdout[:8] == b"\x08\x00\xe0\x03\x00\x00\x00\x00", (
            f"unexpected bytes: {flat.stdout[:8].hex()}"
        )

    def test_function_bytes_match_via_harness(self) -> None:
        """Round-trip through the harness link-at-address extraction API."""
        from harness.io import repo_layout

        obj = _assemble_minimal(self.as_path, self.tmp)
        layout = repo_layout(_REPO_ROOT)

        from harness.match._asm_link import (
            link_object_at_address,
            extract_function_bytes,
        )

        linked = link_object_at_address(
            object_path=obj,
            address=0x80010000,
            undefined_symbols=[],
            layout=layout,
        )
        assert linked == obj.with_suffix(".linked.o")
        assert linked.is_file()
        symbols = subprocess.run(
            [str(self.nm_path), "-n", str(linked)],
            capture_output=True,
            text=True,
            check=True,
        )
        assert "80010000 T func_80010000" in symbols.stdout

        extracted = extract_function_bytes(linked, size=8, layout=layout)
        # jr $ra in little-endian: 08 00 e0 03
        assert extracted == b"\x08\x00\xe0\x03\x00\x00\x00\x00", (
            f"harness extraction mismatch: {extracted.hex()}"
        )
