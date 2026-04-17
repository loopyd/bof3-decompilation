from __future__ import annotations

from typing import Any

from . import asm_differ_backend, objdiff_backend, semantic_diff


class BackendFailure(RuntimeError):
    def __init__(self, message: str, *, returncode: int = 1):
        super().__init__(message)
        self.returncode = returncode


def prepare_asm_backend(
    workspace_dir: Any, workspace_payload: dict[str, Any]
) -> dict[str, Any]:
    try:
        return asm_differ_backend.prepare_backend(workspace_dir, workspace_payload)
    except (FileNotFoundError, ValueError) as exc:
        raise BackendFailure(str(exc)) from exc


def run_viewer(prepared: dict[str, Any]) -> Any:
    return asm_differ_backend.run_viewer(prepared)


def run_diff_backends(
    workspace_dir: Any, workspace_payload: dict[str, Any]
) -> dict[str, Any]:
    prepared = prepare_asm_backend(workspace_dir, workspace_payload)
    result = asm_differ_backend.run_backend(prepared)
    asm_report = asm_differ_backend.write_backend_outputs(prepared, result)
    if result.returncode != 0:
        raise BackendFailure(
            f"asm-differ backend failed; see {asm_report['stderr_path']}",
            returncode=int(result.returncode),
        )

    try:
        obj_prepared = objdiff_backend.prepare_backend(
            workspace_dir,
            workspace_payload,
            asm_backend_report=asm_report,
        )
    except (FileNotFoundError, ValueError) as exc:
        raise BackendFailure(str(exc)) from exc
    obj_result = objdiff_backend.run_backend(obj_prepared)
    obj_report = objdiff_backend.write_backend_outputs(obj_prepared, obj_result)
    if obj_result.returncode != 0:
        raise BackendFailure(
            f"objdiff backend failed; see {obj_report['stderr_path']}",
            returncode=int(obj_result.returncode),
        )
    try:
        semantic_report = semantic_diff.build_backend_report(
            workspace_dir,
            workspace_payload,
            asm_backend_report=asm_report,
            obj_backend_report=obj_report,
        )
    except (FileNotFoundError, ValueError) as exc:
        raise BackendFailure(str(exc)) from exc

    return {
        "asm-differ": asm_report,
        "objdiff": obj_report,
        "semantic-diff": semantic_report,
    }
