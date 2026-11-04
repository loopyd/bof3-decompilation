"""Deliver the five C05 payloads serially after a static admission preflight."""

import hashlib
import json
from pathlib import Path

from harness.build.compilation import configured_command
from harness.common.inputs import InputBatch
from harness.common.process import run_bounded
from harness.domain.functions import parse_function_records
from harness.domain.manifests import _parse_manifest
from harness.emi.catalog_verify import resolve_shipped_entry
from harness.match.extraction import read_function_bytes
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/payload-c05"
PREFLIGHT = "out/source-refinement/payload-c05-preflight-v2.json"
PREFLIGHT_SHA = "9c0287354c75074f42ac9149ed3a352b023ee54a97b734001e4eee68dd619a4b"
JOBS = [
    ("emi/bmagic/magic113/03", "out/payloads/magic113-03"),
    ("emi/bmagic/magic124/03", "out/payloads/magic124-03"),
    ("emi/bmagic/magic131/03", "out/payloads/magic131-03"),
    ("emi/bmagic/magic137/03", "out/payloads/magic137-03"),
    ("emi/bmagic/magic138/03", "out/payloads/magic138-03"),
]
LIMIT = 64 * 1024 * 1024


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def run(inputs, attempt):
    require(Path.cwd() == ROOT, "run from the repository root")
    preflight_bytes = (ROOT / PREFLIGHT).read_bytes()
    require(sha(preflight_bytes) == PREFLIGHT_SHA, "C05 preflight changed")
    preflight = json.loads(preflight_bytes)
    require(
        preflight["schema"] == "bof3.payload-c05-preflight/v2"
        and len(preflight["inputs"]) == 197
        and not preflight["blockers"]
        and preflight["summary"]["eligible_members"] == 7
        and preflight["summary"]["eligible_function_bytes"] == 304,
        "C05 preflight scope or structural admission differs",
    )
    require(
        all(not (ROOT / output).exists() for _, output in JOBS),
        "five producer output directories must all be fresh",
    )
    states = {}

    def bind(name, expected=None, *, absent=False):
        observed, content = inputs.read(
            ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True
        )
        require(
            observed is None if absent else observed is not None,
            f"unexpected input presence: {name}",
        )
        if expected is not None:
            require(observed["sha256"] == expected, f"input hash differs: {name}")
        if name in states:
            require(observed == states[name], f"input/output state drift: {name}")
        states[name] = observed
        return content

    for name, pin in preflight["inputs"].items():
        content = bind(name, None if pin is None else pin["sha256"], absent=pin is None)
        require(
            states[name] == preflight["input_states"][name],
            f"preflight input full state differs: {name}",
        )
        if pin is not None:
            require(
                len(content) == pin["size"], f"preflight input size differs: {name}"
            )
    bind(PREFLIGHT, PREFLIGHT_SHA)
    runner = Path(__file__).relative_to(ROOT).as_posix()
    bind(runner)
    index = bind(".git/index")
    OUT.mkdir(exist_ok=False)
    attempt["created_output"] = True
    (OUT / "preflight.json").write_bytes(preflight_bytes)
    (OUT / "index").write_bytes(index)
    save("starting-input-states.json", states)
    summary = {
        "status": "running",
        "scope": "Five new canonical C05 payloads; current structural admission precedes fresh native acceptance.",
        "preflight_sha256": PREFLIGHT_SHA,
        "runner": runner,
        "runner_sha256": states[runner]["sha256"],
        "commands": [],
        "targets": [],
        "limit": "No replicas are declared for these five targets. Independent acceptance and remaining campaign work stay parent-owned.",
    }
    save("summary.json", summary)
    rows = {row["target"]: row for row in preflight["targets"]}
    manifests = {}
    members = {}
    for target, _ in JOBS:
        row = rows[target]
        manifest = _parse_manifest(
            ROOT / row["manifest"], bind(row["manifest"], row["manifest_sha256"])
        )
        claims = list(manifest.sources + manifest.support_sources)
        require(
            set(claims) == {source["source"] for source in row["sources"]}
            and len(claims) == row["claim_count"]
            and manifest.binary == row["binary"]
            and manifest.disc_id == row["disc_id"],
            f"current manifest claim closure differs: {target}",
        )
        selected = []
        for source in row["sources"]:
            name = source["source"]
            content = bind(name, source["source_sha256"])
            arguments, _ = configured_command(
                ROOT,
                ROOT / name,
                ROOT / "out/source-refinement/c05-unused.o",
                allow_grouped=True,
            )
            require(
                arguments == source["configured_profile"]["ordered_arguments"],
                f"configured argument profile differs: {name}",
            )
            records = parse_function_records(content.decode())
            require(
                len(records) == source["function_records"],
                f"strict metadata closure differs: {name}",
            )
            if name == manifest.psyq_source:
                require(
                    not records and source["psyq_source"],
                    "generated PsyQ support became a contribution",
                )
                continue
            if records:
                captured = capture_members(ROOT, name)
                require(
                    captured["target"] == target and not captured["data_sections"],
                    "unexpected native owner or initialized data",
                )
                for path, state in captured["inputs"].items():
                    bind(
                        path,
                        None if state is None else state["sha256"],
                        absent=state is None,
                    )
                selected.extend(
                    {**function, "source": name} for function in captured["functions"]
                )
        expected = {
            (member["address"], member["symbol"]): (source["source"], member)
            for source in row["sources"]
            for member in source["eligible_members"]
        }
        require(len(selected) == row["eligible_member_count"], "member count differs")
        for function in selected:
            owner, member = expected.pop((function["address"], function["symbol"]))
            require(
                function["source"] == owner
                and function["selector"] == member["selector"]
                and function["file_start"] == member["file_start"]
                and len(function["original"]) == member["original_size"]
                and sha(function["original"]) == member["original_sha256"],
                "complete current member identity/range/hash differs",
            )
        require(not expected, "incomplete current member closure")
        manifests[target] = manifest
        members[target] = selected

    def verify_pins():
        for name, expected in states.items():
            bind(
                name,
                None if expected is None else expected["sha256"],
                absent=expected is None,
            )

    def verify_report(target, report_name):
        report_bytes = bind(report_name)
        report = json.loads(report_bytes)
        output_directory = Path(report_name).parent
        manifest = manifests[target]
        selected = members[target]
        compiled_size = sum(len(member["original"]) for member in selected)
        require(
            report["schema"] == "harness.build-payload/v1"
            and report["target"] == target
            and report["kind"] == "hybrid"
            and report["whole_payload_equal"] is True
            and report["compiled_bytes"] == compiled_size
            and len(report["compiled"]) == len(selected)
            and report["output"] == (output_directory / "payload.bin").as_posix(),
            f"payload report identity/contribution differs: {target}",
        )
        entry = resolve_shipped_entry(ROOT, manifest.disc_id)
        require(
            entry["index"] == rows[target]["shipped_entry"]["index"]
            and entry["load_address"] == manifest.load_address
            and entry["payload_kind"] == "ram",
            "shipped entry identity/load differs",
        )
        for key in ("manifest_path", "archive_path", "payload_path"):
            bind(Path(entry[key]).relative_to(ROOT).as_posix())
        original_name = Path(entry["payload_path"]).relative_to(ROOT).as_posix()
        original = bind(original_name, entry["sha256"])
        normalized = bind(manifest.binary)
        payload = bind(report["output"], report["sha256"])
        require(
            report["original"] == original_name
            and payload == normalized == original
            and len(payload) == report["size"] == entry["size"]
            and report["retained_bytes"] == len(payload) - compiled_size,
            f"actual complete payload equality/totals differ: {target}",
        )
        expected_members = {
            (member["address"], member["symbol"]): member for member in selected
        }
        cursor = 0
        retained = []
        for interval in report["compiled"]:
            member = expected_members.pop((interval["address"], interval["symbol"]))
            start = member["file_start"]
            end = start + len(member["original"])
            require(
                interval["start"] == interval["normalized_start"] == start
                and interval["end"] == end
                and interval["source"] == member["source"]
                and interval["owners"] == [member["selector"]]
                and interval["section"] is None
                and payload[start:end] == member["original"]
                and interval["sha256"] == sha(member["original"])
                and cursor <= start,
                "compiled actual placement/ownership/hash differs",
            )
            if cursor < start:
                retained.append({"start": cursor, "end": start})
            cursor = end
        if cursor < len(payload):
            retained.append({"start": cursor, "end": len(payload)})
        require(
            not expected_members and report["retained"] == retained,
            "full retained complement differs",
        )
        archive = report["archive"]
        require(
            archive["index"] == entry["index"]
            and archive["original"] == entry["archive_path"]
            and archive["whole_archive_equal"] is True
            and archive["entry_equal"] is True
            and archive["output"] == (output_directory / "archive.EMI").as_posix(),
            "archive report identity/equality differs",
        )
        original_archive = bind(
            Path(entry["archive_path"]).relative_to(ROOT).as_posix()
        )
        archive_bytes = bind(archive["output"], archive["sha256"])
        extracted = bind((output_directory / "entry.bin").as_posix())
        require(
            archive_bytes == original_archive and extracted == payload,
            f"actual complete archive or extracted selected entry differs: {target}",
        )
        comparisons = report["source_comparisons"]
        require(
            len(comparisons) == len({member["source"] for member in selected})
            and {row["source"] for row in comparisons}
            == {member["source"] for member in selected},
            "actual native source contribution closure differs",
        )
        for row in comparisons:
            comparison = row["comparison"]
            function = next(
                member for member in selected if member["source"] == row["source"]
            )
            require(
                row["compiled_range_count"] == 1
                and comparison["fresh_compilation"] is True
                and comparison["byte_match"] is True
                and comparison["original_size"]
                == comparison["current_size"]
                == len(function["original"]),
                "fresh complete native function evidence differs",
            )
            require(
                comparison["exact_match"] is True
                and comparison["function"] == function["symbol"]
                and int(comparison["address"], 16) == function["address"]
                and comparison["function_image"]["original_sha256"]
                == sha(function["original"])
                and comparison["function_image"]["current_sha256"]
                == sha(function["original"]),
                "native function identity/full image proof differs",
            )
            for key in ("native_inputs", "include_inputs"):
                for name, expected in comparison[key].items():
                    bind(
                        name,
                        None if expected is None else expected["sha256"],
                        absent=expected is None,
                    )
                    require(
                        states[name] == expected, "recorded native input state differs"
                    )
            for name, expected in comparison["native_outputs"].items():
                bind(name, expected["sha256"])
                require(
                    states[name] == expected,
                    "recorded native output state is no longer current",
                )
            linked = Path(comparison["current_object"]).with_suffix(".linked.o")
            name = linked.relative_to(ROOT).as_posix()
            elf = bind(name, comparison["native_outputs"][name]["sha256"])
            require(
                states[name] == comparison["native_outputs"][name]
                and read_function_bytes(
                    elf, function_name=function["symbol"], address=function["address"]
                ).content
                == function["original"],
                "actual linked ELF/function differs from native output capture",
            )
            saved_elf = (
                OUT / f"{target.replace('/', '_')}-{function['address']:08X}.linked.o"
            )
            saved_elf.write_bytes(elf)
            (saved_elf.with_suffix(".original.bin")).write_bytes(function["original"])
            (saved_elf.with_suffix(".current.bin")).write_bytes(
                read_function_bytes(
                    elf, function_name=function["symbol"], address=function["address"]
                ).content
            )
        for name, expected in report["inputs"].items():
            bind(name, expected, absent=expected is None)
        tool = archive["tool"]
        observed, _ = inputs.read(
            ROOT / tool, max_bytes=LIMIT, include_metadata=True, single_link=False
        )
        require(
            observed == archive["tool_state"]
            and observed["sha256"] == archive["tool_sha256"],
            "recorded read-only EMI tool observation is no longer current",
        )
        return {
            "target": target,
            "report": report_name,
            "report_sha256": sha(report_bytes),
            "claims": list(manifest.sources + manifest.support_sources),
            "metadata_free_support": [
                source["source"]
                for source in rows[target]["sources"]
                if not source["function_records"]
            ],
            "contributing_sources": sorted({member["source"] for member in selected}),
            "function_members": [member["selector"] for member in selected],
            "compiled_bytes": compiled_size,
            "retained_bytes": report["retained_bytes"],
            "payload_size": len(payload),
            "payload_sha256": sha(payload),
            "payload_normalized_shipped_equal": True,
            "archive_size": len(archive_bytes),
            "archive_sha256": sha(archive_bytes),
            "whole_archive_equal": True,
            "entry_index": entry["index"],
            "entry_equal": True,
        }

    for target, output in JOBS:
        verify_pins()
        label = target.replace("/", "_")
        arguments = [
            "bin/harness",
            "build",
            "payload",
            target,
            "--output-dir",
            output,
            "--json",
        ]
        print(" ".join(arguments), flush=True)
        with (
            (OUT / f"{label}.stdout").open("wb") as stdout,
            (OUT / f"{label}.stderr").open("wb") as stderr,
        ):

            def retain(stream, chunk):
                (stdout if stream == "stdout" else stderr).write(chunk)

            result = run_bounded(ROOT, arguments, output_limit=LIMIT, on_output=retain)
        save(f"{label}.process.json", result)
        summary["commands"].append(
            {
                "argv": arguments,
                "exit_code": result["exit_code"],
                "failure": result["failure"],
            }
        )
        save("summary.json", summary)
        require(
            not result["failure"] and result["exit_code"] == 0,
            f"payload producer failed: {target}; retained stdout/stderr/process and partial output remain",
        )
        report_name = f"{output}/report.json"
        require(
            json.loads((ROOT / report_name).read_bytes())
            == json.loads(result["stdout"]),
            "published/stdout payload report differs",
        )
        summary["targets"].append(verify_report(target, report_name))
        verify_pins()
        save("summary.json", summary)
    verify_pins()
    save("terminal-input-states.json", states)
    require(
        len(summary["targets"]) == 5
        and sum(row["compiled_bytes"] for row in summary["targets"]) == 304,
        "incomplete five-target canonical C05 result",
    )
    summary.update(
        status="awaiting guard cleanup",
        new_payload_jobs=5,
        canonical_targets=5,
        compiled_bytes=304,
        preserved_raw_git_index=True,
        no_external_writer_lease=True,
        replica_archive_delivery=False,
        full_campaign_complete=False,
    )
    return summary


def main():
    attempt = {"created_output": False}
    try:
        with InputBatch(ROOT) as inputs:
            summary = run(inputs, attempt)
        summary.update(status="complete", continuous_input_guards_closed=True)
        save("summary.json", summary)
    except BaseException as error:
        if attempt["created_output"] and (OUT / "summary.json").is_file():
            summary = json.loads((OUT / "summary.json").read_bytes())
            summary.update(status="failed", error=f"{type(error).__name__}: {error}")
            save("summary.json", summary)
        raise


if __name__ == "__main__":
    main()
