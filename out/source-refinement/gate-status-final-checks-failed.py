"""Run final concern and discovery checks for the applied gate/status batch."""

from contextlib import closing, redirect_stderr, redirect_stdout
import hashlib
import json
import os
from pathlib import Path

from harness.analysis.index import connect
from harness.commands import compile_commands
from harness.common.lease import acquire_writer
from harness.common.process import run_bounded
from harness.macros.queries import macro_uses_payload, macros_payload

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/gate-status-final-checks"
PLAN = "out/source-refinement/gate-status-ready-draft/plan.json"
PLAN_SHA = "75c6b8e2a788ee8909dd1d09556363c2a6dcc05f04901b6471397862d32644d0"
APPLIED = "out/source-refinement/gate-status-shared-application/report.json"
APPLIED_SHA = "93445e89791cc836c5542129610e99dad47f799ce87f4bb099e1ccfa414bcda7"
HEADER = "include/world/gate_status.h"
MACRO = "WORLD_DEFINE_GATE_STATUS"
TESTS = [
    "tools/python/tests/macros/test_macro_facts.py",
    "tools/python/tests/macros/test_macro_index.py",
    "tools/python/tests/macros/test_macro_opportunities.py",
    "tools/python/tests/analysis/test_reverse_index.py",
    "tools/python/tests/analysis/test_index_command.py",
]


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    require(Path.cwd() == ROOT, "run from the repository root")
    require(sha((ROOT / PLAN).read_bytes()) == PLAN_SHA, "reviewed plan changed")
    require(
        sha((ROOT / APPLIED).read_bytes()) == APPLIED_SHA, "application report changed"
    )
    plan = json.loads((ROOT / PLAN).read_bytes())
    applied = json.loads((ROOT / APPLIED).read_bytes())
    batches = plan["batches"]
    outputs = plan["proposed_output_sha256"]
    old_sources = set(plan["removed_sources"])
    new_sources = {batch["destination"] for batch in batches}
    targets = {batch["target"] for batch in batches}
    require(
        len(targets) == 3
        and len(outputs) == 11
        and len(old_sources) == 12
        and len(new_sources) == 3,
        "reviewed batch scope changed",
    )
    require(
        applied["owned_post_sha256"] == outputs
        and set(applied["removed"]) == old_sources
        and set(applied["git_paths"]) == outputs.keys() | old_sources
        and len(applied["git_paths"]) == 23,
        "application/plan outputs disagree",
    )
    pins = {
        name: digest
        for name, digest in plan["current_input_sha256"].items()
        if name not in old_sources
    }
    pins.update(outputs)
    absent = (set(plan["absent_paths"]) - pins.keys()) | old_sources
    require(len(pins) == 80 and len(absent) == 18, "final pin closure changed")

    def verify_pins():
        require(
            sha((ROOT / PLAN).read_bytes()) == PLAN_SHA
            and sha((ROOT / APPLIED).read_bytes()) == APPLIED_SHA,
            "plan/application input changed",
        )
        for name, digest in pins.items():
            require(
                sha((ROOT / name).read_bytes()) == digest,
                f"final input changed: {name}",
            )
        for name in absent:
            require(
                not os.path.lexists(ROOT / name),
                f"retired/negative path appeared: {name}",
            )

    verify_pins()
    OUT.mkdir(exist_ok=False)
    state = {
        "status": "running",
        "scope": "Applied shared gate/status task3.20 only: three targets, twelve complete forty-byte members.",
        "commands": [],
        "macro_reports": [],
        "plan_sha256": PLAN_SHA,
        "application_sha256": APPLIED_SHA,
        "limits": "Focused existing pytest suite only; no full check recipe or campaign completion is claimed.",
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
        expected = {selector.lower() for selector in batch["members"]}
        require(
            report["total"] == len(expected) == len(report["records"])
            and not report["truncated"]
            and {row["selector"].lower() for row in report["records"]} == expected,
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
                and measurement["original_size"] == measurement["current_size"] == 40
                and measurement["match_percent"] == 100.0
                and claim["status"] == "exact"
                and claim["match_percent"] == 100.0
                and claim["residual"] == "none",
                f"selected member is not complete live exact: {row['selector']}",
            )

    try:
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
                and len(findings[0]["targets"]) == 2,
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
        texts = {name: (ROOT / name).read_text() for name in generated}
        inventory = json.loads(texts[generated[1]])
        database = json.loads(texts[generated[2]])
        files = [entry["file"] for entry in database]
        require(
            len(database) == 2956, "expected 2956 root compilation database entries"
        )
        require(
            HEADER in texts[generated[0]],
            "shared header missing from Ninja dependencies",
        )
        for batch in batches:
            source = batch["destination"]
            require(
                source in texts[generated[0]]
                and source in inventory["sources"]
                and source in inventory["profiles"]
                and files.count(str(ROOT / source)) == 1,
                f"new source discovery differs: {source}",
            )
            require(
                inventory["sources"][source] == (len(batch["members"]) > 1)
                and inventory["profiles"][source]
                == {"compiler_id": None, "flags": None},
                f"new source classification/profile differs: {source}",
            )
            require(
                '#include "world/gate_status.h"' in (ROOT / source).read_text(),
                f"shared header not included: {source}",
            )
        for source in old_sources:
            require(
                all(source not in text for text in texts.values())
                and source not in inventory["sources"]
                and source not in inventory["profiles"]
                and str(ROOT / source) not in files,
                f"retired source remains in discovery: {source}",
            )

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
            (batch["target"], batch["destination"], selector.lower())
            for batch in batches
            for selector in batch["members"]
        }
        actual_uses = {
            (
                row["target_id"],
                row["source_path"],
                row["function_id"].lower() if row["function_id"] else None,
            )
            for row in uses
        }
        require(
            len(uses) == 12
            and len(actual_uses) == 12
            and actual_uses == expected_uses
            and all(
                row["definition_id"] == definitions[0]["id"]
                and row["definition_owner"] == "__shared__"
                and row["definition_path"] == HEADER
                and not row["generated"]
                for row in uses
            ),
            "indexed twelve-use identity/owner closure differs",
        )
        save("macro-index.json", {"definitions": definitions, "uses": uses})
        save(
            "discovery.json",
            {
                "new_sources_present": sorted(new_sources),
                "old_sources_absent": sorted(old_sources),
                "shared_header": HEADER,
                "header_discovery": "Pinned file, three direct includes, Ninja dependencies, unique indexed definition and twelve indexed uses. Inventory v4 and compilation database contain C sources only.",
                "generated_sha256": {
                    name: sha((ROOT / name).read_bytes()) for name in generated
                },
                "compilation_database_entries": len(database),
                "final_input_sha256": pins,
            },
        )
        run("focused-pytest", [".venv/bin/python", "-m", "pytest", "-q", *TESTS])
        run("registry-ruff", ["ruff", "check", "tools/python/harness/macros/index.py"])
        verify_pins()
        state.update(
            status="complete",
            selected_live_exact_members=12,
            three_normal_builds_passed=True,
            shared_macro_checks_passed=False,
            shared_macro_checks_incomplete=True,
            foreign_consumer_reports_preserved=True,
            focused_pytest="Five existing files only; full check recipes were not run.",
            full_campaign_complete=False,
        )
        save("summary.json", state)
    except BaseException as error:
        state.update(status="failed", error=f"{type(error).__name__}: {error}")
        save("summary.json", state)
        raise


if __name__ == "__main__":
    main()
