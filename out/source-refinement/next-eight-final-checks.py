"""Run the applied eight-name batch's final concern and discovery checks once."""

from contextlib import redirect_stderr, redirect_stdout
import hashlib
import json
from pathlib import Path

from harness.commands import compile_commands
from harness.common.lease import acquire_writer
from harness.common.process import run_bounded

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/next-eight-final-checks"
TARGET = "emi/etc/game/00"
INPUTS = {
    "out/source-refinement/next-eight-applied.json": "55db07669bd556b6e9b0567eb84e8c9f2d650fd10f64a7bdf35b561c6b6ef3ec",
    "out/source-refinement/next-eight-naming-ready-draft/plan.json": "163579a7cdc4f7ed4f2dbcf836ff33f330d4381a270209f85f77271b914c2ea7",
    "out/source-refinement/next-eight-naming-pre-checks/check.stdout": "004e11130a7ce6e84eef4b386f8e4ecd88a677e932c6b74cb0512d780ee1dff4",
    "out/source-refinement/next-eight-naming-pre-checks/list.stdout": "02b4576e7044099cec2a28e803f4457363a3f2a427e13c404a887ea7c568a9c6",
}


def sha(content):
    return hashlib.sha256(content).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def main():
    require(Path.cwd() == ROOT, "run from the repository root")
    for name, digest in INPUTS.items():
        require(sha((ROOT / name).read_bytes()) == digest, f"changed input: {name}")
    applied, plan, precheck, prelist = [
        json.loads((ROOT / name).read_text()) for name in INPUTS
    ]
    pins = plan["proposed_output_sha256"]
    moves = applied["moves"]
    require(
        len(moves) == 8 and len(pins) == 16, "expected eight moves and sixteen outputs"
    )
    require(applied["owned_post_sha256"] == pins, "applied/approved outputs disagree")
    require(moves == plan["moves"], "applied/approved moves disagree")
    require(prelist["passed"] and not prelist["truncated"], "PRE list is incomplete")
    require(
        precheck["total"] == 11 and len(precheck["records"]) == 11,
        "PRE closure changed",
    )
    selectors = [move["selector"] for move in moves]
    expected_rows = {row["selector"].lower() for row in precheck["records"]}
    old_names = {move["old_name"] for move in moves}

    def canonical(value):
        return json.dumps(value, sort_keys=True)

    old_findings = {canonical(finding) for finding in precheck["findings"]}

    def verify_owned():
        for name, digest in pins.items():
            require(
                sha((ROOT / name).read_bytes()) == digest,
                f"owned output changed: {name}",
            )
        for move in moves:
            require(
                not (ROOT / move["old_path"]).exists(),
                f"old source remains: {move['old_path']}",
            )

    verify_owned()
    OUT.mkdir(exist_ok=False)
    state = {
        "status": "running",
        "scope": "applied eight-name batch only",
        "commands": [],
    }
    save("summary.json", state)

    def run(label, arguments, expected_exit=0):
        verify_owned()
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
            f"{label} failed; see retained process/output",
        )
        verify_owned()
        return result["stdout"]

    try:
        facts = json.loads(
            run(
                "naming-list",
                [
                    "bin/harness",
                    "naming",
                    "list",
                    TARGET,
                    "--kind",
                    "all",
                    "--limit",
                    "100000",
                    "--json",
                ],
            )
        )
        require(
            facts["passed"]
            and not facts["incomplete"]
            and not facts["truncated"]
            and facts["total"] == len(facts["records"]),
            "fresh naming list is incomplete",
        )
        for move in moves:
            symbols = [
                r
                for r in facts["records"]
                if r["kind"] == "symbol"
                and r["address"].lower() == move["selector"].split("@")[1].lower()
            ]
            require(
                len(symbols) == 1 and symbols[0]["name"] == move["new_name"],
                f"new naming identity missing: {move['selector']}",
            )
        check = json.loads(
            run(
                "naming-check",
                ["bin/harness", "naming", "check", TARGET, *selectors, "--json"],
                expected_exit=1,
            )
        )
        require(
            check["total"] == 11
            and len(check["records"]) == 11
            and not check["truncated"],
            "selected check must retain eleven records",
        )
        require(
            {r["selector"].lower() for r in check["records"]} == expected_rows
            and all(not r["findings"] for r in check["records"]),
            "selected eleven rows changed or have findings",
        )
        residual = {canonical(finding) for finding in check["findings"]}
        require(
            not check["passed"]
            and not check["incomplete"]
            and residual
            and residual < old_findings,
            "global naming findings must remain a nonempty strict subset of PRE",
        )
        require(
            not any(name in finding for name in old_names for finding in residual),
            "resolved eight-name debt remains",
        )
        save(
            "naming-comparison.json",
            {
                "selected_rows_clean": 11,
                "pre_findings": precheck["findings"],
                "post_residual_findings": check["findings"],
                "global_check_exit": 1,
                "global_naming_complete": False,
            },
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
        run(
            "game00-analysis",
            ["bin/harness", "analysis", "rz-project", "analyze", TARGET],
        )
        run("global-index", ["bin/harness", "analysis", "index"])
        verify_owned()
        with (
            (OUT / "compile-commands.stdout").open("w") as stdout,
            (OUT / "compile-commands.stderr").open("w") as stderr,
        ):
            with acquire_writer(ROOT):
                with redirect_stdout(stdout), redirect_stderr(stderr):
                    code = compile_commands.main([])
                require(code == 0, "compile_commands.main([]) failed")
                verify_owned()
        state["commands"].append(
            {
                "label": "compile-commands",
                "call": "compile_commands.main([])",
                "exit_code": code,
                "writer_lease_closed": True,
            }
        )
        save("summary.json", state)
        readiness = json.loads(
            run(
                "game00-readiness",
                ["bin/harness", "analysis", "readiness", TARGET, "--detail", "full"],
            )
        )
        require(
            readiness["ready"]
            and readiness["index_ready"]
            and readiness["stale_facts"]["count"] == 0,
            "analysis readiness is stale",
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
        for move in moves:
            new, old = move["new_path"], move["old_path"]
            require(
                new in texts[generated[0]] and old not in texts[generated[0]],
                f"Ninja discovery differs: {new}",
            )
            require(
                new in inventory["sources"]
                and old not in inventory["sources"]
                and new in inventory["profiles"]
                and old not in inventory["profiles"],
                f"inventory discovery differs: {new}",
            )
            require(
                files.count(str(ROOT / new)) == 1 and str(ROOT / old) not in files,
                f"compilation database discovery differs: {new}",
            )
            require(
                old not in texts[generated[1]] and old not in texts[generated[2]],
                f"obsolete discovery reference: {old}",
            )
        verify_owned()
        save(
            "discovery.json",
            {
                "new_sources_present": [m["new_path"] for m in moves],
                "old_sources_absent": [m["old_path"] for m in moves],
                "generated_sha256": {
                    n: sha((ROOT / n).read_bytes()) for n in generated
                },
                "compilation_database_entries": len(database),
                "owned_output_sha256": pins,
            },
        )
        state.update(
            status="complete",
            selected_rows_clean=11,
            residual_global_naming_findings=check["findings"],
            full_campaign_complete=False,
            normal_build="Not rerun; parent-owned actual POST payload performed the normal build.",
        )
        save("summary.json", state)
    except BaseException as error:
        state.update(status="failed", error=f"{type(error).__name__}: {error}")
        save("summary.json", state)
        raise


if __name__ == "__main__":
    main()
