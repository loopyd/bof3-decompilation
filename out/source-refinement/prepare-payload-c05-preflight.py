"""Read the five C05 claim closures without compiling or refreshing discovery."""

from contextlib import ExitStack, closing
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path

from harness.build.arguments import inspect_arguments
from harness.build.compilation import configured_command
from harness.build.compiler import (
    COMPILER_INPUTS,
    DEFAULT_COMPILER_ID,
    load_compiler_configuration,
    resolve_compiler_settings,
    select_configured_compiler,
)
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.domain.functions import parse_function_records
from harness.domain.manifests import _parse_manifest
from harness.emi.catalog_verify import resolve_shipped_entry
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "out/source-refinement/payload-c05-preflight.json"
TARGETS = [
    "emi/bmagic/magic113/03",
    "emi/bmagic/magic124/03",
    "emi/bmagic/magic131/03",
    "emi/bmagic/magic137/03",
    "emi/bmagic/magic138/03",
]
LIMIT = 64 * 1024 * 1024


def main():
    if OUTPUT.exists():
        raise ValueError("preserve the existing preflight")
    pins, states, rows, blockers = {}, {}, [], []
    with ExitStack() as stack:
        inputs = stack.enter_context(InputBatch(ROOT))

        def bind(name, expected=None):
            state, content = inputs.read(
                ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True
            )
            if name in states and states[name] != state:
                raise ValueError(f"input changed: {name}")
            if expected is not None and state != expected:
                raise ValueError(f"owner input state differs: {name}")
            states[name] = state
            pins[name] = (
                None
                if state is None
                else {"sha256": state["sha256"], "size": len(content)}
            )
            return content

        classification = json.loads(
            bind("out/source-refinement/payload-target-classification.json")
        )
        bind("out/source-refinement/payload-c05-scout.json")
        for name in (
            *COMPILER_INPUTS,
            "CMakeLists.txt",
            "config/compiler/graph.cmake",
            "config/compiler/object-flags.cmake",
            "config/compiler/variants.json",
            "config/sdk/psyq-slus.txt",
        ):
            bind(name)
        flags, compilers = load_compiler_configuration(ROOT)
        for target in TARGETS:
            name = f"config/targets/{target}/target.toml"
            manifest = _parse_manifest(ROOT / name, bind(name))
            claims = list(manifest.sources + manifest.support_sources)
            row = {
                "target": target,
                "manifest": name,
                "manifest_sha256": pins[name]["sha256"],
                "disc_id": manifest.disc_id,
                "binary": manifest.binary,
                "load_address": manifest.load_address,
                "claim_count": len(claims),
                "historical_classification": next(
                    key
                    for key in ("candidate", "retained_only", "metadata_blocked")
                    if target in classification[key]["targets"]
                ),
                "sources": [],
                "blockers": [],
                "native_admission_verified": False,
                "replicas": [str(value) for value in manifest.replicas],
            }
            bind(manifest.splat)
            binary = bind(manifest.binary)
            entry = resolve_shipped_entry(ROOT, manifest.disc_id)
            for key in ("manifest_path", "archive_path", "payload_path"):
                bind(Path(entry[key]).relative_to(ROOT).as_posix())
            original = bind(Path(entry["payload_path"]).relative_to(ROOT).as_posix())
            if binary != original or entry["load_address"] != manifest.load_address:
                raise ValueError(
                    f"normalized/shipped payload or load differs: {target}"
                )
            row["shipped_entry"] = entry
            row["payload_size"] = len(binary)
            forced = set()
            for source in claims:
                content = bind(source)
                source_row = {
                    "source": source,
                    "source_sha256": pins[source]["sha256"],
                    "claim": "source"
                    if source in manifest.sources
                    else "support_source",
                    "psyq_source": source == manifest.psyq_source,
                    "compiled_by_normal_build": True,
                    "blockers": [],
                    "eligible_members": [],
                }
                try:
                    text = content.decode()
                    records = parse_function_records(text)
                    settings = resolve_compiler_settings(source, text, flags, compilers)
                    arguments, _ = configured_command(
                        ROOT,
                        ROOT / source,
                        ROOT / "out/source-refinement/c05-unused.o",
                        allow_grouped=True,
                    )
                    seeds = inspect_arguments(arguments[1:], cwd=ROOT).forced
                    forced.update(value.path for value in seeds)
                    compiler_path = select_configured_compiler(
                        ROOT, settings.compiler_id
                    )
                    bind(compiler_path.relative_to(ROOT).as_posix())
                    source_row.update(
                        function_records=len(records),
                        metadata=[
                            {
                                "selector": f"{target}@0x{record.address:08X}",
                                "spelling": record.spelling,
                                "kind": record.kind,
                                "lifecycle": record.status,
                                "metadata": record.metadata,
                            }
                            for record in records
                        ],
                        configured_profile={
                            "compiler": settings.compiler_id or DEFAULT_COMPILER_ID,
                            "compiler_path": compiler_path.relative_to(ROOT).as_posix(),
                            "ordered_arguments": arguments,
                            "forced_inputs": [
                                value.path.relative_to(ROOT).as_posix()
                                for value in seeds
                            ],
                        },
                    )
                    if records and source != manifest.psyq_source:
                        captured = capture_members(ROOT, source)
                        if captured["target"] != target:
                            raise ValueError("capture owner differs")
                        for path, state in captured["inputs"].items():
                            data = bind(path)
                            if (
                                state is None
                                and data is not None
                                or state is not None
                                and (
                                    data is None
                                    or hashlib.sha256(data).hexdigest()
                                    != state["sha256"]
                                )
                            ):
                                raise ValueError(f"captured input differs: {path}")
                        source_row["eligible_members"] = [
                            {
                                **{
                                    key: value
                                    for key, value in member.items()
                                    if key != "original"
                                },
                                "original_size": len(member["original"]),
                                "file_end_exclusive": member["file_start"]
                                + len(member["original"]),
                                "original_sha256": hashlib.sha256(
                                    member["original"]
                                ).hexdigest(),
                            }
                            for member in captured["functions"]
                        ]
                        source_row["initialized_data_sections"] = {
                            name: {
                                **{
                                    key: value
                                    for key, value in section.items()
                                    if key != "original"
                                },
                                "original_size": len(section["original"]),
                                "original_sha256": hashlib.sha256(
                                    section["original"]
                                ).hexdigest(),
                            }
                            for name, section in captured["data_sections"].items()
                        }
                except (ValueError, OSError, UnicodeError) as error:
                    failure = {"target": target, "source": source, "error": str(error)}
                    source_row["blockers"].append(failure)
                    row["blockers"].append(failure)
                    blockers.append(failure)
                source_row["eligible_candidate"] = bool(source_row["eligible_members"])
                row["sources"].append(source_row)
            c_sources = [ROOT / source for source in claims if source.endswith(".c")]
            snapshot = stack.enter_context(
                closing(
                    IncludeSnapshot(
                        ROOT,
                        c_sources[0],
                        forced=tuple(sorted(set(c_sources[1:]) | forced)),
                    )
                )
            )
            for path, state in snapshot.describe_inputs().items():
                bind(path, state)

            # Successful ExitStack cleanup validates dependencies before closing watches.
            def validate_on_success(error_type, error, traceback, snapshot=snapshot):
                if error_type is None:
                    snapshot.validate()

            stack.push(validate_on_success)
            row["include_inputs"] = snapshot.describe_inputs()
            row["eligible_member_count"] = sum(
                len(source["eligible_members"]) for source in row["sources"]
            )
            row["eligible_original_function_bytes"] = sum(
                member["original_size"]
                for source in row["sources"]
                for member in source["eligible_members"]
            )
            rows.append(row)
        for name, state in list(states.items()):
            bind(name, state)
    report = {
        "schema": "bof3.payload-c05-preflight/v1",
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "scope": TARGETS,
        "targets": rows,
        "inputs": pins,
        "input_states": states,
        "blockers": blockers,
        "summary": {
            "targets": len(rows),
            "claims": sum(row["claim_count"] for row in rows),
            "eligible_members": sum(row["eligible_member_count"] for row in rows),
            "eligible_function_bytes": sum(
                row["eligible_original_function_bytes"] for row in rows
            ),
            "positive_inputs": sum(pin is not None for pin in pins.values()),
            "negative_inputs": sum(pin is None for pin in pins.values()),
        },
        "limitations": "Static metadata/map/layout/include admission only. No native compile, byte-match, rebuild, repack or writer lease. Candidate status is not native acceptance; metadata-free support remains in normal builds.",
    }
    OUTPUT.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    print(json.dumps(report["summary"], sort_keys=True))
    print(json.dumps(blockers, sort_keys=True))


if __name__ == "__main__":
    main()
