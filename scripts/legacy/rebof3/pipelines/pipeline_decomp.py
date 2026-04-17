from __future__ import annotations

"""Concrete decomp bundle pipeline builder."""

from ..lib.pipeline import PipelineTask, pipeline
from ..tasks.decomp import (
    GenerateM2CContextTask,
    GhidraBundleExportTask,
    NormalizeAsmForM2CTask,
    PersistGhidraCArtifactTask,
    RunM2CTask,
    SelectAsmArtifactTask,
    SpimdisasmAsmTask,
)


def pipeline_decomp(*, include_m2c: bool = True) -> PipelineTask:
    """Build the decomp bundle pipeline, with an optional `m2c` lane."""

    task_list: list[PipelineTask] = [
        GhidraBundleExportTask(),
        PersistGhidraCArtifactTask(),
        SpimdisasmAsmTask(),
        SelectAsmArtifactTask(),
    ]
    if include_m2c:
        task_list.extend(
            [
                GenerateM2CContextTask(),
                NormalizeAsmForM2CTask(),
                RunM2CTask(),
            ]
        )
    return pipeline(*task_list, task_name="decomp_bundle")


__all__ = ["pipeline_decomp"]
