#!/usr/bin/env python3
"""Render and verify the renderer-owned BOF3 lane workflow template."""

import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from lift_workflow_template import TEMPLATE  # noqa: E402

OLD_SELECTOR = 'const SELECTORS = [\n  "emi/example/00@0x80123456"\n];'
OLD_KEY = 'const RUN_KEY = "replace-with-unique-wave-id";'
OLD_ATTEMPTS = "const MAX_ATTEMPTS = 20;"


def rendered(selector: str, run_key: str, max_attempts: int = 20) -> str:
    text = TEMPLATE
    if text.count(OLD_SELECTOR) != 1 or text.count(OLD_KEY) != 1 or text.count(OLD_ATTEMPTS) != 1:
        raise SystemExit("canonical workflow constants changed")
    return text.replace(
        OLD_SELECTOR, f"const SELECTORS = [\n  {json.dumps(selector)}\n];"
    ).replace(OLD_KEY, f"const RUN_KEY = {json.dumps(run_key)};").replace(
        OLD_ATTEMPTS, f"const MAX_ATTEMPTS = {max_attempts};"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("render", "verify"))
    parser.add_argument("--selector", required=True)
    parser.add_argument("--run-key", required=True)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--max-attempts", type=int, default=20)
    args = parser.parse_args()
    if args.max_attempts < 1:
        parser.error("--max-attempts must be positive")
    expected = rendered(args.selector, args.run_key, args.max_attempts).encode()
    if args.command == "render":
        args.output.write_bytes(expected)
        return 0
    actual = args.output.read_bytes()
    result = {"verified": actual == expected, "sha256": hashlib.sha256(expected).hexdigest()}
    print(json.dumps(result))
    return 0 if result["verified"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
