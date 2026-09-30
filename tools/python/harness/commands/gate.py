"""``bin/harness lift gate``: combined per-selector native lift gates.

One call runs the per-selector native gates plus the target metadata gates and
prints a compact PASS/FAIL summary, retrying the shared build-tree / sibling-lane
manifest races in-process so a lane spends one tool call instead of several.

Cost contract (measured): the five stages used to sum to ~13.8 s while the command
took 53-128 s under 8-lane fan-out, because every stage was its own ``bin/harness``
subprocess and ``asm-diff`` plus ``byte-match`` were two separate native passes over
the one shared ``build/cmake`` tree.  This owner now

* resolves the selector once and takes **one** in-process native measurement, whose
  payload carries both the instruction diff and the byte match;
* runs the target metadata stages in the **same interpreter** instead of spawning
  ``bin/harness`` per stage, and skips ``symbols normalize --write`` when the map is
  already sorted (it was the single largest stage);
* accepts several selectors for one target (``--target``), so the target-scoped
  stages run once per target rather than once per selector.
"""

from __future__ import annotations

import argparse
import contextlib
import io
import os
import re
import subprocess
import sys
import time
from pathlib import Path

from harness.common.cli import add_example_argument, resolve_function_selector, run_main
from harness.domain.ids import normalize_target_id
from harness.io import repo_layout

USAGE = "usage: bin/harness lift gate TARGET@0xADDRESS [TARGET]\n       bin/harness lift gate --target TARGET SELECTOR [SELECTOR ...]"
EXAMPLE = "bin/harness lift gate exe/logo@0x801CE758 exe/logo"

_RACE_RE = re.compile(
    r"stale configured build inventory|manifest claim directory changed|"
    r"claimed sources file missing|Build inventory changed during configure|"
    r"Directory not empty|build\.ninja|GLOB mismatch|"
    r"watched input entry or ancestor changed|reconfigure the build tree|"
    r"no such file or directory|object build failed|"
    r"Configuring incomplete|translation-unit inventory failed|"
    r"loading \.build\.ninja|PosixPath\("
)
_MAP_ROW = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;\s*$")


def _map_is_sorted(path: Path) -> bool:
    """True when every ``name = 0xADDR;`` row is already in canonical sorted order."""

    if not path.is_file():
        return False
    addresses: list[int] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        match = _MAP_ROW.match(line)
        if match is None:
            return False
        addresses.append(int(match.group(2), 16))
    return addresses == sorted(addresses)


def _run_module(
    module: str, argv: list[str], retries: int, label: str
) -> tuple[int, str]:
    """Run one owner stage in this interpreter, capturing its output text."""

    from importlib import import_module

    handler = getattr(import_module(module), "main")
    attempt = 1
    while True:
        buffer = io.StringIO()
        rc = 0
        with contextlib.redirect_stdout(buffer), contextlib.redirect_stderr(buffer):
            try:
                rc = int(handler(list(argv)))
            except SystemExit as exit_signal:  # run_main exits with the stage code
                rc = int(exit_signal.code or 0)
            except (OSError, RuntimeError, ValueError) as error:
                buffer.write(str(error))
                rc = 1
        output = buffer.getvalue()
        if rc == 0:
            return 0, output
        if _RACE_RE.search(output) and attempt < retries:
            sys.stderr.write(
                f"[lift-gate] shared-tree/sibling race in {label} "
                f"(attempt {attempt}/{retries}); retrying\n"
            )
            attempt += 1
            time.sleep(3)
            continue
        return rc, output


def _evidence_line(payload: dict) -> str:
    """Render the asm-diff summary line from one measurement payload."""

    instructions = payload.get("instruction_count") or {}
    matching = instructions.get("matching")
    original = instructions.get("original")
    percent = instructions.get("match_percent")
    insns = (
        f"{matching}/{original}({percent:.2f}%)"
        if matching is not None and original
        else "-"
    )
    current = payload.get("current_size")
    delta = payload.get("size_delta")
    delta_text = f"{delta:+d}" if isinstance(delta, int) else "?"
    first = payload.get("first_mismatch") or "-"
    return (
        f"insn={insns} bytes={payload.get('original_size')}->{current}({delta_text}) "
        f"first={first}"
    )


