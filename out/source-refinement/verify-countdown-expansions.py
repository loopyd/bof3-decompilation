"""Capture original configured expansions, then compare the two countdown groups."""

import argparse
from contextlib import ExitStack, closing
import hashlib
import json
from pathlib import Path
import re

from harness.build.arguments import inspect_arguments
from harness.build.compilation import configured_command
from harness.build.compiler import (
    COMPILER_INPUTS,
    DEFAULT_COMPILER_ID,
    load_compiler_configuration,
    resolve_compiler_settings,
)
from harness.build.dependencies import IncludeSnapshot
from harness.build.expansion import expand_function_members
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.domain.functions import parse_function_records
from harness.match.members import capture_members

ROOT = Path(__file__).resolve().parents[2]
DIRECTORY = ROOT / "out/source-refinement/countdown-ready-draft"
PLAN_SHA = "2b492891e7d9d6e72a47d666e563ea16c60011986d5bbcf1bce48e83fdf84c8e"
PRE = ROOT / "out/source-refinement/countdown-expansion-pre.json"
LIMIT = 64 * 1024 * 1024


def sha(content):
    return hashlib.sha256(content).hexdigest()


def tokens(text):
    return re.findall(r"\w+|[^\w\s]", text)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("stage", choices=("pre", "private", "shared"))
    parser.add_argument("--pre-sha256")
    arguments = parser.parse_args()
    stage = arguments.stage
    require(Path.cwd() == ROOT, "run from the repository root")
    require(
        stage == "pre" or re.fullmatch(r"[0-9a-f]{64}", arguments.pre_sha256 or ""),
        "private/shared require the independently reviewed --pre-sha256",
    )
    output = ROOT / f"out/source-refinement/countdown-expansion-{stage}.json"
    require(not output.exists(), "preserve the existing stage report")
    rows, profiles, states, snapshots = [], {}, {}, []
    with acquire_writer(ROOT), ExitStack() as stack:
        batch = stack.enter_context(InputBatch(ROOT))

        def bind(name, expected=None, *, absent=False):
            state, content = batch.read(
                ROOT / name, max_bytes=LIMIT, include_metadata=True, single_link=True
            )
            require(
                state is None if absent else state is not None,
                f"input presence differs: {name}",
            )
            if expected is not None:
                require(state["sha256"] == expected, f"input hash differs: {name}")
            if name in states:
                require(state == states[name], f"input state changed: {name}")
            states[name] = state
            return content

        plan_name = (DIRECTORY / "plan.json").relative_to(ROOT).as_posix()
        plan = json.loads(bind(plan_name, PLAN_SHA))
        expected = {member["selector"]: member for member in plan["members"]}
        require(
            len(expected) == 6 and len(plan["batches"]) == 2,
            "six-member/two-owner scope differs",
        )
        require(
            [len(group["members"]) for group in plan["batches"]] == [2, 4],
            "independent two-plus-four emission closure differs",
        )
        require(
            len(plan["metadata_edits"]) == 2,
            "reviewed metadata correction count differs",
        )
        for member in plan["members"]:
            corrected = member["metadata_original"]
            for edit in plan["metadata_edits"]:
                if edit["source"] == member["source"]:
                    require(
                        corrected.count(edit["old"]) == 1,
                        "metadata predecessor differs",
                    )
                    corrected = corrected.replace(edit["old"], edit["new"])
            require(
                corrected == member["metadata_proposed"], "unreviewed metadata change"
            )

        pins = {
            name: None if state is None else state["sha256"]
            for name, state in plan["input_states"].items()
        }
        outputs = {} if stage == "pre" else plan[f"{stage}_output_sha256"]
        if stage != "pre":
            pins.update({name: None for name in plan["source_deletions"]})
            pins.update(outputs)
        for name, digest in pins.items():
            bind(name, digest, absent=digest is None)
            if name not in outputs and not (
                stage != "pre" and name in plan["source_deletions"]
            ):
                require(
                    states[name] == plan["input_states"][name],
                    f"unchanged plan state differs: {name}",
                )
        for name in (
            *COMPILER_INPUTS,
            "CMakeLists.txt",
            "config/compiler/graph.cmake",
            "config/compiler/object-flags.cmake",
            "config/compiler/variants.json",
            ".git/index",
        ):
            bind(name)

        baseline = {}
        if stage != "pre":
            pre_bytes = bind(PRE.relative_to(ROOT).as_posix(), arguments.pre_sha256)
            pre = json.loads(pre_bytes)
            require(
                pre["stage"] == "pre"
                and pre["plan_sha256"] == PLAN_SHA
                and pre["configured_expansion_complete"] is True
                and len(pre["members"]) == 6,
                "reviewed original configured expansion report differs",
            )
            baseline = {member["selector"]: member for member in pre["members"]}
            require(set(baseline) == set(expected), "PRE selector closure differs")

        flags, compilers = load_compiler_configuration(ROOT)
        jobs = []
        if stage == "pre":
            jobs = [
                (member["source"], [member["selector"]]) for member in plan["members"]
            ]
        else:
            jobs = [
                (group["destination"], group["members"]) for group in plan["batches"]
            ]
        forced = set()
        for source, selectors in jobs:
            content = bind(source)
            text = content.decode("utf-8")
            settings = resolve_compiler_settings(source, text, flags, compilers)
            command, _ = configured_command(
                ROOT,
                ROOT / source,
                ROOT / "out/source-refinement/countdown-unused.i",
                allow_grouped=True,
                preprocess=True,
            )
            operands = inspect_arguments(command[1:], cwd=ROOT)
            forced.update(value.path for value in operands.forced)
            actual = {
                "compiler_id": settings.compiler_id,
                "flags": None if settings.flags is None else list(settings.flags),
            }
            for selector in selectors:
                require(
                    actual == expected[selector]["current_profile"],
                    "effective compiler/flags differ",
                )
                require(
                    command[:-4] == expected[selector]["ordered_arguments"],
                    "ordered configured profile differs",
                )
            profiles[source] = {
                "effective_compiler": settings.compiler_id or DEFAULT_COMPILER_ID,
                "settings": actual,
                "configured_preprocess_arguments": command,
                "forced_inputs": [
                    {
                        "option": value.option,
                        "path": value.path.relative_to(ROOT).as_posix(),
                    }
                    for value in operands.forced
                ],
            }
        seeds = [ROOT / source for source, _ in jobs]
        snapshot = stack.enter_context(
            closing(
                IncludeSnapshot(
                    ROOT,
                    seeds[0],
                    forced=tuple(sorted(set(seeds[1:]) | forced)),
                )
            )
        )
        snapshots.append(snapshot)
        for name, expected_state in snapshot.describe_inputs().items():
            bind(
                name,
                None if expected_state is None else expected_state["sha256"],
                absent=expected_state is None,
            )
            require(states[name] == expected_state, "include input state differs")

        def guard():
            for name, state in list(states.items()):
                bind(
                    name,
                    None if state is None else state["sha256"],
                    absent=state is None,
                )
            for include in snapshots:
                include.validate()

        def terminal(error_type, error, traceback):
            if error_type is None:
                guard()

        stack.push(terminal)
        for source, selectors in jobs:
            guard()
            text = bind(source).decode("utf-8")
            records = parse_function_records(text)
            by_address = {record.address: record for record in records}
            wanted = {
                int(expected[selector]["native_range"]["address_start"], 16): expected[
                    selector
                ]
                for selector in selectors
            }
            require(
                len(records) == len(wanted) and set(by_address) == set(wanted),
                "metadata emission closure differs",
            )
            captured = capture_members(ROOT, source)
            require(
                captured["target"] == wanted[next(iter(wanted))]["target"]
                and not captured["data_sections"],
                "explicit target/data ownership differs",
            )
            require(
                len(captured["functions"]) == len(wanted),
                "original range closure differs",
            )
            for function in captured["functions"]:
                member = wanted[function["address"]]
                require(
                    function["selector"] == member["selector"]
                    and function["symbol"] == member["symbol"]
                    and function["file_start"] == member["native_range"]["file_start"]
                    and len(function["original"])
                    == member["native_range"]["size"]
                    == 56
                    and sha(function["original"])
                    == member["native_range"]["original_sha256"],
                    "reviewed complete original range/binding differs",
                )
            for name, state in captured["inputs"].items():
                bind(
                    name,
                    None if state is None else state["sha256"],
                    absent=state is None,
                )
            expanded = expand_function_members(ROOT, ROOT / source)
            require(
                set(expanded) == set(wanted) and len(expanded) == len(wanted),
                "configured function emission closure differs",
            )
            for address, emitted in expanded.items():
                member, record = wanted[address], by_address[address]
                require(
                    emitted.address == address
                    and emitted.symbol == member["symbol"]
                    and emitted.source == source
                    and emitted.source_line == record.line,
                    "configured member/source invocation attribution differs",
                )
                metadata = (
                    member["metadata_original"]
                    if stage == "pre"
                    else member["metadata_proposed"]
                )
                require(
                    record.metadata == metadata,
                    "reviewed metadata corrections/progress differ",
                )
                invocation = (
                    member["implementation"]
                    if stage == "pre"
                    else (
                        f"{plan['macro']['name']}({member['symbol']}, {member['parameters']['when_spent_mode']})"
                    )
                )
                require(
                    tokens(emitted.invocation) == tokens(invocation),
                    "authored invocation/body tokens differ",
                )
                if stage != "pre":
                    original = baseline[member["selector"]]
                    require(
                        original["source"] == member["source"]
                        and original["symbol"] == member["symbol"]
                        and original["source_sha256"] == member["source_sha256"]
                        and original["implementation_sha256"]
                        == sha(original["implementation"].encode())
                        and tokens(emitted.implementation)
                        == tokens(original["implementation"]),
                        "complete configured body/ABI tokens differ from reviewed original PRE",
                    )
                signature = emitted.implementation.partition("{")[0]
                require(
                    tokens(signature) == ["void", member["symbol"], "(", "void", ")"],
                    "void(void) ABI differs",
                )
                rows.append(
                    {
                        "selector": member["selector"],
                        "source": source,
                        "source_sha256": states[source]["sha256"],
                        "symbol": emitted.symbol,
                        "address": address,
                        "source_line": emitted.source_line,
                        "invocation": emitted.invocation,
                        "metadata": record.metadata,
                        "implementation": emitted.implementation,
                        "implementation_sha256": sha(emitted.implementation.encode()),
                        "signature": signature,
                        "complete_configured_tokens_equal_reviewed_pre": None
                        if stage == "pre"
                        else True,
                    }
                )
            guard()
        require(
            len(rows) == 6 and {row["selector"] for row in rows} == set(expected),
            "complete six-member closure differs",
        )
        guard()
    report = {
        "stage": stage,
        "plan_sha256": PLAN_SHA,
        "configured_expansion_complete": True,
        "pre_sha256": None if stage == "pre" else arguments.pre_sha256,
        "members": rows,
        "effective_profiles": profiles,
        "input_states": states,
        "metadata_corrections": plan["metadata_edits"],
        "include_inputs": snapshot.describe_inputs(),
        "limit": "Actual configured preprocessing and metadata-attributed function tokens only. No hard-coded macro normalization. Complete native ELF/receipt checks and independent acceptance remain separate.",
    }
    with output.open("x") as file:
        file.write(json.dumps(report, indent=2, sort_keys=True) + "\n")
    print(
        json.dumps(
            {
                "stage": stage,
                "members": len(rows),
                "report": output.relative_to(ROOT).as_posix(),
                "sha256": sha(output.read_bytes()),
            }
        )
    )


if __name__ == "__main__":
    main()
