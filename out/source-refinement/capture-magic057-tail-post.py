"""Capture the two strict MAGIC057 POST images after the single layout repair."""

from dataclasses import asdict
import hashlib
import json
from pathlib import Path

from harness.build.compiler import (
    DEFAULT_COMPILER_ID,
    build_compiler_arguments,
    load_compiler_configuration,
    resolve_compiler_settings,
)
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.domain.functions import parse_function_records
from harness.domain.layout import parse_splat_layout
from harness.match.extraction import read_function_bytes
from harness.match.images import capture_source_images
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
PLAN = "out/source-refinement/magic057-tail-boundary-draft/report.json"
PLAN_SHA = "0e80abf1002451654852420a06bc0f869861096cf190d0859c403327309a79f2"
PRE = "out/source-refinement/magic057-tail-boundary-public-pre"
PRE_SHA = "b8f9cc3580933802bc3da3f5c1225503dc8651be3edf8718fe5264fd8004aa60"
REFUSAL_SHA = "3092d4dfc8a2f7a4eae930d160fe677a1d7d591214cd1484eeef735ef671bb7d"
OUT = ROOT / "out/source-refinement/magic057-tail-boundary-strict-post"
TARGET = "emi/bmagic/magic057/03"
BINARY = "out/binaries/emi/bmagic/magic057/03.bin"
SPLAT = f"config/targets/{TARGET}/splat.yaml"
SPLAT_SHA = "1b79c814f644ca303882f19a6c338cd74f7edb0a47a98ef1329823b14f0a0466"
LIMIT = 64 * 1024 * 1024
FUNCTION_SHA = "b0f3eb35b529fbeb3a0af17e770ebe867315b41a83b504d1adaf066c0eedbc68"


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def read(batch, name, *, absent=False):
    return batch.read(
        ROOT / name,
        max_bytes=1 if absent else LIMIT,
        include_metadata=True,
        single_link=True,
    )


