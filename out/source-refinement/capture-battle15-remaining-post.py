"""Capture the five applied Battle15 metadata groups against their accepted PRE."""

from contextlib import closing
import argparse
import hashlib
import json
import os
from pathlib import Path
import re

from harness.build.compilation import configured_command
from harness.build.dependencies import IncludeSnapshot
from harness.common.inputs import InputBatch
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.functions import parse_function_records
from harness.domain.layout import parse_splat_text
from harness.domain.manifests import load_target_manifests
from harness.domain.sources import compiled_symbol_name
from harness.match.extraction import read_function_bytes, read_linked_image

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "out/source-refinement"
OUT = BASE / "battle15-remaining-metadata-post"
PRE = "out/source-refinement/battle15-remaining-metadata-pre"
TARGET = "emi/battle/battle/15"
LIMIT = 64 * 1024 * 1024
POOL_SHA = "025ec2c0501f9d7c7b444e22d407f1f9eee08a400385752344f619992eeeca9d"


def sha(content):
    return hashlib.sha256(content).hexdigest()


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--pre-summary-sha256", required=True)
    parser.add_argument("--application-report", required=True)
    parser.add_argument("--application-sha256", required=True)
    args = parser.parse_args()
    application_path = Path(args.application_report)
    assert not application_path.is_absolute() and ".." not in application_path.parts
    assert Path.cwd() == ROOT and not OUT.exists()
    OUT.mkdir()
    summary = {"status": "running", "members": [], "limits": "POST for five separately reviewed groups. Current sources must equal accepted PRE proposal hashes. Complete function raw images, metrics and configured profiles must remain identical; whole ELF hashes may differ with debug lines. Splat ASMs are producer outputs, not continuous authored inputs. No source/progress edits, acceptance or Git/index writes."}
    try:
        with InputBatch(ROOT) as inputs:
            states = {}

            def bind(name):
                state, body = inputs.read(ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True)
                assert name not in states or states[name] == state, name
                states[name] = state
                return body

            def guard():
                for name in list(states):
                    bind(name)

            pre_data = bind(PRE + "/summary.json")
            assert sha(pre_data) == args.pre_summary_sha256
            pre = json.loads(pre_data)
            assert pre["status"] == "complete PRE captures; independent acceptance pending" and pre["guards_closed"] and pre["member_count"] == 34 and pre["complete_original_bytes"] == 1632
            previous = {row["selector"]: row for row in pre["members"]}
            previous_sources = {row["source"]: row for row in pre["members"]}
            assert len(previous) == len(previous_sources) == 34
            application = bind(args.application_report)
            assert sha(application) == args.application_sha256
            json.loads(application)
            terminal = json.loads(bind(PRE + "/terminal-input-states.json"))
            for name, expected in terminal.items():
                if name in previous_sources:
                    continue
                assert not name.startswith("out/splat/"), "Generated Splat output cannot become an authored input"
                bind(name)
                assert states[name] == expected, name
            for before in pre["members"]:
                folder = PRE + "/" + before["symbol"]
                original = bind(folder + "/original.bin")
                current = bind(folder + "/current.bin")
                elf = bind(folder + "/linked.o")
                assert sha(original) == before["original_sha256"] and sha(current) == before["report"]["function_image"]["current_sha256"] and sha(elf) == before["elf_sha256"]
                assert read_function_bytes(elf, function_name=before["symbol"], address=before["address"]).content == current
            summary.update(pre_summary_sha256=args.pre_summary_sha256, application_report=args.application_report, application_sha256=args.application_sha256)
            pool_data = bind("out/source-refinement/next-macro-pool-plan.json")
            assert sha(pool_data) == POOL_SHA
            batches = [row for row in json.loads(pool_data)["metadata_followups"]["batches"] if row["id"] in {f"metadata-emi-battle-battle-15-{number:02d}" for number in range(2, 7)}]
            assert [len(row["sources"]) for row in batches] == [8, 8, 8, 8, 2]
            manifest = load_target_manifests(ROOT)[TARGET]
            for name in (f"config/targets/{TARGET}/target.toml", manifest.splat, f"config/targets/{TARGET}/symbols.txt", manifest.binary, "config/splat.yaml", "config/compiler/object-flags.cmake", "config/compiler/variants.json", "tools/python/harness/domain/functions.py", "tools/python/harness/build/translation.py", "tools/python/harness/naming/source_identifiers.py", "tools/python/harness/build/expansion.py", "bin/cc", "bin/harness"):
                bind(name)
            binary = bind(manifest.binary)
            assert sha(binary) == "77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140"
            layout = parse_splat_text(bind(manifest.splat).decode(), manifest.load_address, origin=ROOT / manifest.splat)
            members = []
            for batch in batches:
                number = batch["id"].rsplit("-", 1)[1]
                for name in batch["sources"]:
                    current = bind(name).decode()
                    proposal_name = f"out/source-refinement/metadata-battle15-{number}-comments-draft/files/{name}"
                    proposal = bind(proposal_name).decode()
                    record, = parse_function_records(proposal)
                    assert current == proposal and record.kind == "function"
                    before = previous_sources[name]
                    assert states[name]["sha256"] == states[proposal_name]["sha256"] == before["proposal_sha256"]
                    assert before["proposal"] == proposal_name and before["batch"] == batch["id"]
                    assert re.findall(r"@(status|match|residual)\b[^\n]*", current) == re.findall(r"@(status|match|residual)\b[^\n]*", proposal)
                    assert name in manifest.sources
                    boundary, = [row for row in layout.boundaries if row.virtual_start == record.address]
                    assert boundary.kind == "c" and boundary.source in (None, name)
                    symbol = compiled_symbol_name(ROOT, ROOT / name, record.address, manifest=manifest, layout=layout)
                    assert symbol == record.spelling
                    original = binary[boundary.file_start:boundary.file_end]
                    assert len(original) == boundary.virtual_size and original
                    members.append({"batch": batch["id"], "source": name, "source_sha256": states[name]["sha256"], "proposal": proposal_name, "proposal_sha256": states[proposal_name]["sha256"], "selector": f"{TARGET}@0x{record.address:08X}", "symbol": symbol, "address": record.address, "file_start": boundary.file_start, "file_end": boundary.file_end, "original_size": len(original), "original_sha256": sha(original)})
            assert len(members) == 34 and len({row["source"] for row in members}) == 34 and sum(row["original_size"] for row in members) == 1632
            bind(Path(__file__).resolve().relative_to(ROOT).as_posix())
            assert {row["selector"] for row in members} == set(previous)
            assert {row["batch"] for row in members} == {f"metadata-emi-battle-battle-15-{number:02d}" for number in range(2, 7)}
            summary["reviewed_members"] = members
            save("summary.json", summary)
            sources = [ROOT / row["source"] for row in members]
            environment = dict(os.environ)
            with closing(IncludeSnapshot(ROOT, sources[0], forced=tuple(sources[1:]))) as includes:
                for name, expected in includes.describe_inputs().items():
                    bind(name)
                    assert states[name] == expected, name
                save("starting-input-states.json", states)
                for member in members:
                    guard()
                    includes.validate()
                    symbol = member["symbol"]
                    folder = OUT / symbol
                    folder.mkdir()
                    print(member["selector"], flush=True)
                    result = run_bounded(ROOT, ["bin/harness", "lift", "asm-diff", member["selector"], "--json", "--detail", "full", "--image-evidence"], env=environment, output_limit=LIMIT)
                    save(symbol + ".process.json", result)
                    try:
                        available = json.loads(result["stdout"])
                    except (ValueError, TypeError):
                        available = {}
                    for key, name in available.get("outputs", {}).items():
                        if not isinstance(name, str):
                            continue
                        artifact = Path(name)
                        if artifact.is_absolute() and artifact.is_relative_to(ROOT) and artifact.is_file():
                            (folder / ("available-" + key + "-" + artifact.name)).write_bytes(artifact.read_bytes())
                    assert result["failure"] is None and result["exit_code"] in (0, 1), symbol
                    report = json.loads(result["stdout"])
                    assert report["schema"] == "harness.asm-diff-one/v2" and report["function"] == symbol and int(report["address"], 16) == member["address"]
                    assert report["source"] == str(ROOT / member["source"]) and report["original_size"] == member["original_size"] and report["function_image"]["original_sha256"] == member["original_sha256"]
                    elf_path = Path(report["current_object"]).with_suffix(".linked.o")
                    assert elf_path.is_absolute() and elf_path.is_relative_to(ROOT)
                    elf = bind(elf_path.relative_to(ROOT).as_posix())
                    image = read_linked_image(elf, limit=LIMIT)
                    defined = [row for row in image.symbols if row.info & 15 == 2 and row.section]
                    assert len(defined) == 1 and defined[0].name == symbol and not image.relocations
                    current = read_function_bytes(elf, function_name=symbol, address=member["address"]).content
                    original = binary[member["file_start"]:member["file_end"]]
                    assert len(current) == report["current_size"] and sha(current) == report["function_image"]["current_sha256"]
                    for suffix, body in (("linked.o", elf), ("current.bin", current), ("original.bin", original)):
                        (folder / suffix).write_bytes(body)
                    for key, name in report["outputs"].items():
                        artifact = Path(name)
                        assert artifact.is_absolute() and artifact.is_relative_to(ROOT)
                        if artifact.is_file():
                            name = artifact.relative_to(ROOT).as_posix()
                            body = artifact.read_bytes() if name.startswith("out/splat/") else bind(name)
                            (folder / (key + "-" + artifact.name)).write_bytes(body)
                    arguments, _ = configured_command(ROOT, ROOT / member["source"], Path(report["current_object"]))
                    before = previous[member["selector"]]
                    assert arguments == before["configured_arguments"], "Configured source profile changed"
                    assert result["exit_code"] == before["actual_comparison_exit_code"]
                    for key in ("original_size", "current_size", "size_delta", "instruction_count", "first_mismatch", "byte_match", "exact_match", "function_image"):
                        assert report[key] == before["report"][key], (symbol, key)
                    assert current == bind(PRE + f"/{symbol}/current.bin") and original == bind(PRE + f"/{symbol}/original.bin"), "Complete PRE/POST raw images differ"
                    summary["members"].append({**member, "report": report, "actual_comparison_exit_code": result["exit_code"], "elf_sha256": sha(elf), "configured_arguments": arguments, "complete_pre_images_and_metrics_equal": True})
                    save("summary.json", summary)
                    assert report["byte_match"] and current == original, "Existing exact claim differs; keep applied comments and repair cause"
                    guard()
                    includes.validate()
                save("terminal-input-states.json", states)
        summary.update(status="complete POST captures; independent acceptance pending", guards_closed=True, member_count=34, complete_original_bytes=1632)
        save("summary.json", summary)
    except BaseException as error:
        summary.update(status="failed; keep evidence and fix cause", error=f"{type(error).__name__}: {error}")
        if isinstance(error, ProcessCleanupError):
            summary["cleanup"] = "unconfirmed; stop native work"
            summary["cleanup_observation"] = error.observed
        save("summary.json", summary)
        raise


if __name__ == "__main__":
    main()
