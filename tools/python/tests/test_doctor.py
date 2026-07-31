"""Focused tests: doctor toolchain task uses shared managed_toolchains factory."""

from __future__ import annotations

import io
from pathlib import Path

from harness.commands import doctor
from harness.toolchain import managed_toolchains
from harness.io import RepoLayout


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

    def fake_managed(root, layout):
        called_with_root.append(root)
        return fake_toolchains

    monkeypatch.setattr(doctor, "managed_toolchains", fake_managed)

    result = doctor.TASKS[0].run(tmp_path)

    assert called_with_root == [tmp_path]
    assert result == "demo"


def test_managed_toolchains_factory_order(tmp_path: Path) -> None:
    """managed_toolchains() returns known toolchains in registration order."""
    # Use a clean temp layout so constructors don't look up real paths
    resolved = tmp_path.resolve()
    layout = RepoLayout(
        root=resolved,
        build_dir=resolved / "build",
        out_dir=resolved / "out",
        toolchains_dir=resolved / "toolchains",
        third_party_dir=resolved / "third_party",
        inputs_dir=resolved / "inputs",
        downloads_dir=resolved / "toolchains" / "downloads",
        private_assets_dir=resolved / "inputs" / "external" / "private-assets",
        harness_disk_src=resolved / "tools" / "rust" / "bof3-disk",
        emi_ex_src=resolved / "tools" / "rust" / "emi-ex",
        harness_disk_bin=resolved / "toolchains" / "bof3-disk" / "bof3-disk",
        emi_ex_bin=resolved / "toolchains" / "emi-ex" / "emi-ex",
        psn00b_toolchain_root=resolved / "toolchains" / "psn00b_toolchain",
        psn00b_sdk_root=resolved / "toolchains" / "psn00bsdk",
        gcc272_psx_root=resolved / "toolchains" / "gcc-2.7.2-psx",
        gcc_variants_root=resolved / "toolchains" / "gcc-variants",
        psyq_root=resolved / "toolchains" / "psyq",
    )
    toolchains = managed_toolchains(resolved, layout)
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


def test_doctor_tool_failure_renders_under_its_label(tmp_path: Path, monkeypatch) -> None:
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
        doctor, "managed_toolchains",
        lambda root, layout: (FailToolchain(), PassToolchain()),
    )
    try:
        doctor.TASKS[0].run(tmp_path)
        assert False, "expected RuntimeError"
    except RuntimeError as exc:
        assert "intentional failure" in str(exc)
