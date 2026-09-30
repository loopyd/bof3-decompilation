"""Canonical ``bin/harness`` domain/action command tree.

The ``bin/harness`` shell entry point only bootstraps the project Python
environment.  This module validates the requested domain/action and forwards
the remaining arguments unchanged to the owning policy module, which keeps its
own parser, defaults, output destination and exit status.  It is dispatch only:
no domain policy lives here.

A domain with a pass-through owner (for example ``naming``) forwards unknown
actions to that owner, so its native subcommands and ``--help`` stay reachable
without duplicating its parser here.
"""

from __future__ import annotations

import sys
from .common.dispatch import Argv, Domain
from .registry import DOMAINS


def root_help() -> str:
    lines = [
        "usage: bin/harness <domain> <action> [options]",
        "",
        "domains and actions:",
    ]
    for name, domain in DOMAINS.items():
        names = list(domain.actions)
        if domain.passthrough is not None:
            names.append(
                "<args>" if domain.passthrough.kind == "passthrough" else "<selector>"
            )
        lines.append(f"  {name:<10} {' '.join(names)}")
    lines += [
        "",
        "run `bin/harness <domain> --help` for a domain's actions and",
        "`bin/harness <domain> <action> --help` for the owner's options.",
    ]
    return "\n".join(lines)


def domain_help(name: str, domain: Domain) -> str:
    lines = [f"usage: bin/harness {name} <action> [options]", "", f"{domain.help}:", ""]
    for action_name, action in domain.actions.items():
        lines.append(f"  {action_name:<16} {action.help}")
        if action.example:
            lines.append(f"  {'':<16}   {action.example}")
    if domain.passthrough is not None:
        lines.append(f"  {'<native>':<16} {domain.passthrough.help}")
    return "\n".join(lines)


def domain_example(name: str, domain: Domain) -> str | None:
    if domain.passthrough is not None and domain.passthrough.example:
        return domain.passthrough.example
    for action in domain.actions.values():
        if action.example:
            return action.example
    return None


def main(argv: Argv | None = None) -> int:
    args = sys.argv[1:] if argv is None else list(argv)
    if not args or args[0] in {"-h", "--help"}:
        print(root_help())
        return 0
    if args[0] == "--example":
        print("bin/harness lift asm-diff exe/logo@0x801CE758 --detail normal")
        print("bin/harness build exe/logo@0x801CE758")
        return 0
    domain_name = args[0]
    domain = DOMAINS.get(domain_name)
    if domain is None:
        print(f"bin/harness: unknown domain: {domain_name}", file=sys.stderr)
        print(root_help(), file=sys.stderr)
        return 2
    rest = args[1:]
    if not rest:
        # A pass-through domain keeps its owner's no-argument default (for
        # example `build` builds every lift); others report a usage error.
        if domain.passthrough is not None:
            return domain.passthrough.run([])
        print(domain_help(domain_name, domain), file=sys.stderr)
        return 2
    if rest[0] in {"-h", "--help"}:
        if domain.passthrough is not None:
            return domain.passthrough.run(["--help"])
        print(domain_help(domain_name, domain))
        return 0
    if rest[0] == "--example":
        example = domain_example(domain_name, domain)
        if example is None:
            print(
                f"bin/harness {domain_name}: no example for this domain",
                file=sys.stderr,
            )
            return 2
        print(example)
        return 0
    action = domain.actions.get(rest[0]) or domain.passthrough
    if action is None:
        print(
            f"bin/harness {domain_name}: unknown action: {rest[0]}",
            file=sys.stderr,
        )
        print(domain_help(domain_name, domain), file=sys.stderr)
        return 2
    if action.example and rest[1:] == ["--example"]:
        #: The registry example is authoritative for the command tree, so a
        #: native forwarder never receives ``--example`` and enters its own
        #: prompt.  Owners keep their own flag for standalone use.
        print(action.example)
        return 0
    return action.run(rest)


if __name__ == "__main__":
    raise SystemExit(main())
