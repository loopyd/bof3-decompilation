#!/usr/bin/env python3

from __future__ import annotations

import importlib
import sys
from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class CommandSpec:
    group: str
    command: str
    module_name: str
    forwarded_mode: str | None = None


class CommandRegistry:
    def __init__(
        self,
        *,
        command_specs: tuple[CommandSpec, ...],
        group_modules: dict[str, str],
        command_groups: dict[str, list[str]],
        group_descriptions: dict[str, str],
        hidden_groups: set[str] | None = None,
    ):
        self.command_specs = command_specs
        self.group_modules = dict(group_modules)
        self.command_groups = dict(command_groups)
        self.group_descriptions = dict(group_descriptions)
        self.hidden_groups = set(hidden_groups or ())
        self.public_command_modules = {
            (spec.group, spec.command): spec.module_name for spec in self.command_specs
        }

    def render_help(self) -> str:
        lines = [
            "usage: python3 -m scripts.rebof3 <group> <command> [args...]",
            "",
            "groups:",
        ]
        for group, commands in self.command_groups.items():
            if group in self.hidden_groups:
                continue
            description = self.group_descriptions[group]
            lines.append(f"  {group}: {', '.join(commands)}")
            lines.append(f"    {description}")
        return "\n".join(lines)

    def render_group_help(self, group: str) -> str:
        commands = self.command_groups[group]
        description = self.group_descriptions[group]
        return "\n".join(
            [
                f"usage: python3 -m scripts.rebof3 {group} <command> [args...]",
                "",
                f"{group}:",
                f"  {description}",
                "",
                "commands:",
                *[f"  {command}" for command in commands],
            ]
        )

    def resolve(self, args: list[str]) -> tuple[str | None, list[str], str | None]:
        if not args:
            return None, [], None
        if args[0] in self.group_modules and len(args) == 1:
            return self.group_modules[args[0]], ["--help"], None
        if args[0] in self.group_modules and args[1] in {"-h", "--help", "help"}:
            return self.group_modules[args[0]], ["--help"], None
        if len(args) < 2:
            return None, [], f"unknown command: {' '.join(args)}".strip()
        key = (args[0], args[1])
        spec = next(
            (
                candidate
                for candidate in self.command_specs
                if (candidate.group, candidate.command) == key
            ),
            None,
        )
        if spec is not None:
            forwarded_args = list(args[2:])
            if spec.forwarded_mode is not None:
                forwarded_args = [spec.forwarded_mode, *forwarded_args]
            return spec.module_name, forwarded_args, None
        if args[0] in self.group_modules:
            if args[1] not in self.command_groups.get(args[0], []):
                return None, [], f"unknown command: {args[0]} {args[1]}"
            return self.group_modules[args[0]], list(args[1:]), None
        return None, [], f"unknown command: {args[0]} {args[1]}"


