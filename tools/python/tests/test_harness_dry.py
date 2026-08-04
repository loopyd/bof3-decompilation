"""Lock the harness DRY/naming contract: shared --root/--example, one run_main."""

from __future__ import annotations

import ast
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
COMMANDS = ROOT / "tools" / "python" / "harness" / "commands"
HARNESS = ROOT / "tools" / "python" / "harness"


def _defs(path: Path) -> set[str]:
    tree = ast.parse(path.read_text(encoding="utf-8"))
    return {
        node.name
        for node in ast.walk(tree)
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
    }


def test_root_boilerplate_only_in_common() -> None:
    offenders = []
    for path in COMMANDS.glob("*.py"):
        if path.name == "_common.py":
            continue
        tree = ast.parse(path.read_text(encoding="utf-8"))
        for node in ast.walk(tree):
            if (
                isinstance(node, ast.Call)
                and isinstance(node.func, ast.Attribute)
                and node.func.attr == "add_argument"
                and node.args
                and isinstance(node.args[0], ast.Constant)
                and node.args[0].value == "--root"
            ):
                offenders.append(f"{path.name}:{node.lineno}")
    assert offenders == []


def test_permute_delegates_to_run_main() -> None:
    text = (COMMANDS / "permute.py").read_text(encoding="utf-8")
    assert (
        "def main(argv: list[str] | None = None) -> int:\n    return run_main(build_parser, argv)"
        in text
    )
    assert "add_example_argument(" in text
    assert "add_root_argument(parser)" in text


def test_example_mechanism_only_in_common() -> None:
    text = (COMMANDS / "_common.py").read_text(encoding="utf-8")
    assert "def add_example_argument(" in text
    assert '"--example" in raw' in text
    for path in COMMANDS.glob("*.py"):
        if path.name == "_common.py":
            continue
        body = path.read_text(encoding="utf-8")
        assert '["--example"]' not in body or "add_example_argument" in body, path


def test_no_semantic_name_collisions() -> None:
    owners = {
        "archive_path_looks_valid": {HARNESS / "toolchain" / "releases.py"},
        "index_path": {HARNESS / "reverse_index.py"},
    }
    for name, allowed in owners.items():
        found = {p for p in HARNESS.rglob("*.py") if name in _defs(p)}
        assert found == allowed, f"{name}: {found}"
    assert "function_name" not in _defs(COMMANDS / "permute.py")


def test_harness_modules_stay_decomposed() -> None:
    """Seal: no harness module regrows past the decomposition ceiling."""
    ceiling = 450
    oversized = []
    for path in HARNESS.rglob("*.py"):
        if "__pycache__" in path.parts:
            continue
        lines = len(path.read_text(encoding="utf-8").splitlines())
        if lines > ceiling:
            oversized.append(f"{path.relative_to(HARNESS)}: {lines}")
    assert oversized == [], f"decompose before growing past {ceiling}: {oversized}"
