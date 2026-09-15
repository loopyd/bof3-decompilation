"""Validate preserved compiler and ownership contracts for a live translation unit."""

from __future__ import annotations

import copy
import hashlib
import json
import re
import tomllib
from pathlib import Path

import yaml

from harness.build.compiler import CONFIGURATION_PATH
from harness.build.migration import (
    read_migration_settings,
    validate_configuration_migration,
)
from harness.build.profiles import ProfileContext, collect_profile_sources
from harness.common.deadlines import check_deadline
from harness.common.files import read_file
from harness.common.inputs import InputBatch, relative
from harness.domain.claims import collect_manifest_source_addresses
from harness.domain.cache import collect_manifest_paths
from harness.domain.functions import collect_lift_metadata, parse_function_records
from harness.domain.layout import parse_splat_text
from harness.domain.sources import compiled_symbol_name, expected_lift_sources
from harness.domain.symbols import parse_map
from harness.io import unique_object

PROFILE_SCHEMA = "bof3.combiner-profile/v3"
PRESERVATION_SCHEMA = "bof3.combiner-preservation/v5"
PRESERVATION_OWNERS = (
    "tools/python/harness/combiner/preservation.py",
    "tools/python/harness/build/preservation.py",
)
_LIMIT = 4 * 1024 * 1024
_LAYOUT_LOADER = yaml.SafeLoader


def hash_preservation(value: object) -> str:
    check_deadline()
    try:
        encoded = json.dumps(
            value, sort_keys=True, separators=(",", ":"), allow_nan=False
        ).encode()
    except (TypeError, RecursionError) as error:
        raise ValueError(
            "preservation requires bounded JSON-compatible values"
        ) from error
    check_deadline()
    result = hashlib.sha256(encoded).hexdigest()
    check_deadline()
    return result


def validate_fingerprint(value: object) -> None:
    if not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{64}", value):
        raise ValueError("preservation requires an external SHA-256 fingerprint")


def validate_preservation_size(document: dict) -> None:
    """Keep captured records readable within the CLI's existing document bound."""
    check_deadline()
    content = (
        json.dumps(document, indent=2, sort_keys=True, allow_nan=False) + "\n"
    ).encode("utf-8")
    if len(content) > _LIMIT:
        raise ValueError("preservation document exceeds the byte bound")
    check_deadline()


def _read_document_bytes(path: Path, *, expected_sha256: str | None = None) -> bytes:
    if expected_sha256 is not None:
        validate_fingerprint(expected_sha256)
    content = read_file(path.parent, path.name, max_bytes=_LIMIT)
    if (
        expected_sha256 is not None
        and hashlib.sha256(content).hexdigest() != expected_sha256
    ):
        raise ValueError("preservation document bytes drifted")
    return content


def read_preservation_document(
    path: Path, *, expected_sha256: str | None = None
) -> dict:
    content = _read_document_bytes(path, expected_sha256=expected_sha256)
    try:
        value = json.loads(content, object_pairs_hook=unique_object)
    except RecursionError as error:
        raise ValueError("preservation JSON nesting exceeds the bound") from error
    if not isinstance(value, dict):
        raise ValueError("preservation document must be an object")
    check_deadline()
    return value


def read_destination_source(
    path: Path | None, *, expected_sha256: str | None
) -> str | None:
    """Read an optional prospective C image only with an external byte digest."""
    if path is None:
        if expected_sha256 is not None:
            raise ValueError("a destination digest requires a destination source")
        return None
    validate_fingerprint(expected_sha256)
    content = _read_document_bytes(path, expected_sha256=expected_sha256)
    try:
        return content.decode("utf-8")
    except UnicodeError as error:
        raise ValueError("destination source must be UTF-8 text") from error


def validate_destination_text(
    text: str | None, target: str, members: list[dict]
) -> None:
    """Bind prospective attached records to the complete selected member set."""
    check_deadline()
    if text is None:
        return
    if not isinstance(text, str) or len(text.encode("utf-8")) > _LIMIT:
        raise ValueError("destination source must be bounded UTF-8 text")
    expected = {
        function["selector"]: function["symbol"]
        for member in members
        for function in member["functions"]
    }
    records = parse_function_records(text)
    actual = {f"{target}@0x{record.address:08X}": record for record in records}
    if len(expected) != sum(len(member["functions"]) for member in members):
        raise ValueError("PRE function selectors are duplicated")
    if set(actual) != set(expected):
        raise ValueError("destination metadata differs from complete PRE membership")
    if any(
        record.kind == "function" and record.spelling != expected[selector]
        for selector, record in actual.items()
    ):
        raise ValueError("destination function spelling differs from PRE identity")
    check_deadline()


