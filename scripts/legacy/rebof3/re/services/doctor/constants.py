from __future__ import annotations

from pathlib import Path

from ....config import (
    ASM_DIFFER_SCRIPT,
    BOF3_DISK_BINARY,
    EMI_EX_BINARY,
    GCC272_PSX_GCC,
    GCC272_PSX_ROOT,
    GHIDRA_MAIN_MODULE,
    GHIDRA_SRC_DIR,
    MASPSX_CC_WRAPPER,
    M2C_SCRIPT,
    MIPSMATCH_BINARY,
    OBJDIFF_BINARY,
    PSN00B_TOOLCHAIN_BIN,
    PSYQ_ORIGINAL_40_ROOT,
    ROOT,
)

EXTRACT_PROJECT_XML_CANDIDATES = (
    "Breath of Fire III (USA).xml",
    "Breath of Fire III (v1.1).xml",
)
WORKFLOW_STATUS_DIR = ROOT / "tmp" / "workflow_status"
EXTRACT_STATUS = WORKFLOW_STATUS_DIR / "extract.stamp"
UNPACK_STATUS = WORKFLOW_STATUS_DIR / "unpack.stamp"
INVENTORY_STATUS = WORKFLOW_STATUS_DIR / "inventory.stamp"
UNPACK_SENTINELS = (
    ROOT / "processed" / "emi_raw" / "BIN" / "ETC" / "FIRST" / "emi.json",
    ROOT / "processed" / "emi_raw" / "BIN" / "ETC" / "GAME" / "emi.json",
    ROOT / "processed" / "emi_raw" / "BIN" / "SCENARIO" / "SCENA16" / "emi.json",
)
INVENTORY_SENTINELS = (ROOT / "processed" / "inventory" / "inventory.sqlite",)
INVENTORY_DB = ROOT / "processed" / "inventory" / "inventory.sqlite"
INVENTORY_REQUIRED_TABLES = (
    "slot_map",
    "archives",
    "emi_entries",
    "overlay_aliases",
    "overlay_entry_tables",
    "programs",
    "functions",
    "metadata_rows",
)
OPTIONAL_GHIDRA_PROJECT = ROOT / "tmp" / "bof3_ghidra" / "main" / "bof3_main.gpr"
MASPSX_CC = MASPSX_CC_WRAPPER
DEFAULT_GHIDRA_HOME = Path("/opt/ghidra")
PSX_TOOLCHAIN_NAMES = {
    "mipsel g++": ("mipsel-none-elf-g++", "mipsel-linux-gnu-g++"),
    "mipsel as": ("mipsel-none-elf-as", "mipsel-linux-gnu-as"),
    "mipsel ld": ("mipsel-none-elf-ld", "mipsel-linux-gnu-ld"),
    "mipsel ar": ("mipsel-none-elf-ar", "mipsel-linux-gnu-ar"),
    "mipsel ranlib": ("mipsel-none-elf-ranlib", "mipsel-linux-gnu-ranlib"),
    "mipsel objcopy": ("mipsel-none-elf-objcopy", "mipsel-linux-gnu-objcopy"),
    "mipsel objdump": ("mipsel-none-elf-objdump", "mipsel-linux-gnu-objdump"),
}

__all__ = [
    "ASM_DIFFER_SCRIPT",
    "BOF3_DISK_BINARY",
    "DEFAULT_GHIDRA_HOME",
    "EMI_EX_BINARY",
    "EXTRACT_PROJECT_XML_CANDIDATES",
    "EXTRACT_STATUS",
    "GCC272_PSX_GCC",
    "GCC272_PSX_ROOT",
    "GHIDRA_MAIN_MODULE",
    "GHIDRA_SRC_DIR",
    "INVENTORY_DB",
    "INVENTORY_REQUIRED_TABLES",
    "INVENTORY_SENTINELS",
    "INVENTORY_STATUS",
    "M2C_SCRIPT",
    "MASPSX_CC",
    "MIPSMATCH_BINARY",
    "OBJDIFF_BINARY",
    "OPTIONAL_GHIDRA_PROJECT",
    "PSN00B_TOOLCHAIN_BIN",
    "PSX_TOOLCHAIN_NAMES",
    "PSYQ_ORIGINAL_40_ROOT",
    "ROOT",
    "UNPACK_SENTINELS",
    "UNPACK_STATUS",
    "WORKFLOW_STATUS_DIR",
]
