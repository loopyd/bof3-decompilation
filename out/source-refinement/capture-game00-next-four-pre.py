"""Fresh complete Game00 naming PRE through existing native comparison owners."""
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
OUT = BASE / "game00-next-four-pre"
PLAN = "out/source-refinement/next-four-game00-naming-ready-draft/plan.json"
PLAN_SHA = "99b6fc91f89d584c970be5437fe494796bf5ec27d87107f514abbefa57101ff7"
BASELINE = "out/source-refinement/game00-next-eight-post-images/members.json"
BASELINE_SHA = "004b1866984082bc09d379f4f75b7d9e864d98f4a1514bff1ad97550df0dbf5f"
LIMIT = 64 << 20
TARGET = "emi/etc/game/00"


def sha(body):
    return hashlib.sha256(body).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, default=str) + "\n")


def coverage(dump, raw, address):
    seen = set()
    for line in dump.splitlines():
        match = INSTRUCTION_RE.match(line)
        if match is None:
            continue
        pc = int(match.group("address"), 16)
        offset = pc - address
        assert 0 <= offset < len(raw) and offset % 4 == 0 and pc not in seen
        printed = line[match.end("address"):match.start("instruction")]
        assert int(printed.strip().removeprefix(":").strip(), 16) == int.from_bytes(raw[offset:offset + 4], "little")
        seen.add(pc)
    omitted = []
    for offset in range(0, len(raw), 4):
        if address + offset not in seen:
            assert raw[offset:offset + 4] == b"\0" * 4, "Nonzero omitted disassembly word"
            omitted.append({"address": f"0x{address + offset:08X}", "offset": offset, "bytes_le": "00000000"})
    return {"raw_word_count": len(raw) // 4, "rendered_word_count": len(seen), "omitted_zero_words": omitted, "all_words_rendered": not omitted}


def main():
    assert Path.cwd() == ROOT and not OUT.exists()
    OUT.mkdir()
    started = time.monotonic()
    summary = {"status": "running", "task": "3.36", "plan_sha256": PLAN_SHA, "sources": [], "members": [], "partials": [], "limits": "Fresh complete PRE only; no naming/progress/native acceptance. Historical 247 images are comparison baselines, never substituted for current emission. One metadata-free binding TU is guarded for later normal build, with zero function credit. Raw images are authoritative; public inferred-range scores and complete rendered metrics remain separate. No wall-clock timeout is provided by run_bounded; operations are serial and output-bounded."}
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
            closure = plan["header_consumer_closure"]
            consumers = closure["consumers"]
            authored = [c for c in consumers if c["members"]]
            assert plan["target"] == TARGET and len(consumers) == closure["translation_units"] == 220
            assert len(authored) == 219 and sum(len(c["members"]) for c in authored) == closure["members"] == 247
            assert closure["counts"] == {"exact": 226, "partial": 18, "tag_free": 3}
            assert [c["source"] for c in consumers if not c["members"]] == closure["metadata_free_binding_sources"] == ["src/bof3/support/game00_symbols.c"]
            assert sorted(len(c["members"]) for c in authored if len(c["members"]) > 1) == [3, 3, 25]
            for name, expected in plan["current_input_states"].items():
                bind(name)
                assert states[name] == expected, name
            for name, expected in plan["include_inputs"].items():
                bind(name)
                assert states[name] == expected, name
            for name in plan["owned_paths"]:
                body = bind(name)
                destination = OUT / "owned-pre" / name
                destination.parent.mkdir(parents=True, exist_ok=True)
                assert body is not None
                destination.write_bytes(body)
                bind(destination.relative_to(ROOT).as_posix())
            (OUT / "plan.json").write_bytes(plan_body)
            bind((OUT / "plan.json").relative_to(ROOT).as_posix())
            (OUT / "index.pre.bin").write_bytes(bind(".git/index"))
            bind((OUT / "index.pre.bin").relative_to(ROOT).as_posix())
            summary["runtime_index_state"] = states[".git/index"]
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            baseline_body = bind(BASELINE)
            assert sha(baseline_body) == BASELINE_SHA
            old = {m["selector"]: m for m in json.loads(baseline_body)["members"]}
            assert len(old) == 247 and set(old) == {m["selector"] for c in authored for m in c["members"]}
            for member in old.values():
                assert sha(bind(member["images"]["original"])) == member["original_sha256"]
                assert sha(bind(member["images"]["current"])) == member["current_sha256"]
            sources = [ROOT / c["source"] for c in consumers]
            flags, compilers = load_compiler_configuration(ROOT)
            commands = {}
            representatives = {}
            for consumer in consumers:
                name = consumer["source"]
                text = bind(name).decode()
                assert sha(text.encode()) == consumer["sha256"]
                settings = asdict(resolve_compiler_settings(name, text, flags, compilers))
                assert json.loads(json.dumps(settings)) == consumer["profile"]
                assert (settings["compiler_id"] or DEFAULT_COMPILER_ID) == consumer["effective_compiler"]
                command, _ = configured_command(ROOT, ROOT / name, OUT / "unused.o", allow_grouped=True)
                assert command[:-4] == consumer["configured_arguments"]
                assert not any(arg.startswith(("-include", "-imacros")) for arg in command), "Re-review forced include closure"
                commands[name] = command
                if consumer["members"]:
                    representatives.setdefault(tuple(command[:-4]), name)
            assert len(representatives) == 2
            assert sum(c["profile"]["flags"] is None for c in consumers) == 219
            assert next(c for c in consumers if c["profile"]["flags"] is not None)["source"] == "src/bof3/ui/resetSelectionState.c"
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
                save(OUT / "starting-input-states.json", states)
                prepared = []
                for consumer in authored:
                    source = consumer["source"]
                    text = bind(source).decode()
                    records = {r.address: r for r in parse_function_records(text)}
                    members = {int(m["selector"].split("@")[1], 16): m for m in consumer["members"]}
                    assert set(records) == set(members)
                    for address, record in records.items():
                        member = members[address]
                        assert record.kind == member["kind"] and record.spelling == member["symbol"]
                        assert sha(record.metadata.encode()) == member["metadata_sha256"]
                        assert sha(text[record.implementation_start:record.implementation_end].encode()) == member["implementation_sha256"]
                        progress = parse_progress_tags(record.metadata)
                        assert (list(progress) if progress is not None else None) == member["progress"]
                    captured = capture_members(ROOT, source)
                    assert captured["target"] == TARGET and {m["address"] for m in captured["functions"]} == set(records)
                    for name, expected in captured["inputs"].items():
                        bind(name)
                        assert (states[name] is None) == (expected is None)
                        assert expected is None or states[name]["sha256"] == expected["sha256"]
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
                    for key in ("native_inputs", "include_inputs"):
                        for name, expected in report[key].items():
                            bind(name)
                            assert states[name] == expected, name
                    with InputBatch(ROOT) as artifacts:
                        saved_outputs = {}
                        for name, expected in outputs.items():
                            observed, body = artifacts.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                            assert observed == expected, name
                            path = directory / ("retained-" + Path(name).name)
                            path.write_bytes(body)
                            bind(path.relative_to(ROOT).as_posix())
                            saved_outputs[name] = {"path": path.relative_to(ROOT).as_posix(), "state": observed}
                        elf = (ROOT / linked_names[0]).read_bytes()
                        assert sha(elf) == outputs[linked_names[0]]["sha256"]
                        (directory / "actual.elf").write_bytes(elf)
                        bind((directory / "actual.elf").relative_to(ROOT).as_posix())
                        image = read_linked_image(elf, limit=LIMIT)
                        assert not image.relocations
                        assert {s.name for s in image.symbols if s.info & 15 == 2 and s.section} == {m["symbol"] for m in captured["functions"]}
                        for name, expected in outputs.items():
                            assert artifacts.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)[0] == expected
                    summary["sources"].append({"source": source, "member_count": len(records), "comparison": (directory / "comparison.json").relative_to(ROOT).as_posix(), "saved_outputs": saved_outputs, "profile": consumer["profile"], "configured_arguments": commands[source]})
                    for member in captured["functions"]:
                        address, selector, symbol = member["address"], member["selector"], member["symbol"]
                        original = member["original"]
                        current = read_function_bytes(elf, function_name=symbol, address=address).content
                        previous = old[selector]
                        folder = directory / f"{address:08X}"
                        folder.mkdir()
                        (folder / "original.bin").write_bytes(original)
                        (folder / "current.bin").write_bytes(current)
                        bind((folder / "original.bin").relative_to(ROOT).as_posix())
                        bind((folder / "current.bin").relative_to(ROOT).as_posix())
                        assert original == bind(previous["images"]["original"])
                        assert current == bind(previous["images"]["current"]), "Fresh bytes changed from historical baseline; retain and investigate"
                        emitted = [item for item in comparison.ranges if item.section is None and item.address == address]
                        if current == original:
                            assert len(emitted) == 1 and emitted[0].content == current and emitted[0].source == source and emitted[0].symbol == symbol and emitted[0].original_file_start == member["file_start"] and emitted[0].owners == (selector,)
                        save(folder / "metadata-profile.json", {"record": asdict(records[address]), "planned_member": next(m for m in consumer["members"] if m["selector"] == selector), "profile": consumer["profile"], "configured_arguments": commands[source]})
                        bind((folder / "metadata-profile.json").relative_to(ROOT).as_posix())
                        if grouped:
                            actual = next(m for m in report["members"] if m["address"] == address)
                            assert actual["symbol"] == symbol and actual["current_sha256"] == sha(current) and actual["original_sha256"] == sha(original)
                        else:
                            assert report["function"] == symbol and int(report["address"], 16) == address
                            assert report["original_size"] == len(original) and report["current_size"] == len(current)
                            assert report["function_image"] == {"original_sha256": sha(original), "current_sha256": sha(current)}
                        planned = next(m for m in consumer["members"] if m["selector"] == selector)
                        assert planned["progress"] is None or planned["progress"][0] != "exact" or current == original
                        summary["members"].append({"selector": selector, "source": source, "symbol": symbol, "original_size": len(original), "current_size": len(current), "original_sha256": sha(original), "current_sha256": sha(current), "byte_match": current == original, "progress": planned["progress"], "frozen": planned["frozen"], "directory": folder.relative_to(ROOT).as_posix(), "actual_elf": (directory / "actual.elf").relative_to(ROOT).as_posix(), "elf_sha256": sha(elf), "owner_instruction_metrics": None})
                    if grouped:
                        assert report["all_native_bytes_match"] and report["all_data_bytes_match"]
                    guard()
                    save(OUT / "summary.json", summary)
                # Public partial diagnostics are distinct from the complete owner ranges.
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
                    assert public["schema"] == "harness.asm-diff-one/v2" and public["function"] == member["symbol"]
                    assert int(public["address"], 16) == int(member["selector"].split("@")[1], 16)
                    public_elf = Path(public["current_object"]).with_suffix(".linked.o").read_bytes()
                    current = (folder / "current.bin").read_bytes()
                    assert read_function_bytes(public_elf, function_name=member["symbol"], address=int(public["address"], 16)).content == current
                    assert public["function_image"]["current_sha256"] == sha(current)
                    (folder / "public.linked.o").write_bytes(public_elf)
                    bind((folder / "public.linked.o").relative_to(ROOT).as_posix())
                    for key, name in public["outputs"].items():
                        artifact = Path(name)
                        assert artifact.is_absolute() and artifact.is_relative_to(ROOT)
                        if artifact.is_file():
                            saved_artifact = folder / ("public-" + key + "-" + artifact.name)
                            saved_artifact.write_bytes(artifact.read_bytes())
                            bind(saved_artifact.relative_to(ROOT).as_posix())
                    original = (folder / "original.bin").read_bytes()
                    public_original = Path(public["outputs"]["original_bytes"]).read_bytes()
                    assert original.startswith(public_original) and len(public_original) == public["original_size"] and sha(public_original) == public["function_image"]["original_sha256"]
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
                    metrics = {"boundary": "Complete raw images; normalized metrics describe rendered coverage. Omitted words must be zero; no full-word percentage is claimed when words are omitted.", "original_coverage": coverage(dumps["original"], original, address), "current_coverage": coverage(dumps["current"], current, address), "rendered_instruction_count": {"original": len(before), "current": len(after), "matching": matching, "match_percent": round(100 * matching / max(len(before), len(after), 1), 2)}, "first_rendered_mismatch": first_instruction_mismatch(before, after)}
                    save(folder / "complete-rendered-metrics.json", metrics)
                    bind((folder / "complete-rendered-metrics.json").relative_to(ROOT).as_posix())
                    summary["partials"].append({"selector": member["selector"], "public_instruction_count": public["instruction_count"], "public_first_mismatch": public["first_mismatch"], "public_original_size": public["original_size"], "public_current_size": public["current_size"], "complete_metrics": (folder / "complete-rendered-metrics.json").relative_to(ROOT).as_posix()})
                    guard()
                    save(OUT / "summary.json", summary)
                assert len(summary["sources"]) == 219 and len(summary["members"]) == 247 and len(summary["partials"]) == 18
                assert sum(m["original_size"] for m in summary["members"]) == 60312
                assert {m["selector"] for m in summary["members"] if m["frozen"]} == set(closure["frozen_selectors"])
                assert {m["selector"] for m in summary["members"] if m["progress"] is None} == set(closure["tag_free_selectors"])
                summary["program_observations"] = [description for _, _, description in programs.snapshots.values()]
                guard()
                save(OUT / "terminal-input-states.json", states)
        summary.update(status="complete PRE captures; independent acceptance pending", native_sources=219, normal_translation_units=220, member_count=247, complete_original_bytes=60312, guards_and_writer_closed=True, seconds=round(time.monotonic() - started, 3))
        save(OUT / "summary.json", summary)
    except BaseException as error:
        summary.update(status="failed; keep evidence and fix cause", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary.update(cleanup="unconfirmed; stop native work", cleanup_observation=error.observed)
        save(OUT / "summary.json", summary)
        raise


if __name__ == "__main__":
    main()
