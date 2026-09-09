"""CLI transport for human-value macro ranking and bounded selection."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Callable

from harness.common.cli import resolved_root
from harness.common.evidence import evidence_output_path, write_evidence_output
from harness.macros.ranking import (
    create_ranking_request,
    rank_candidates,
    validate_ranking,
)


def add_ranking_commands(sub: Any, read: Callable[[Path], Any]) -> None:
    def draft(args: argparse.Namespace) -> int:
        policy = {
            name: getattr(args, name)
            for name in (
                "min_instructions",
                "target",
                "pool_size",
                "top_n",
                "require_source",
            )
        }
        print(
            json.dumps(
                create_ranking_request(resolved_root(args), policy),
                indent=2,
                sort_keys=True,
            )
        )
        return 0

    def rank(args: argparse.Namespace) -> int:
        root = resolved_root(args)
        report = rank_candidates(
            root, read(args.request), expected_pool_digest=args.expected_pool_digest
        )
        output = evidence_output_path(root, args.output.as_posix())
        write_evidence_output(root, output, report)
        print(
            json.dumps(
                {
                    "digest": report["digest"],
                    "selected": len(report["queue"]),
                    "counts": report["counts"],
                },
                indent=2,
                sort_keys=True,
            )
        )
        return 0

    def verify(args: argparse.Namespace) -> int:
        report = validate_ranking(
            resolved_root(args),
            read(args.report),
            expected_ranking_digest=args.expected_ranking_digest,
        )
        print(
            json.dumps(
                {
                    "digest": report["digest"],
                    "selected": len(report["queue"]),
                    "counts": report["counts"],
                },
                indent=2,
                sort_keys=True,
            )
        )
        return 0

    parser = sub.add_parser(
        "rank-input", help="draft assessments for a largest-first block pool"
    )
    parser.add_argument("--min-instructions", type=int, required=True)
    parser.add_argument("--target")
    parser.add_argument("--pool-size", type=int, required=True)
    parser.add_argument("--top-n", type=int, required=True)
    parser.add_argument(
        "--require-source",
        action="store_true",
        help="require canonical C sources for every global member",
    )
    parser.set_defaults(handler=draft)
    parser = sub.add_parser(
        "rank", help="validate human-value assessments and select top N"
    )
    parser.add_argument("request", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--expected-pool-digest", required=True)
    parser.set_defaults(handler=rank)
    parser = sub.add_parser(
        "validate-ranking",
        help="replay an externally pinned ranking against fresh candidates",
    )
    parser.add_argument("report", type=Path)
    parser.add_argument("--expected-ranking-digest", required=True)
    parser.set_defaults(handler=verify)
