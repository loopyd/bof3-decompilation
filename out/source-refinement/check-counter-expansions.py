"""Compare seven configured COUNTER emissions to their independently owned PRE bodies."""
import argparse
from contextlib import closing
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
import re
from harness.build.compilation import configured_command
from harness.build.compiler import load_compiler_configuration, resolve_compiler_settings
from harness.build.dependencies import IncludeSnapshot
from harness.build.expansion import expand_function_members
from harness.common.inputs import InputBatch
from harness.common.lease import acquire_writer
from harness.common.process import ProcessCleanupError
from harness.domain.functions import parse_function_records

root = Path.cwd()
base = root / "out/source-refinement"
parser = argparse.ArgumentParser()
parser.add_argument("--stage", choices=("pre", "post"), default="post")
parser.add_argument("--pre-summary-sha256")
parser.add_argument("--pre-expansion-sha256")
parser.add_argument("--post-plan-sha256")
parser.add_argument("--application-sha256")
args = parser.parse_args()
sha = lambda value: hashlib.sha256(value).hexdigest()
tokens = lambda text: re.findall(r"\w+|[^\w\s]", text)
plan_name = "out/source-refinement/counter-advance-" + ("amended-draft/plan.json" if args.stage == "pre" else "post-plan.json")
plan_sha = "aa82f851029d93b85ec11ea8b3f51df2b68411cbf5a772d9df107b158b1493b5" if args.stage == "pre" else args.post_plan_sha256
assert plan_sha and (args.pre_summary_sha256 if args.stage == "pre" else args.pre_expansion_sha256 and args.application_sha256)
out = base / ("counter-advance-expansions-" + args.stage + ".json")
assert not out.exists()
report = {"status": "running", "stage": args.stage, "plan_sha256": plan_sha, "members": []}
def save():
    out.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
save()
try:
    with acquire_writer(root), InputBatch(root) as batch:
        states = {}
        def bind(name):
            state, content = batch.read(root / name, max_bytes=64 * 1024 * 1024, include_metadata=True, single_link=True)
            assert name not in states or state == states[name], name
            states[name] = state
            return content
        data = bind(plan_name)
        assert sha(data) == plan_sha
        plan = json.loads(data)
        members = plan["selected"] + plan["existing_consumers"]
        assert len(members) == len({m["selector"] for m in members}) == 7 and len({m["target"] for m in members}) == 5
        for name, expected in plan["current_input_states"].items():
            bind(name)
            assert states[name] == expected, name
        bind(Path(__file__).resolve().relative_to(root).as_posix())
        bind(".git/index")
        if args.stage == "pre":
            pre_data = bind("out/source-refinement/counter-advance-pre/summary.json")
            assert sha(pre_data) == args.pre_summary_sha256
            native_pre = json.loads(pre_data)
            assert native_pre["status"] == "complete PRE captures" and native_pre["member_count"] == 7 and native_pre["complete_original_bytes"] == 280 and native_pre["guards_and_writer_closed"]
            previous = {}
        else:
            pre_data = bind("out/source-refinement/counter-advance-expansions-pre.json")
            assert sha(pre_data) == args.pre_expansion_sha256
            previous_report = json.loads(pre_data)
            assert previous_report["status"] == "configured emissions verified" and previous_report["stage"] == "pre" and previous_report["guards_and_writer_closed"]
            previous = {row["selector"]: row for row in previous_report["members"]}
            assert set(previous) == {m["selector"] for m in members}
            application = bind(plan["application_report"])
            assert sha(application) == args.application_sha256 == plan["application_sha256"]
            assert json.loads(application)["accepted_cpp_pre_sha256"] == args.pre_expansion_sha256
        sources = [root / m["source"] for m in members]
        with closing(IncludeSnapshot(root, sources[0], forced=tuple(sources[1:]))) as includes:
            for name, expected in includes.describe_inputs().items():
                bind(name)
                assert states[name] == expected, name
            flags, compilers = load_compiler_configuration(root)
            for member in members:
                source = member["source"]
                text = bind(source).decode()
                records = parse_function_records(text)
                expected_record = member["record"] if args.stage == "pre" else member["proposed_record"]
                assert [asdict(r) for r in records] == [expected_record], source
                profile = asdict(resolve_compiler_settings(source, text, flags, compilers))
                arguments, _ = configured_command(root, root / source, Path(member["configured_arguments"][-1]), allow_grouped=True)
                assert profile == member["effective_settings"] and arguments == member["configured_arguments"], source
                expanded = expand_function_members(root, root / source)
                assert set(expanded) == {records[0].address}, source
                actual = expanded[records[0].address]
                assert actual.symbol == member["symbol"] and actual.source == source and actual.source_line == records[0].line, source
                declarations = [tokens(decl) for decl in re.findall(r"\bextern\b[^;]*;", text)]
                if args.stage == "pre" and member in plan["selected"]:
                    assert tokens(actual.implementation) == tokens(text[records[0].implementation_start:records[0].implementation_end]), source
                if args.stage == "post":
                    old = previous[member["selector"]]
                    expected = old["implementation"]
                    if member in plan["selected"]:
                        assert len(re.findall(r"\bcount\b", expected)) == 3 and len(re.findall(r"\bD_8014932A\b", expected)) == 2 and len(re.findall(r"\bD_80149333\b", expected)) == 1
                        expected = re.sub(r"\bcount\b", "count_", expected)
                        expected = re.sub(r"\b0x02\b", "2", expected)
                        expected = re.sub(r"\b(D_8014932A|D_80149333)\b", r"(\1)", expected)
                        invocation = f"COUNTER_ADVANCE({member['symbol']}, D_8014932A, D_80149333)"
                    else:
                        invocation = old["invocation"]
                    assert tokens(actual.implementation) == tokens(expected) and tokens(actual.invocation) == tokens(invocation), source
                    assert tokens(actual.implementation.split("{", 1)[0]) == tokens(old["implementation"].split("{", 1)[0]), source
                    assert declarations == old["extern_declarations"] and profile == old["effective_settings"] and arguments == old["configured_arguments"], source
                report["members"].append({"selector": member["selector"], "macro_name": member["macro_name"], **asdict(actual), "extern_declarations": declarations, "effective_settings": profile, "configured_arguments": arguments, "tokens_equal_reviewed_body": args.stage == "post"})
                print(member["selector"], flush=True)
                includes.validate()
                for name in list(states):
                    bind(name)
                save()
        report["current_input_states"] = states
    assert len(report["members"]) == 7
    report.update(status="configured emissions verified", guards_and_writer_closed=True)
    save()
except BaseException as error:
    report.update(status="failed", error=f"{type(error).__name__}: {error}")
    if isinstance(error, ProcessCleanupError):
        report["cleanup"] = "unconfirmed; stop native work"
        report["cleanup_observation"] = error.observed
    save()
    raise