def main():
    require(Path.cwd() == ROOT, "run from the repository root")
    plan_bytes = (ROOT / PLAN).read_bytes()
    require(sha(plan_bytes) == PLAN_SHA, "review and repin the changed MAGIC057 draft")
    plan = json.loads(plan_bytes)
    require(
        plan["target"] == TARGET
        and len(plan["inputs"]) == 143
        and plan["evidence_files"][plan["draft_config"]]["sha256"] == SPLAT_SHA,
        "reviewed pool/config changed",
    )
    OUT.mkdir(exist_ok=False)
    (OUT / "plan.json").write_bytes(plan_bytes)
    summary = {
        "status": "running",
        "scope": "Two strict complete32B MAGIC057 POST captures after one Splat layout repair.",
        "plan_sha256": PLAN_SHA,
        "pre_summary_sha256": PRE_SHA,
        "members": [],
        "limit": "No duplicate public comparison. Complete raw POST equality preserves the accepted PRE instruction/byte proof. Normal concern/build/Splat/payload gates and independent acceptance remain parent-owned.",
    }
    save(OUT / "summary.json", summary)
    try:
        with acquire_writer(ROOT):
            with InputBatch(ROOT) as inputs:
                states = {}

                def retain(name, *, digest=None, absent=False):
                    state, content = read(inputs, name, absent=absent)
                    require(
                        state is None if absent else state is not None,
                        f"unexpected input presence: {name}",
                    )
                    if digest is not None:
                        require(
                            state["sha256"] == digest, f"input hash changed: {name}"
                        )
                    states[name] = state
                    return content

                retain(PLAN, digest=PLAN_SHA)
                pre_bytes = retain(f"{PRE}/summary.json", digest=PRE_SHA)
                pre = json.loads(pre_bytes)
                require(
                    pre["status"] == "complete"
                    and pre["selected_members"] == 2
                    and pre["public_comparisons"] == 4
                    and pre["original_function_bytes"] == 64
                    and pre["strict_terminal_layout_admitted"] is False
                    and pre["guards_and_writer_closed"] is True,
                    "complete guarded PRE is missing",
                )
                refusal = json.loads(
                    retain(f"{PRE}/strict-layout-refusal.json", digest=REFUSAL_SHA)
                )
                require(
                    refusal["layout_admitted"] is False
                    and refusal["boundary"] == plan["original_boundary"],
                    "historical35B refusal changed",
                )
                save(OUT / "pre-strict-layout-refusal.json", refusal)
                pins = dict(plan["inputs"])
                pins[SPLAT] = plan["evidence_files"][plan["draft_config"]]
                asset = plan["tail"]["future_asset"]
                binary = None
                source_images = {}
                for name, expected in pins.items():
                    if name == asset:
                        state, content = read(inputs, name)
                        require(
                            state is None
                            or (
                                len(content) == 3
                                and content.hex() == "0a0e0e"
                                and state["sha256"]
                                == plan["tail"]["future_expected_sha256"]
                            ),
                            "generated tail asset is neither absent nor the exact reviewed3B suffix",
                        )
                        states[name] = state
                        summary["generated_asset_at_start"] = state
                        continue
                    content = retain(
                        name,
                        digest=None if expected is None else expected["sha256"],
                        absent=expected is None,
                    )
                    if expected is not None:
                        require(
                            len(content) == expected["size"],
                            f"input size changed: {name}",
                        )
                    if name == BINARY:
                        binary = content
                    if name.endswith(".c"):
                        source_images[name] = content
                retain(".git/index")
                save(OUT / "input-states.json", states)
                require(
                    binary is not None
                    and len(binary) == 3807
                    and sha(binary) == plan["shipped_identity"]["sha256"],
                    "original full payload changed",
                )
                full, tail = binary[3772:3807], binary[3804:3807]
                require(
                    len(full) == 35
                    and len(tail) == 3
                    and tail.hex() == "0a0e0e"
                    and sha(full)
                    == plan["evidence_files"][
                        "out/source-refinement/magic057-tail-boundary-draft/original-full35.bin"
                    ]["sha256"],
                    "complete original terminal span/tail changed",
                )
                (OUT / "original-full35.bin").write_bytes(full)
                (OUT / "original-tail3.bin").write_bytes(tail)
                layout = parse_splat_layout(
                    ROOT / SPLAT, plan["shipped_identity"]["load_address"]
                )
                function_boundary = layout.find_containing_boundary(0x801EFABC)
                tail_boundary = layout.find_containing_boundary(0x801EFADC)
                require(
                    function_boundary is not None
                    and asdict(function_boundary) == plan["proposed_function_boundary"]
                    and function_boundary.file_size == 32
                    and tail_boundary is not None
                    and asdict(tail_boundary) == plan["proposed_tail_boundary"]
                    and tail_boundary.file_size == 3
                    and not tail_boundary.is_function,
                    "repaired32B C/3B bin boundaries differ",
                )
                save(
                    OUT / "layout.json",
                    {
                        "function": asdict(function_boundary),
                        "tail": asdict(tail_boundary),
                        "EOF": 3807,
                        "tail_semantic_kind": "unknown",
                    },
                )

                flags, compilers = load_compiler_configuration(ROOT)
                prepared = []
                sources = [
                    row
                    for row in plan["profiles"]["all_target_sources"]
                    if row["strict_records"]
                ]
                require(len(sources) == 2, "reviewed source pool changed")
                for row in sources:
                    source = row["source"]
                    record_rows = parse_function_records(source_images[source].decode())
                    require(
                        len(record_rows) == 1, "source no longer ordinary singleton"
                    )
                    record = record_rows[0]
                    selector = f"{TARGET}@0x{record.address:08X}"
                    require(
                        record.address in {0x801EFABC, 0x801EF81C}
                        and record.kind == "function"
                        and record.spelling == row["strict_records"][0]["name"],
                        "strict metadata identity changed",
                    )
                    before = json.loads(
                        retain(f"{PRE}/{record.address:08X}/metadata-profile.json")
                    )
                    settings = resolve_compiler_settings(
                        source, source_images[source].decode(), flags, compilers
                    )
                    arguments = build_compiler_arguments(ROOT, settings.flags)
                    require(
                        before["selector"] == selector
                        and before["source"] == source
                        and before["source_sha256"] == sha(source_images[source])
                        and before["metadata"] == record.metadata
                        and before["metadata_sha256"] == sha(record.metadata.encode())
                        and before["implementation"]
                        == source_images[source].decode()[
                            record.implementation_start : record.implementation_end
                        ]
                        and before["compiler"]
                        == (settings.compiler_id or DEFAULT_COMPILER_ID)
                        == row["compiler"]
                        and before["flags"] == settings.flags
                        and before["ordered_arguments"] == arguments,
                        "PRE/current metadata/body/ABI/profile changed",
                    )
                    captured = capture_members(ROOT, source)
                    require(
                        captured["target"] == TARGET
                        and captured["binary"] == BINARY
                        and len(captured["functions"]) == 1
                        and not captured["data_sections"],
                        "strict owner/member/data closure differs",
                    )
                    function = captured["functions"][0]
                    require(
                        function["selector"] == selector
                        and function["symbol"] == record.spelling
                        and function["address"] == record.address
                        and function["file_start"] == before["file_start"]
                        and len(function["original"]) == 32
                        and sha(function["original"]) == FUNCTION_SHA,
                        "strict complete32B member differs",
                    )
                    pre_rows = [
                        item for item in pre["members"] if item["selector"] == selector
                    ]
                    require(
                        len(pre_rows) == 2
                        and {item["operation"] for item in pre_rows}
                        == {"asm-diff", "byte-match"},
                        "both reviewed PRE comparisons missing",
                    )
                    for item in pre_rows:
                        prefix = f"{PRE}/{record.address:08X}/{item['operation']}"
                        original = retain(prefix + "/original.bin", digest=FUNCTION_SHA)
                        current = retain(prefix + "/current.bin", digest=FUNCTION_SHA)
                        elf = retain(
                            prefix + "/current.linked.o", digest=item["elf_sha256"]
                        )
                        actual = read_function_bytes(
                            elf, function_name=record.spelling, address=record.address
                        ).content
                        require(
                            len(original) == len(current) == 32
                            and actual == original == current == function["original"],
                            "actual PRE ELF/raw images differ",
                        )
                    prepared.append((before, captured))
                require(
                    {item[0]["address"] for item in prepared}
                    == {0x801EFABC, 0x801EF81C},
                    "incomplete two-member admission",
                )
                save(OUT / "input-states.json", states)

                for metadata, captured in prepared:
                    destination = OUT / f"{metadata['address']:08X}"
                    destination.mkdir()
                    comparison = capture_source_images(
                        ROOT, metadata["source"], destination
                    )
                    report = comparison.report
                    save(destination / "comparison.json", report)
                    require(
                        report["schema"] == "harness.byte-match-one/v1"
                        and report["fresh_compilation"] is True
                        and report["byte_match"]
                        and report["exact_match"]
                        and report["original_size"] == report["current_size"] == 32
                        and len(comparison.ranges) == 1,
                        "fresh strict POST exact32B evidence missing",
                    )
                    linked = Path(report["current_object"]).with_suffix(".linked.o")
                    require(
                        linked.is_absolute() and linked.is_relative_to(ROOT),
                        "actual POST linked ELF escaped repository",
                    )
                    name = linked.relative_to(ROOT).as_posix()
                    with InputBatch(ROOT) as artifacts:
                        linked_state, elf = read(artifacts, name)
                        require(
                            linked_state == report["native_outputs"][name],
                            "actual POST ELF state differs from fresh capture",
                        )
                        current = read_function_bytes(
                            elf,
                            function_name=metadata["symbol"],
                            address=metadata["address"],
                        ).content
                        emitted = comparison.ranges[0]
                        original = captured["functions"][0]["original"]
                        require(
                            emitted.address == metadata["address"]
                            and emitted.symbol == metadata["symbol"]
                            and emitted.section is None
                            and emitted.original_file_start == metadata["file_start"]
                            and emitted.content == current == original
                            and len(current) == 32
                            and sha(current) == FUNCTION_SHA
                            and report["function_image"]
                            == {
                                "original_sha256": FUNCTION_SHA,
                                "current_sha256": FUNCTION_SHA,
                            },
                            "complete POST immutable/ELF/original/PRE bytes differ",
                        )
                        (destination / "current.linked.o").write_bytes(elf)
                        save(destination / "linked-state.json", linked_state)
                    (destination / "original.bin").write_bytes(original)
                    (destination / "current.bin").write_bytes(current)
                    save(destination / "metadata-profile.json", metadata)
                    summary["members"].append(
                        {
                            "selector": metadata["selector"],
                            "size": 32,
                            "original_sha256": sha(original),
                            "current_sha256": sha(current),
                            "equals_both_actual_PRE_ELF_images": True,
                            "saved_elf": str(
                                (destination / "current.linked.o").relative_to(ROOT)
                            ),
                            "elf_sha256": sha(elf),
                        }
                    )
                    save(OUT / "summary.json", summary)
                    print(
                        f"{metadata['selector']}: strict complete32B exact; actual ELF and both PRE images agree",
                        flush=True,
                    )
                for name, expected in states.items():
                    observed, _ = read(inputs, name, absent=expected is None)
                    require(
                        observed == expected, f"terminal input/PRE/index drift: {name}"
                    )
        require(len(summary["members"]) == 2, "incomplete strict POST capture")
        summary.update(
            status="complete",
            selected_members=2,
            fresh_strict_captures=2,
            complete_original_function_bytes=64,
            original_terminal_span=35,
            retained_unknown_tail_bytes=3,
            guards_and_writer_closed=True,
            duplicated_public_comparisons=0,
        )
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed", error=f"{type(error).__name__}: {error}")
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
