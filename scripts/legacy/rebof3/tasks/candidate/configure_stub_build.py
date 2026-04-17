from __future__ import annotations

import shutil
from dataclasses import dataclass
from pathlib import Path
from ...common import ROOT, relative_to_root, run_command, write_text_output
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from .common import build_stub_configure_command, compile_commands_has_source
from .options import force_reconfigure


@dataclass(frozen=True, slots=True)
class ConfigureStubBuildTask(PipelineTask):
    """Ensure the stub build tree knows about the generated candidate source."""

    task_name = "configure_stub_build"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        build_root = Path(str(context["build_root"])).resolve()
        compile_commands_path = build_root / "compile_commands.json"
        source_file = Path(str(context["candidate_source_file"])).resolve()
        if compile_commands_has_source(
            compile_commands_path, source_file=source_file
        ) and not force_reconfigure(options):
            context["compile_commands_path"] = compile_commands_path
            logger.debug(
                f"reuse compile_commands {relative_to_root(compile_commands_path)}"
            )
            return context

        command = build_stub_configure_command(build_root)
        logger.debug(f"configure build_root={relative_to_root(build_root)}")
        result = run_command(command, cwd=ROOT)
        combined_output = result.stdout + (
            "" if not result.stderr else "\n" + result.stderr
        )
        if result.returncode != 0 and "does not match the source" in combined_output:
            shutil.rmtree(build_root / "CMakeFiles", ignore_errors=True)
            (build_root / "CMakeCache.txt").unlink(missing_ok=True)
            result = run_command(command, cwd=ROOT)
            combined_output = result.stdout + (
                "" if not result.stderr else "\n" + result.stderr
            )

        log_path = (
            Path(str(context["workspace_json"])).resolve().parent / "configure.log"
        )
        write_text_output(log_path, combined_output)
        if result.returncode != 0:
            raise RuntimeError(
                f"cmake configure failed; see {relative_to_root(log_path)}"
            )
        if not compile_commands_has_source(
            compile_commands_path,
            source_file=source_file,
        ):
            raise LookupError(
                f"compile_commands is missing candidate source: {relative_to_root(source_file)}"
            )
        context["compile_commands_path"] = compile_commands_path
        logger.debug(f"configure log {relative_to_root(log_path)}")
        return context


__all__ = ["ConfigureStubBuildTask"]
