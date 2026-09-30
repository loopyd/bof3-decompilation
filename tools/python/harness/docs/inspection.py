"""Inspect dialogue code, CLI declarations and retained evidence for drift."""

from __future__ import annotations

import ast
import re
from pathlib import Path

from .claims import (
    HARNESS_CLI,
    BINARY,
    EVIDENCE_DIR,
    EVIDENCE_EXEMPT,
    COMMAND_CLASS,
    ENUM_VARIANT,
    SPEC_COMMAND_ROW,
)


def _read(root: Path, relative: str) -> str | None:
    path = root / relative
    return path.read_text(encoding="utf-8") if path.is_file() else None


def _evidence(root: Path) -> dict[str, str]:
    """Retained evidence documents, minus the one that documents this check."""
    directory = root / EVIDENCE_DIR
    if not directory.is_dir():
        return {}
    out: dict[str, str] = {}
    for path in sorted(directory.glob("*.md")):
        relative = path.relative_to(root).as_posix()
        if relative not in EVIDENCE_EXEMPT:
            out[relative] = path.read_text(encoding="utf-8")
    return out


def _text_class_variants(models: str) -> list[str]:
    start = models.find("pub enum TextClass")
    if start < 0:
        return []
    end = models.find("\n}", start)
    return ENUM_VARIANT.findall(models[start:end])


def _command_classes(models: str) -> dict[str, tuple[str, str, int]]:
    """code -> (token, class name, operand count) as the code defines them."""
    out: dict[str, tuple[str, str, int]] = {}
    for name, code, token, operands in COMMAND_CLASS.findall(models):
        out[code.replace("_", "").lower()] = (token, name, int(operands))
    return out


def _spec_command_rows(spec: str) -> dict[str, tuple[str, str]]:
    """code -> (token, operand-cell) as the specification claims them."""
    return {
        code.lower(): (token, cell)
        for code, token, cell in SPEC_COMMAND_ROW.findall(spec)
        if token.startswith("{")
    }


def _mode_help(root: Path, mode: str) -> str:
    """The `--help` text of one mode, or an empty string when it cannot be read."""
    import subprocess

    binary = root / BINARY
    if not binary.is_file():
        return ""
    result = subprocess.run(
        [str(binary), mode, "--help"], capture_output=True, text=True, check=False
    )
    return result.stdout


def _text_actions(root: Path) -> tuple[dict[str, ast.Call], dict[str, str]]:
    """Read the owning declaration syntax without importing checkout code."""
    tree = ast.parse(_read(root, HARNESS_CLI) or "", filename=HARNESS_CLI)
    constants = {}
    for statement in tree.body:
        if isinstance(statement, ast.Assign) and isinstance(
            statement.value, ast.Constant
        ):
            for target in statement.targets:
                if isinstance(target, ast.Name) and isinstance(
                    statement.value.value, str
                ):
                    constants[target.id] = statement.value.value
    for node in ast.walk(tree):
        if not isinstance(node, ast.Dict):
            continue
        for key, domain in zip(node.keys, node.values):
            if (
                not isinstance(key, ast.Constant)
                or key.value != "text"
                or not isinstance(domain, ast.Call)
            ):
                continue
            if not isinstance(domain.func, ast.Name) or domain.func.id != "Domain":
                continue
            actions = next(
                (item.value for item in domain.keywords if item.arg == "actions"), None
            )
            if isinstance(actions, ast.Dict):
                return {
                    key.value: action
                    for key, action in zip(actions.keys, actions.values)
                    if isinstance(key, ast.Constant)
                    and isinstance(key.value, str)
                    and isinstance(action, ast.Call)
                    and isinstance(action.func, ast.Name)
                    and action.func.id == "Action"
                }, constants
    return {}, constants


def _harness_actions(root: Path) -> dict[str, bool]:
    """Map declared text actions to their passthrough behavior."""
    actions, _ = _text_actions(root)
    return {
        name: any(
            item.arg == "kind"
            and isinstance(item.value, ast.Constant)
            and item.value.value == "passthrough"
            for item in action.keywords
        )
        for name, action in actions.items()
    }


def _harness_examples(root: Path) -> set[str]:
    """Every action the harness carries a canonical example for."""
    actions, constants = _text_actions(root)
    examples = set()
    for name, action in actions.items():
        for item in action.keywords:
            if item.arg != "example":
                continue
            value = item.value
            text = (
                value.value
                if isinstance(value, ast.Constant)
                else constants.get(value.id)
                if isinstance(value, ast.Name)
                else None
            )
            if isinstance(text, str) and text.strip():
                examples.add(name)
    return examples


def _cli_modes(root: Path) -> set[str] | None:
    import subprocess

    binary = root / BINARY
    if not binary.is_file():
        return None
    result = subprocess.run(
        [str(binary), "--help"], capture_output=True, text=True, check=False
    )
    if result.returncode != 0:
        return None
    modes: set[str] = set()
    for line in result.stdout.splitlines():
        match = re.match(r"^  ([a-z][a-z-]+)\b", line)
        if match and match.group(1) not in {
            "usage",
            "options",
            "arguments",
            "commands",
            "help",
        }:
            modes.add(match.group(1))
    return modes
