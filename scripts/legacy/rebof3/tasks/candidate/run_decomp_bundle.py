from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from ...common import ROOT
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from ...re.services.ghidra.decomp_runtime import run_decomp_bundle
from .options import asm_backend, emit_spimdisasm, enable_m2c, force_decomp


@dataclass(frozen=True, slots=True)
class RunDecompBundleTask(PipelineTask):
    """Produce the decomp bundle used as the seed for candidate generation."""

    task_name = "run_decomp_bundle"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        if context.get("bundle_payload") is not None and not force_decomp(options):
            logger.debug("reuse existing decomp bundle from context")
            return context

        logger.debug(
            " ".join(
                [
                    f"source={context['source_text']}",
                    f"entry={context['entry_hex']}",
                    f"asm_backend={asm_backend(options)}",
                ]
            )
        )
        returncode, bundle_payload = run_decomp_bundle(
            source_text=str(context["source_text"]),
            address_text=str(context["entry_hex"]),
            artifacts_dir=Path(str(context["artifacts_dir"])),
            asm_backend=asm_backend(options),
            emit_spimdisasm=emit_spimdisasm(options),
            no_m2c=not enable_m2c(options),
            logger=logger,
        )
        if returncode != 0 or bundle_payload is None:
            raise RuntimeError(
                f"ghidra-decomp failed for {context['source_text']} {context['entry_hex']}"
            )
        context["bundle_payload"] = bundle_payload
        context["bundle_json"] = ROOT / str(bundle_payload["files"]["json"])
        logger.debug(f"bundle={context['bundle_json']}")
        return context


__all__ = ["RunDecompBundleTask"]
