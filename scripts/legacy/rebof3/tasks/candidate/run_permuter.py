from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from ...common import ROOT, relative_to_root
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from ...match import permuter as permuter_lib
from ...match import pipeline_ready
from .options import (
    permuter_args,
    permuter_threads,
    permuter_timeout_seconds,
    permuter_variant,
)


@dataclass(frozen=True, slots=True)
class RunPermuterTask(PipelineTask):
    """Prepare and run decomp-permuter once the workspace is diff-ready."""

    task_name = "run_permuter"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        workspace_json = Path(str(context["workspace_json"])).resolve()
        workspace_payload = dict(context["workspace_payload"])
        compile_commands = Path(str(context["compile_commands_path"])).resolve()
        state = pipeline_ready.refresh_expected_baseline(
            pipeline_ready.build_workspace_state(workspace_json, workspace_payload)
        )
        status, next_steps = pipeline_ready.diff_status(state)
        if status != "ready_for_backend_diff":
            raise RuntimeError(
                f"workspace is not ready for permuter setup: {status} ({'; '.join(next_steps)})"
            )

        prepared = permuter_lib.prepare_permuter_dir(
            state.workspace_json,
            state.workspace_payload,
            compile_commands=compile_commands,
            variant=permuter_variant(options),
        )
        command = [
            "python3",
            str(permuter_lib.DECOMP_PERMUTER_SCRIPT),
            str(ROOT / str(prepared["permuter_dir"])),
            "-j",
            str(max(permuter_threads(options), 1)),
            *permuter_args(options),
        ]
        perm_dir = ROOT / str(prepared["permuter_dir"])
        permuter_log_path = perm_dir / "permuter.log"
        logger.debug(f"permuter dir {relative_to_root(perm_dir)}")
        result, timed_out = permuter_lib.run_permuter(
            command,
            timeout_seconds=permuter_timeout_seconds(options),
            log_path=permuter_log_path,
        )
        context["permuter"] = {
            "prepared": prepared,
            "command": command,
            "returncode": int(result.returncode),
            "timed_out": bool(timed_out),
            "log_path": relative_to_root(permuter_log_path),
        }
        logger.debug(
            " ".join(
                [
                    f"log={relative_to_root(permuter_log_path)}",
                    f"returncode={result.returncode}",
                    f"timed_out={timed_out}",
                ]
            )
        )
        return context


__all__ = ["RunPermuterTask"]
