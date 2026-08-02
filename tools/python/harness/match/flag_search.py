"""Search the compiler-flag catalog for one target-qualified function."""

from ..toolchain.gcc_variants import EmptyCatalog, lookup_variant
CMAKE_ENV_ASSIGNMENT_RE = re.compile(r"^[^=]+=.*$")
    matches = [
        row for row in rows if Path(row.get("file", "")).resolve() == resolved
    ]
            f"expected 1 compile command for {source}, found {len(matches)}"
def _strip_embedded_psx_gcc(command: list[str]) -> list[str]:
    """Remove PSX_GCC only from the leading ``cmake -E env`` assignments."""
    if command[:3] != ["cmake", "-E", "env"]:
        return command
    index = 3
    env: list[str] = []
    while index < len(command) and CMAKE_ENV_ASSIGNMENT_RE.match(command[index]):
        if not command[index].startswith("PSX_GCC="):
            env.append(command[index])
        index += 1
    return [*command[:3], *env, *command[index:]]


    """Replace canonical -O flags with candidate flags and set output path."""
    *,
    layout: RepoLayout,
    source: Path,
    catalog_path: Path,
    compiler_id: str | None = None,
    original = original_path.read_text(encoding="utf-8").splitlines()

    # Resolve variant environment when a compiler_id is given.
    variant_env: dict[str, str] = {}
    variant_label = "canonical"
    if compiler_id is not None:
        variant = lookup_variant(layout, compiler_id)
        if isinstance(variant, EmptyCatalog):
            raise ValueError(
                f"compiler variant {compiler_id!r} not available (empty catalog)"
            )
        variant.verify(layout)
        variant_env = {"PSX_GCC": str(variant.install_path(layout) / variant.executable_relpath)}
        variant_label = variant.label

    cmd, cmd_dir = _compile_command(layout, source)
    if compiler_id is not None:
        cmd = _strip_embedded_psx_gcc(cmd)
        work = Path(tmp)
        # candidates is a list of flag lists: [["-O0"], ["-O1"], ...]
        for flags in catalog.get("candidates", []):
            object_path = work / "candidate.o"
            candidate = _with_candidate(cmd, flags, object_path)
            try:
                    candidate,
                    cwd=str(cmd_dir),
                    env={**os.environ, **variant_env},
            except FileNotFoundError:
                match_ok, compiled = function_bytes_match(
                    object_path=object_path,
                linked_path = object_path.with_suffix('.linked.o')
                if match_ok:
                    status = "exact_match"
                else:
                    # Bytes differ; still compute instruction percentage
                    try:
                    except RuntimeError:
                        percent = 0.0
                    status = "different"


    results.sort(key=lambda row: (-row["match_percent"], str(row["flags"])))
    payload: dict[str, Any] = {
        "exact_matches": [r for r in results if r["status"] == "exact_match"],
    if compiler_id is not None:
        payload["compiler_id"] = compiler_id
        payload["variant_label"] = variant_label
        payload["address"] = address
        payload["size"] = original_size
    return payload
