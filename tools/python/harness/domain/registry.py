def lookup_target_manifest(root: Path, value: str) -> TargetManifest | None:
    """Return the ``TargetManifest`` for a shipped or canonical selector.

    Unlike :func:`resolve_target`, this never constructs resolved paths and
    never requires a target binary to exist.  Raises ``ValueError`` if the
    selector itself is malformed; returns ``None`` for a valid but unknown
    target.
    return load_target_manifests(root).get(target_id.value)


    "lookup_target_manifest",