def validate_states(states: object) -> dict:
    if not isinstance(states, dict):
        raise ValueError("POST must map exact repository paths to file states")
    for path, state in states.items():
        if not isinstance(path, str) or relative(path) != path:
            raise ValueError("POST requires canonical repository paths")
        if state is None:
            continue
        if not isinstance(state, dict) or set(state) != {"sha256", "mode"}:
            raise ValueError("POST states require exactly sha256 and mode")
        validate_fingerprint(state["sha256"])
        if type(state["mode"]) is not int or not 0 <= state["mode"] <= 0o7777:
            raise ValueError("POST state has an invalid file mode")
    return states


def _validate_layout_events(text: str) -> None:
    events = yaml.parse(text, Loader=_LAYOUT_LOADER)
    depth = 0
    try:
        for event in events:
            check_deadline()
            if (
                isinstance(event, yaml.events.AliasEvent)
                or getattr(event, "anchor", None) is not None
                or getattr(event, "tag", None) is not None
            ):
                raise ValueError(
                    "preservation does not accept YAML aliases, anchors or tags"
                )
            if (
                isinstance(
                    event, (yaml.events.CollectionStartEvent, yaml.events.ScalarEvent)
                )
                and depth > 64
            ):
                raise ValueError("preservation layout nesting exceeds the bound")
            if isinstance(event, yaml.events.CollectionStartEvent):
                depth += 1
            elif isinstance(event, yaml.events.CollectionEndEvent):
                depth -= 1
    finally:
        events.close()


def _validate_layout_node(node: yaml.Node, depth: int = 0) -> None:
    check_deadline()
    if depth > 64:
        raise ValueError("preservation layout nesting exceeds the bound")
    if isinstance(node, yaml.ScalarNode):
        return
    if isinstance(node, yaml.SequenceNode):
        for entry in node.value:
            _validate_layout_node(entry, depth + 1)
        return
    if isinstance(node, yaml.MappingNode):
        keys = set()
        for key, value in node.value:
            if (
                not isinstance(key, yaml.ScalarNode)
                or key.tag != "tag:yaml.org,2002:str"
                or key.value in keys
            ):
                raise ValueError("preservation layout requires unique string keys")
            keys.add(key.value)
            _validate_layout_node(value, depth + 1)
        return
    raise ValueError("preservation requires a YAML document")


def decode_layout(text: str) -> object:
    check_deadline()
    if len(text.encode()) > _LIMIT:
        raise ValueError("preservation layout exceeds the byte bound")
    try:
        _validate_layout_events(text)
        check_deadline()
        loader = _LAYOUT_LOADER(text)
        try:
            document = loader.get_single_node()
            check_deadline()
            _validate_layout_node(document)
            result = loader.construct_document(document)
            check_deadline()
        finally:
            loader.dispose()
    except (yaml.YAMLError, RecursionError) as error:
        raise ValueError("preservation requires bounded valid YAML") from error
    check_deadline()
    hash_preservation(result)
    return result


def validate_transition(profile: dict, post: dict) -> tuple[str, str, list[str]]:
    if (
        not isinstance(profile, dict)
        or profile.get("schema") != PROFILE_SCHEMA
        or "destination_text" not in profile
        or "configuration" not in profile
    ):
        raise ValueError("preservation requires a configured PRE profile")
    members = profile.get("members")
    if not isinstance(members, list) or any(
        not isinstance(member, dict)
        or not isinstance(member.get("source"), str)
        or not isinstance(member.get("profile"), dict)
        or not isinstance(member.get("functions"), list)
        or not member["functions"]
        or any(
            not isinstance(function, dict)
            or set(function) != {"selector", "symbol"}
            or any(not isinstance(value, str) for value in function.values())
            for function in member["functions"]
        )
        for member in members
    ):
        raise ValueError("preservation requires complete PRE member records")
    sources = [member["source"] for member in members]
    destination = profile.get("destination")
    if not 2 <= len(sources) <= 32 or len(set(sources)) != len(sources):
        raise ValueError("preservation requires two to 32 distinct member sources")
    for source in [*sources, destination]:
        if (
            relative(source) != source
            or not source.startswith("src/bof3/")
            or not source.endswith(".c")
        ):
            raise ValueError("preservation requires canonical C source paths")
    if not isinstance(profile.get("target"), str):
        raise ValueError("preservation requires a target identity")
    manifest = f"config/targets/{profile['target']}/target.toml"
    relative(manifest)
    inputs = validate_states(profile.get("inputs"))
    for path in [manifest, *sources]:
        if inputs.get(path) is None:
            raise ValueError("preservation is missing a PRE input")
    if not isinstance(profile.get("common_profile"), dict) or not isinstance(
        profile.get("destination_profile"), dict
    ):
        raise ValueError("preservation requires complete configured profiles")
    layouts = [path for path in inputs if path.endswith("/splat.yaml")]
    if len(layouts) != 1:
        raise ValueError("preservation requires one pinned target Splat layout")
    layout = layouts[0]
    expected = {destination, manifest, layout, *sources}
    if profile["configuration"] is not None:
        expected.add(CONFIGURATION_PATH)
    if set(post) != expected or any(
        post[path] is None for path in [destination, manifest, layout]
    ):
        raise ValueError(
            "POST must pin exactly the destination, member removals, manifest, Splat and declared configuration migration"
        )
    if any(post[source] is not None for source in sources if source != destination):
        raise ValueError("every superseded member must be absent in POST")
    text = profile["destination_text"]
    validate_destination_text(text, profile["target"], members)
    if text is not None and (
        hashlib.sha256(text.encode("utf-8")).hexdigest() != post[destination]["sha256"]
    ):
        raise ValueError("POST destination bytes differ from the inspected draft")
    validate_configuration_migration(
        profile["configuration"], sources, destination, inputs, post
    )
    return manifest, layout, sources


