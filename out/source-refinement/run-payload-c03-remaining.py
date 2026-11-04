"""Deliver four remaining canonical C03 payloads and verify current MAGIC057 reuse."""

import hashlib
import json
from pathlib import Path

from harness.common.inputs import InputBatch
from harness.common.process import run_bounded
from harness.domain.functions import parse_function_records
from harness.domain.manifests import _parse_manifest
from harness.emi.catalog_verify import resolve_shipped_entry
from harness.match.extraction import read_function_bytes
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/payload-c03-remaining"
PREFLIGHT = "out/source-refinement/payload-c03-preflight.json"
PREFLIGHT_SHA = "319278f24ef76137a56d478280914f73581b3b01e2dcb9c4858ee9ff0e77fe43"
REPAIRED_SPLAT = "config/targets/emi/bmagic/magic057/03/splat.yaml"
REPAIRED_SHA = "1b79c814f644ca303882f19a6c338cd74f7edb0a47a98ef1329823b14f0a0466"
REUSE_TARGET = "emi/bmagic/magic057/03"
REUSE_REPORT = "out/payloads/magic057-tail-boundary/report.json"
JOBS = [
    (f"emi/bmagic/magic{number}/03", f"out/payloads/magic{number}-03")
    for number in ("039", "046", "052", "056")
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
    require(sha(preflight_bytes) == PREFLIGHT_SHA, "historical C03 preflight changed")
    preflight = json.loads(preflight_bytes)
    require(len(preflight["inputs"]) == 126, "historical126input closure changed")
    require(
        all(not (ROOT / output).exists() for _, output in JOBS),
        "four producer output directories must all be fresh",
    )
    require(
        (ROOT / REUSE_REPORT).is_file(),
        "accepted MAGIC057 payload report is required before starting four new jobs",
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
        if name == REPAIRED_SPLAT:
            bind(name, REPAIRED_SHA)
        else:
            content = bind(
                name, None if pin is None else pin["sha256"], absent=pin is None
            )
            if pin is not None:
                require(
                    len(content) == pin["size"],
                    f"historical input size differs: {name}",
                )
    bind(PREFLIGHT, PREFLIGHT_SHA)
    index = bind(".git/index")
    OUT.mkdir(exist_ok=False)
    attempt["created_output"] = True
    (OUT / "historical-preflight.json").write_bytes(preflight_bytes)
    (OUT / "index").write_bytes(index)
    save("starting-input-states.json", states)
    summary = {
        "status": "running",
        "scope": "Four new canonical C03 payloads plus current MAGIC057 task6.8 reuse.",
        "historical_preflight_sha256": PREFLIGHT_SHA,
        "approved_historical_delta": {REPAIRED_SPLAT: REPAIRED_SHA},
        "commands": [],
        "targets": [],
        "limit": "MAGIC046's declared MAGIC047 replica archive is not repacked here. Independent acceptance and remaining campaign work stay parent-owned.",
    }
    save("summary.json", summary)
    rows = {row["target"]: row for row in preflight["targets"]}
    manifests = {}
    members = {}
    for target in [REUSE_TARGET, *[target for target, _ in JOBS]]:
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
                selected.extend(
                    {**function, "source": name} for function in captured["functions"]
                )
        require(
            len(selected) == (2 if target == REUSE_TARGET else 1)
            and sum(len(function["original"]) for function in selected)
            == (
                64
                if target == REUSE_TARGET
                else row["eligible_original_function_bytes"]
            ),
            f"current complete member closure differs: {target}",
        )
        manifests[target] = manifest
        members[target] = selected

    def verify_pins():
        for name, expected in states.items():
            bind(
                name,
                None if expected is None else expected["sha256"],
                absent=expected is None,
            )

    def verify_report(target, report_name, *, reused):
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
            entry["index"] == 3
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
            archive["index"] == 3
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
            f"actual complete archive or extracted entry3 differs: {target}",
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
            if saved_elf.exists():
                require(
                    saved_elf.read_bytes() == elf, "previously saved reused ELF changed"
                )
            else:
                saved_elf.write_bytes(elf)
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
        if target == REUSE_TARGET:
            require(
                report["size"] == 3807
                and compiled_size == 64
                and report["retained_bytes"] == 3743
                and payload[3804:3807].hex() == "0a0e0e"
                and {"start": 3804, "end": 3807} in retained,
                "reused MAGIC057 terminal proof differs",
            )
        return {
            "target": target,
            "reused": reused,
            "report": report_name,
            "report_sha256": sha(report_bytes),
            "claims": list(manifest.sources + manifest.support_sources),
            "metadata_free_support": manifest.psyq_source,
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
            "entry_index": 3,
            "entry_equal": True,
        }

    summary["targets"].append(verify_report(REUSE_TARGET, REUSE_REPORT, reused=True))
    verify_pins()
    save("summary.json", summary)
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
        summary["targets"].append(verify_report(target, report_name, reused=False))
        verify_pins()
        save("summary.json", summary)
    # Recheck reuse after all four producers; its artifacts must remain current.
    verify_report(REUSE_TARGET, REUSE_REPORT, reused=True)
    verify_pins()
    save("terminal-input-states.json", states)
    require(
        len(summary["targets"]) == 5
        and sum(row["compiled_bytes"] for row in summary["targets"]) == 200,
        "incomplete five-target canonical C03 result",
    )
    summary.update(
        status="awaiting guard cleanup",
        new_payload_jobs=4,
        reused_payload_jobs=1,
        canonical_targets=5,
        compiled_bytes=200,
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
