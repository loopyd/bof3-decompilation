from __future__ import annotations

from dataclasses import dataclass
from ...common import relative_to_root, write_text_output
from ...lib.pipeline import PipelineContext, PipelineOptions, PipelineTask, option_logger
from ...re.services.ghidra.bundle_export import run_bundle_export


@dataclass(frozen=True, slots=True)
class GhidraBundleExportTask(PipelineTask):
    """Export the raw Ghidra bundle and persist the primary asm artifact."""

    task_name = "ghidra_export"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        logger.debug(
            " ".join(
                [
                    f"source={context['source_text']}",
                    f"program={context['program_name']}",
                    f"address=0x{int(context['requested_address']):08x}",
                ]
            )
        )
        returncode, export_payload = run_bundle_export(
            source_text=context["source_text"],
            project_dir=context["project_dir"],
            project_name=context["project_name"],
            program_name=context["program_name"],
            requested_address=context["requested_address"],
            exported_json_path=context["exported_json_path"],
            asm_path=context["ghidra_asm_path"],
            loader_mode=context["loader_mode"],
            base_addr=context["base_addr"],
            noanalysis=context["noanalysis"],
        )
        if returncode != 0 or export_payload is None:
            context["returncode"] = returncode
            context["export_payload"] = None
            logger.debug(f"export failed returncode={returncode}")
            return context

        context["returncode"] = 0
        context["export_payload"] = export_payload
        context["function_payload"] = export_payload["function_payload"]
        context["ghidra_asm_text"] = export_payload["asm_text"]
        write_text_output(context["ghidra_asm_path"], export_payload["asm_text"])
        logger.debug(f"wrote asm {relative_to_root(context['ghidra_asm_path'])}")
        return context


__all__ = ["GhidraBundleExportTask"]
