"""Adapt reconstruction arguments to preparation, generation and reporting."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import (
    add_example_argument,
    add_root_argument,
    resolved_root,
    run_main,
)
from harness.io import repo_layout
from harness.reconstruction import (
    contract,
    invocation,
    ledger,
    nativegeneration,
    nativeoutput,
    prepare,
    reporting,
)

EXAMPLE = (
    "bin/harness reconstruction prepare --run pilot "
    "--baseline tools/retdec-psx/corpus.json "
    '--native-receipt "$native_receipt"'
)


def _ledger_arguments(command):
    command.add_argument("--attempt-ledger", type=Path, required=True)
    command.add_argument("--expected-ledger-digest", required=True)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness reconstruction",
        description="118-function corpus preparation and bounded plugin runs",
    )
    add_root_argument(parser, default=repo_layout().root / contract.DEFAULT_RUN_ROOT)
    add_example_argument(parser, EXAMPLE)
    commands = parser.add_subparsers(dest="operation", required=True)
    command = commands.add_parser(
        "prepare",
        help="verify the pinned baseline and source closure, then publish problem, answer and preparation",
    )
    command.add_argument(
        "--run", required=True, help="new exclusive run id under the run root"
    )
    command.add_argument(
        "--baseline",
        type=Path,
        default=repo_layout().root / contract.DEFAULT_BASELINE_PATH,
        help="fixed reconstruction corpus manifest",
    )
    command.add_argument(
        "--native-receipt",
        type=Path,
        required=True,
        help="verified current sibling fork build receipt",
    )
    command.add_argument(
        "--json", action="store_true", help="print the command document as JSON"
    )
    command.set_defaults(handler=run_prepare)
    command = commands.add_parser(
        "run", help="run the selected plugin on prepared original members"
    )
    command.add_argument("--prepared-run", type=Path, required=True)
    command.add_argument("--destination", type=Path, required=True)
    command.add_argument("--native-receipt", type=Path, required=True)
    command.add_argument("--configuration", type=Path, required=True)
    command.add_argument(
        "--rizin", type=Path, default=repo_layout().root / "toolchains/rizin/bin/rizin"
    )
    selection = command.add_mutually_exclusive_group(required=True)
    selection.add_argument(
        "--member",
        action="append",
        help="frozen member key; repeat to select multiple members in attempt order",
    )
    selection.add_argument(
        "--all-members",
        action="store_true",
        help="select all 118 frozen Problem members in recorded order",
    )
    command.add_argument(
        "--resume-from",
        type=Path,
        help="exact prior report or progress file in a retained source-search campaign",
    )
    command.add_argument(
        "--resume-sha256", help="required complete SHA-256 of --resume-from"
    )
    command.add_argument("--output-limit", type=int, default=16 << 20)
    command.add_argument(
        "--mount-image",
        action="append",
        default=[],
        help="mount a permitted original image ID in order; later images replace overlapping bytes; unavailable with source-form search",
    )
    command.add_argument(
        "--entry-receipt",
        type=Path,
        help="passed native target capture for one discovered function",
    )
    command.add_argument(
        "--entry-cpu-receipt",
        type=Path,
        help="passed native state query of that entry save state",
    )
    forms = command.add_mutually_exclusive_group()
    forms.add_argument(
        "--source-form",
        choices=tuple(form.value for form in nativeoutput.SourceForm),
        help="select one original-only native C source form for this run",
    )
    forms.add_argument(
        "--search-source-forms",
        action="store_true",
        help="compare both original-only source forms and retain each member's best complete-byte result",
    )
    command.add_argument("--json", action="store_true")
    _ledger_arguments(command)
    command.set_defaults(handler=run_native)
    command = commands.add_parser(
        "capture-supplied",
        help="capture unchanged ordinary pdz source from Answer-associated supplied aids",
    )
    command.add_argument("--prepared-run", type=Path, required=True)
    command.add_argument("--destination", type=Path, required=True)
    command.add_argument("--native-receipt", type=Path, required=True)
    command.add_argument(
        "--context",
        type=Path,
        required=True,
        help="bounded supplied-context-v1 JSON with Answer member/prototype references",
    )
    command.add_argument(
        "--rizin", type=Path, default=repo_layout().root / "toolchains/rizin/bin/rizin"
    )
    command.add_argument("--output-limit", type=int, required=True)
    command.add_argument("--json", action="store_true")
    _ledger_arguments(command)
    command.set_defaults(handler=run_supplied)
    account = commands.add_parser(
        "ledger", help="retain cumulative attempts and execution recovery"
    )
    stages = account.add_subparsers(dest="ledger_operation", required=True)
    for operation in ("init", "status", "import", "close"):
        command = stages.add_parser(operation)
        command.add_argument("path", type=Path)
        command.set_defaults(handler=run_ledger)
        if operation == "init":
            command.add_argument("--prepared-run", type=Path, required=True)
            command.add_argument("--mission", required=True)
        elif operation != "status":
            command.add_argument("--expected-ledger-digest", required=True)
        if operation == "import":
            command.add_argument("--operation-id", required=True)
            command.add_argument("--units", type=int, required=True)
        if operation in {"import", "close"}:
            command.add_argument("--evidence", type=Path, required=True)
            command.add_argument("--expected-evidence-sha256", required=True)
        if operation == "close":
            command.add_argument("--epoch", required=True)
            command.add_argument(
                "--writer-tree-stopped", action="store_true", required=True
            )
    for operation in ("verify", "report"):
        command = commands.add_parser(
            operation,
            help="independently verify the fixed corpus"
            if operation == "verify"
            else "reproduce terminal fixed-corpus evidence without tool execution",
        )
        command.add_argument(
            "--baseline",
            type=Path,
            default=repo_layout().root / contract.DEFAULT_BASELINE_PATH,
        )
        command.add_argument("--prepared-run", type=Path, required=True)
        command.add_argument("--destination", type=Path, required=True)
        command.add_argument(
            "--generation" if operation == "verify" else "--verification",
            type=Path,
            action="append",
            required=operation == "report",
        )
        if operation == "verify":
            _ledger_arguments(command)
            selection = command.add_mutually_exclusive_group(required=True)
            selection.add_argument("--member", action="append")
            selection.add_argument("--all-members", action="store_true")
        for name in (
            "input-read-limit",
            "artifact-limit",
            "output-limit",
        ):
            command.add_argument(f"--{name}", type=int, required=True)
        command.add_argument("--require-mode", choices=reporting.MODES, required=True)
        command.add_argument("--json", action="store_true")
        command.set_defaults(handler=run_reporting)
    return parser


def render_text(command: dict) -> None:
    """Print every field of the command document in text form."""

    print(
        f"{command['operation']} status={command['status']} exit={command['exit']} "
        f"schema={command['schema']} run_id={command['run_id']}"
    )
    summary = command.get("summary")
    if summary is None:
        print("  summary=null")
    else:
        for key in sorted(summary):
            print(f"  summary.{key}={summary[key]}")
    for artifact in command["artifacts"]:
        path = artifact["path"] if artifact["path"] is not None else "null"
        print(
            f"  artifact role={artifact['role']} schema={artifact['schema']} "
            f"path={path} bundle_digest={artifact['bundle_digest']}"
        )
    message = command.get("message")
    print(f"  message={message if message is not None else 'null'}")


def run_prepare(args: argparse.Namespace) -> int:
    command = prepare.run_prepare(
        root=repo_layout().root,
        run_root=resolved_root(args),
        run_id=args.run,
        baseline_path=args.baseline,
        native_receipt=args.native_receipt,
    )
    if args.json:
        print(json.dumps(command, indent=2, sort_keys=True))
    else:
        render_text(command)
    return int(command["exit"])


def run_native(args: argparse.Namespace) -> int:
    destination = args.destination
    if not destination.is_absolute():
        destination = resolved_root(args) / destination
    report = nativegeneration.run(
        root=repo_layout().root,
        run_dir=args.prepared_run,
        destination=destination,
        member_keys=tuple(args.member or ()),
        all_members=args.all_members,
        native_receipt=args.native_receipt,
        configuration=args.configuration,
        rizin=args.rizin,
        output_limit=args.output_limit,
        source_form=nativeoutput.parse_source_form(args.source_form)
        if args.source_form is not None
        else None,
        search_forms=args.search_source_forms,
        overlay_mounts=tuple(args.mount_image),
        entry_receipt=args.entry_receipt,
        entry_cpu_receipt=args.entry_cpu_receipt,
        resume_from=args.resume_from,
        resume_sha256=args.resume_sha256,
        attempt_ledger=args.attempt_ledger,
        expected_ledger_digest=args.expected_ledger_digest,
    )
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(
            f"run status={report['status']} exit={report['exit']} "
            f"denominator={report['denominator']} destination={report['destination']}"
        )
        for row in report["members"]:
            if row["identity"]["key"] in report["selected"]:
                print(
                    f"  {row['identity']['key']} generation={row['generation']} "
                    f"compilation={row['compilation']}"
                )
    return int(report["exit"])


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


def run_supplied(args: argparse.Namespace) -> int:
    destination = args.destination
    if not destination.is_absolute():
        destination = resolved_root(args) / destination
    report = invocation.capture_supplied_run(
        root=repo_layout().root,
        prepared_run=args.prepared_run,
        destination=destination,
        native_receipt=args.native_receipt,
        context=args.context,
        rizin=args.rizin,
        output_limit=args.output_limit,
        attempt_ledger=args.attempt_ledger,
        expected_ledger_digest=args.expected_ledger_digest,
    )
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(
            f"capture-supplied status={report['status']} exit={report['exit']} denominator={report['denominator']} destination={report['destination']}"
        )
        for row in report["members"]:
            if row["identity"]["key"] in report["selected"]:
                print(
                    f"  {row['identity']['key']} generation={row['generation']} compilation={row['compilation']}"
                )
    return int(report["exit"])


def run_reporting(args: argparse.Namespace) -> int:
    destination = args.destination
    if not destination.is_absolute():
        destination = resolved_root(args) / destination
    limits = reporting.Limits(
        args.input_read_limit,
        args.artifact_limit,
        args.output_limit,
    )
    common = dict(
        baseline=args.baseline.resolve(),
        prepared_run=args.prepared_run.resolve(),
        destination=destination,
        limits=limits,
        required_mode=args.require_mode,
    )
    if args.operation == "verify":
        outcome = reporting.verify(
            reporting.VerificationRequest(
                **common,
                selection=reporting.AllMembers()
                if args.all_members
                else reporting.SelectedMembers(tuple(args.member)),
                generation_directories=tuple(
                    path.resolve() for path in (args.generation or ())
                ),
            ),
            attempt_ledger=args.attempt_ledger,
            expected_ledger_digest=args.expected_ledger_digest,
        )
    else:
        outcome = reporting.report(
            reporting.ReportRequest(
                **common,
                verification_directories=tuple(
                    path.resolve() for path in args.verification
                ),
            )
        )
    if args.json:
        print(json.dumps(outcome.document, indent=2, sort_keys=True))
    else:
        print(
            f"{args.operation} status={outcome.document['status']} "
            f"exit={outcome.exit_code} denominator={outcome.document['denominator']} "
            f"destination={outcome.document['destination']}"
        )
        summary = outcome.document.get("summary") or {}
        for key, value in sorted(summary.items()):
            if value is None or isinstance(value, (str, int, float, bool)):
                print(f"  summary.{key}={value}")
        print(f"  failures={len(outcome.failures)}")
        for artifact in outcome.published_artifacts:
            print(
                f"  artifact path={artifact.path} bytes={artifact.bytes} "
                f"sha256={artifact.sha256}"
            )
    return outcome.exit_code


def run_ledger(args: argparse.Namespace) -> int:
    from harness.reconstruction import provenance

    root = repo_layout().root
    operation = args.ledger_operation
    if operation == "init":
        binding = provenance.open_problem_binding(
            args.prepared_run.resolve(), io_mode="exact"
        )
        result = ledger.initialize(root, args.path, binding, args.mission)
    elif operation == "status":
        result = ledger.inspect(root, args.path)
    elif operation == "import":
        result = ledger.import_operation(
            root,
            args.path,
            args.expected_ledger_digest,
            args.operation_id,
            args.units,
            args.evidence,
            args.expected_evidence_sha256,
        )
    else:
        result = ledger.close_execution(
            root,
            args.path,
            args.expected_ledger_digest,
            args.epoch,
            args.evidence,
            args.expected_evidence_sha256,
        )
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0
