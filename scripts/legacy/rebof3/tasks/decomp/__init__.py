from __future__ import annotations

"""Small reusable tasks for the decomp bundle pipeline."""

from .generate_m2c_context import GenerateM2CContextTask
from .ghidra_bundle_export import GhidraBundleExportTask
from .normalize_asm_for_m2c import NormalizeAsmForM2CTask
from .persist_ghidra_c import PersistGhidraCArtifactTask
from .run_m2c import RunM2CTask
from .select_asm_artifact import SelectAsmArtifactTask
from .spimdisasm_asm import SpimdisasmAsmTask

__all__ = [
    "GenerateM2CContextTask",
    "GhidraBundleExportTask",
    "NormalizeAsmForM2CTask",
    "PersistGhidraCArtifactTask",
    "RunM2CTask",
    "SelectAsmArtifactTask",
    "SpimdisasmAsmTask",
]