def _measure_native(root: Path, selector: str, retries: int) -> tuple[int, str, str]:
    """One in-process native measurement yielding both the diff and byte match.

    Returns ``(returncode, output_text, byte_status)`` where ``byte_status`` is
    ``MATCH`` or ``DIFFER`` so the caller can report the byte-match gate without a
    second native pass.
    """

    from harness.commands.lift import _run_match

    resolved = resolve_function_selector(selector)
    function, manifest, source = resolved
    if source is None:
        return (
            1,
            f"error: no authored source for {selector} (lifted source does not exist)\n",
            "N/A",
        )

    attempt = 1
    while True:
        try:
            payload = _run_match(function, manifest, source, diagnostics=True)
        except (OSError, RuntimeError, ValueError) as error:
            text = str(error)
            if _RACE_RE.search(text) and attempt < retries:
                sys.stderr.write(
                    f"[lift-gate] shared-tree/sibling race in asm-diff "
                    f"(attempt {attempt}/{retries}); retrying\n"
                )
                attempt += 1
                time.sleep(3)
                continue
            return 1, f"error: {text}\n", "N/A"
        byte_status = "MATCH" if payload.get("byte_match") else "DIFFER"
        status = "MATCH" if payload.get("exact_match") else "DIFF"
        line = f"{status} {function.value} {_evidence_line(payload)}\n"
        return 0 if payload.get("exact_match") else 1, line, byte_status


def _run(args: argparse.Namespace) -> int:
    root = repo_layout().root
    selectors: list[str] = list(args.selectors)
    target = args.target_id
    # Legacy form is `gate SELECTOR TARGET`; argparse assigns both positionals to
    # `selectors`, and a selector always carries `@` while a target id never does.
    if target is None and len(selectors) >= 2 and "@" not in selectors[-1]:
        target = selectors.pop()
    if not selectors:
        raise ValueError("lift gate requires a selector")
    if len(selectors) > 1 and args.target_id is None:
        raise ValueError("multiple selectors require --target TARGET")

    # Resolve the complete membership before any target metadata write. Registry
    # refs and canonical selectors must name the same target as the metadata gates.
    resolved = [resolve_function_selector(selector) for selector in selectors]
    if target is None:
        target = resolved[0][0].target.value
    else:
        target = normalize_target_id(target).value
    for selector, (function, _manifest, source) in zip(selectors, resolved):
        if function.target.value != target:
            raise ValueError(f"selector {selector} does not belong to target {target}")
        if source is None:
            raise ValueError(f"no authored source for {selector}")

    # Missing claims are an owner repair, never permission to rewrite other lanes.
    retries = int(os.environ.get("BOF3_GATE_RETRIES") or "4")

    rc = 0
    # ---- target-scoped metadata stages, once per target ---------------------
    map_path = root / "config" / "targets" / target / "symbols.txt"
    if _map_is_sorted(map_path):
        print(f"== symbols normalize {target} == skipped (map already sorted)")
    else:
        print(f"== symbols normalize {target} ==")
        stage_rc, output = _run_module(
            "harness.commands.symbols",
            ["normalize", target, "--write"],
            retries,
            "symbols-normalize",
        )
        if stage_rc:
            print(output.rstrip())
            rc = 1
    print(f"== symbols check {target} ==")
    stage_rc, output = _run_module(
        "harness.commands.symbols", ["check", target], retries, "symbols"
    )
    print(output.rstrip())
    if stage_rc:
        rc = 1
    print(f"== splat {target} ==")
    stage_rc, output = _run_module("harness.commands.splat", [target], retries, "splat")
    print(output.rstrip())
    if stage_rc:
        rc = 1

    # ---- one native measurement per selector --------------------------------
    for selector in selectors:
        print(f"== asm-diff {selector} ==")
        native_rc, output, byte_status = _measure_native(root, selector, retries)
        print(output.rstrip())
        print(f"== byte-match {selector} ==\n{byte_status} (single measurement)")
        if native_rc:
            rc = 1

    print("== git diff --check (informational) ==")
    diff = subprocess.run(
        ["git", "-C", str(root), "diff", "--check", "--quiet"], check=False
    )
    if diff.returncode == 0:
        print("diff-check: clean")
    else:
        print("diff-check: findings (pre-existing unrelated whitespace may be present)")

    print(f"== lift-gate {', '.join(selectors)}: {'PASS' if rc == 0 else 'FAIL'} ==")
    return rc


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness lift gate", description=__doc__)
    parser.add_argument(
        "selectors",
        nargs="+",
        help="TARGET@0xADDRESS selectors; the legacy form appends the target id",
    )
    parser.add_argument(
        "--target",
        dest="target_id",
        help="target id; required for multiple selectors so target stages run once",
    )
    add_example_argument(parser, EXAMPLE)
    parser.set_defaults(handler=_run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
