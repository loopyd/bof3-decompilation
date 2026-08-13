#!/usr/bin/env python3
"""Render and verify the checked-in BOF3 lane workflow."""

import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
REFERENCE = ROOT / ".pi/skills/bof3-lift-loop/references/workflow-script.md"


def template() -> str:
    text = REFERENCE.read_text()
    start = text.index("```js\n") + 6
    return text[start:text.index("\n```", start)]


def rendered(selector: str, run_key: str, max_attempts: int = 20) -> str:
    text = template()
    old_selector = 'const SELECTORS = [\n  "emi/example/00@0x80123456"\n];'
    old_key = 'const RUN_KEY = "replace-with-unique-wave-id";'
    old_attempts = "const MAX_ATTEMPTS = 20;"
    if text.count(old_selector) != 1 or text.count(old_key) != 1 or text.count(old_attempts) != 1:
        raise SystemExit("canonical workflow constants changed")
    selector_json = json.dumps(selector)
    key_json = json.dumps(run_key)
    return text.replace(old_selector, f"const SELECTORS = [\n  {selector_json}\n];").replace(old_key, f"const RUN_KEY = {key_json};").replace(old_attempts, f"const MAX_ATTEMPTS = {max_attempts};")


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
