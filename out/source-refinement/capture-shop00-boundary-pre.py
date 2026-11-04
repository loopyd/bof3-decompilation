"""Capture the unchanged SHOP52B original member before its reviewed layout promotion."""

import argparse
from contextlib import closing
from dataclasses import asdict
import hashlib
import json
import os
from pathlib import Path

from harness.build.compilation import configured_command
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.functions import parse_function_records
from harness.io import repo_layout
from harness.match._asm_disasm import INSTRUCTION_RE, extract_instructions
from harness.match._asm_diff_payload import first_instruction_mismatch, matching_instruction_count
from harness.match._asm_resolve import object_path_for_source
from harness.match.extraction import read_function_bytes, read_linked_image
from harness.match.status_programs import StatusPrograms

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/shop00-boundary-pre"
PLAN = "out/source-refinement/shop00-boundary-draft/plan.json"
PLAN_SHA = "bc371844d3abab4acaae2f54de5f2a66c4a7ce70b498d1d51d4034d5237f9bf1"
REVIEW = "out/source-refinement/shop00-boundary-static-independent-review.json"
parser = argparse.ArgumentParser()
parser.add_argument("--review-sha256", required=True)
REVIEW_SHA = parser.parse_args().review_sha256
LIMIT = 64 * 1024 * 1024


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def coverage(disassembly, content, address):
    require(content and len(content) % 4 == 0, "Incomplete aligned function image")
    seen = set()
    for line in disassembly.splitlines():
        match = INSTRUCTION_RE.match(line)
        if match is None:
            continue
        pc = int(match.group("address"), 16)
        offset = pc - address
        require(pc not in seen and offset >= 0 and offset % 4 == 0 and offset + 4 <= len(content), "Duplicate or escaped rendered word")
        word = int(line[match.end("address"):match.start("instruction")].strip().removeprefix(":").strip(), 16)
        require(word == int.from_bytes(content[offset:offset + 4], "little"), "Rendered word differs from raw image")
        seen.add(pc)
    require(seen == set(range(address, address + len(content), 4)), "Zero-inclusive rendering omitted raw words")
    return {"raw_word_count": len(content) // 4, "rendered_word_count": len(seen), "rendered_addresses": [f"0x{pc:08X}" for pc in sorted(seen)], "all_words_rendered": True}


def main():
    require(Path.cwd() == ROOT and not OUT.exists(), "Run at root with fresh output")
    OUT.mkdir()
    summary = {"status": "running", "plan_sha256": PLAN_SHA, "review_sha256": REVIEW_SHA, "members": [], "limits": "One unchanged singleton public PRE captures only. Original endpoints come from independent original review, never emitted sizes. Preserve actual exact/partial sizes and metrics without progress adoption. Public diagnostics and complete zero-inclusive rendered metrics remain separate; raw images are authoritative. All143 normal source/include states remain guarded; only one member compiles in this PRE. Unsupported caller prose is a later separate application output. No source, layout, profile or Git/index edits; no acceptance claimed."}
    save(OUT / "summary.json", summary)
    try:
        with acquire_writer(ROOT), InputBatch(ROOT) as inputs:
            states = {}

            def bind(name):
                state, content = inputs.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                require(name not in states or states[name] == state, f"Input changed: {name}")
                states[name] = state
                return content

            data = bind(PLAN)
            require(sha(data) == PLAN_SHA and sha(bind(REVIEW)) == REVIEW_SHA, "Approved proposal/review changed")
            plan = json.loads(data)
            members = plan["members"]
            require(len(members) == 1 and sum(m["original_size"] for m in members) == 52, "One-member range closure changed")
            for name, expected in plan["current_input_states"].items():
                bind(name)
                require(states[name] == expected, f"Reviewed state changed: {name}")
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            index = bind(".git/index")
            require(index is not None, "Current index absent")
            (OUT / "index-pre").write_bytes(index)
            (OUT / "plan.json").write_bytes(data)
            summary["runtime_index_state"] = states[".git/index"]
            binary = bind(plan["original_binary"]["path"])
            require(sha(binary) == plan["original_binary"]["sha256"], "Immutable SHOP00 binary changed")
            for member in members:
                source = bind(member["source"])
                require([asdict(r) for r in parse_function_records(source.decode())] == [member["record"]], "Source metadata or implementation changed")
                original = bind(member["original_path"])
                first = member["original_file_start"]
                require(original == binary[first:first + member["original_size"]] and sha(original) == member["original_sha256"], "Complete original range changed")
                bind(member["cfg_evidence"])
                destination = OUT / "owned-pre" / member["source"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(source)
            environment = dict(os.environ)
            sources = [ROOT / unit["source"] for unit in plan["normal_build_units"]]
            require(len(sources) == len(set(sources)) == 143, "Normal source closure changed")
            with closing(IncludeSnapshot(ROOT, sources[0], forced=tuple(sources[1:]))) as includes, closing(StatusPrograms(ROOT)) as programs:
                for name, expected in includes.describe_inputs().items():
                    bind(name)
                    require(states[name] == expected, f"Include changed: {name}")
                require(includes.describe_inputs() == plan["include_input_states"], "Literal dependency/negative lookup closure changed")
                summary["configured_program_fingerprints"] = {m["source"]: programs.fingerprint(ROOT / m["source"]) for m in members}
                summary["comparison_programs"] = programs.common
                objdump = repo_layout(ROOT).psyqo_toolchain_root / "bin/mipsel-none-elf-objdump"
                summary["effective_objdump"] = str(objdump)

                def guard():
                    for name in list(states):
                        bind(name)
                    includes.validate()
                    programs.validate()

                def run(argv, folder, label):
                    guard()
                    with (folder / (label + ".stdout")).open("wb") as stdout, (folder / (label + ".stderr")).open("wb") as stderr:
                        def retain(stream, chunk):
                            (stdout if stream == "stdout" else stderr).write(chunk)
                        result = run_bounded(ROOT, argv, env=environment, output_limit=LIMIT, on_output=retain)
                    save(folder / (label + ".process.json"), result)
                    return result

                save(OUT / "starting-input-states.json", states)
                for member in members:
                    symbol, address = member["symbol"], member["original_start"]
                    folder = OUT / symbol
                    folder.mkdir()
                    arguments, _ = configured_command(ROOT, ROOT / member["source"], Path(member["configured_arguments"][-1]))
                    require(arguments == member["configured_arguments"], "Ordered configured profile changed")
                    summary["active_selector"] = member["selector"]
                    save(OUT / "summary.json", summary)
                    argv = ["bin/harness", "lift", "asm-diff", member["selector"], "--json", "--detail", "full", "--image-evidence"]
                    print(" ".join(argv), flush=True)
                    result = run(argv, folder, "public")
                    try:
                        report = json.loads(result["stdout"])
                    except (ValueError, TypeError):
                        report = {}
                    for key, name in report.get("outputs", {}).items():
                        artifact = Path(name)
                        if artifact.is_absolute() and artifact.is_relative_to(ROOT) and artifact.is_file():
                            (folder / ("public-" + key + "-" + artifact.name)).write_bytes(bind(artifact.relative_to(ROOT).as_posix()))
                    if result["failure"] or result["exit_code"] not in (0, 1):
                        object_path = object_path_for_source(repo_layout(ROOT), ROOT / member["source"])
                        for artifact in (object_path, object_path.with_name(object_path.name + ".s"), object_path.with_suffix(".linked.o")):
                            if artifact.is_file():
                                (folder / ("failed-unverified-" + artifact.name)).write_bytes(bind(artifact.relative_to(ROOT).as_posix()))
                    require(not result["failure"] and result["exit_code"] in (0, 1), f"Public PRE unavailable: {symbol}")
                    require(report["schema"] == "harness.asm-diff-one/v2" and report["function"] == symbol and int(report["address"], 16) == address and report["source"] == str(ROOT / member["source"]), "Public identity changed")
                    require(report["original_size"] == member["original_size"] and report["function_image"]["original_sha256"] == member["original_sha256"], "Public inference differs from independently closed original range")
                    linked = Path(report["current_object"]).with_suffix(".linked.o")
                    require(linked.is_absolute() and linked.is_relative_to(ROOT), "Escaped linked image")
                    elf = bind(linked.relative_to(ROOT).as_posix())
                    image = read_linked_image(elf, limit=LIMIT)
                    functions = [s for s in image.symbols if s.info & 15 == 2 and s.section]
                    require(len(functions) == 1 and functions[0].name == symbol and not image.relocations, "Unexpected linked function/relocation inventory")
                    current = read_function_bytes(elf, function_name=symbol, address=address).content
                    original = bind(member["original_path"])
                    public_original = bind(Path(report["outputs"]["original_bytes"]).relative_to(ROOT).as_posix())
                    require(public_original == original and len(current) == report["current_size"] and sha(current) == report["function_image"]["current_sha256"], "Actual complete native/raw image changed")
                    require(report["size_delta"] == len(current) - len(original) and report["byte_match"] == (current == original) and result["exit_code"] == (0 if current == original else 1), "Public size/byte status inconsistent")
                    for suffix, content in (("linked.o", elf), ("original.bin", original), ("current.bin", current)):
                        (folder / suffix).write_bytes(content)
                        bind((folder / suffix).relative_to(ROOT).as_posix())
                    save(folder / "elf-inventory.json", {"symbols": [asdict(s) for s in image.symbols], "sections": [{k: v for k, v in asdict(s).items() if k != "content"} | {"sha256": None if s.content is None else sha(s.content)} for s in image.sections]})
                    disassembly = {}
                    for side, argv in (("original", [str(objdump), "-D", "-z", "-b", "binary", "-m", "mips:3000", "-EL", f"--adjust-vma=0x{address:08x}", str(folder / "original.bin")]), ("current", [str(objdump), "-d", "-z", f"--start-address=0x{address:X}", f"--stop-address=0x{address + len(current):X}", str(folder / "linked.o")])):
                        rendered = run(argv, folder, side + "-complete-objdump")
                        require(not rendered["failure"] and rendered["exit_code"] == 0, "Complete original/current rendering failed")
                        disassembly[side] = rendered["stdout"]
                    cover = {side: coverage(disassembly[side], raw, address) for side, raw in (("original", original), ("current", current))}
                    left, right = [extract_instructions(disassembly[side]) for side in ("original", "current")]
                    require(len(left) == len(original) // 4 and len(right) == len(current) // 4, "Complete word/instruction coverage differs")
                    matching = matching_instruction_count(left, right)
                    metrics = {"instruction_count": {"original": len(left), "current": len(right), "matching": matching, "match_percent": round(100 * matching / max(len(left), len(right), 1), 2)}, "first_mismatch": first_instruction_mismatch(left, right), "raw_word_counts": {side: v["raw_word_count"] for side, v in cover.items()}, "coverage": cover, "method": "Existing normalized instruction/SequenceMatcher owners over bounded zero-inclusive objdump of complete saved raw/ELF extents. No inferred or fabricated instructions."}
                    save(folder / "complete-metrics.json", metrics)
                    summary["members"].append({**member, "report": report, "actual_comparison_exit_code": result["exit_code"], "elf_sha256": sha(elf), "saved_elf": str((folder / "linked.o").relative_to(ROOT)), "full_metrics": metrics, "raw_exact": current == original, "current_size": len(current), "authored_metadata_preserved": member["record"]["metadata"], "configured_arguments": arguments})
                    summary["active_selector"] = None
                    save(OUT / "summary.json", summary)
                    guard()
                guard()
                save(OUT / "terminal-input-states.json", states)
        summary.update(status="complete PRE captures; independent acceptance pending", guards_and_writer_closed=True, member_count=1, complete_original_bytes=52)
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed; keep evidence and fix cause", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary.update(cleanup="unconfirmed; stop native work", cleanup_observation=error.observed)
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
