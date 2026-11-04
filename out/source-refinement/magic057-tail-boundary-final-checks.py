"""Run the finite MAGIC057 terminal-boundary concern and payload checks once."""

from contextlib import ExitStack, redirect_stderr, redirect_stdout
import hashlib
import json
from pathlib import Path

from harness.commands import compile_commands
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import run_bounded
from harness.match.extraction import read_function_bytes

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/magic057-tail-boundary-final-checks"
PAYLOAD = "out/payloads/magic057-tail-boundary"
PLAN = "out/source-refinement/magic057-tail-boundary-draft/report.json"
PLAN_SHA = "0e80abf1002451654852420a06bc0f869861096cf190d0859c403327309a79f2"
TARGET = "emi/bmagic/magic057/03"
SPLAT = f"config/targets/{TARGET}/splat.yaml"
SPLAT_SHA = "1b79c814f644ca303882f19a6c338cd74f7edb0a47a98ef1329823b14f0a0466"
SELECTORS = [f"{TARGET}@0x801EFABC", f"{TARGET}@0x801EF81C"]
LIMIT = 64 * 1024 * 1024


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def read(batch, name):
    return batch.read(
        ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True
    )


def check(inputs, asset_stack, attempt):
    require(Path.cwd() == ROOT, "run from the repository root")
    plan_bytes = (ROOT / PLAN).read_bytes()
    require(sha(plan_bytes) == PLAN_SHA, "reviewed MAGIC057 draft changed")
    plan = json.loads(plan_bytes)
    require(
        plan["target"] == TARGET and len(plan["inputs"]) == 143,
        "reviewed input closure changed",
    )
    require(not (ROOT / PAYLOAD).exists(), "payload output must be fresh")
    pins = dict(plan["inputs"])
    pins[SPLAT] = plan["evidence_files"][plan["draft_config"]]
    require(pins[SPLAT]["sha256"] == SPLAT_SHA, "approved Splat hash changed")
    asset = plan["tail"]["future_asset"]
    states = {}
    asset_inputs = asset_stack.enter_context(InputBatch(ROOT))
    for name, expected in pins.items():
        observed, content = read(asset_inputs if name == asset else inputs, name)
        require(
            observed is None
            if expected is None
            else observed is not None
            and observed["sha256"] == expected["sha256"]
            and len(content) == expected["size"],
            f"starting input differs: {name}",
        )
        states[name] = observed
    index_state, index = read(inputs, ".git/index")
    require(index_state is not None, "raw index is absent")
    states[".git/index"] = index_state
    plan_state, _ = read(inputs, PLAN)
    require(
        plan_state is not None and plan_state["sha256"] == PLAN_SHA,
        "draft report drifted",
    )
    states[PLAN] = plan_state
    OUT.mkdir(exist_ok=False)
    attempt["created_output"] = True
    (OUT / "plan.json").write_bytes(plan_bytes)
    (OUT / "index").write_bytes(index)
    save("starting-input-states.json", states)
    state = {
        "status": "running",
        "scope": "MAGIC057 task6.8 only",
        "commands": [],
        "plan_sha256": PLAN_SHA,
    }
    save("summary.json", state)

    def verify_pins(*, splat_created_asset=False):
        require(sha((ROOT / PLAN).read_bytes()) == PLAN_SHA, "draft report changed")
        for name, expected in states.items():
            observed, content = read(asset_inputs if name == asset else inputs, name)
            if name == asset and splat_created_asset:
                require(
                    expected is None
                    and observed is not None
                    and len(content) == 3
                    and content.hex() == "0a0e0e"
                    and observed["sha256"] == plan["tail"]["future_expected_sha256"],
                    "Splat did not create precisely the reviewed3B tail asset",
                )
                states[name] = observed
            else:
                require(observed == expected, f"input/index drift: {name}")

    def run(label, arguments):
        nonlocal asset_inputs
        verify_pins()
        if label == "source-splat":
            # Only this output changes phase; all other input watches stay live.
            asset_stack.close()
        print(" ".join(arguments), flush=True)
        with (
            (OUT / f"{label}.stdout").open("wb") as stdout,
            (OUT / f"{label}.stderr").open("wb") as stderr,
        ):

            def retain(stream, chunk):
                (stdout if stream == "stdout" else stderr).write(chunk)

            result = run_bounded(ROOT, arguments, output_limit=LIMIT, on_output=retain)
        save(f"{label}.process.json", result)
        state["commands"].append(
            {
                "label": label,
                "argv": arguments,
                "exit_code": result["exit_code"],
                "failure": result["failure"],
            }
        )
        save("summary.json", state)
        require(
            not result["failure"] and result["exit_code"] == 0,
            f"{label} failed; inspect retained process/output",
        )
        if label == "source-splat":
            asset_inputs = asset_stack.enter_context(InputBatch(ROOT))
        verify_pins(splat_created_asset=label == "source-splat")
        save("current-input-states.json", states)
        return result["stdout"]

    try:
        metadata = json.loads(
            run(
                "metadata-check",
                [
                    "bin/harness",
                    "source",
                    "metadata",
                    "check",
                    TARGET,
                    *SELECTORS,
                    "--json",
                ],
            )
        )
        require(
            metadata["passed"]
            and not metadata["incomplete"]
            and not metadata["truncated"]
            and not metadata["findings"]
            and metadata["total"] == len(metadata["records"]) == 2
            and {row["selector"].lower() for row in metadata["records"]}
            == {value.lower() for value in SELECTORS},
            "metadata selected closure differs",
        )
        for row in metadata["records"]:
            measurement = row["measurement"]
            require(
                not row["findings"]
                and measurement["state"] == "live"
                and measurement["status"] == "exact"
                and measurement["original_size"] == measurement["current_size"] == 32
                and measurement["match_percent"] == 100.0,
                "metadata is not complete32B live exact",
            )
        run(
            "source-validation",
            [
                "bin/harness",
                "source",
                "validate",
                "--out",
                str(OUT / "source-validation.json"),
            ],
        )
        run("symbol-check", ["bin/harness", "source", "symbols", "check", TARGET])
        run("source-splat", ["bin/harness", "source", "splat", TARGET])
        run("normal-build", ["bin/harness", "build", TARGET])
        verify_pins()
        with (
            (OUT / "compile-commands.stdout").open("w") as stdout,
            (OUT / "compile-commands.stderr").open("w") as stderr,
        ):
            with acquire_writer(ROOT):
                with redirect_stdout(stdout), redirect_stderr(stderr):
                    code = compile_commands.main([])
                require(code == 0, "compile_commands.main([]) failed")
                verify_pins()
        state["commands"].append(
            {
                "label": "compile-commands",
                "call": "compile_commands.main([])",
                "exit_code": code,
                "writer_lease_closed": True,
            }
        )
        save("summary.json", state)
        generated = [
            "build/cmake/build.ninja",
            "build/cmake/bof3-inventory.json",
            "compile_commands.json",
        ]
        texts = {name: (ROOT / name).read_text() for name in generated}
        inventory = json.loads(texts[generated[1]])
        database = json.loads(texts[generated[2]])
        files = [entry["file"] for entry in database]
        for row in plan["profiles"]["all_target_sources"]:
            source = row["source"]
            require(
                source in texts[generated[0]]
                and source in inventory["sources"]
                and inventory["sources"][source] is False
                and inventory["profiles"][source]
                == {"compiler_id": None, "flags": row["flags"]}
                and files.count(str(ROOT / source)) == 1,
                f"target source/profile discovery differs: {source}",
            )
        save(
            "discovery.json",
            {
                "target_sources": [
                    row["source"] for row in plan["profiles"]["all_target_sources"]
                ],
                "generated_sha256": {
                    name: sha((ROOT / name).read_bytes()) for name in generated
                },
                "compilation_database_entries": len(database),
                "tail_asset": states[asset],
            },
        )
        run(
            "target-analysis",
            ["bin/harness", "analysis", "rz-project", "analyze", TARGET],
        )
        run("global-index", ["bin/harness", "analysis", "index"])
        readiness = json.loads(
            run(
                "target-readiness",
                ["bin/harness", "analysis", "readiness", TARGET, "--detail", "full"],
            )
        )
        require(
            readiness["ready"]
            and readiness["index_ready"]
            and readiness["stale_facts"]["count"] == 0,
            "analysis readiness is stale",
        )
        report = json.loads(
            run(
                "build-payload",
                [
                    "bin/harness",
                    "build",
                    "payload",
                    TARGET,
                    "--output-dir",
                    PAYLOAD,
                    "--json",
                ],
            )
        )
        report_path = ROOT / PAYLOAD / "report.json"
        require(
            json.loads(report_path.read_bytes()) == report,
            "published/stdout payload reports differ",
        )
        require(
            report["schema"] == "harness.build-payload/v1"
            and report["target"] == TARGET
            and report["kind"] == "hybrid"
            and report["size"] == 3807
            and report["compiled_bytes"] == 64
            and report["retained_bytes"] == 3743
            and report["whole_payload_equal"] is True
            and report["output"] == f"{PAYLOAD}/payload.bin",
            "payload report totals/identity differ",
        )
        with InputBatch(ROOT) as outputs:
            _, payload = read(outputs, report["output"])
            _, normalized = read(outputs, "out/binaries/emi/bmagic/magic057/03.bin")
            original_name = (
                Path(plan["shipped_identity"]["payload_path"])
                .relative_to(ROOT)
                .as_posix()
            )
            _, original = read(outputs, original_name)
            require(
                report["original"] == original_name
                and len(payload) == 3807
                and payload == normalized == original
                and sha(payload)
                == report["sha256"]
                == plan["shipped_identity"]["sha256"]
                and payload[3804:3807].hex() == "0a0e0e",
                "actual payload/normalized/shipped bytes differ",
            )
            compiled = report["compiled"]
            retained = report["retained"]
            require(
                [(row["start"], row["end"]) for row in compiled]
                == [(3100, 3132), (3772, 3804)]
                and retained
                == [
                    {"start": 0, "end": 3100},
                    {"start": 3132, "end": 3772},
                    {"start": 3804, "end": 3807},
                ],
                "full compiled/retained interval reconciliation differs",
            )
            for row in compiled:
                selector = f"{TARGET}@0x{row['address']:08X}"
                require(
                    selector in SELECTORS
                    and row["owners"] == [selector]
                    and row["section"] is None
                    and row["symbol"] is not None
                    and row["normalized_start"]
                    == row["start"]
                    == row["address"] - plan["shipped_identity"]["load_address"]
                    and sha(payload[row["start"] : row["end"]]) == row["sha256"],
                    "compiled placement/ownership/hash differs",
                )
            archive = report["archive"]
            require(
                archive["index"] == 3
                and archive["entry_equal"] is True
                and archive["whole_archive_equal"] is True
                and archive["original"] == plan["shipped_identity"]["archive_path"]
                and archive["output"] == f"{PAYLOAD}/archive.EMI",
                "archive report identity/equality differs",
            )
            _, rebuilt_archive = read(outputs, archive["output"])
            _, original_archive = read(
                outputs, Path(archive["original"]).relative_to(ROOT).as_posix()
            )
            _, entry = read(outputs, f"{PAYLOAD}/entry.bin")
            require(
                len(rebuilt_archive) == len(original_archive) == 116736
                and rebuilt_archive == original_archive
                and sha(rebuilt_archive)
                == archive["sha256"]
                == plan["shipped_identity"]["archive_sha256"]
                and entry == payload,
                "actual full archive or extracted entry3 differs",
            )
            comparisons = report["source_comparisons"]
            require(
                len(comparisons) == 2
                and {row["source"] for row in comparisons}
                == {
                    row["source"]
                    for row in plan["profiles"]["all_target_sources"]
                    if row["strict_records"]
                },
                "payload native source closure differs",
            )
            for row in comparisons:
                comparison = row["comparison"]
                require(
                    row["compiled_range_count"] == 1
                    and comparison["fresh_compilation"] is True
                    and comparison["byte_match"]
                    and comparison["original_size"] == comparison["current_size"] == 32,
                    "payload omitted fresh exact32B source evidence",
                )
                linked = Path(comparison["current_object"]).with_suffix(".linked.o")
                name = linked.relative_to(ROOT).as_posix()
                observed, elf = read(outputs, name)
                require(
                    observed == comparison["native_outputs"][name],
                    "actual payload ELF differs from reported native state",
                )
                address = int(comparison["address"], 16)
                current = read_function_bytes(
                    elf, function_name=comparison["function"], address=address
                ).content
                start = address - plan["shipped_identity"]["load_address"]
                require(
                    current == payload[start : start + 32] and len(current) == 32,
                    "actual payload ELF member differs",
                )
                (OUT / f"{address:08X}-payload.linked.o").write_bytes(elf)
            for name, expected in report["inputs"].items():
                observed, _ = read(outputs, name)
                require(
                    (None if observed is None else observed["sha256"]) == expected,
                    f"payload recorded input drift: {name}",
                )
        verify_pins()
        save(
            "payload-verification.json",
            {
                "report": str(report_path.relative_to(ROOT)),
                "report_sha256": sha(report_path.read_bytes()),
                "size": 3807,
                "compiled_bytes": 64,
                "retained_bytes": 3743,
                "payload_normalized_original_equal": True,
                "archive_size": 116736,
                "whole_archive_equal": True,
                "entry_index": 3,
                "entry_equal": True,
                "tail": {
                    "start": 3804,
                    "end": 3807,
                    "hex": "0a0e0e",
                    "compiled": False,
                },
                "compiled": compiled,
                "retained": retained,
            },
        )
        save("terminal-input-states.json", states)
        state.update(
            status="awaiting guard cleanup",
            payload_report=str(report_path.relative_to(ROOT)),
            compiled_bytes=64,
            retained_bytes=3743,
            preserved_raw_index=True,
            tail_asset_exact=True,
            full_campaign_complete=False,
        )
        save("summary.json", state)
        return state
    except BaseException as error:
        state.update(status="failed", error=f"{type(error).__name__}: {error}")
        save("summary.json", state)
        raise


def main():
    attempt = {"created_output": False}
    try:
        with InputBatch(ROOT) as inputs, ExitStack() as asset_stack:
            state = check(inputs, asset_stack, attempt)
        state.update(status="complete", continuous_input_guards_closed=True)
        save("summary.json", state)
    except BaseException as error:
        if attempt["created_output"] and (OUT / "summary.json").is_file():
            state = json.loads((OUT / "summary.json").read_bytes())
            state.update(status="failed", error=f"{type(error).__name__}: {error}")
            save("summary.json", state)
        raise


if __name__ == "__main__":
    main()
