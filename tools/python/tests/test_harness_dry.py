"""Lock the harness DRY/naming contract: shared --root/--example, one run_main."""

from __future__ import annotations

import ast
from collections import deque
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
COMMANDS = ROOT / "tools" / "python" / "harness" / "commands"
HARNESS = ROOT / "tools" / "python" / "harness"


def _defs(path: Path) -> set[str]:
    tree = ast.parse(path.read_text(encoding="utf-8"))
    return {
        node.name
        for node in ast.walk(tree)
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef))
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
        "index_path": {HARNESS / "analysis" / "index.py"},
        "MapSymbol": {HARNESS / "domain" / "symbols.py"},
        "SplatBoundary": {HARNESS / "domain" / "layout.py"},
        "ReviewedSplatLayout": {HARNESS / "domain" / "layout.py"},
        "AnalysisSnapshot": {HARNESS / "analysis" / "snapshot.py"},
        "RizinProjectSpec": {HARNESS / "analysis" / "project.py"},
        "MatchStatusCache": {HARNESS / "match" / "status_cache.py"},
        "resolve_function_selector": {HARNESS / "commands" / "_common.py"},
    }
    for name, allowed in owners.items():
        found = {p for p in HARNESS.rglob("*.py") if name in _defs(p)}
        assert found == allowed, f"{name}: {found}"
    assert "function_name" not in _defs(COMMANDS / "permute.py")


def _dotted_module_names() -> dict[Path, str]:
    """Map every harness module file to its dotted package name.

    ``__init__.py`` files map to their package name (``domain/__init__.py``
    becomes ``harness.domain``) so package initializers are real graph nodes.
    """
    names: dict[Path, str] = {}
    for path in HARNESS.rglob("*.py"):
        if "__pycache__" in path.parts:
            continue
        parts = path.relative_to(HARNESS).with_suffix("").parts
        if parts[-1] == "__init__":
            parts = parts[:-1]
        names[path] = "harness" if not parts else "harness." + ".".join(parts)
    return names


def _module_dependencies(path: Path, module: str) -> set[str]:
    """Collect harness imports made at module scope from one file."""
    tree = ast.parse(path.read_text(encoding="utf-8"))
    # Relative imports in a package initializer resolve against the package
    # itself (``harness/domain/__init__.py`` is ``harness.domain``), not its
    # parent package, so ``from .ids import ...`` maps to ``harness.domain.ids``.
    package = (
        module
        if path.name == "__init__.py"
        else module.rsplit(".", 1)[0]
        if "." in module
        else "harness"
    )
    dependencies: set[str] = set()
    for node in tree.body:
        if isinstance(node, ast.Import):
            for alias in node.names:
                if alias.name.startswith("harness"):
                    dependencies.add(alias.name)
        elif isinstance(node, ast.ImportFrom):
            if node.level:
                prefix = package.split(".")
                for _ in range(node.level - 1):
                    if prefix:
                        prefix.pop()
                base = ".".join(prefix)
                target = f"{base}.{node.module}" if node.module else base
            else:
                target = node.module or ""
            if target.startswith("harness"):
                dependencies.add(target)
    return dependencies


def _harness_import_graph() -> dict[str, set[str]]:
    """Resolve every harness module's module-scope harness imports as edges."""
    dotted = _dotted_module_names()
    return {
        name: {
            dependency
            for dependency in _module_dependencies(path, name) & set(dotted.values())
            if dependency != name  # a package may reference its own dotted name
        }
        for path, name in dotted.items()
    }


def test_package_initializer_edges_are_locked() -> None:
    """Every package initializer resolves relative imports from its own package."""
    graph = _harness_import_graph()
    expected = {
        "harness.media": {"harness.media.str_media"},
        "harness.analysis": set(),
        "harness.build": set(),
        "harness.decomp": set(),
        "harness.domain": {
            "harness.domain.ids",
            "harness.domain.manifests",
            "harness.domain.registry",
            "harness.domain.sources",
            "harness.domain.tags",
        },
        "harness.emi": {"harness.emi.operations"},
        "harness.psyq": {"harness.psyq.fingerprints", "harness.psyq.headers"},
        "harness.toolchain": {"harness.io"},
        "harness.commands": set(),
        "harness.match": set(),
        "harness": set(),
    }
    initializer_edges = {
        name: graph[name]
        for path, name in _dotted_module_names().items()
        if path.name == "__init__.py"
    }
    assert initializer_edges == expected, (
        f"initializer edges drifted: {initializer_edges}"
    )


def test_harness_imports_resolve_and_are_acyclic() -> None:
    """Every harness module imports cleanly; module-level imports form a DAG."""
    import importlib

    dotted = _dotted_module_names()
    for path, name in sorted(dotted.items(), key=lambda item: item[1]):
        importlib.import_module(name)  # fails loudly on any broken import

    graph = _harness_import_graph()

    # Kahn's algorithm: a cycle leaves nodes with nonzero in-degree.
    in_degree = dict.fromkeys(graph, 0)
    for dependencies in graph.values():
        for dependency in dependencies:
            in_degree[dependency] += 1
    ready = deque(name for name, degree in in_degree.items() if degree == 0)
    visited = 0
    while ready:
        name = ready.popleft()
        visited += 1
        for dependency in graph[name]:
            in_degree[dependency] -= 1
            if in_degree[dependency] == 0:
                ready.append(dependency)
    assert visited == len(graph), "harness import cycle: " + ", ".join(
        sorted(n for n, d in in_degree.items() if d)
    )


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
