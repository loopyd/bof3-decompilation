import struct
from .io import repo_layout

        "-E",
        "little",
def find_engine(name: str = "rizin", *, root: Path | None = None) -> EngineIdentity:
    executable = repo_layout(root).toolchains_dir / "rizin" / "bin" / "rizin"
    if not executable.is_file():
        raise FileNotFoundError(f"missing project Rizin: {executable}; run `just setup`")
        "-E",
        "little",
    recorded_callsites: set[tuple[str, int]] = set()
        recorded_callsites.add((caller, callsite))
    # Rizin's `aa` omits direct JAL xrefs whose destination is outside this
    # image. Preserve those calls as unresolved graph edges.
    for start, end, caller in ranges:
        for callsite in range(start, end - 3, 4):
            if (caller, callsite) in recorded_callsites:
                continue
            offset = callsite - load_address
            word = struct.unpack_from("<I", binary, offset)[0]
            if word >> 26 != 3:
                continue
            target = ((callsite + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
                        kind="static_jal",
                    )
                )

        unresolved_calls=tuple(
            sorted(
                set(unresolved),
                key=lambda call: (call.caller, call.callsite, call.target_address, call.kind),
            )
        ),
    from .domain import lookup_target_manifest, normalize_target_id
    manifest = lookup_target_manifest(root, target_id)
        raise ValueError(f"unknown target: {normalize_target_id(target_id).value}")
    analyze_project(root, manifest.id.value, timeout=timeout)
    output = snapshot_path(root, manifest.id.value)
