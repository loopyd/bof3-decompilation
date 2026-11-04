"""Capture folder03's29 unchanged moved members through native owners."""
from contextlib import closing
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
from harness.build.compilation import configured_command
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes, read_linked_image
from harness.match.images import capture_source_images
from harness.match.members import capture_members

root = Path(__file__).resolve().parents[2]
assert Path.cwd() == root, "Run from repository root"
plan_name = "out/source-refinement/world-folder-03-ready-successor/plan.json"
plan_sha = "397188d6fb3a988e488e8ef6b35acdbaf4289248dfe69915479c9f885c3d33d2"
out = root / "out/source-refinement/world-folder-03-pre"
assert not out.exists()
sha = lambda value: hashlib.sha256(value).hexdigest()
def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
data = (root / plan_name).read_bytes()
assert sha(data) == plan_sha
plan = json.loads(data)
groups = [{**m, "members": [{**m, "record": m["current_record"]}]} for m in plan["selected"]] + plan["untouched_authored_siblings"]
assert len(groups) == 29 and sum(len(g["members"]) for g in groups) == 29 and sum(m["original_size"] for g in groups for m in g["members"]) == 1312
out.mkdir()
(out / "plan.json").write_bytes(data)
summary = {"status": "running", "task": "4.9", "plan_sha256": plan_sha, "sources": [], "members": [], "limits": "Complete29-member/1312B PRE only. No application, foreign native credit, metadata promotion or independent acceptance."}
save(out / "summary.json", summary)
try:
    with acquire_writer(root), InputBatch(root) as batch:
        states = {}
        def bind(name):
            state, body = batch.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
            assert name not in states or state == states[name], name
            states[name] = state
            return body
        assert sha(bind(plan_name)) == plan_sha
        for name, expected in plan["current_input_states"].items():
            body = bind(name)
            assert states[name] == expected, name
            if name in plan["owned_paths"]["existing"]:
                saved = out / "owned-pre" / name
                saved.parent.mkdir(parents=True, exist_ok=True)
                saved.write_bytes(body)
        index = bind(".git/index")
        (out / "index.pre.bin").write_bytes(index)
        summary["runtime_index_state"] = states[".git/index"]
        bind(Path(__file__).resolve().relative_to(root).as_posix())
        sources = [root / unit["source"] for unit in plan["normal_build_units"]]
        assert len(sources) == len(set(sources)) == 34
        with closing(IncludeSnapshot(root, sources[0], forced=tuple(sources[1:]))) as includes:
            for name, expected in includes.describe_inputs().items():
                bind(name)
                assert states[name] == expected, name
            def guard():
                includes.validate()
                for name in list(states):
                    bind(name)
            flags, compilers = load_compiler_configuration(root)
            prepared = []
            for group in groups:
                source = group["source"]
                text = bind(source).decode()
                assert sha(text.encode()) == group["source_sha256"]
                assert [asdict(rec) for rec in parse_function_records(text)] == [m["record"] for m in group["members"]]
                assert asdict(resolve_compiler_settings(source, text, flags, compilers)) == group["effective_settings"]
                command, _ = configured_command(root, root / source, Path(group["configured_arguments"][-1]), allow_grouped=True)
                assert command == group["configured_arguments"]
                captured = capture_members(root, source)
                planned = {m["selector"]: m for m in group["members"]}
                assert captured["target"] == group["target"] and {m["selector"] for m in captured["functions"]} == set(planned) and not captured["data_sections"] and not any(captured["section_placements"].values())
                for member in captured["functions"]:
                    old = planned[member["selector"]]
                    assert member["symbol"] == old["symbol"] and member["file_start"] == old["file_start"] and member["original"] == bind(old["original_path"]) and len(member["original"]) == old["original_size"] and sha(member["original"]) == old["original_sha256"]
                for name, expected in captured["inputs"].items():
                    bind(name)
                    assert (expected is None and states[name] is None) or expected is not None and states[name] is not None and expected["sha256"] == states[name]["sha256"], name
                prepared.append((group, planned))
            guard()
            save(out / "starting-input-states.json", states)
            for group, planned in prepared:
                source = group["source"]
                destination = out / (group["target"].replace("/", "--") + "-" + Path(source).stem)
                destination.mkdir()
                guard()
                comparison = capture_source_images(root, source, destination)
                report = comparison.report
                save(destination / "comparison.json", report)
                assert report["fresh_compilation"] and len(comparison.ranges) == len(planned)
                if len(planned) == 1:
                    assert report["schema"] == "harness.byte-match-one/v1" and report["status"] == "exact_match" and report["byte_match"] and report["exact_match"] and report["size_delta"] == 0 and report["original_size"] == report["current_size"] == next(iter(planned.values()))["original_size"]
                    outputs = report["native_outputs"]
                    linked = [name for name in outputs if name.endswith(".linked.o")]
                else:
                    assert report["status"] == "exact" and report["all_native_bytes_match"] and report["member_count"] == len(planned) and not report["data_sections"]
                    outputs = report["outputs"]
                    linked = [name for name in outputs if name.endswith(".elf")]
                assert len(linked) == 1
                for name, expected in outputs.items():
                    body = bind(name)
                    assert states[name] == expected, name
                    (destination / ("retained-" + Path(name).name)).write_bytes(body)
                elf = bind(linked[0])
                (destination / "actual.elf").write_bytes(elf)
                image = read_linked_image(elf, limit=64 * 1024 * 1024)
                assert not image.relocations and {s.name for s in image.symbols if s.info & 15 == 2 and s.section} == {m["symbol"] for m in planned.values()}
                assert {r.owners[0] for r in comparison.ranges} == set(planned) and all(len(r.owners) == 1 and r.section is None for r in comparison.ranges)
                for emitted in comparison.ranges:
                    member = planned[emitted.owners[0]]
                    original = bind(member["original_path"])
                    actual = read_function_bytes(elf, function_name=member["symbol"], address=member["record"]["address"]).content
                    assert actual == emitted.content == original and emitted.source == source and emitted.address == member["record"]["address"] and emitted.original_file_start == member["file_start"]
                    saved = destination / f"{member['record']['address']:08X}"
                    saved.mkdir()
                    (saved / "original.bin").write_bytes(original)
                    (saved / "current.bin").write_bytes(actual)
                    save(saved / "metadata-profile.json", {"record": member["record"], "effective_settings": group["effective_settings"], "configured_arguments": group["configured_arguments"]})
                    summary["members"].append({"selector": member["selector"], "source": source, "symbol": member["symbol"], "size": len(actual), "original_sha256": sha(original), "current_sha256": sha(actual), "raw_word_count": len(actual) // 4, "saved_at": str(saved.relative_to(root)), "elf_path": str((destination / "actual.elf").relative_to(root)), "elf_sha256": sha(elf), "instruction_count": None, "instruction_metric_boundary": "Diagnostics=False provides no rendered instruction metrics; full raw images and actual ELF functions are authoritative."})
                guard()
                summary["sources"].append({"source": source, "target": group["target"], "member_count": len(planned), "comparison": str((destination / "comparison.json").relative_to(root))})
                save(out / "summary.json", summary)
                print(source, flush=True)
            guard()
            save(out / "terminal-input-states.json", states)
    assert len(summary["sources"]) == 29 and len(summary["members"]) == 29 and sum(m["size"] for m in summary["members"]) == 1312
    summary.update(status="complete PRE captures", member_count=29, complete_original_bytes=1312, guards_and_writer_closed=True)
    save(out / "summary.json", summary)
except BaseException as error:
    summary.update(status="failed", error=f"{type(error).__name__}: {error}")
    if isinstance(error, ProcessCleanupError):
        summary["cleanup"] = "unconfirmed; stop native work"
        summary["cleanup_observation"] = error.observed
    save(out / "summary.json", summary)
    raise
