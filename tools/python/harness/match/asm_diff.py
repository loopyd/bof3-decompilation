from __future__ import annotations

from typing import Any

from ..io import RepoLayout, repo_layout

from ._asm_diff_payload import AsmDiffRequest
from ._asm_diff_run import _asm_diff_compare, _asm_diff_resolve, run_build_object

__all__ = [
    "run_asm_diff_one",
]

# -- orchestration ----------------------------------------------------------------------

def run_asm_diff_one(
    request: AsmDiffRequest,
    *,
    layout: RepoLayout | None = None,
) -> dict[str, Any]:
    repo = layout or repo_layout()
    resolved = _asm_diff_resolve(repo, request)
    run_build_object(
        repo,
        resolved["source_path"],
        resolved["output_dir"] / "build.log" if request.diagnostics else None,
    )
    return _asm_diff_compare(repo, request, resolved)