def validate_ownership_images(
    record: dict, manifest_text: str, layout_text: str
) -> None:
    """Require proposed ownership text to make only the captured source relocation."""
    check_deadline()
    profile = record["profile"]
    destination = profile["destination"]
    members = {member["source"] for member in profile["members"]}
    expected_manifest = copy.deepcopy(record["manifest_pre"])
    retained = []
    for source in expected_manifest["sources"]:
        if source in members:
            if destination not in retained:
                retained.append(destination)
        else:
            retained.append(source)
    expected_manifest["sources"] = retained
    if hash_preservation(tomllib.loads(manifest_text)) != hash_preservation(
        expected_manifest
    ):
        raise ValueError("POST manifest changes more than selected source ownership")
    if hash_preservation(decode_layout(layout_text)) != hash_preservation(
        record["layout_post"]
    ):
        raise ValueError("POST Splat changes more than selected source ownership")
    check_deadline()


def verify_preservation(
    root: Path,
    record: dict,
    *,
    expected_fingerprint: str,
    compiler: Path | None = None,
) -> dict:
    """Verify exact POST ownership and configured identity against an external PRE pin."""
    validate_fingerprint(expected_fingerprint)
    if not isinstance(record, dict):
        raise ValueError("preservation record must be an object")
    record = copy.deepcopy(record)
    fingerprint = record.pop("fingerprint", None)
    if (
        fingerprint != expected_fingerprint
        or hash_preservation(record) != expected_fingerprint
    ):
        raise ValueError("preservation fingerprint drifted")
    root = root.resolve()
    if record.get("schema") != PRESERVATION_SCHEMA or record.get("root") != str(root):
        raise ValueError("preservation schema or repository root differs")
    if set(record) != {
        "schema",
        "root",
        "profile",
        "inputs",
        "post",
        "manifest",
        "layout",
        "manifest_pre",
        "source_pre",
        "layout_post",
        "source_inventory",
        "manifest_inventory",
        "native_verified",
        "producer_freshness_verified",
        "write_authorized",
    }:
        raise ValueError("preservation record fields differ from the schema")
    profile = record["profile"]
    post = validate_states(record["post"])
    manifest_path, layout_path, sources = validate_transition(profile, post)
    owner_states = {path: record["inputs"].get(path) for path in PRESERVATION_OWNERS}
    if validate_states(record["inputs"]) != {
        **profile["inputs"],
        **owner_states,
    } or any(state is None for state in owner_states.values()):
        raise ValueError("preservation input closure differs")
    for name in ("source_inventory", "manifest_inventory"):
        inventory = record[name]
        if (
            not isinstance(inventory, list)
            or any(
                not isinstance(path, str) or relative(path) != path
                for path in inventory
            )
            or inventory != sorted(set(inventory))
        ):
            raise ValueError("preservation requires canonical unique inventories")
    if not isinstance(record["manifest_pre"], dict) or not isinstance(
        record["manifest_pre"].get("sources"), list
    ):
        raise ValueError("preservation requires the PRE manifest")
    if record["manifest"] != manifest_path or record["layout"] != layout_path:
        raise ValueError("preservation target paths differ")
    destination = profile["destination"]
    expected_inventory = sorted(
        (set(record["source_inventory"]) - set(sources)) | {destination}
    )
    if (
        collect_profile_sources(root, destination) != expected_inventory
        or collect_manifest_paths(root) != record["manifest_inventory"]
    ):
        raise ValueError("POST source or manifest inventory differs")
    context = ProfileContext(root, compiler=compiler)
    expected_inputs = {**record["inputs"], **post}
    if not context.inputs.keys() <= expected_inputs.keys():
        raise ValueError("POST input closure differs")
    with InputBatch(root) as batch:
        for path, state in expected_inputs.items():
            context.read(path, required=state is not None, batch=batch)
            if context.inputs[path] != state:
                raise ValueError(f"POST input differs: {path}")
    common = profile["common_profile"]
    if compiler is not None and compiler != root / common["compiler"]["executable"]:
        raise ValueError("dispatched compiler differs from the preserved selection")
    source_pre = record["source_pre"]
    if not isinstance(source_pre, dict) or set(source_pre) != set(sources):
        raise ValueError("preservation requires every original source image")
    migration = profile["configuration"]
    configuration_text = (migration["before"] or "") if migration is not None else None
    for member in profile["members"]:
        source = member["source"]
        text = source_pre[source]
        if (
            not isinstance(text, str)
            or hashlib.sha256(text.encode("utf-8")).hexdigest()
            != profile["inputs"][source]["sha256"]
        ):
            raise ValueError("preserved source image differs from its PRE pin")
        if (
            migration is not None
            and source == sources[0]
            and (
                context.resolve_settings(
                    source, text=text, configuration_text=configuration_text
                )
                != read_migration_settings(migration["settings"])
            )
        ):
            raise ValueError(
                "configuration migration settings differ from retained PRE source"
            )
        if (
            context.resolve(source, text=text, configuration_text=configuration_text)
            != common
            or member["profile"] != common
        ):
            raise ValueError("PRE member compiler profile was not preserved")
    if (
        context.resolve(destination) != common
        or profile["destination_profile"] != common
    ):
        raise ValueError("POST destination compiler profile differs")
    layout_text = context.read(layout_path).decode("utf-8")
    validate_ownership_images(
        record, context.read(manifest_path).decode("utf-8"), layout_text
    )
    manifests = context.read_manifests()
    manifest = manifests[profile["target"]]
    if manifest.splat != layout_path:
        raise ValueError("POST manifest selects another layout")
    owners = [
        entry.id.value
        for entry in manifests.values()
        if destination in entry.sources + entry.support_sources
    ]
    if owners != [profile["target"]] or any(
        source in entry.sources + entry.support_sources
        for entry in manifests.values()
        for source in sources
        if source != destination
    ):
        raise ValueError(
            "POST must have exactly one destination owner and no old member owners"
        )
    layout = parse_splat_text(
        layout_text, manifest.load_address, origin=root / layout_path
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
    text = context.read(destination).decode("utf-8")
    records = collect_lift_metadata(text)
    selectors = {
        function["selector"]: function["symbol"]
        for member in profile["members"]
        for function in member["functions"]
    }
    if len(selectors) != sum(len(member["functions"]) for member in profile["members"]):
        raise ValueError("PRE selectors are duplicated")
    map_name = f"config/targets/{profile['target']}/symbols.txt"
    symbols = parse_map(
        context.read(map_name).decode("utf-8"), source=str(root / map_name)
    )
    actual = {
        f"{profile['target']}@0x{address:08X}": compiled_symbol_name(
            root,
            root / destination,
            address,
            manifest=manifest,
            layout=layout,
            symbols=symbols,
        )
        for address in records
    }
    if actual != selectors or {
        address for source, address in ownership if source == root / destination
    } != set(records):
        raise ValueError("POST function selectors, symbols or claims differ from PRE")
    if context.inputs != expected_inputs:
        raise ValueError("POST input closure differs")
    context.verify()
    if (
        collect_profile_sources(root, destination) != expected_inventory
        or collect_manifest_paths(root) != record["manifest_inventory"]
    ):
        raise ValueError("POST inventory changed during verification")
    result = {
        "schema": "bof3.combiner-preservation-result/v1",
        "preservation_fingerprint": fingerprint,
        "target": profile["target"],
        "destination": destination,
        "selectors": sorted(selectors),
        "profile": common,
        "inputs": context.inputs,
        "configured_preserved": True,
        "native_verified": False,
        "producer_freshness_verified": False,
        "write_authorized": False,
    }
    check_deadline()
    return {**result, "fingerprint": hash_preservation(result)}
