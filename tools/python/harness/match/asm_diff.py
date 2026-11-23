"""Compile and compare one selected function through its owning native route."""

from __future__ import annotations

from typing import Any

from ..domain.manifests import TargetManifest
from ..domain.symbols import load_target_bindings
from ..io import RepoLayout, repo_layout
from ..domain.tags import count_function_metadata

from ._asm_diff_payload import AsmDiffRequest
from ._asm_diff_run import _asm_diff_compare, _asm_diff_resolve, run_build_object
from .grouped import compare_selected_member

__all__ = [
    "match_target_source",
    "run_asm_diff_one",
]

# -- orchestration ----------------------------------------------------------------------


def match_target_source(
    root,
    manifest: TargetManifest,
    address: int,
    source_path,
    *,
    diagnostics: bool = True,
    image_evidence: bool = False,
):
    """Compile one authored source and compare it against its original range.

    The request is built from the owning manifest, so a caller that rewrote a
    source (a rename, say) proves the same rendered range the lift loop proves.
    """
    return run_asm_diff_one(
        AsmDiffRequest(
            source_path=source_path,
            address=address,
            binary_path=root / manifest.binary,
            load_address=manifest.load_address,
            output_root=root / "out" / "asm-diff",
            canonical_bindings=load_target_bindings(root, manifest),
            section_placements=manifest.section_placements.get(address, ()),
            diagnostics=diagnostics,
            image_evidence=image_evidence,
            manifests={manifest.id.value: manifest} if manifest is not None else None,
        )
    )


def run_asm_diff_one(
    request: AsmDiffRequest,
    *,
    layout: RepoLayout | None = None,
) -> dict[str, Any]:
    repo = layout or repo_layout()
    if (
        count_function_metadata(
            request.source_path.expanduser().read_text(encoding="utf-8")
        )
        > 1
    ):
        return compare_selected_member(request, layout=repo)
    resolved = _asm_diff_resolve(repo, request, manifests=request.manifests)
    run_build_object(
        repo,
        resolved["source_path"],
        resolved["output_dir"] / "build.log" if request.diagnostics else None,
    )
    return _asm_diff_compare(repo, request, resolved, manifests=request.manifests)
