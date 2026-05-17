from __future__ import annotations

from collections.abc import Callable, Iterable
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..core import Pipeline, Task
from ..paths import repo_layout
from .ghidra_bootstrap import run_ghidra_bootstrap_pipeline


PipelineBuilder = Callable[[], Pipeline]


@dataclass(frozen=True)
class PipelineRegistration:
    name: str
    description: str
    builder: PipelineBuilder

    def build(self) -> Pipeline:
        pipeline = self.builder()
        if pipeline.name != self.name:
            raise ValueError(
                f"registered pipeline {self.name!r} built {pipeline.name!r}"
            )
        return pipeline


class PipelineRegistry:
    def __init__(self, registrations: Iterable[PipelineRegistration] = ()) -> None:
        self._registrations: dict[str, PipelineRegistration] = {}
        for registration in registrations:
            self.add(registration)

    def add(self, registration: PipelineRegistration) -> None:
        if not registration.name:
            raise ValueError("pipeline name must not be empty")
        if registration.name in self._registrations:
            raise ValueError(f"pipeline already registered: {registration.name}")
        self._registrations[registration.name] = registration

    def register(
        self,
        name: str,
        description: str,
        builder: PipelineBuilder,
    ) -> None:
        self.add(PipelineRegistration(name, description, builder))

    def names(self) -> list[str]:
        return sorted(self._registrations)

    def list(self) -> list[PipelineRegistration]:
        return [self._registrations[name] for name in self.names()]

    def get(self, name: str) -> PipelineRegistration:
        try:
            return self._registrations[name]
        except KeyError as exc:
            raise KeyError(f"unknown pipeline: {name}") from exc

    def build(self, name: str) -> Pipeline:
        return self.get(name).build()


def _format_paths(result: dict[str, Path]) -> dict[str, str]:
    return {name: str(path) for name, path in result.items()}


def build_ghidra_bootstrap_pipeline() -> Pipeline:
    layout = repo_layout()

    def run(_: Any) -> dict[str, str]:
        outputs = run_ghidra_bootstrap_pipeline(
            slus_path=layout.slus_path,
            logo_path=layout.logo_path,
            emi_root=layout.emi_root,
            output_dir=layout.ghidra_bootstrap_dir,
        )
        return _format_paths(outputs)

    return Pipeline(
        name="ghidra-bootstrap",
        description="Build Ghidra bootstrap inventory, duplicate groups, and manifest",
        tasks=[
            Task(
                name="ghidra-bootstrap",
                description="Scan inputs and write Ghidra bootstrap outputs",
                runner=run,
            ),
        ],
    )


def build_default_registry() -> PipelineRegistry:
    registry = PipelineRegistry()
    from .build_match import build_build_ready_pipeline, build_match_loop_pipeline
    from .harness import (
        build_binary_parity_pipeline,
        build_harness_ready_pipeline,
        build_lift_ready_pipeline,
    )
    from .reverse import (
        build_decomp_full_ready_pipeline,
        build_decomp_ready_pipeline,
        build_extract_assets_pipeline,
        build_ghidra_ready_pipeline,
        build_inventory_refresh_pipeline,
    )
    from .setup_open import build_setup_open_pipeline

    registry.register(
        "setup-open",
        "Prepare a fresh clone for open-source workspace usage",
        build_setup_open_pipeline,
    )
    registry.register(
        "build-ready",
        "Configure and build the workspace",
        build_build_ready_pipeline,
    )
    registry.register(
        "match-loop",
        "Build, diff, and report matching status",
        build_match_loop_pipeline,
    )
    registry.register(
        "harness-ready",
        "Refresh harness state, binary maps, reports, and dashboard",
        build_harness_ready_pipeline,
    )
    registry.register(
        "lift-ready",
        "Refresh cheap harness state for function lifting",
        build_lift_ready_pipeline,
    )
    registry.register(
        "binary-parity",
        "Build compiled raw bins and diff them against extracted EMI bins",
        build_binary_parity_pipeline,
    )
    registry.register(
        "extract-assets",
        "Extract the disc and unpack EMI archives into workspace outputs",
        build_extract_assets_pipeline,
    )
    registry.register(
        "inventory-refresh",
        "Refresh maintained inventory artifacts from extracted assets",
        build_inventory_refresh_pipeline,
    )
    registry.register(
        "ghidra-ready",
        "Prepare extracted assets, inventory, and Ghidra bootstrap outputs",
        build_ghidra_ready_pipeline,
    )
    registry.register(
        "decomp-ready",
        "Export/import Ghidra symbols and verify decomp/matching readiness",
        build_decomp_ready_pipeline,
    )
    registry.register(
        "decomp-full-ready",
        "Refresh extraction, Ghidra, symbol indexes, and harness state",
        build_decomp_full_ready_pipeline,
    )
    registry.register(
        "ghidra-bootstrap",
        "Build Ghidra bootstrap inventory, duplicate groups, and manifest",
        build_ghidra_bootstrap_pipeline,
    )
    return registry
