from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from ...common import relative_to_root
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from ...match import build as build_lib, compile_one
from .options import profile


@dataclass(frozen=True, slots=True)
class CompileWorkspaceTask(PipelineTask):
    """Compile the generated candidate source through the canonical stub build."""

    task_name = "compile_workspace"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        workspace_json = Path(str(context["workspace_json"])).resolve()
        workspace_payload = dict(context["workspace_payload"])
        workspace_dir = workspace_json.parent
        build_root = Path(str(context["build_root"])).resolve()
        build_profile = profile(options)
        try:
            plan = compile_one.plan_compile_one(
                workspace_payload, build_root=build_root
            )
        except (FileNotFoundError, LookupError) as exc:
            raise RuntimeError(f"compile-one unavailable: {exc}") from exc
        logger.debug(
            " ".join(
                [
                    f"profile={build_profile}",
                    f"source={relative_to_root(Path(str(plan['source_file'])))}",
                    f"object={relative_to_root(Path(str(plan['object_path'])))}",
                ]
            )
        )

        result, _ = compile_one.run_compile_one(
            workspace_payload,
            build_root=build_root,
            profile=build_profile,
        )
        build_log = workspace_dir / "build.log"
        build_json = workspace_dir / "build.json"
        compile_one.write_build_outputs(
            workspace_payload,
            profile=build_profile,
            build_mode="compile-one",
            log_path=build_log,
            status_path=build_json,
            build_root=build_root,
            result=result,
            command=list(plan["command"]),
            compile_commands_path=Path(str(plan["compile_commands_path"])),
            source_file=Path(str(plan["source_file"])),
            object_path=Path(str(plan["object_path"])),
        )
        build_lib.record_build_attempt(
            workspace_dir,
            workspace_payload,
            build_mode="compile-one",
            result=result,
            log_path=build_log,
            build_root=build_root,
            command=list(plan["command"]),
            source_file=Path(str(plan["source_file"])),
            object_path=Path(str(plan["object_path"])),
        )
        if result.returncode != 0:
            raise RuntimeError(f"compile-one failed; see {relative_to_root(build_log)}")

        context["build_status_path"] = build_json
        context["build_status"] = json.loads(build_json.read_text(encoding="utf-8"))
        logger.debug(
            " ".join(
                [
                    f"build={relative_to_root(build_json)}",
                    f"log={relative_to_root(build_log)}",
                ]
            )
        )
        return context


__all__ = ["CompileWorkspaceTask"]
