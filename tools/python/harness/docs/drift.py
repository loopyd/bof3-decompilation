"""Check dialogue documentation against code and CLI evidence without rewriting."""

from __future__ import annotations

import re
from pathlib import Path

from .claims import (
    SPEC,
    TOOL_DOCS,
    MODELS,
    HARNESS_CLI,
    GLOBAL_FLAGS,
    BINARY,
    EXTRA_RETIRED,
    RETIRED,
    REMOVAL,
    STALE_INDEX_RULES,
    STALE_INDEX_RECORD,
    BEHAVIOUR_CLAIMS,
    BEHAVIOUR_RECORD,
    SUPERSEDED_COUNTS,
    SUPERSEDED_RECORD,
    FILTER_CLAIMS,
    FILTER_CLASS_CLAIMS,
    BACKTICK_PATH,
)
from .inspection import (
    _read,
    _evidence,
    _text_class_variants,
    _command_classes,
    _spec_command_rows,
    _mode_help,
    _harness_actions,
    _harness_examples,
    _cli_modes,
)


def collect_drift(root: Path) -> list[dict[str, str]]:
    findings: list[dict[str, str]] = []

    def note(kind: str, where: str, detail: str) -> None:
        findings.append({"rule": kind, "where": where, "detail": detail})

    models = _read(root, MODELS)
    spec = _read(root, SPEC)
    if models is None or spec is None:
        note("missing", SPEC if spec is None else MODELS, "required file is absent")
        return findings

    documents = {
        name: text for name in (SPEC,) + TOOL_DOCS if (text := _read(root, name))
    }
    evidence = _evidence(root)
    for name in EXTRA_RETIRED:
        if (text := _read(root, name)) is not None:
            evidence[name] = text

    # Rule 0: functional text logic must not return to the harness. The crate owns every text
    # behaviour; the harness resolves the binary, forwards arguments, holds examples and passes the
    # exit code through. A second text module, or a functional marker in the one that exists, is a
    # second owner.
    harness_commands = root / "tools" / "python" / "harness" / "commands"
    for path in sorted(harness_commands.glob("text*.py")):
        relative = path.relative_to(root).as_posix()
        if path.name != "text.py":
            note(
                "harness-functional",
                relative,
                "a second text module appeared: functional text behaviour belongs to the crate",
            )
            continue
        body = path.read_text(encoding="utf-8")
        for marker in (
            "hashlib",
            "json.load",
            "re.compile",
            "body_record",
            "_parse_windows",
            "_identified_payloads",
        ):
            if marker in body:
                note(
                    "harness-functional",
                    relative,
                    f"functional text logic ({marker}) is implemented in the harness; it belongs in the crate",
                )

    # Rule 1: retired-codec claims must not survive anywhere in the documentation or
    # in the retained evidence.
    for name, text in {**documents, **evidence}.items():
        lines = text.splitlines()
        for index, line in enumerate(lines):
            window = "\n".join(lines[max(0, index - 1) : index + 2])
            if SUPERSEDED_COUNTS.search(line):
                if not SUPERSEDED_RECORD.search(window):
                    note(
                        "superseded-count",
                        f"{name}:{index + 1}",
                        f"carries a superseded measurement without saying so: {line.strip()[:80]}",
                    )
                continue
            if FILTER_CLASS_CLAIMS.search(line):
                note(
                    "filter-class-claim",
                    f"{name}:{index + 1}",
                    "filter contracts are declared in models.rs and every implementation lives in "
                    f"filters.rs, with Readable implementing PostFilter: {line.strip()[:80]}",
                )
                continue
            if any(pattern.search(line) for pattern, _ in FILTER_CLAIMS):
                for pattern, why in FILTER_CLAIMS:
                    if pattern.search(line):
                        note(
                            "filter-claim",
                            f"{name}:{index + 1}",
                            f"{why}: {line.strip()[:90]}",
                        )
                continue
            if any(pattern.search(line) for pattern, _ in BEHAVIOUR_CLAIMS):
                if BEHAVIOUR_RECORD.search(window):
                    continue
                for pattern, why in BEHAVIOUR_CLAIMS:
                    if pattern.search(line):
                        note(
                            "behaviour-claim",
                            f"{name}:{index + 1}",
                            f"{why}: {line.strip()[:90]}",
                        )
                continue
            if any(pattern.search(line) for pattern, _ in STALE_INDEX_RULES):
                if STALE_INDEX_RECORD.search(window):
                    continue
                for pattern, why in STALE_INDEX_RULES:
                    if pattern.search(line):
                        note(
                            "stale-index-claim",
                            f"{name}:{index + 1}",
                            f"{why}: {line.strip()[:90]}",
                        )
                continue
            if REMOVAL.search(window):
                continue
            for pattern, why in RETIRED:
                if pattern.search(line):
                    note(
                        "retired-claim",
                        f"{name}:{index + 1}",
                        f"{why}: {line.strip()[:90]}",
                    )

    # Rule 2: every command class in code must be in the spec, with the same token
    # and the same operand count where the spec states a plain number.
    spec_rows = _spec_command_rows(documents.get(SPEC, ""))
    for code, (token, klass, operands) in sorted(_command_classes(models).items()):
        row = spec_rows.get(code)
        if row is None:
            note(
                "command-missing",
                SPEC,
                f"{code} ({klass}/{token}) is in code but not in the spec table",
            )
            continue
        spec_token, cell = row
        # The spec writes the token with its operand placeholders (`{name(0xNN)}`);
        # the code names the class only (`name`). Compare the names themselves.
        spec_name = re.sub(r"\(.*\)$", "", spec_token.strip("{}"))
        if spec_name != token:
            note(
                "command-token",
                SPEC,
                f"{code}: code says {token}, spec says {spec_token}",
            )
        if cell.isdigit() and int(cell) != operands:
            note(
                "command-operands",
                SPEC,
                f"{code} {token}: code says {operands}, spec says {cell}",
            )

    # Rule 3: every code command class must be exported in COMMAND_CLASSES.
    for code, (_, klass, _) in _command_classes(models).items():
        if f"{klass}::CODE" not in models:
            note(
                "command-registry",
                MODELS,
                f"{klass} ({code}) is defined but not registered",
            )

    # Rule 4: every text class in code must be documented.
    spec_and_docs = "\n".join(documents.values())
    for variant in _text_class_variants(models):
        if variant.lower() not in spec_and_docs.lower():
            note(
                "class-undocumented",
                SPEC,
                f"text class {variant} is in code but in no document",
            )

    # Rule 5: documented CLI modes must exist in the built binary.
    modes = _cli_modes(root)
    commands_doc = documents.get("tools/rust/bof3-text/docs/commands.md", "")
    if modes is None:
        note(
            "cli-unbuilt",
            BINARY,
            "binary absent, so documented modes could not be checked",
        )
    else:
        for mode in sorted(modes):
            if f"`{mode} " not in commands_doc and f"`{mode}`" not in commands_doc:
                note(
                    "mode-undocumented",
                    TOOL_DOCS[2],
                    f"CLI mode {mode} exists but is not documented",
                )
        for guess in re.findall(r"^## `?([a-z][a-z-]+)", commands_doc, re.M):
            if guess not in modes:
                note(
                    "mode-absent",
                    TOOL_DOCS[2],
                    f"commands.md documents {guess}, which the binary does not offer",
                )

    # Rule 7: every mode the binary offers must be reachable through the harness, with a
    # canonical example. This is the coverage hole that let four modes ship unreachable from
    # `bin/harness text` while this check still reported zero findings.
    if modes is not None:
        actions = _harness_actions(root)
        examples = _harness_examples(root)
        for mode in sorted(modes):
            if mode not in actions:
                note(
                    "mode-unreachable",
                    HARNESS_CLI,
                    f"the binary offers `{mode}` but the harness forwards no `text {mode}` action",
                )
            elif mode not in examples:
                note(
                    "example-missing",
                    HARNESS_CLI,
                    f"the harness forwards `{mode}` but carries no example for it",
                )
        for action in sorted(name for name, forwards in actions.items() if forwards):
            if action not in modes:
                note(
                    "action-absent",
                    HARNESS_CLI,
                    f"the harness offers passthrough `text {action}`, which the binary does not",
                )

    # Rule 8: a mode's own long flags must be named in its section of commands.md. Global
    # flags are documented once in the global table.
    commands_doc = documents.get("tools/rust/bof3-text/docs/commands.md", "")
    for mode in sorted(modes or []):
        section = re.search(
            rf"^## `{re.escape(mode)}\b.*?(?=^## |\Z)", commands_doc, re.M | re.S
        )
        if section is None:
            continue  # an undocumented mode is already reported by rule 5
        for flag in sorted(
            set(re.findall(r"--[a-z][a-z-]*", _mode_help(root, mode))) - GLOBAL_FLAGS
        ):
            if flag not in section.group(0):
                note(
                    "flag-undocumented",
                    "tools/rust/bof3-text/docs/commands.md",
                    f"`{mode}` offers {flag}, which its section does not mention",
                )

    # Rule 9: every `bin/harness text <mode>` reference in the agent tool guide must name a mode
    # the binary actually offers, so that guide cannot advertise a mode that does not exist.
    if modes is not None:
        for name in EXTRA_RETIRED:
            text = _read(root, name)
            if text is None:
                continue
            actions = _harness_actions(root)
            for named in sorted(
                set(re.findall(r"bin/harness text\s+([a-z][a-z-]*)", text))
            ):
                # A harness-owned action (the window registry) has no binary mode by design.
                if actions.get(named) is False:
                    continue
                if named not in modes:
                    note(
                        "mode-unknown",
                        name,
                        f"names `bin/harness text {named}`, which the binary does not offer",
                    )

    # Rule 6: referenced repository paths in the documentation must exist.
    for name, text in documents.items():
        for target in sorted(set(BACKTICK_PATH.findall(text))):
            if any(character in target for character in "*?") or target.endswith("/"):
                continue
            if not (root / target).exists():
                note(
                    "dead-reference", name, f"references {target}, which does not exist"
                )

    return findings

