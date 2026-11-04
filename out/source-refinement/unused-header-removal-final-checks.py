"""Verify the eight applied task 4.18 paths with existing checks only."""

from __future__ import annotations

import argparse
from contextlib import closing, redirect_stderr, redirect_stdout
import hashlib
import json
from pathlib import Path
import shlex
import stat

from harness.commands import compile_commands
from harness.common.inputs import InputBatch, relative
from harness.common.lease import acquire_writer
from harness.common.observation import PathWatch
from harness.common.process import ProcessCleanupError, run_bounded

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "out/source-refinement/unused-header-removal-post"
PLAN = "out/source-refinement/unused-header-removal-draft/report.json"
PLAN_SHA = "bc426ce317d244254829cebb0f7d942be928431a0104d1f848bf1385328a01c9"
APPLICATION = "out/source-refinement/unused-header-removal-application/report.json"
LIMIT = 64 * 1024 * 1024


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(content):
    return hashlib.sha256(content).hexdigest()


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def read(inputs, name):
    return inputs.read(
        ROOT / relative(name),
        max_bytes=LIMIT,
        include_metadata=True,
        single_link=True,
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--application-sha256", required=True)
    args = parser.parse_args()
    require(Path.cwd() == ROOT, "run from the repository root")
    require(
        len(args.application_sha256) == 64
        and all(c in "0123456789abcdef" for c in args.application_sha256),
        "supply the independently reviewed application report SHA256",
    )
    require(not OUT.exists(), "verification output must be new")
    with InputBatch(ROOT) as inputs:
        plan_state, plan_bytes = read(inputs, PLAN)
        app_state, app_bytes = read(inputs, APPLICATION)
        require(plan_state and sha(plan_bytes) == PLAN_SHA, "reviewed draft changed")
        require(
            app_state and sha(app_bytes) == args.application_sha256,
            "application report differs from the reviewed SHA256",
        )
        plan, applied = json.loads(plan_bytes), json.loads(app_bytes)
        owned = set(plan["owned_tracked_paths"])
        headers = set(plan["delete_headers"])
        post = {r["path"]: r["sha256"] for r in plan["draft_manifest_files"]}
        require(
            len(owned) == 8 and len(headers) == 6 and len(post) == 2,
            "approved eight-path scope changed",
        )
        require(applied["owned_post_sha256"] == post, "applied manifest hashes differ")
        require(
            len(applied["git_paths"]) == 8 and set(applied["git_paths"]) == owned,
            "application report must retain the exact eight owned paths",
        )
        require(
            len(applied["removed"]) == 6 and set(applied["removed"]) == headers,
            "applied deletion scope differs",
        )
        retired = {r["path"]: r for r in applied["retired"]}
        require(
            len(applied["retired"]) == len(retired) == 8 and set(retired) == owned,
            "application must retain all eight original inodes",
        )
        require(
            len(plan["inputs"]) == 170
            and sum(v is not None for v in plan["inputs"].values()) == 137,
            "approved 137 present and 33 absent draft inputs changed",
        )
        states = {PLAN: plan_state, APPLICATION: app_state}
        for name, expected in plan["inputs"].items():
            observed, content = read(inputs, name)
            if name in headers:
                require(observed is None, f"deleted header remains: {name}")
            elif name in post:
                require(
                    observed and observed["sha256"] == post[name],
                    f"approved POST manifest differs: {name}",
                )
            elif expected is None:
                require(observed is None, f"absent draft input appeared: {name}")
            else:
                require(
                    observed == {k: expected[k] for k in ("sha256", "metadata")}
                    and len(content) == expected["size"],
                    f"preserved draft input changed: {name}",
                )
            states[name] = observed
        for name, row in retired.items():
            destination = relative(row["preserved_at"])
            require(
                destination.startswith(str(Path(APPLICATION).parent) + "/"),
                "retired original must remain inside the application evidence folder",
            )
            expected = plan["owned_pre_pins"][name]
            observed, content = read(inputs, destination)
            require(
                observed
                and observed["sha256"] == expected["sha256"]
                and len(content) == expected["size"]
                and observed["metadata"]["st_dev"]
                == expected["metadata"]["st_dev"]
                == row["device"]
                and observed["metadata"]["st_ino"]
                == expected["metadata"]["st_ino"]
                == row["inode"],
                f"retired original identity or bytes differ: {name}",
            )
            require(destination not in states, "retired evidence paths must be unique")
            states[destination] = observed
        foreign = json.loads(
            read(
                inputs, plan["foreign_gate_status_preservation"]["application_report"]
            )[1]
        )
        foreign_inputs = json.loads(
            read(inputs, "out/source-refinement/gate-status-shared/input-states.json")[
                1
            ]
        )
        require(
            len(foreign["owned_post_sha256"]) == 11
            and len(foreign_inputs["present"]) == 80
            and len(foreign_inputs["absent"]) == 18,
            "preserved gate/status scope changed",
        )
        for name in [*foreign_inputs["present"], *foreign_inputs["absent"]]:
            observed, _ = read(inputs, name)
            expected = foreign_inputs["present"].get(name)
            require(observed == expected, f"foreign gate/status input changed: {name}")
            if name in states:
                require(states[name] == observed, "foreign input samples disagree")
            states[name] = observed
        for name, digest in foreign["owned_post_sha256"].items():
            require(states[name]["sha256"] == digest, f"foreign output changed: {name}")
        index_state, _ = read(inputs, ".git/index")
        require(index_state is not None, "raw index is absent")
        states[".git/index"] = index_state

        def verify_inputs():
            for name, expected in states.items():
                observed, _ = read(inputs, name)
                require(observed == expected, f"input or index drift: {name}")

        verify_inputs()
        OUT.mkdir(exist_ok=False)
        state = {
            "status": "running",
            "task": "4.18",
            "application_sha256": args.application_sha256,
            "commands": [],
            "semantic_native_members": 0,
            "new_matched_bytes": 0,
            "payload_credit": 0,
        }
        save("summary.json", state)
        save("starting-input-states.json", states)

        def run(label, command):
            verify_inputs()
            print(" ".join(command), flush=True)
            with (
                (OUT / f"{label}.stdout").open("wb") as stdout,
                (OUT / f"{label}.stderr").open("wb") as stderr,
            ):

                def retain(stream, chunk):
                    (stdout if stream == "stdout" else stderr).write(chunk)

                result = run_bounded(
                    ROOT, command, output_limit=LIMIT, on_output=retain
                )
            save(f"{label}.process.json", result)
            state["commands"].append(
                {
                    "label": label,
                    "argv": command,
                    "exit_code": result["exit_code"],
                    "failure": result["failure"],
                }
            )
            save("summary.json", state)
            require(
                not result["failure"] and result["exit_code"] == 0,
                f"{label} failed; inspect retained process/output",
            )
            verify_inputs()

        try:
            commands = plan["future_commands"]
            require(len(commands) == 4, "approved command sequence changed")
            for label, command in zip(
                ("source-validation", "pytest", "bate-build", "batl-re2-build"),
                commands,
            ):
                run(label, command)
            verify_inputs()
            with (
                (OUT / "compile-commands.stdout").open("w") as stdout,
                (OUT / "compile-commands.stderr").open("w") as stderr,
                acquire_writer(ROOT),
            ):
                with redirect_stdout(stdout), redirect_stderr(stderr):
                    code = compile_commands.main([])
                require(code == 0, "compile_commands.main([]) failed")
                verify_inputs()
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
            with closing(PathWatch({ROOT / name for name in generated})) as watch:
                inventory_bytes = (ROOT / generated[1]).read_bytes()
                database_bytes = (ROOT / generated[2]).read_bytes()
                inventory = json.loads(inventory_bytes)
                database = json.loads(database_bytes)
                require(
                    len(database) == 2956,
                    "root compilation database must contain 2956 entries",
                )
                require(
                    all(
                        inventory["manifests"].get(name) == digest
                        for name, digest in post.items()
                    ),
                    "generated inventory does not bind both approved POST manifests",
                )
                require(
                    set(inventory["sources"])
                    == {str(Path(r["file"]).relative_to(ROOT)) for r in database}
                    and len(inventory["sources"]) == len(database),
                    "configured C source set and root compilation database differ",
                )
                profiles = [o["source_profiles"][0] for o in plan["owners"]]
                source_commands = {}
                edge_counts = {r["source"]: 0 for r in profiles}
                object_names = {
                    r["source"]: str(
                        ROOT / "build" / Path(r["source"]).with_suffix(".o")
                    )
                    for r in profiles
                }
                digest = hashlib.sha256()
                before = (ROOT / generated[0]).stat(follow_symlinks=False)
                require(
                    stat.S_ISREG(before.st_mode) and before.st_nlink == 1,
                    "Ninja file must be an ordinary single-link file",
                )
                pending = None
                with (ROOT / generated[0]).open("rb") as stream:
                    for raw in stream:
                        digest.update(raw)
                        require(
                            not any(h.encode() in raw for h in headers),
                            "deleted header remains in the actual Ninja graph",
                        )
                        if pending is not None:
                            require(
                                raw.startswith(b"  COMMAND = "),
                                "Ninja owner edge has no command",
                            )
                            source_commands[pending] = (
                                raw.decode().removeprefix("  COMMAND = ").strip()
                            )
                            pending = None
                        for source, obj in object_names.items():
                            if raw.startswith(
                                ("build " + obj + ": CUSTOM_COMMAND ").encode()
                            ):
                                edge_counts[source] += 1
                                pending = source
                after = (ROOT / generated[0]).stat(follow_symlinks=False)
                fields = ("st_dev", "st_ino", "st_size", "st_mtime_ns", "st_ctime_ns")
                require(
                    all(getattr(before, f) == getattr(after, f) for f in fields),
                    "Ninja changed during readback",
                )
                for row in profiles:
                    source = row["source"]
                    require(
                        edge_counts[source] == 1,
                        f"owner Ninja object edge differs: {source}",
                    )
                    require(
                        inventory["sources"].get(source) is False
                        and inventory["profiles"][source]
                        == {"compiler_id": None, "flags": None},
                        f"PsyQ source profile differs: {source}",
                    )
                    expected = [
                        str(ROOT / "bin/cc"),
                        *row["ordered_arguments"],
                        "-c",
                        str(ROOT / source),
                        "-o",
                        object_names[source],
                    ]
                    entries = [r for r in database if r["file"] == str(ROOT / source)]
                    require(
                        len(entries) == 1 and entries[0]["arguments"] == expected,
                        f"compilation database profile differs: {source}",
                    )
                    command = shlex.split(source_commands[source])
                    driver = str(ROOT / "bin/cc")
                    require(
                        command.count(driver) == 1
                        and command[command.index(driver) :] == expected,
                        f"actual Ninja command profile differs: {source}",
                    )
                watch.validate()
                save(
                    "discovery.json",
                    {
                        "deleted_headers_absent_from_actual_ninja": sorted(headers),
                        "owner_ninja_edges": edge_counts,
                        "owner_ninja_commands": source_commands,
                        "compilation_database_entries": len(database),
                        "generated_sha256": {
                            generated[0]: digest.hexdigest(),
                            generated[1]: sha(inventory_bytes),
                            generated[2]: sha(database_bytes),
                        },
                        "semantic_native_members": 0,
                        "payload_credit": 0,
                    },
                )
                watch.validate()
            verify_inputs()
            save("final-input-states.json", states)
            state["status"] = "passed existing checks and generated graph verification"
            state["preserved_input_states"] = len(states)
            state["retired_original_inodes_verified"] = 8
            save("summary.json", state)
        except BaseException as error:
            state["status"] = "failed"
            state["error"] = str(error)
            state["error_type"] = type(error).__name__
            if isinstance(error, ProcessCleanupError):
                state["cleanup"] = (
                    "unconfirmed; no subsequent command or restoration is authorized"
                )
                if error.observed is not None:
                    save("cleanup-failure.json", error.observed)
            save("summary.json", state)
            raise


if __name__ == "__main__":
    main()
