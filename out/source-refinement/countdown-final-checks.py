"""Run the finite final concern, discovery and payload checks for countdown only."""

import argparse
from contextlib import closing, redirect_stderr, redirect_stdout
import hashlib
import json
import os
from pathlib import Path
import re

from harness.analysis.index import connect
from harness.commands import compile_commands
from harness.common.files import read_file
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.observation import PathWatch
from harness.common.process import ProcessCleanupError, run_bounded
from harness.domain.ids import parse_function_id
from harness.domain.manifests import _parse_manifest
from harness.emi.catalog_verify import resolve_shipped_entry
from harness.macros.queries import macro_uses_payload, macros_payload
from harness.match.extraction import read_function_bytes

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/countdown-final-checks"
PLAN = "out/source-refinement/countdown-ready-draft/plan.json"
PLAN_SHA = "2b492891e7d9d6e72a47d666e563ea16c60011986d5bbcf1bce48e83fdf84c8e"
APPLIED = "out/source-refinement/countdown-shared-application/report.json"
SHARED = "out/source-refinement/countdown-shared/summary.json"
CPP = "out/source-refinement/countdown-expansion-shared.json"
INDEX_REPIN = "out/source-refinement/countdown-pre-final-index-repin/report.json"
VARIANTS = "config/compiler/variants.json"
VARIANTS_SHA = "96685df945605d42971a188afccbb19dbfc0cbe0faffdb981e5fc9051292e9fd"
HEADER = "include/world/countdown.h"
MACRO = "WORLD_DEFINE_COUNTDOWN_TO_MODE"
LIMIT = 64 * 1024 * 1024
EVIDENCE = {
    "out/source-refinement/countdown-pre/summary.json": "394985e8d2e4bff50428d0e14129d97d6cd3c08f37035e9c92c04c9f1f55aab2",
    "out/source-refinement/countdown-private/summary.json": "3387f3e88fde3607b346aad35763cb33c26231cf44f1d05ce48999d7db765b5b",
    "out/source-refinement/countdown-expansion-pre.json": "395965767815c7f3684a9f361b2300499f89579da75f73189fa073e189dc7490",
    "out/source-refinement/countdown-expansion-private.json": "4f255b48abaa458703f3cd3a6f05fb39e2c213c9bc07d79ac49a0a6f441a197b",
}
GATE_PLAN = "out/source-refinement/gate-status-ready-draft/plan.json"
GATE_PLAN_SHA = "75c6b8e2a788ee8909dd1d09556363c2a6dcc05f04901b6471397862d32644d0"
PAYLOADS = {
    "emi/world03/area119/13": "out/payloads/countdown-area11913-shared",
    "emi/world03/area134/13": "out/payloads/countdown-area13413-shared",
}
SELECTED_INTERVALS = {
    "emi/world03/area119/13": [(224, 280), (452, 508)],
    "emi/world03/area134/13": [(1620, 1676), (1764, 1820), (3964, 4020), (4108, 4164)],
}
TESTS = [
    "tools/python/tests/macros/test_macro_facts.py",
    "tools/python/tests/macros/test_macro_index.py",
    "tools/python/tests/macros/test_macro_opportunities.py",
    "tools/python/tests/analysis/test_reverse_index.py",
    "tools/python/tests/analysis/test_index_command.py",
    "tools/python/tests/build/test_build.py",
    "tools/python/tests/build/test_splat.py",
]


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def stream_ninja(name, present, absent):
    """Search and hash the actual generated graph in bounded chunks."""
    needles = {value: value.encode() for value in present | absent}
    found = set()
    digest = hashlib.sha256()
    overlap = max(map(len, needles.values())) - 1
    tail = b""
    with closing(PathWatch({ROOT / name})) as watch:
        with (ROOT / name).open("rb") as file:
            while chunk := file.read(1024 * 1024):
                digest.update(chunk)
                window = tail + chunk
                found.update(
                    value for value, spelling in needles.items() if spelling in window
                )
                tail = window[-overlap:]
        watch.validate()
    require(
        present <= found and not absent & found,
        "actual Ninja new/header/retired source discovery differs",
    )
    return {
        "sha256": digest.hexdigest(),
        "present": sorted(present),
        "absent": sorted(absent),
    }