# Keep this entrypoint limited to stable user-facing commands.
# Internal plumbing commands remain importable directly from their modules.
PUBLIC_COMMAND_SPECS = (
    CommandSpec("stubs", "sync", "scripts.rebof3.stubs.sync"),
    CommandSpec(
        "match",
        "init",
        "scripts.rebof3.match.workspace",
    ),
    CommandSpec("match", "target", "scripts.rebof3.match.target_cmd"),
    CommandSpec("match", "repair-wave", "scripts.rebof3.match.repair_wave"),
    CommandSpec("match", "seed-wave", "scripts.rebof3.match.seed_wave"),
    CommandSpec("match", "promote-wave", "scripts.rebof3.match.promote_wave"),
    CommandSpec("match", "frontier-backlog", "scripts.rebof3.match.frontier_backlog"),
    CommandSpec("match", "import-wave", "scripts.rebof3.match.import_wave"),
    CommandSpec("match", "import-backlog", "scripts.rebof3.match.import_backlog"),
    CommandSpec("match", "refresh", "scripts.rebof3.match.refresh"),
    CommandSpec("match", "enhanced-report", "scripts.rebof3.match.enhanced_report"),
    CommandSpec("match", "report", "scripts.rebof3.match.report"),
    CommandSpec("match", "scaffold", "scripts.rebof3.match.scaffold"),
    CommandSpec("match", "candidate-prepare", "scripts.rebof3.match.candidate_prepare"),
    CommandSpec("match", "candidate-build", "scripts.rebof3.match.candidate_build"),
    CommandSpec("match", "candidate-full", "scripts.rebof3.match.candidate_full"),
    CommandSpec("match", "asm-patch", "scripts.rebof3.match.expected_asm_patch"),
    CommandSpec("match", "scoreboard", "scripts.rebof3.match.scoreboard"),
    CommandSpec("match", "semantic-diff", "scripts.rebof3.match.semantic_diff"),
    CommandSpec("match", "status", "scripts.rebof3.match.status"),
    CommandSpec("match", "compiler-report", "scripts.rebof3.match.compiler_report"),
    CommandSpec(
        "match",
        "workspace-init",
        "scripts.rebof3.match.workspace",
        forwarded_mode="__workspace_init_compat__",
    ),
    CommandSpec("match", "build", "scripts.rebof3.match.build"),
    CommandSpec("match", "view", "scripts.rebof3.match.view"),
    CommandSpec("match", "diff", "scripts.rebof3.match.diff"),
    CommandSpec("match", "permuter", "scripts.rebof3.match.permuter"),
    CommandSpec("match", "sweep", "scripts.rebof3.match.sweep"),
    CommandSpec("re", "doctor", "scripts.rebof3.re.commands.doctor"),
    CommandSpec("re", "ghidra-decomp", "scripts.rebof3.re.commands.ghidra_decomp"),
    CommandSpec("re", "pipeline-decomp", "scripts.rebof3.re.commands.pipeline_decomp"),
    CommandSpec(
        "re",
        "bootstrap",
        "scripts.rebof3.re.commands.bootstrap",
    ),
    CommandSpec("re", "setup-psn00bsdk-toolchain", "scripts.rebof3.toolchain.psn00b"),
    CommandSpec("re", "setup-old-gcc", "scripts.rebof3.toolchain.old_gcc"),
    CommandSpec("re", "setup-psyq-40", "scripts.rebof3.toolchain.psyq40"),
    CommandSpec("re", "metadata", "scripts.rebof3.re.commands.metadata"),
)

PUBLIC_COMMAND_MODULES = {
    ("stubs", "sync"): "scripts.rebof3.stubs.sync",
    ("match", "init"): "scripts.rebof3.match.workspace",
    ("match", "target"): "scripts.rebof3.match.target_cmd",
    ("match", "repair-wave"): "scripts.rebof3.match.repair_wave",
    ("match", "seed-wave"): "scripts.rebof3.match.seed_wave",
    ("match", "promote-wave"): "scripts.rebof3.match.promote_wave",
    ("match", "frontier-backlog"): "scripts.rebof3.match.frontier_backlog",
    ("match", "import-wave"): "scripts.rebof3.match.import_wave",
    ("match", "import-backlog"): "scripts.rebof3.match.import_backlog",
    ("match", "refresh"): "scripts.rebof3.match.refresh",
    ("match", "enhanced-report"): "scripts.rebof3.match.enhanced_report",
    ("match", "report"): "scripts.rebof3.match.report",
    ("match", "scaffold"): "scripts.rebof3.match.scaffold",
    ("match", "candidate-prepare"): "scripts.rebof3.match.candidate_prepare",
    ("match", "candidate-build"): "scripts.rebof3.match.candidate_build",
    ("match", "candidate-full"): "scripts.rebof3.match.candidate_full",
    ("match", "asm-patch"): "scripts.rebof3.match.expected_asm_patch",
    ("match", "scoreboard"): "scripts.rebof3.match.scoreboard",
    ("match", "semantic-diff"): "scripts.rebof3.match.semantic_diff",
    ("match", "status"): "scripts.rebof3.match.status",
    ("match", "compiler-report"): "scripts.rebof3.match.compiler_report",
    ("match", "workspace-init"): "scripts.rebof3.match.workspace",
    ("match", "build"): "scripts.rebof3.match.build",
    ("match", "view"): "scripts.rebof3.match.view",
    ("match", "diff"): "scripts.rebof3.match.diff",
    ("match", "permuter"): "scripts.rebof3.match.permuter",
    ("match", "sweep"): "scripts.rebof3.match.sweep",
    ("re", "doctor"): "scripts.rebof3.re.commands.doctor",
    ("re", "ghidra-decomp"): "scripts.rebof3.re.commands.ghidra_decomp",
    ("re", "pipeline-decomp"): "scripts.rebof3.re.commands.pipeline_decomp",
    (
        "re",
        "bootstrap",
    ): "scripts.rebof3.re.commands.bootstrap",
    ("re", "setup-psn00bsdk-toolchain"): "scripts.rebof3.toolchain.psn00b",
    ("re", "setup-old-gcc"): "scripts.rebof3.toolchain.old_gcc",
    ("re", "setup-psyq-40"): "scripts.rebof3.toolchain.psyq40",
    ("re", "metadata"): "scripts.rebof3.re.commands.metadata",
}

