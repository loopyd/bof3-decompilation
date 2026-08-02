from typing import Any, Mapping

from ..domain.manifests import SectionPlacement
    "_asm_diff_compare",
    "_asm_diff_resolve",
    canonical_bindings: Mapping[str, int] | None = None
    section_placements: tuple[SectionPlacement, ...] | None = None
    diagnostics: bool = True
    layout: RepoLayout, source_path: Path, build_log_path: Path | None
    if build_log_path is not None:
        suffix = "" if build_log_path is None else f"; see {build_log_path}"
        raise RuntimeError(f"object build failed for {target}{suffix}")
def _asm_diff_resolve(
    repo: RepoLayout, request: AsmDiffRequest
    """Resolve and prepare every input needed by the comparison step.

    Returns a dict with keys:
      source_path, address, function_name, binary_path, load_address,
      original_size, object_path, output_dir

    This is extracted so the status-audit batch path can know what to build
    without duplicating resolution logic.
    """
    if request.diagnostics:
    return {
        "source_path": source_path,
        "address": address,
        "function_name": function_name,
        "binary_path": binary_path,
        "load_address": load_address,
        "object_path": object_path,
        "output_dir": output_dir,
    }


def _asm_diff_compare(
    repo: RepoLayout,
    resolved: dict[str, Any],
) -> dict[str, Any]:
    """Run the link, byte-match, placement, size, and diagnostic steps.

    *Assumes* the object already exists (built by the caller).  Object
    freshness is verified via ``st_mtime``.
    """
    source_path = resolved["source_path"]
    address = resolved["address"]
    function_name = resolved["function_name"]
    binary_path = resolved["binary_path"]
    load_address = resolved["load_address"]
    original_size = resolved["original_size"]
    object_path = resolved["object_path"]
    output_dir = resolved["output_dir"]
    if request.diagnostics and not os.access(objdump_path, os.X_OK):
    if request.diagnostics:
    if request.section_placements is None:
    else:
        placements = request.section_placements
    if request.diagnostics and not current_compiler_asm.is_file():
        canonical_bindings=request.canonical_bindings,
    if not request.diagnostics:
        return {
            "schema": "harness.byte-match-one/v1",
            "status": "exact_match" if byte_match else "different",
            "exact_match": byte_match,
            "outputs": {},
        }
    resolved = _asm_diff_resolve(repo, request)
    run_build_object(
        repo,
        resolved["source_path"],
        resolved["output_dir"] / "build.log" if request.diagnostics else None,
    )
    return _asm_diff_compare(repo, request, resolved)
