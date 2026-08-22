"""Focused tests: doctor toolchain task uses shared managed_toolchains factory."""

from __future__ import annotations

import io
from pathlib import Path

from harness.commands import doctor
from harness.toolchain import managed_toolchains
from harness.io import repo_layout


class DummyToolchain:
    """Minimal toolchain-like object for verifying factory delegation."""

    def __init__(self, label: str) -> None:
        self.label = label

    def verify(self) -> str:
        return self.label


def test_doctor_toolchain_uses_managed_toolchains(tmp_path: Path, monkeypatch) -> None:
    """Doctor _toolchain task delegates to managed_toolchains()."""
    called_with_root: list[object] = []
    dummy = DummyToolchain("demo")
    fake_toolchains = (dummy,)

    def fake_lifecycle(layout, *, verify_only=False):
        called_with_root.extend((layout.root, verify_only))
        return (toolchain.verify() for toolchain in fake_toolchains)

    monkeypatch.setattr(doctor, "managed_lifecycle", fake_lifecycle)

    result = doctor.TASKS[0].run(tmp_path)

    assert called_with_root == [tmp_path.resolve(), True]
    assert result == "demo"


def test_managed_toolchains_factory_order(tmp_path: Path) -> None:
    """managed_toolchains() returns known toolchains in registration order."""
    layout = repo_layout(tmp_path)
    toolchains = managed_toolchains(layout)
    labels = [t.label for t in toolchains]
    assert labels == [
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
    ]


def test_doctor_tool_failure_renders_under_its_label(
    tmp_path: Path, monkeypatch
) -> None:
    """A failing toolchain verify produces a [FAIL] for that task."""

    class FailToolchain:
        label = "failtool"

        def verify(self) -> str:
            raise RuntimeError("intentional failure")

    class PassToolchain:
        label = "passtool"

        def verify(self) -> str:
            return "passtool"

    buf = io.StringIO()
    monkeypatch.setattr("sys.stdout", buf)
    monkeypatch.setattr(
        doctor,
        "managed_lifecycle",
        lambda layout, **kwargs: (
            toolchain.verify() for toolchain in (FailToolchain(), PassToolchain())
        ),
    )
    try:
        doctor.TASKS[0].run(tmp_path)
        assert False, "expected RuntimeError"
    except RuntimeError as exc:
        assert "intentional failure" in str(exc)
