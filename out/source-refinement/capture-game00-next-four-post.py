"""Fresh complete Game00 naming POST compared against the accepted fresh PRE."""
from contextlib import closing
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
import time

from harness.build.compilation import configured_command
from harness.build.compiler import DEFAULT_COMPILER_ID, load_compiler_configuration, resolve_compiler_settings
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.functions import parse_function_records
from harness.domain.tags import parse_progress_tags
from harness.io import repo_layout
from harness.match._asm_disasm import INSTRUCTION_RE, extract_instructions
from harness.match._asm_diff_payload import first_instruction_mismatch, matching_instruction_count
from harness.match.extraction import read_function_bytes, read_linked_image
from harness.match.images import capture_source_images
from harness.match.members import capture_members
from harness.match.status_programs import StatusPrograms

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "out/source-refinement"
OUT = BASE / "game00-next-four-post"
PLAN = "out/source-refinement/next-four-game00-naming-ready-draft/plan.json"
PLAN_SHA = "99b6fc91f89d584c970be5437fe494796bf5ec27d87107f514abbefa57101ff7"
PRE = "out/source-refinement/game00-next-four-pre/summary.json"
PRE_SHA = "c27d43c1bf2b300fe34f2d8d6e34057cba947b7a5ff0471958d01c3bcbc03c24"
LIMIT = 64 << 20
TARGET = "emi/etc/game/00"


def sha(body):
    return hashlib.sha256(body).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, default=str) + "\n")


