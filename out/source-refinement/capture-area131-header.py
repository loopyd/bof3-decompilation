"""Capture the two Area131 header/move members; strict member admission applies."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil

from harness.build.compiler import (
    DEFAULT_COMPILER_ID,
    build_compiler_arguments,
    load_compiler_configuration,
    resolve_compiler_settings,
)
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import run_bounded
from harness.domain.functions import parse_function_records
from harness.domain.tags import parse_progress_tags
from harness.match.images import capture_source_images
from harness.match.members import capture_members
from harness.match.extraction import read_function_bytes
from harness.match.validation import _exact_evidence

ROOT = Path(__file__).resolve().parents[2]
PLAN = ROOT / "out/source-refinement/area131-header-ready-draft/plan.json"
PLAN_SHA = "b6ed0e773351f01ff3893a82aca4482934f37c200e57f91de40960d8036e35d4"


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("stage", choices=("public-pre", "pre", "post"))
    args = parser.parse_args()
    require(Path.cwd() == ROOT, "run from the repository root")
    content = PLAN.read_bytes()
    require(
        sha(content) == PLAN_SHA,
        "Area131 plan changed; review and repin this fixed helper",
    )
    plan = json.loads(content)
    members = plan["members"]
    require(
        len(members) == 2
        and {m["reviewed_original"]["address"] for m in members}
        == {"0x801F2D78", "0x801F2DC8"},
        "expected exactly two reviewed Area131 members",
    )
    pins = (
        plan["current_source_config_pins"]
        if args.stage != "post"
        else plan["projected_post_pins"]["sha256"]
    )
    require(
        len(plan["current_source_config_pins"]) == 119
        and len(pins) == (119 if args.stage != "post" else 120),
        "reviewed input count changed",
    )
    absent = set(
        plan["current_absence_pins"]
        if args.stage != "post"
        else plan["literal_closure"]["negative_lookup_absences"]
    )
    if args.stage == "post":
        absent.update(m["old_source"] for m in members)
    pre_absent = [plan["new_header"]["path"], *(m["new_source"] for m in members)]
    output = ROOT / f"out/source-refinement/area131-header-{args.stage}"
    summary = {
        "status": "running",
        "stage": args.stage,
        "plan_sha256": PLAN_SHA,
        "members": [],
        "acceptance": "Complete selected native images only; independent review and remaining gates are parent-owned.",
    }
    output.mkdir(exist_ok=False)
    (output / "plan.json").write_bytes(content)
    save(output / "summary.json", summary)
    try:
        with acquire_writer(ROOT):
            with InputBatch(ROOT) as batch:
                states = {}
                saved = {}
                for name, digest in pins.items():
                    state, image = batch.read(
                        ROOT / name,
                        max_bytes=64 * 1024 * 1024,
                        include_metadata=True,
                        single_link=True,
                    )
                    require(
                        state is not None and state["sha256"] == digest,
                        f"reviewed input changed: {name}",
                    )
                    states[name] = state
                    if name in plan["owned_before_sha256"]:
                        saved[name] = image
                for name in absent:
                    state, image = batch.read(
                        ROOT / name,
                        max_bytes=1,
                        include_metadata=True,
                        single_link=True,
                    )
                    require(
                        state is None and image is None,
                        f"expected absent input: {name}",
                    )
                index_state, index = batch.read(
                    ROOT / ".git/index",
                    max_bytes=64 * 1024 * 1024,
                    include_metadata=True,
                    single_link=True,
                )
                require(index_state is not None, "raw index is absent")
                (output / "index").write_bytes(index)
                save(output / "index-state.json", index_state)
                if args.stage != "post":
                    require(
                        len(saved) == 4 and len(pre_absent) == 3,
                        "expected four owned PRE files and three destination absences",
                    )
                    for name, image in saved.items():
                        destination = output / "original" / name
                        destination.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(ROOT / name, destination)
                        require(
                            destination.read_bytes() == image,
                            f"PRE copy changed: {name}",
                        )
                    save(
                        output / "owned-pre.json",
                        {
                            "present": {n: states[n] for n in saved},
                            "absent": pre_absent,
                        },
                    )
                save(
                    output / "input-states.json",
                    {"present": states, "absent": sorted(absent)},
                )

                # Admit both sources before starting either native compilation.
                prepared = []
                flags, compilers = load_compiler_configuration(ROOT)
                for member in members:
                    source = (
                        member["old_source"]
                        if args.stage != "post"
                        else member["new_source"]
                    )
                    text = (ROOT / source).read_text()
                    records = parse_function_records(text)
                    require(len(records) == 1, f"not a reviewed singleton: {source}")
                    record = records[0]
                    require(
                        record.address
                        == int(member["reviewed_original"]["address"], 16)
                        and record.kind == member["kind"]
                        and record.spelling == member["lexical_spelling"],
                        f"metadata identity changed: {source}",
                    )
                    require(
                        sha(record.metadata.encode()) == member["metadata_sha256"]
                        and sha(
                            text[
                                record.implementation_start : record.implementation_end
                            ].encode()
                        )
                        == member["implementation_or_invocation_sha256"]
                        and list(parse_progress_tags(record.metadata))
                        == member["progress"],
                        f"metadata/body/progress changed: {source}",
                    )
                    settings = resolve_compiler_settings(source, text, flags, compilers)
                    profile = {
                        "compiler_id": settings.compiler_id,
                        "flags": list(settings.flags)
                        if settings.flags is not None
                        else None,
                    }
                    require(
                        profile == member["profile"]
                        and (settings.compiler_id or DEFAULT_COMPILER_ID)
                        == member["effective_compiler"]
                        and build_compiler_arguments(ROOT, settings.flags)
                        == member["configured_arguments"],
                        f"profile changed: {source}",
                    )
                    if args.stage == "public-pre":
                        prepared.append(
                            (member, source, None, record.metadata, profile)
                        )
                        continue
                    captured = capture_members(ROOT, source)
                    require(
                        len(captured["functions"]) == 1
                        and not captured["data_sections"],
                        f"unexpected additional function/data ownership: {source}",
                    )
                    function = captured["functions"][0]
                    reviewed = member["reviewed_original"]
                    require(
                        function["selector"] == member["selector"]
                        and function["symbol"] == member["name"]
                        and function["file_start"] == reviewed["file_start"]
                        and len(function["original"]) == reviewed["size"] == 40
                        and sha(function["original"]) == reviewed["sha256"],
                        f"reviewed original range changed: {source}",
                    )
                    prepared.append(
                        (member, source, function, record.metadata, profile)
                    )
                for member, source, function, metadata, profile in prepared:
                    destination = output / member["reviewed_original"][
                        "address"
                    ].removeprefix("0x")
                    destination.mkdir()
                    if args.stage == "public-pre":
                        command = [
                            "bin/harness",
                            "lift",
                            "byte-match",
                            member["selector"],
                            "--json",
                            "--image-evidence",
                        ]
                        with (
                            (destination / "stdout").open("wb") as stdout,
                            (destination / "stderr").open("wb") as stderr,
                        ):

                            def retain(stream, chunk):
                                (stdout if stream == "stdout" else stderr).write(chunk)

                            result = run_bounded(
                                ROOT,
                                command,
                                output_limit=64 * 1024 * 1024,
                                on_output=retain,
                            )
                        save(destination / "process.json", result)
                        require(
                            not result["failure"] and result["exit_code"] == 0,
                            f"public PRE failed: {source}",
                        )
                        check = {
                            "argv": command,
                            "target": plan["target"],
                            "selector": member["selector"],
                            "source": source,
                            "function": member["name"],
                        }
                        require(
                            _exact_evidence(check, result["stdout"], ROOT),
                            f"public exact evidence invalid: {source}",
                        )
                        report = json.loads(result["stdout"])
                        reviewed = member["reviewed_original"]
                        require(
                            report["original_size"]
                            == report["current_size"]
                            == reviewed["size"]
                            == 40,
                            f"public PRE did not cover complete forty bytes: {source}",
                        )
                        binary = Path(report["original_binary"])
                        linked = Path(report["current_object"]).with_suffix(".linked.o")
                        require(
                            binary.is_relative_to(ROOT) and linked.is_relative_to(ROOT),
                            "public image path escaped repository",
                        )
                        binary_state, binary_image = batch.read(
                            binary,
                            max_bytes=64 * 1024 * 1024,
                            include_metadata=True,
                            single_link=True,
                        )
                        require(
                            binary_state == states[binary.relative_to(ROOT).as_posix()],
                            "original binary state changed",
                        )
                        original = binary_image[
                            reviewed["file_start"] : reviewed["file_end"]
                        ]
                        linked_state, elf = batch.read(
                            linked,
                            max_bytes=64 * 1024 * 1024,
                            include_metadata=True,
                            single_link=True,
                        )
                        current = read_function_bytes(
                            elf,
                            function_name=member["name"],
                            address=int(reviewed["address"], 16),
                        ).content
                        require(
                            len(original) == len(current) == 40
                            and original == current
                            and sha(original) == reviewed["sha256"]
                            and report["function_image"]
                            == {
                                "original_sha256": sha(original),
                                "current_sha256": sha(current),
                            },
                            f"actual public PRE images differ: {source}",
                        )
                        (destination / "original.bin").write_bytes(original)
                        (destination / "current.bin").write_bytes(current)
                        (destination / "current.linked.o").write_bytes(elf)
                        save(destination / "comparison.json", report)
                        save(
                            destination / "metadata-profile.json",
                            {
                                "selector": member["selector"],
                                "source": source,
                                "metadata": metadata,
                                "profile": profile,
                                "configured_arguments": member["configured_arguments"],
                                "linked_state": linked_state,
                                "limit": "Public byte-match evidence and observed inputs; strict capture_members admission is a separate pending prerequisite.",
                            },
                        )
                        summary["members"].append(
                            {
                                "selector": member["selector"],
                                "source": source,
                                "size": 40,
                                "original_sha256": sha(original),
                                "current_sha256": sha(current),
                            }
                        )
                        save(output / "summary.json", summary)
                        continue
                    comparison = capture_source_images(ROOT, source, destination)
                    save(destination / "comparison.json", comparison.report)
                    require(
                        len(comparison.ranges) == 1,
                        f"unexpected emitted range ownership: {source}",
                    )
                    emitted = comparison.ranges[0]
                    require(
                        emitted.address == function["address"]
                        and emitted.symbol == function["symbol"]
                        and emitted.original_file_start == function["file_start"]
                        and emitted.section is None
                        and emitted.content == function["original"],
                        f"complete current image differs: {source}",
                    )
                    (destination / "original.bin").write_bytes(function["original"])
                    (destination / "current.bin").write_bytes(emitted.content)
                    save(
                        destination / "metadata-profile.json",
                        {
                            "selector": member["selector"],
                            "source": source,
                            "symbol": function["symbol"],
                            "metadata": metadata,
                            "profile": profile,
                            "configured_arguments": member["configured_arguments"],
                            "reviewed_original": member["reviewed_original"],
                        },
                    )
                    summary["members"].append(
                        {
                            "selector": member["selector"],
                            "source": source,
                            "size": 40,
                            "original_sha256": sha(function["original"]),
                            "current_sha256": sha(emitted.content),
                        }
                    )
                    save(output / "summary.json", summary)
                for name, expected in {**states, ".git/index": index_state}.items():
                    observed, _ = batch.read(
                        ROOT / name,
                        max_bytes=64 * 1024 * 1024,
                        include_metadata=True,
                        single_link=True,
                    )
                    require(observed == expected, f"terminal input/index drift: {name}")
                for name in absent:
                    observed, _ = batch.read(
                        ROOT / name,
                        max_bytes=1,
                        include_metadata=True,
                        single_link=True,
                    )
                    require(observed is None, f"terminal absent input appeared: {name}")
        require(len(summary["members"]) == 2, "incomplete two-member capture")
        summary.update(
            status="complete", complete_original_bytes=80, guards_and_writer_closed=True
        )
        save(output / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        save(output / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