PUBLIC_GROUP_MODULES = {
    "inventory": "scripts.rebof3.inventory.inventory",
}

PUBLIC_COMMAND_GROUPS = {
    "stubs": ["sync"],
    "match": [
        "init",
        "target",
        "build",
        "diff",
        "view",
        "permuter",
        "refresh",
        "enhanced-report",
        "report",
        "scaffold",
        "candidate-prepare",
        "candidate-build",
        "candidate-full",
        "asm-patch",
        "status",
        "scoreboard",
        "semantic-diff",
        "compiler-report",
        "sweep",
        "frontier-backlog",
        "import-backlog",
        "import-wave",
        "promote-wave",
        "repair-wave",
        "seed-wave",
    ],
    "re": [
        "doctor",
        "ghidra-decomp",
        "pipeline-decomp",
        "metadata",
        "bootstrap",
        "setup-psn00bsdk-toolchain",
        "setup-old-gcc",
        "setup-psyq-40",
    ],
    "inventory": [
        "build",
        "slot-map",
        "emi-catalog",
        "overlay-catalog",
        "overlay-clusters",
        "unique-overlay-map",
        "overlay-entry-tables",
        "ghidra-symbols",
    ],
}

GROUP_DESCRIPTIONS = {
    "stubs": "manage disabled stub files before promotion into live source",
    "match": "create and inspect one function-matching workspace",
    "re": "run reverse-engineering helpers and exports",
    "inventory": "build machine-generated BOF3 catalogs used by extraction and decomp",
}

COMMAND_REGISTRY = CommandRegistry(
    command_specs=PUBLIC_COMMAND_SPECS,
    group_modules=PUBLIC_GROUP_MODULES,
    command_groups=PUBLIC_COMMAND_GROUPS,
    group_descriptions=GROUP_DESCRIPTIONS,
    hidden_groups={"inventory"},
)


def render_help() -> str:
    return COMMAND_REGISTRY.render_help()


def render_group_help(group: str) -> str:
    return COMMAND_REGISTRY.render_group_help(group)


def main(argv: list[str] | None = None) -> int:
    args = list(sys.argv[1:] if argv is None else argv)
    if not args or args[0] in {"-h", "--help", "help"}:
        print(render_help())
        return 0
    if args[0] in COMMAND_REGISTRY.command_groups and (
        len(args) == 1 or args[1] in {"-h", "--help", "help"}
    ):
        print(render_group_help(args[0]))
        return 0
    if len(args) < 2:
        print(render_help(), file=sys.stderr)
        return 1
    module_name, forwarded_args, error = COMMAND_REGISTRY.resolve(args)
    if module_name is None:
        print(error or render_help(), file=sys.stderr)
        return 1
    module = importlib.import_module(module_name)
    return int(module.main(forwarded_args))


if __name__ == "__main__":
    raise SystemExit(main())