def main():
    assert Path.cwd() == ROOT and not OUT.exists()
    OUT.mkdir()
    started = time.monotonic()
    summary = {
        "status": "running",
        "task": "3.36",
        "plan_sha256": PLAN_SHA,
        "pre_sha256": PRE_SHA,
        "sources": [],
        "members": [],
        "partials": [],
        "limits": "Fresh complete POST only. Spelling-only equivalence is proved by identical current bytes and unchanged partial metrics; no new semantic or progress claim.",
    }
    try:
        with acquire_writer(ROOT), InputBatch(ROOT) as batch:
            states = {}
            def bind(name):
                state, body = batch.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                assert name not in states or states[name] == state, name
                states[name] = state
                return body

            plan_body = bind(PLAN)
            assert sha(plan_body) == PLAN_SHA
            plan = json.loads(plan_body)
            pre_body = bind(PRE)
            assert sha(pre_body) == PRE_SHA
            pre = json.loads(pre_body)
            assert pre["status"].startswith("complete PRE")
            pre_members = {m["selector"]: m for m in pre["members"]}
            pre_partials = {p["selector"]: p for p in pre["partials"]}
            assert len(pre_members) == 247 and len(pre_partials) == 18

            closure = plan["header_consumer_closure"]
            consumers = closure["consumers"]
            authored = [c for c in consumers if c["members"]]
            assert plan["target"] == TARGET and len(consumers) == closure["translation_units"] == 220
            assert len(authored) == 219 and sum(len(c["members"]) for c in authored) == closure["members"] == 247
            assert set(plan["proposed_output_sha256"]) == set(plan["owned_input_sha256"])
            for name, expected in plan["proposed_output_sha256"].items():
                assert sha(bind(name)) == expected, name
            for group in ("current_input_states", "include_inputs"):
                for name, expected in plan[group].items():
                    bind(name)
                    want = plan["proposed_output_sha256"].get(name)
                    if want is not None:
                        assert states[name]["sha256"] == want, name
                    elif expected is not None:
                        assert states[name]["sha256"] == expected["sha256"], name
            for name in plan["owned_paths"]:
                destination = OUT / "owned-post" / name
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(bind(name))
                bind(destination.relative_to(ROOT).as_posix())
            (OUT / "plan.json").write_bytes(plan_body)
            bind((OUT / "plan.json").relative_to(ROOT).as_posix())
            (OUT / "pre-summary.json").write_bytes(pre_body)
            bind((OUT / "pre-summary.json").relative_to(ROOT).as_posix())
            (OUT / "index.post.bin").write_bytes(bind(".git/index"))
            bind((OUT / "index.post.bin").relative_to(ROOT).as_posix())
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())

            names = plan["names"]
            sources = [ROOT / c["source"] for c in consumers]
            flags, compilers = load_compiler_configuration(ROOT)
            commands = {}
            representatives = {}
            for consumer in consumers:
                name = consumer["source"]
                text = bind(name).decode()
                settings = asdict(resolve_compiler_settings(name, text, flags, compilers))
                assert json.loads(json.dumps(settings, default=str)) == consumer["profile"]
                assert (settings["compiler_id"] or DEFAULT_COMPILER_ID) == consumer["effective_compiler"]
                command, _ = configured_command(ROOT, ROOT / name, OUT / "unused.o", allow_grouped=True)
                assert command[:-4] == consumer["configured_arguments"]
                assert not any(arg.startswith(("-include", "-imacros")) for arg in command), "Re-review forced include closure"
                commands[name] = command
                if consumer["members"]:
                    representatives.setdefault(tuple(command[:-4]), name)
            assert len(representatives) == 2
            with closing(IncludeSnapshot(ROOT, sources[0], forced=tuple(sources[1:]))) as includes, closing(StatusPrograms(ROOT)) as programs:
                for name, expected in includes.describe_inputs().items():
                    bind(name)
                    assert states[name] == expected, name
                summary["program_fingerprints_by_representative_source"] = {name: programs.fingerprint(ROOT / name) for name in representatives.values()}
                def guard():
                    includes.validate()
                    programs.validate()
                    for name in list(states):
                        bind(name)
                guard()
                prepared = []
                for consumer in authored:
                    source = consumer["source"]
                    text = bind(source).decode()
                    records = {r.address: r for r in parse_function_records(text)}
                    members = {int(m["selector"].split("@")[1], 16): m for m in consumer["members"]}
                    assert set(records) == set(members)
                    for address, record in records.items():
                        member = members[address]
                        expected_symbol = names.get(member["symbol"], member["symbol"])
                        assert record.kind == member["kind"], (source, hex(address))
                        assert record.spelling == expected_symbol, (source, hex(address), record.spelling)
                        progress = parse_progress_tags(record.metadata)
                        assert (list(progress) if progress is not None else None) == member["progress"], (source, hex(address))
                    captured = capture_members(ROOT, source)
                    assert captured["target"] == TARGET and {m["address"] for m in captured["functions"]} == set(records)
                    prepared.append((consumer, records, captured))
                assert sum(len(m["original"]) for _, _, c in prepared for m in c["functions"]) == 60312
                for consumer, records, captured in prepared:
                    guard()
                    source = consumer["source"]
                    directory = OUT / Path(source).stem
                    directory.mkdir()
                    print(source, flush=True)
                    comparison = capture_source_images(ROOT, source, directory)
                    report = comparison.report
                    save(directory / "comparison.json", report)
                    bind((directory / "comparison.json").relative_to(ROOT).as_posix())
                    assert report["fresh_compilation"]
                    grouped = len(captured["functions"]) > 1
                    if grouped:
                        assert report["schema"] == "bof3.grouped-comparison/v8" and report["member_count"] == len(records)
                        outputs = report["outputs"]
                        linked_names = [n for n in outputs if n.endswith(".elf")]
                    else:
                        assert report["schema"] == "harness.byte-match-one/v1"
                        outputs = report["native_outputs"]
                        linked_names = [n for n in outputs if n.endswith(".linked.o")]
                    assert len(linked_names) == 1
                    elf = (ROOT / linked_names[0]).read_bytes()
                    (directory / "actual.elf").write_bytes(elf)
                    bind((directory / "actual.elf").relative_to(ROOT).as_posix())
                    image = read_linked_image(elf, limit=LIMIT)
                    assert not image.relocations
                    assert {s.name for s in image.symbols if s.info & 15 == 2 and s.section} == {m["symbol"] for m in captured["functions"]}
                    summary["sources"].append({"source": source, "member_count": len(records), "comparison": (directory / "comparison.json").relative_to(ROOT).as_posix(), "profile": consumer["profile"], "configured_arguments": commands[source]})
                    for member in captured["functions"]:
                        address, selector, symbol = member["address"], member["selector"], member["symbol"]
                        original = member["original"]
                        current = read_function_bytes(elf, function_name=symbol, address=address).content
                        previous = pre_members[selector]
                        folder = directory / f"{address:08X}"
                        folder.mkdir()
                        (folder / "original.bin").write_bytes(original)
                        (folder / "current.bin").write_bytes(current)
                        bind((folder / "original.bin").relative_to(ROOT).as_posix())
                        bind((folder / "current.bin").relative_to(ROOT).as_posix())
                        assert sha(original) == previous["original_sha256"], f"original changed: {selector}"
                        assert sha(current) == previous["current_sha256"], f"current bytes changed: {selector}"
                        assert len(current) == previous["current_size"]
                        summary["members"].append({"selector": selector, "source": source, "symbol": symbol, "original_size": len(original), "current_size": len(current), "original_sha256": sha(original), "current_sha256": sha(current), "byte_match": current == original, "progress": previous["progress"], "frozen": previous["frozen"], "directory": folder.relative_to(ROOT).as_posix(), "actual_elf": (directory / "actual.elf").relative_to(ROOT).as_posix(), "elf_sha256": sha(elf)})
                    if grouped:
                        assert report["all_native_bytes_match"] and report["all_data_bytes_match"]
                    guard()
                    save(OUT / "summary.json", summary)
                objdump = repo_layout(ROOT).psyqo_toolchain_root / "bin/mipsel-none-elf-objdump"
                for member in summary["members"]:
                    if member["progress"] is None or member["progress"][0] != "partial":
                        continue
                    guard()
                    folder = ROOT / member["directory"]
                    result = run_bounded(ROOT, ["bin/harness", "lift", "asm-diff", member["selector"], "--json", "--detail", "full", "--image-evidence"], output_limit=LIMIT)
                    save(folder / "public.process.json", result)
                    bind((folder / "public.process.json").relative_to(ROOT).as_posix())
                    assert result["failure"] is None and result["exit_code"] in (0, 1)
                    public = json.loads(result["stdout"])
                    save(folder / "public.json", public)
                    bind((folder / "public.json").relative_to(ROOT).as_posix())
                    assert public["schema"] == "harness.asm-diff-one/v2"
                    assert int(public["address"], 16) == int(member["selector"].split("@")[1], 16)
                    current = (folder / "current.bin").read_bytes()
                    assert public["function_image"]["current_sha256"] == sha(current)
                    previous_partial = pre_partials[member["selector"]]
                    assert public["instruction_count"] == previous_partial["public_instruction_count"]
                    assert public["first_mismatch"] == previous_partial["public_first_mismatch"]
                    address = int(public["address"], 16)
                    dumps = {}
                    for kind, command in (("original", [str(objdump), "-D", "-b", "binary", "-m", "mips:3000", "-EL", f"--adjust-vma=0x{address:08X}", str(folder / "original.bin")]), ("current", [str(objdump), "-d", f"--start-address=0x{address:X}", f"--stop-address=0x{address + len(current):X}", str(ROOT / member["actual_elf"])])):
                        guard()
                        process = run_bounded(ROOT, command, output_limit=LIMIT)
                        save(folder / (kind + ".objdump.process.json"), process)
                        bind((folder / (kind + ".objdump.process.json")).relative_to(ROOT).as_posix())
                        assert process["failure"] is None and process["exit_code"] == 0
                        (folder / (kind + ".objdump")).write_text(process["stdout"])
                        bind((folder / (kind + ".objdump")).relative_to(ROOT).as_posix())
                        dumps[kind] = process["stdout"]
                        guard()
                    before, after = extract_instructions(dumps["original"]), extract_instructions(dumps["current"])
                    matching = matching_instruction_count(before, after)
                    metrics = {"rendered_instruction_count": {"original": len(before), "current": len(after), "matching": matching, "match_percent": round(100 * matching / max(len(before), len(after), 1), 2)}, "first_rendered_mismatch": first_instruction_mismatch(before, after)}
                    previous_metrics = json.loads((ROOT / previous_partial["complete_metrics"]).read_text())
                    assert metrics["rendered_instruction_count"] == previous_metrics["rendered_instruction_count"], member["selector"]
                    assert metrics["first_rendered_mismatch"] == previous_metrics["first_rendered_mismatch"], member["selector"]
                    save(folder / "complete-rendered-metrics.json", metrics)
                    bind((folder / "complete-rendered-metrics.json").relative_to(ROOT).as_posix())
                    summary["partials"].append({"selector": member["selector"], "complete_metrics": (folder / "complete-rendered-metrics.json").relative_to(ROOT).as_posix(), "unchanged_from_pre": True})
                    guard()
                    save(OUT / "summary.json", summary)
                assert len(summary["sources"]) == 219 and len(summary["members"]) == 247 and len(summary["partials"]) == 18
                assert sum(m["original_size"] for m in summary["members"]) == 60312
                assert sum(1 for m in summary["members"] if m["byte_match"]) == 229
                save(OUT / "terminal-input-states.json", states)
        summary.update(status="complete POST captures; independent acceptance pending", native_sources=219, normal_translation_units=220, member_count=247, complete_original_bytes=60312, guards_and_writer_closed=True, seconds=round(time.monotonic() - started, 3))
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed; keep evidence and fix cause", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary.update(cleanup="unconfirmed; stop native work", cleanup_observation=error.observed)
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
