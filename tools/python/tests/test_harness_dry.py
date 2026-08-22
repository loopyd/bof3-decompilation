"""Lock the harness DRY/naming contract: shared --root/--example, one run_main."""

from __future__ import annotations

import ast
from collections import deque
import dataclasses
import os
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
COMMANDS = ROOT / "tools" / "python" / "harness" / "commands"
HARNESS = ROOT / "tools" / "python" / "harness"


def test_required_untracked_file_modes_are_normalized() -> None:
    result = subprocess.run(
        ["git", "ls-files", "--others", "--exclude-standard", "-z"],
        cwd=ROOT,
        capture_output=True,
        check=True,
    )
    names = [
        name
        for name in result.stdout.decode(errors="surrogateescape").split("\0")
        if name and (ROOT / name).is_file()
    ]
    if not names:
        return
    with tempfile.TemporaryDirectory() as temporary:
        index_name = str(Path(temporary) / "index")
        env = {**os.environ, "GIT_INDEX_FILE": index_name}
        subprocess.run(["git", "read-tree", "HEAD"], cwd=ROOT, env=env, check=True)
        for name in names:
            blob = subprocess.run(
                ["git", "hash-object", "-w", "--", name],
                cwd=ROOT,
                text=True,
                capture_output=True,
                check=True,
            ).stdout.strip()
            mode = "100755" if name.startswith("bin/") else "100644"
            subprocess.run(
                ["git", "update-index", "--add", "--cacheinfo", mode, blob, name],
                cwd=ROOT,
                env=env,
                check=True,
            )
        indexed = subprocess.run(
            ["git", "ls-files", "--stage", "--", *names],
            cwd=ROOT,
            env=env,
            text=True,
            capture_output=True,
            check=True,
        ).stdout.splitlines()
    actual = {line.split(maxsplit=3)[3]: line.split()[0] for line in indexed}
    expected = {
        name: "100755" if name.startswith("bin/") else "100644" for name in names
    }
    assert actual == expected


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
    # The toolchain package intentionally imports harness.io plus the shared
    # base at module scope; its concrete registry members are imported lazily
    # inside ``_managed_types`` and locked by
    # ``test_toolchain_initializer_imports_base_plus_concrete_registrations``.
    # The commands package initializer stays a one-line docstring-only file.
    expected = {
        "harness.media": {"harness.media.str_media"},
        "harness.analysis": set(),
        "harness.build": set(),
        "harness.decomp": set(),
        "harness.domain": {
            "harness.domain.ids",
            "harness.domain.manifests",
            "harness.domain.tags",
        },
        "harness.context": {
            "harness.context.base",
            "harness.context.bof3_cleanup",
        },
        "harness.emi": {"harness.emi.operations"},
        "harness.psyq": {"harness.psyq.fingerprints", "harness.psyq.headers"},
        "harness.toolchain": {"harness.io", "harness.toolchain.base"},
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


def test_toolchain_initializer_imports_base_plus_concrete_registrations() -> None:
    """Toolchain initializer: shared base at module scope, registry members lazily."""
    graph = _harness_import_graph()
    assert graph["harness.toolchain"] == {"harness.io", "harness.toolchain.base"}
    tree = ast.parse(
        (HARNESS / "toolchain" / "__init__.py").read_text(encoding="utf-8")
    )
    lazy = {
        node.module
        for node in ast.walk(tree)
        if isinstance(node, ast.ImportFrom)
        and node.level == 1
        and node.module is not None
        and node.module != "base"
    }
    assert lazy == {
        "asm_differ",
        "gcc",
        "m2c",
        "maspsx",
        "permuter",
        "psn00b",
        "rizin",
        "signatures",
        "splat",
        "spimdisasm",
    }


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


def test_function_collision_matrix() -> None:
    from harness.domain.identity import ReviewedFunctionIdentity, collision_findings

    def identity(target: str, address: int, parent: str, digest: str):
        return ReviewedFunctionIdentity(
            target,
            address,
            address + 4,
            "c",
            "sameName",
            "sameName",
            parent,
            f"{parent}/wrapper.c",
            digest,
            4,
        )

    same = collision_findings(
        [
            identity("a", 1, "src/bof3/ui", "x"),
            identity("b", 2, "src/bof3/ui", "x"),
        ]
    )
    rejected = collision_findings(
        [
            identity("a", 1, "src/bof3/ui", "x"),
            identity("b", 2, "src/bof3/ui", "y"),
        ]
    )
    allowed = collision_findings(
        [
            identity("a", 1, "src/bof3/ui", "x"),
            identity("b", 2, "src/bof3/world", "y"),
        ]
    )
    assert not any(f.verdict == "reject" for f in same)
    assert any(f.verdict == "reject" for f in rejected)
    assert not any(f.verdict == "reject" for f in allowed)
    same_target = collision_findings(
        [
            identity("a", 1, "src/bof3/ui", "x"),
            identity("a", 2, "src/bof3/world", "x"),
        ]
    )
    casefolded = collision_findings(
        [
            identity("a", 1, "src/bof3/ui", "x"),
            dataclasses.replace(
                identity("b", 2, "src/bof3/ui", "y"), compiled_name="SameName"
            ),
        ]
    )
    assert any(f.verdict == "reject" for f in same_target)
    assert any(f.verdict == "reject" for f in casefolded)


def test_shared_function_templates_stay_target_owned() -> None:
    template = ROOT / "src/shared/ui/panel_task.inc"
    assert template.is_file()
    wrappers = [
        path
        for path in (ROOT / "src/bof3").rglob("*.c")
        if '"shared/ui/panel_task.inc"' in path.read_text(encoding="utf-8")
    ]
    assert len(wrappers) >= 2
    assert all("@source" in path.read_text(encoding="utf-8") for path in wrappers)
    assert "PANEL_ADVANCE_X" not in (ROOT / "include/ui/panel_task.h").read_text(
        encoding="utf-8"
    )
    assert not any(path.suffix == ".inc" for path in (ROOT / "src/shared").rglob("*.c"))


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