def verify_payload(plan, batch, report, output):
    """Reconcile complete actual files and the exact selected intervals."""
    target = batch["target"]
    manifest_path = ROOT / f"config/targets/{target}/target.toml"
    manifest = _parse_manifest(manifest_path, manifest_path.read_bytes())
    entry = resolve_shipped_entry(ROOT, manifest.disc_id)
    require(
        entry["index"] == 13
        and entry["load_address"] == manifest.load_address
        and entry["payload_kind"] == "ram",
        "shipped entry/load identity differs",
    )
    original = read_file(
        ROOT, Path(entry["payload_path"]).relative_to(ROOT).as_posix(), max_bytes=LIMIT
    )
    normalized = read_file(ROOT, manifest.binary, max_bytes=LIMIT)
    payload = read_file(ROOT, output + "/payload.bin", max_bytes=LIMIT)
    archive = report["archive"]
    rebuilt_archive = read_file(ROOT, output + "/archive.EMI", max_bytes=LIMIT)
    original_archive = read_file(
        ROOT, Path(entry["archive_path"]).relative_to(ROOT).as_posix(), max_bytes=LIMIT
    )
    extracted = read_file(ROOT, output + "/entry.bin", max_bytes=LIMIT)
    require(
        report["schema"] == "harness.build-payload/v1"
        and report["target"] == target
        and report["kind"] == "hybrid"
        and report["whole_payload_equal"] is True
        and report["original"]
        == Path(entry["payload_path"]).relative_to(ROOT).as_posix()
        and report["output"] == output + "/payload.bin"
        and payload == normalized == original == extracted
        and len(payload) == report["size"] == entry["size"]
        and sha(payload) == report["sha256"] == entry["sha256"],
        "complete actual payload/original/normalized/entry equality differs",
    )
    require(
        archive["index"] == entry["index"]
        and archive["original"] == entry["archive_path"]
        and archive["output"] == output + "/archive.EMI"
        and archive["whole_archive_equal"] is True
        and archive["entry_equal"] is True
        and rebuilt_archive == original_archive
        and sha(rebuilt_archive) == archive["sha256"],
        "complete actual archive preservation differs",
    )
    cursor = compiled_size = 0
    retained = []
    for interval in report["compiled"]:
        start, end = interval["start"], interval["end"]
        require(
            cursor <= start < end <= len(payload)
            and interval["normalized_start"] == start
            and interval["address"] == manifest.load_address + start
            and sha(payload[start:end]) == interval["sha256"]
            and interval["owners"],
            "compiled range bounds/placement/hash/ownership differs",
        )
        if cursor < start:
            retained.append({"start": cursor, "end": start})
        compiled_size += end - start
        cursor = end
    if cursor < len(payload):
        retained.append({"start": cursor, "end": len(payload)})
    require(
        report["retained"] == retained
        and report["compiled_bytes"] == compiled_size > 0
        and report["retained_bytes"]
        == sum(row["end"] - row["start"] for row in retained)
        and report["retained_bytes"] + compiled_size == len(payload),
        "complete compiled/retained coverage differs",
    )
    selected = [
        member for member in plan["members"] if member["selector"] in batch["members"]
    ]
    require(
        sorted(
            (
                member["native_range"]["file_start"],
                member["native_range"]["file_end_exclusive"],
            )
            for member in selected
        )
        == SELECTED_INTERVALS[target],
        "reviewed selected interval closure differs",
    )
    for member in selected:
        start, end = (
            member["native_range"]["file_start"],
            member["native_range"]["file_end_exclusive"],
        )
        matches = [
            interval
            for interval in report["compiled"]
            if interval["start"] == start and interval["end"] == end
        ]
        require(
            len(matches) == 1
            and matches[0]["source"] == batch["destination"]
            and matches[0]["symbol"] == member["symbol"]
            and matches[0]["section"] is None
            and [parse_function_id(value).value for value in matches[0]["owners"]]
            == [parse_function_id(member["selector"]).value]
            and end - start == 56
            and sha(payload[start:end]) == member["native_range"]["original_sha256"],
            "selected complete countdown contribution differs",
        )
    return {
        "target": target,
        "report": output + "/report.json",
        "report_sha256": sha(read_file(ROOT, output + "/report.json", max_bytes=LIMIT)),
        "payload_size": len(payload),
        "payload_sha256": sha(payload),
        "compiled_bytes": compiled_size,
        "retained_bytes": report["retained_bytes"],
        "selected_bytes": len(selected) * 56,
        "selected_intervals": SELECTED_INTERVALS[target],
        "whole_payload_equal": True,
        "whole_archive_equal": True,
        "entry_equal": True,
        "archive_sha256": sha(rebuilt_archive),
    }


