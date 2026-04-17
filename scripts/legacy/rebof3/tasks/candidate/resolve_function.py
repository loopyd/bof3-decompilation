from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from ...common import ROOT, default_artifacts_dir, parse_hexish, parse_source_spec
from ...config import DEFAULT_GHIDRA_DECOMP_ROOT
from ...inventory.layout import INVENTORY_SQLITE
from ...lib.pipeline import PipelineContext, PipelineOptions, PipelineTask, option_logger
from ...match import target
from .common import (
    DEFAULT_CANDIDATE_BUILD_ROOT,
    DEFAULT_CANDIDATE_WORKSPACE_ROOT,
    candidate_stub_source_path,
    candidate_workspace_json_path,
    stable_function_name,
)


@dataclass(frozen=True, slots=True)
class ResolveFunctionTask(PipelineTask):
    """Resolve one function row and derive the default paths for later tasks."""

    task_name = "resolve_function"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        inventory_db = Path(str(context.get("inventory_db") or INVENTORY_SQLITE)).resolve()
        rows = target.load_function_rows(inventory_db)
        program_rows = target.load_program_rows(inventory_db)
        row = target.find_function_row(
            rows,
            program=str(context["program_selector"]),
            entry=str(context["entry"]),
            program_rows=program_rows,
        )
        context["inventory_db"] = inventory_db
        context["function_row"] = row
        context["entry_hex"] = str(row["entry_hex"])
        context["function_name"] = stable_function_name(str(row["entry_hex"]))
        context["program_path"] = str(row["program_path"])
        context["program_name"] = str(row["program_name"])
        context["source_text"] = str(
            context.get("source_text") or row.get("source_hint") or ""
        )

        workspace_root = Path(
            str(context.get("workspace_root") or DEFAULT_CANDIDATE_WORKSPACE_ROOT)
        ).resolve()
        context["workspace_root"] = workspace_root
        context["workspace_json"] = candidate_workspace_json_path(workspace_root, row)
        context["candidate_source_file"] = candidate_stub_source_path(
            str(row["program_path"]),
            str(row["entry_hex"]),
        )

        artifacts_dir = context.get("artifacts_dir")
        if artifacts_dir is None:
            source_hint = str(row.get("source_hint") or "").strip()
            if not source_hint:
                raise LookupError("function row is missing source_hint for ghidra bundle")
            source_spec = parse_source_spec(source_hint)
            source_path = source_spec.path
            if not source_path.is_absolute():
                source_path = (ROOT / source_path).resolve()
            context["artifacts_dir"] = default_artifacts_dir(
                Path(DEFAULT_GHIDRA_DECOMP_ROOT),
                source_path,
                parse_hexish(str(row["entry_hex"])),
                source_spec.entry_index,
            )
        else:
            context["artifacts_dir"] = Path(str(artifacts_dir)).resolve()

        context["build_root"] = Path(
            str(context.get("build_root") or DEFAULT_CANDIDATE_BUILD_ROOT)
        ).resolve()
        logger.debug(
            " ".join(
                [
                    f"program={context['program_path']}",
                    f"entry={context['entry_hex']}",
                    f"workspace={context['workspace_json']}",
                ]
            )
        )
        return context


__all__ = ["ResolveFunctionTask"]
