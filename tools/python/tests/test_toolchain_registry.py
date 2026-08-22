"""Structural contracts for repository layout and managed toolchain ownership."""

from __future__ import annotations

import ast
import inspect
from pathlib import Path

import pytest

import harness.toolchain as registry
from harness.io import repo_layout
from harness.toolchain import managed_lifecycle, managed_toolchain, managed_toolchains
from harness.toolchain.base import Toolchain


EXPECTED_LABELS = (
    "PSn00b",
    "GCC 2.7.2",
    "maspsx",
    "Rizin",
    "m2c",
    "asm-differ",
    "decomp-permuter",
    "PsyQ signatures",
    "splat",
    "spimdisasm",
)


def test_repo_layout_construction_is_factory_only() -> None:
    python_root = Path(__file__).parents[1]
    offenders = []
    factory_module = python_root / "harness" / "io.py"
    for path in python_root.rglob("*.py"):
        if path == factory_module or "__pycache__" in path.parts:
            continue
        tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
        if any(
            isinstance(node, ast.Call)
            and isinstance(node.func, ast.Name)
            and node.func.id == "RepoLayout"
            for node in ast.walk(tree)
        ):
            offenders.append(path.relative_to(python_root).as_posix())
    assert offenders == []


def test_managed_registry_order_and_uniform_layout_contract(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    toolchains = managed_toolchains(layout)
    assert tuple(toolchain.label for toolchain in toolchains) == EXPECTED_LABELS
    assert all(toolchain.layout is layout for toolchain in toolchains)
    assert all(
        tuple(inspect.signature(type(toolchain)).parameters) == ("layout",)
        for toolchain in toolchains
    )


def test_executable_lookup_uses_registry_keys(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    assert managed_toolchain(layout, "maspsx").label == "maspsx"
    assert managed_toolchain(layout, "rizin").label == "Rizin"
    assert managed_toolchain(layout, "spimdisasm").label == "spimdisasm"


def test_base_lifecycle_order_and_error_propagation(tmp_path: Path) -> None:
    calls = []

    class RecordingToolchain(Toolchain):
        label = "recording"

        def install(self, *, force: bool = False) -> str:
            calls.append(("install", force))
            return "installed"

        def build(self) -> str:
            calls.append(("build", False))
            return "built"

        def verify(self) -> str:
            calls.append(("verify", False))
            return "verified"

    assert RecordingToolchain(repo_layout(tmp_path)).run(force=True) == "verified"
    assert calls == [("install", True), ("build", False), ("verify", False)]


def test_registry_lifecycle_stops_on_labeled_run_and_verify_errors(
    tmp_path: Path, monkeypatch
) -> None:
    calls = []

    class Failing:
        label = "failing"

        def run(self, *, force=False):
            calls.append(("run", self.label))
            raise RuntimeError(f"{self.label}: run failed")

        def verify(self):
            calls.append(("verify", self.label))
            raise RuntimeError(f"{self.label}: verify failed")

    class Unreached:
        label = "unreached"

        def run(self, *, force=False):
            calls.append(("run", self.label))

        def verify(self):
            calls.append(("verify", self.label))

    monkeypatch.setattr(
        registry, "managed_toolchains", lambda layout: (Failing(), Unreached())
    )
    layout = repo_layout(tmp_path)
    with pytest.raises(RuntimeError, match=r"^failing: run failed$"):
        next(managed_lifecycle(layout))
    with pytest.raises(RuntimeError, match=r"^failing: verify failed$"):
        next(managed_lifecycle(layout, verify_only=True))
    assert calls == [("run", "failing"), ("verify", "failing")]


def test_registry_lifecycle_returns_same_ordered_inventory(
    tmp_path: Path, monkeypatch
) -> None:
    layout = repo_layout(tmp_path)
    monkeypatch.setattr(
        Toolchain, "run", lambda self, force=False: f"{self.label}:{force}"
    )
    assert tuple(managed_lifecycle(layout, force=True)) == tuple(
        f"{label}:True" for label in EXPECTED_LABELS
    )