def execute(args, inputs, attempt):
    require(Path.cwd() == ROOT, "run from the repository root")
    states = {}

    def bind(name, digest=None, *, absent=False):
        state, content = inputs.read(
            ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True
        )
        require(
            state is None if absent else state is not None,
            f"input presence differs: {name}",
        )
        if digest is not None:
            require(state["sha256"] == digest, f"input hash differs: {name}")
        if name in states:
            require(state == states[name], f"input/index/evidence state drift: {name}")
        states[name] = state
        return content

    plan = json.loads(bind(PLAN, PLAN_SHA))
    outputs = plan["shared_output_sha256"]
    old_sources = set(plan["source_deletions"])
    batches = plan["batches"]
    new_sources = {batch["destination"] for batch in batches}
    targets = {batch["target"] for batch in batches}
    require(
        len(plan["input_states"]) == 168
        and len(outputs) == 8
        and len(old_sources) == 6
        and len(new_sources) == 2
        and targets == PAYLOADS.keys(),
        "approved final batch scope differs",
    )
    pins = {
        name: None if state is None else state["sha256"]
        for name, state in plan["input_states"].items()
    }
    pins.update(dict.fromkeys(old_sources))
    pins.update(outputs)
    for name, digest in pins.items():
        bind(name, digest, absent=digest is None)
        if name not in outputs and name not in old_sources:
            require(
                states[name] == plan["input_states"][name],
                f"preserved plan state differs: {name}",
            )
    bind(VARIANTS, VARIANTS_SHA)
    gate = json.loads(bind(GATE_PLAN, GATE_PLAN_SHA))
    gate_outputs = {
        name: digest
        for name, digest in gate["proposed_output_sha256"].items()
        if name != "tools/python/harness/macros/index.py"
    }
    require(
        len(gate_outputs) == 10 and len(gate["removed_sources"]) == 12,
        "gate/status preservation scope differs",
    )
    for name, digest in gate_outputs.items():
        bind(name, digest)
    for name in gate["removed_sources"]:
        bind(name, absent=True)
    index = bind(".git/index")
    applied = json.loads(bind(APPLIED, args.application_sha256))
    require(
        applied["status"] == "applied reviewed shared promotion"
        and applied["owned_post_sha256"] == outputs
        and set(applied["removed"]) == old_sources
        and set(applied["git_paths"]) == set(plan["owned_paths"]["final"])
        and len(applied["git_paths"]) == 14
        and applied["preserved_private_inputs"] is True
        and applied["raw_index_unchanged"] is True
        and set(applied["changed_paths"])
        == new_sources | {HEADER, "tools/python/harness/macros/index.py"}
        and len(applied["retired"]) == 2
        and {row["path"] for row in applied["retired"]} == new_sources
        and applied["plan_sha256"] == PLAN_SHA,
        "reviewed shared application differs",
    )
    evidence = {
        **EVIDENCE,
        SHARED: args.shared_native_sha256,
        CPP: args.shared_cpp_sha256,
    }
    reports = {
        name: json.loads(bind(name, digest)) for name, digest in evidence.items()
    }
    for retired in applied["retired"]:
        require(
            retired["sha256"] == plan["private_output_sha256"][retired["path"]],
            "retired private source predecessor differs",
        )
        bind(retired["preserved_at"], retired["sha256"])
    members = {
        parse_function_id(member["selector"]).value: member
        for member in plan["members"]
    }
    for member in plan["members"]:
        original = bind(
            "out/source-refinement/countdown-ready-draft/"
            + member["original_artifact"],
            member["native_range"]["original_sha256"],
        )
        require(len(original) == 56, "saved original range differs")
    # Preserve immutable stage evidence rather than watching legitimate transient compiler outputs.
    for name in (
        "out/source-refinement/countdown-pre/summary.json",
        "out/source-refinement/countdown-private/summary.json",
        SHARED,
    ):
        report = reports[name]
        require(
            report["plan_sha256"] == PLAN_SHA
            and report["guards_and_writer_closed"] is True
            and report["member_count"] == 6
            and report["complete_original_bytes"] == 336,
            "native stage evidence is incomplete",
        )
        ordinary = "members" in report
        groups = report["members"] if ordinary else report["sources"]
        require(
            report["status"]
            == ("complete PRE captures" if ordinary else "complete stage captures")
            and len(groups) == (6 if ordinary else 2),
            "native stage status/group closure differs",
        )
        seen = set()
        for group in groups:
            if not ordinary:
                selected_batch = next(
                    batch for batch in batches if batch["target"] == group["target"]
                )
                require(
                    group["source"] == selected_batch["destination"]
                    and {
                        parse_function_id(row["selector"]).value
                        for row in group["members"]
                    }
                    == {
                        parse_function_id(selector).value
                        for selector in selected_batch["members"]
                    }
                    and len(group["members"]) == len(selected_batch["members"]),
                    "native stage independent two-plus-four group identity differs",
                )
            elf = bind(group["saved_elf"], group["elf_sha256"])
            bind(group["comparison"])
            rows = [group] if ordinary else group["members"]
            for row in rows:
                identity = parse_function_id(row["selector"]).value
                member = members[identity]
                seen.add(identity)
                expected = member["native_range"]["original_sha256"]
                require(
                    row["size"] == 56
                    and row["original_sha256"] == row["current_sha256"] == expected,
                    "native saved member hash/extent differs",
                )
                directory = Path(group["saved_elf"]).parent
                original_name = (
                    "original.bin"
                    if ordinary
                    else f"{int(member['native_range']['address_start'], 16):08X}-original.bin"
                )
                current_name = (
                    "current.bin"
                    if ordinary
                    else f"{int(member['native_range']['address_start'], 16):08X}-current.bin"
                )
                original = bind((directory / original_name).as_posix(), expected)
                current = bind((directory / current_name).as_posix(), expected)
                require(
                    original
                    == current
                    == read_function_bytes(
                        elf,
                        function_name=member["symbol"],
                        address=int(member["native_range"]["address_start"], 16),
                    ).content,
                    "actual frozen stage ELF/raw bytes differ",
                )
        require(seen == members.keys(), "native saved stage member closure differs")
    cpp = reports[CPP]
    require(
        cpp["stage"] == "shared"
        and cpp["plan_sha256"] == PLAN_SHA
        and cpp["configured_expansion_complete"] is True
        and len(cpp["members"]) == 6,
        "reviewed shared CPP evidence differs",
    )
    original_cpp = reports["out/source-refinement/countdown-expansion-pre.json"]
    original_expansions = {
        parse_function_id(row["selector"]).value: row for row in original_cpp["members"]
    }
    for report_name in (
        "out/source-refinement/countdown-expansion-private.json",
        CPP,
    ):
        expansion = reports[report_name]
        require(
            expansion["plan_sha256"] == PLAN_SHA
            and expansion["configured_expansion_complete"] is True
            and expansion["pre_sha256"]
            == EVIDENCE["out/source-refinement/countdown-expansion-pre.json"]
            and len(expansion["members"]) == 6,
            "configured stage baseline/closure differs",
        )
        seen = set()
        for row in expansion["members"]:
            identity = parse_function_id(row["selector"]).value
            member = members[identity]
            selected_batch = next(
                batch for batch in batches if member["selector"] in batch["members"]
            )
            require(
                row["source"] == selected_batch["destination"]
                and row["symbol"] == member["symbol"]
                and row["metadata"] == member["metadata_proposed"]
                and re.findall(r"\w+|[^\w\s]", row["implementation"])
                == re.findall(
                    r"\w+|[^\w\s]", original_expansions[identity]["implementation"]
                ),
                "configured stage source/metadata/body/ABI differs",
            )
            seen.add(identity)
        require(seen == members.keys(), "configured stage selector closure differs")
    repin = json.loads(bind(INDEX_REPIN, args.index_repin_sha256))
    require(
        cpp["input_states"][".git/index"]
        == applied["index_repin"]["current"]
        == repin["historical_index_state"]
        and states[".git/index"] == repin["current_index"]["state"]
        and set(repin["owned_paths"]) == set(plan["owned_paths"]["final"])
        and repin["cached_diff_empty"] is True
        and repin["unchanged_non_index_states"]
        == {
            name: state
            for name, state in cpp["input_states"].items()
            if name != ".git/index"
        }
        and len(repin["unchanged_non_index_states"]) == 219,
        "historical/current index repin or preserved input closure differs",
    )
    index_listings = {}
    index_processes = {}
    for label in ("prior", "current"):
        record = repin[label + "_index"]
        expected_state = record["state"]
        bind(record["path"], expected_state["sha256"])
        if label == "prior":
            require(
                states[record["path"]] == expected_state
                and expected_state["sha256"]
                == repin["historical_index_state"]["sha256"],
                "retained prior index state differs",
            )
        listing_path = str(Path(INDEX_REPIN).parent / (label + ".entries"))
        listing = bind(listing_path)
        actual = bytearray()

        def retain_listing(stream, chunk):
            if stream == "stdout":
                actual.extend(chunk)

        result = run_bounded(
            ROOT,
            ["git", "--no-optional-locks", "ls-files", "--stage", "-z"],
            output_limit=LIMIT,
            env={**os.environ, "GIT_INDEX_FILE": str(ROOT / record["path"])},
            on_output=retain_listing,
        )
        require(
            not result["failure"]
            and result["exit_code"] == 0
            and bytes(actual) == listing,
            f"retained {label} index listing differs from actual saved index",
        )
        bind(record["path"], expected_state["sha256"])
        bind(listing_path)
        rows = listing.decode().removesuffix("\0").split("\0")
        entries = {}
        for row in rows:
            entry, name = row.split("\t", 1)
            require(name not in entries, f"duplicate index entry: {name}")
            entries[name] = entry
        index_listings[label] = entries
        index_processes[label] = result
    prior, current = index_listings["prior"], index_listings["current"]
    delta = [
        {"path": name, "before": prior.get(name), "after": current.get(name)}
        for name in sorted(prior.keys() | current.keys())
        if prior.get(name) != current.get(name)
    ]
    require(
        delta == repin["index_delta"]
        and len(delta) == 16
        and all(prior.get(name) == current.get(name) for name in repin["owned_paths"]),
        "index delta differs or changes countdown entries",
    )
    for name, state in cpp["input_states"].items():
        if name == ".git/index":
            continue
        bind(name, None if state is None else state["sha256"], absent=state is None)
        require(
            states[name] == state, f"current shared CPP input state differs: {name}"
        )
    for target in targets:
        manifest_name = f"config/targets/{target}/target.toml"
        manifest = _parse_manifest(ROOT / manifest_name, bind(manifest_name))
        entry = resolve_shipped_entry(ROOT, manifest.disc_id)
        for key in ("manifest_path", "archive_path", "payload_path"):
            bind(Path(entry[key]).relative_to(ROOT).as_posix())
    require(
        all(not (ROOT / output).exists() for output in PAYLOADS.values()),
        "both payload output directories must be fresh",
    )

    def verify_pins():
        for name, state in list(states.items()):
            bind(name, None if state is None else state["sha256"], absent=state is None)

    verify_pins()
    OUT.mkdir(exist_ok=False)
    attempt["created_output"] = True
    (OUT / "index").write_bytes(index)
    for label, result in index_processes.items():
        save(f"index-repin-{label}.process.json", result)
    save("starting-input-states.json", states)
    state = {
        "status": "running",
        "scope": "Applied countdown task3.21 only: two targets, six 56B selected members; full target payload totals may exceed 336B.",
        "commands": [],
        "macro_reports": [],
        "payloads": [],
        "plan_sha256": PLAN_SHA,
        "application_sha256": args.application_sha256,
        "shared_native_sha256": args.shared_native_sha256,
        "shared_cpp_sha256": args.shared_cpp_sha256,
        "index_repin_sha256": args.index_repin_sha256,
        "index_repin_report": INDEX_REPIN,
        "verified_foreign_index_delta": delta,
        "raw_index_entry_state": states[".git/index"],
        "reviewed_shared_cpp_index_state": cpp["input_states"][".git/index"],
        "index_scope": "Current index captured at entry after the independently reviewed foreign-index repin; historical stage indexes are evidence, not replayed as current inputs.",
        "limits": "Focused existing pytest suite only. Macro concern CLI retains foreign-consumer incomplete flags; reviewed combined native/CPP evidence is separate. No full recipe, runtime or campaign completion.",
    }
    save("summary.json", state)

    def run(label, arguments, expected_exit=0):
        verify_pins()
        print(" ".join(arguments), flush=True)
        with (
            (OUT / f"{label}.stdout").open("wb") as stdout,
            (OUT / f"{label}.stderr").open("wb") as stderr,
        ):

            def retain(stream, chunk):
                (stdout if stream == "stdout" else stderr).write(chunk)

            result = run_bounded(
                ROOT, arguments, output_limit=64 * 1024 * 1024, on_output=retain
            )
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
            not result["failure"] and result["exit_code"] == expected_exit,
            f"{label} failed; inspect retained process/output",
        )
        verify_pins()
        return result["stdout"]

    def verify_live(report, batch):
        expected = {parse_function_id(selector).value for selector in batch["members"]}
        require(
            report["total"] == len(expected) == len(report["records"])
            and not report["truncated"]
            and {parse_function_id(row["selector"]).value for row in report["records"]}
            == expected,
            f"selected member closure differs: {batch['target']}",
        )
        for row in report["records"]:
            measurement = row["measurement"]
            claim = row["claim"]
            require(
                row["source"] == batch["destination"]
                and not row["findings"]
                and measurement["state"] == "live"
                and measurement["status"] == "exact"
                and measurement["original_size"] == measurement["current_size"] == 56
                and measurement["match_percent"] == 100.0
                and claim["status"] == "exact"
                and claim["match_percent"] == 100.0
                and claim["residual"] == "none",
                f"selected member is not complete live exact: {row['selector']}",
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
    # Validated macro queries require the current analysis/index predecessors.
    for batch in batches:
        run(
            batch["target"].replace("/", "_") + "-analysis",
            ["bin/harness", "analysis", "rz-project", "analyze", batch["target"]],
        )
    run("global-index", ["bin/harness", "analysis", "index"])
    for batch in batches:
        target = batch["target"]
        label = target.replace("/", "_")
        macro_report = json.loads(
            run(
                label + "-macros",
                [
                    "bin/harness",
                    "macros",
                    "check",
                    target,
                    *batch["members"],
                    "--json",
                ],
                expected_exit=1,
            )
        )
        verify_live(macro_report, batch)
        findings = macro_report["findings"]
        require(
            macro_report["passed"] is False
            and macro_report["incomplete"] is True
            and len(findings) == 1
            and findings[0]["status"] == "unavailable"
            and findings[0]["code"] == "shared_consumers"
            and set(findings[0]["targets"]) == targets - {target}
            and len(findings[0]["targets"]) == 1,
            f"unexpected macro finding/global flags: {target}",
        )
        definitions = macro_report["definitions"]
        require(
            len(definitions) == 1
            and definitions[0]["name"] == MACRO
            and definitions[0]["source_path"] == HEADER
            and definitions[0]["owner_target"] == "__shared__",
            f"selected macro definition differs: {target}",
        )
        state["macro_reports"].append(
            {
                "target": target,
                "selected_live_exact_members": len(batch["members"]),
                "exit_code": 1,
                "passed": macro_report["passed"],
                "incomplete": macro_report["incomplete"],
                "findings": findings,
            }
        )
        save("summary.json", state)
        combined = json.loads(
            run(
                label + "-combiner",
                [
                    "bin/harness",
                    "combiner",
                    "check",
                    target,
                    *batch["members"],
                    "--json",
                ],
            )
        )
        verify_live(combined, batch)
        require(
            combined["passed"]
            and not combined["incomplete"]
            and not combined["findings"],
            f"combiner concern incomplete: {target}",
        )
        metadata = json.loads(
            run(
                label + "-metadata",
                [
                    "bin/harness",
                    "source",
                    "metadata",
                    "check",
                    target,
                    *batch["members"],
                    "--json",
                ],
            )
        )
        verify_live(metadata, batch)
        require(
            metadata["passed"]
            and not metadata["incomplete"]
            and not metadata["findings"],
            "metadata concern incomplete",
        )
        run(
            label + "-symbols",
            ["bin/harness", "source", "symbols", "check", target],
        )
        run(label + "-splat", ["bin/harness", "source", "splat", target])
        run(label + "-build", ["bin/harness", "build", target])

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
    for batch in batches:
        readiness = json.loads(
            run(
                batch["target"].replace("/", "_") + "-readiness",
                [
                    "bin/harness",
                    "analysis",
                    "readiness",
                    batch["target"],
                    "--detail",
                    "full",
                ],
            )
        )
        require(
            readiness["ready"]
            and readiness["index_ready"]
            and readiness["stale_facts"]["count"] == 0,
            f"analysis readiness stale: {batch['target']}",
        )

    generated = [
        "build/cmake/build.ninja",
        "build/cmake/bof3-inventory.json",
        "compile_commands.json",
    ]
    discovery = stream_ninja(generated[0], {HEADER, *new_sources}, old_sources)
    inventory_bytes = read_file(ROOT, generated[1], max_bytes=LIMIT)
    database_bytes = read_file(ROOT, generated[2], max_bytes=LIMIT)
    inventory = json.loads(inventory_bytes)
    database = json.loads(database_bytes)
    files = [entry["file"] for entry in database]
    require(
        len(database) == len(set(files)) == 2952,
        "expected 2952 unique root compilation database sources",
    )
    for batch in batches:
        source = batch["destination"]
        require(
            source in inventory["sources"]
            and inventory["sources"][source] is True
            and inventory["profiles"][source] == {"compiler_id": None, "flags": None}
            and files.count(str(ROOT / source)) == 1
            and '#include "world/countdown.h"' in (ROOT / source).read_text(),
            "new grouped source/profile/discovery differs",
        )
    for source in old_sources:
        require(
            source not in inventory["sources"]
            and source not in inventory["profiles"]
            and str(ROOT / source) not in files,
            "retired source remains in discovery",
        )
    generated_hashes = {
        generated[0]: discovery["sha256"],
        generated[1]: sha(inventory_bytes),
        generated[2]: sha(database_bytes),
    }
    verify_pins()
    with closing(connect(ROOT)) as connection:
        connection.execute("PRAGMA query_only = ON")
        definitions = [
            row
            for row in macros_payload(
                connection, target=None, pattern=MACRO, classification=None, limit=0
            )
            if row["name"] == MACRO
        ]
        uses = [
            row
            for row in macro_uses_payload(
                connection, target=None, pattern=MACRO, limit=0
            )
            if row["name"] == MACRO
        ]
    require(
        len(definitions) == 1
        and definitions[0]["owner_target"] == "__shared__"
        and definitions[0]["source_path"] == HEADER
        and definitions[0]["source_sha256"] == pins[HEADER]
        and not definitions[0]["diagnostic"],
        "indexed shared definition differs",
    )
    expected_uses = {
        (batch["target"], batch["destination"], parse_function_id(selector).value)
        for batch in batches
        for selector in batch["members"]
    }
    actual_uses = {
        (
            row["target_id"],
            row["source_path"],
            parse_function_id(row["function_id"]).value if row["function_id"] else None,
        )
        for row in uses
    }
    require(
        len(uses) == 6
        and len(actual_uses) == 6
        and actual_uses == expected_uses
        and all(
            row["definition_id"] == definitions[0]["id"]
            and row["definition_owner"] == "__shared__"
            and row["definition_path"] == HEADER
            and not row["generated"]
            for row in uses
        ),
        "indexed six-use identity/owner closure differs",
    )
    save("macro-index.json", {"definitions": definitions, "uses": uses})
    save(
        "discovery.json",
        {
            "new_sources_present": sorted(new_sources),
            "old_sources_absent": sorted(old_sources),
            "shared_header": HEADER,
            "header_discovery": "Pinned file, two direct includes, Ninja dependencies, unique indexed definition and six indexed uses. Inventory v4 and compilation database contain C sources only.",
            "generated_sha256": generated_hashes,
            "compilation_database_entries": len(database),
            "final_input_sha256": pins,
        },
    )
    run("focused-pytest", [".venv/bin/python", "-m", "pytest", "-q", *TESTS])
    run("registry-ruff", ["ruff", "check", "tools/python/harness/macros/index.py"])
    verify_pins()
    for batch in batches:
        target = batch["target"]
        output = PAYLOADS[target]
        result = run(
            target.replace("/", "_") + "-payload",
            [
                "bin/harness",
                "build",
                "payload",
                target,
                "--output-dir",
                output,
                "--json",
            ],
        )
        report = json.loads(read_file(ROOT, output + "/report.json", max_bytes=LIMIT))
        require(report == json.loads(result), "published/stdout payload report differs")
        state["payloads"].append(verify_payload(plan, batch, report, output))
        save("summary.json", state)
        verify_pins()
    state.update(
        status="awaiting guard cleanup",
        selected_live_exact_members=6,
        two_normal_builds_passed=True,
        shared_macro_checks_passed=False,
        shared_macro_checks_incomplete=True,
        foreign_consumer_reports_preserved=True,
        focused_pytest="Seven existing files only; full check recipes were not run.",
        full_campaign_complete=False,
    )
    save("summary.json", state)
    verify_pins()
    save("terminal-input-states.json", states)
    return state


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--application-sha256", required=True)
    parser.add_argument("--shared-native-sha256", required=True)
    parser.add_argument("--shared-cpp-sha256", required=True)
    parser.add_argument("--index-repin-sha256", required=True)
    args = parser.parse_args()
    for digest in (
        args.application_sha256,
        args.shared_native_sha256,
        args.shared_cpp_sha256,
        args.index_repin_sha256,
    ):
        require(
            re.fullmatch(r"[0-9a-f]{64}", digest),
            "supply independently reviewed SHA256 values",
        )
    attempt = {"created_output": False}
    try:
        with InputBatch(ROOT) as inputs:
            state = execute(args, inputs, attempt)
        state.update(
            status="complete",
            continuous_input_guards_closed=True,
            raw_index_unchanged=True,
            payload_producers_own_writer_leases=True,
            only_compile_commands_uses_parent_writer=True,
        )
        save("summary.json", state)
    except BaseException as error:
        if attempt["created_output"] and (OUT / "summary.json").is_file():
            state = json.loads((OUT / "summary.json").read_bytes())
            state.update(status="failed", error=f"{type(error).__name__}: {error}")
            if isinstance(error, ProcessCleanupError):
                state["cleanup"] = "unconfirmed; stop further native work"
            save("summary.json", state)
        raise


if __name__ == "__main__":
    main()
