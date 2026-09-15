"""Pin and compare configured profiles before any source consolidation."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

from harness.build.compiler import (
    CONFIGURATION_PATH,
    DEFAULT_COMPILER_ID,
    sanitize_identifier,
)
from harness.build.migration import plan_configuration
from harness.build.profiles import ProfileContext, collect_profile_sources
from harness.build.preservation import (
    PROFILE_SCHEMA,
    validate_destination_text,
    validate_preservation_size,
)
from harness.common.deadlines import check_deadline
from harness.common.inputs import relative
from harness.domain.claims import collect_manifest_source_addresses
from harness.domain.functions import collect_lift_metadata
from harness.domain.ids import normalize_target_id
from harness.domain.layout import parse_splat_text
from harness.domain.sources import (
    compiled_symbol_name,
    expected_lift_sources,
    source_expected_key,
)
from harness.domain.symbols import parse_map


def _resolve_source_path(value: str) -> str:
    path = Path(relative(value))
    if path.parts[:2] != ("src", "bof3") or path.suffix != ".c":
        raise ValueError("profile source must be a canonical src/bof3/ C path")
    return value


def inspect_profiles(
    root: Path,
    target: str,
    destination: str,
    sources: list[str],
    *,
    expected_fingerprint: str | None = None,
    destination_text: str | None = None,
    migrate_configuration: bool = False,
) -> dict:
    """Require one configured profile for explicit whole-file members and destination."""
    root = root.resolve()
    if type(migrate_configuration) is not bool:
        raise ValueError("configuration migration selection must be boolean")
    target = normalize_target_id(target).value
    destination = _resolve_source_path(destination)
    sources = [_resolve_source_path(source) for source in sources]
    if not 2 <= len(sources) <= 32 or len(set(sources)) != len(sources):
        raise ValueError("profile requires two to 32 distinct existing source files")
    if expected_fingerprint is not None and not re.fullmatch(
        r"[0-9a-f]{64}", expected_fingerprint
    ):
        raise ValueError("expected profile fingerprint must be a SHA-256 digest")
    context = ProfileContext(root)
    manifests = context.read_manifests()
    if target not in manifests:
        raise ValueError("unknown profile target")
    manifest = manifests[target]
    if not manifest.has_explicit_sources or manifest.companions:
        raise ValueError(
            "profile requires explicit target ownership without companion overlays"
        )
    for source in sources:
        owners = [
            entry.id.value
            for entry in manifests.values()
            if source in entry.sources + entry.support_sources
        ]
        if owners != [target] or source not in manifest.sources:
            raise ValueError("profile members require exactly one explicit lift owner")
    if (root / destination).exists() and destination not in sources:
        raise ValueError("an existing destination must be a selected member")
    if (
        any(
            destination in entry.sources + entry.support_sources
            for entry in manifests.values()
        )
        and destination not in sources
    ):
        raise ValueError("unselected destination already has a source owner")
    context.read(destination, required=False)
    inventory = collect_profile_sources(root, destination)
    keys: dict[str, list[str]] = {}
    for source in inventory:
        key = sanitize_identifier(Path(source).relative_to("src").as_posix())
        keys.setdefault(key, []).append(source)
    for source in [*sources, destination]:
        key = sanitize_identifier(Path(source).relative_to("src").as_posix())
        if len(keys[key]) != 1:
            raise ValueError("ambiguous sanitized source compiler key")
    for name in (
        manifest.splat,
        f"config/targets/{target}/symbols.txt",
        "config/targets/shared/symbols.txt",
        f"config/sdk/psyq-{manifest.psyq_space}.txt",
    ):
        context.read(name, required=name != "config/targets/shared/symbols.txt")
    for name in manifest.sources + manifest.support_sources:
        context.read(name)
    layout_text = context.read(manifest.splat).decode("utf-8")
    layout = parse_splat_text(
        layout_text, manifest.load_address, origin=root / manifest.splat
    )
    expected = expected_lift_sources(layout, root / manifest.source_dir)
    ownership = collect_manifest_source_addresses(
        root,
        manifest,
        expected_lifts=expected,
        read_source=lambda path: context.read(path.relative_to(root).as_posix()).decode(
            "utf-8"
        ),
    )
    map_name = f"config/targets/{target}/symbols.txt"
    symbols = parse_map(
        context.read(map_name).decode("utf-8"), source=str(root / map_name)
    )
    members = []
    for source in sorted(sources):
        text = context.read(source).decode("utf-8")
        records = collect_lift_metadata(text)
        key = source_expected_key(root / manifest.source_dir, root / source)
        addresses = expected.get(key)
        addresses = set(addresses) if isinstance(addresses, tuple) else {addresses}
        if not records or set(records) != addresses:
            raise ValueError("profile requires complete reviewed C boundary membership")
        for address in records:
            boundaries = [
                boundary
                for boundary in layout.boundaries
                if boundary.kind == "c" and boundary.virtual_start == address
            ]
            if len(boundaries) != 1:
                raise ValueError("profile requires one reviewed C boundary per member")
            boundary = boundaries[0]
            if (
                not 0 <= address < 2**32
                or address % 4
                or boundary.file_end is None
                or boundary.virtual_end is None
                or not 0 <= boundary.file_start < boundary.file_end <= 2**32
                or not address < boundary.virtual_end <= 2**32
                or boundary.file_start % 4
                or boundary.file_end % 4
                or boundary.virtual_end % 4
                or boundary.file_size != boundary.virtual_size
            ):
                raise ValueError("profile requires aligned 32-bit MIPS C boundaries")
        if {address for path, address in ownership if path == root / source} != set(
            records
        ):
            raise ValueError("profile source ownership differs from attached metadata")
        members.append(
            {
                "source": source,
                "sha256": context.inputs[source]["sha256"],
                "functions": [
                    {
                        "selector": f"{target}@0x{address:08X}",
                        "symbol": compiled_symbol_name(
                            root,
                            root / source,
                            address,
                            manifest=manifest,
                            layout=layout,
                            symbols=symbols,
                        ),
                    }
                    for address in sorted(records)
                ],
                "profile": context.resolve(source),
            }
        )
        profile = members[-1]["profile"]
        command = list(profile["arguments"])
        if profile["compiler"]["id"] != DEFAULT_COMPILER_ID:
            command = [
                "cmake",
                "-E",
                "env",
                f"PSX_GCC={root / profile['compiler']['executable']}",
                *command,
            ]
        members[-1]["configured_command"] = [
            *command,
            "-c",
            str(root / source),
            "-o",
            str(root / "build" / Path(source).with_suffix(".o")),
        ]
    common = members[0]["profile"]
    if any(member["profile"] != common for member in members[1:]):
        raise ValueError("member compiler profiles are incompatible")
    validate_destination_text(destination_text, target, members)
    configuration = None
    if migrate_configuration:
        before = context.read(CONFIGURATION_PATH, required=False).decode("utf-8")
        configuration = plan_configuration(
            before if context.inputs[CONFIGURATION_PATH] is not None else None,
            sources,
            destination,
            context.resolve_settings(sources[0]),
        )
    destination_profile = context.resolve(
        destination,
        text=destination_text,
        configuration_text=configuration["after"]
        if configuration is not None
        else None,
    )
    if destination_profile != common:
        raise ValueError("destination would change the captured compiler profile")
    for name in (
        "tools/python/harness/combiner/profiles.py",
        "tools/python/harness/combiner/cli.py",
    ):
        context.read(name)
    context.verify()
    if inventory != collect_profile_sources(root, destination):
        raise ValueError("source inventory changed during profile capture")
    context.verify_manifest_inventory()
    result = {
        "schema": PROFILE_SCHEMA,
        "target": target,
        "destination": destination,
        "members": members,
        "common_profile": common,
        "destination_profile": destination_profile,
        "destination_text": destination_text,
        "configuration": configuration,
        "inputs": context.inputs,
        "source_inventory": {
            "count": len(inventory),
            "sha256": hashlib.sha256(
                json.dumps(inventory, separators=(",", ":")).encode()
            ).hexdigest(),
        },
        "configured_compatible": True,
        "native_verified": False,
        "compiler_identity_verified": False,
        "producer_freshness_verified": False,
        "coverage_verified": False,
        "write_authorized": False,
    }
    digest = hashlib.sha256(
        json.dumps(result, sort_keys=True, separators=(",", ":")).encode()
    ).hexdigest()
    if expected_fingerprint is not None and digest != expected_fingerprint:
        raise ValueError("compiler profile fingerprint drifted")
    result = {**result, "fingerprint": digest}
    validate_preservation_size(result)
    check_deadline()
    return result
