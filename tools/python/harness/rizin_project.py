from .domain import lookup_target_manifest
from .snapshot import SNAPSHOT_SCHEMA, snapshot_path, write_snapshot
    manifest = lookup_target_manifest(root, target_id)
    overlay = root / "config" / "targets" / manifest.id.value / "reviewed.rz"
    replay = _baseline(
        load_target_symbols(root, manifest.id.value), roots
    ) + _reviewed_overlay(overlay)
        target=manifest.id.value,
        snapshot=snapshot_path(root, manifest.id.value),
    engine = find_engine("rizin", root=root)
                payload.get("schema") == SNAPSHOT_SCHEMA
                and payload.get("target") == target.target
