---
name: bof3-docs
description: Project documentation for BOF3 reverse engineering. Use when you need to understand the project structure, setup steps, specs, or troubleshooting. Loads the relevant doc file from docs/ on demand.
---

## How to use
Read the doc you need with the Read tool. Don't pre-load all docs.

## Doc index

### Project
- `docs/README.md` — docs overview
- `README.md` — shortest setup and reversing path
- `CONTEXT.md` — canonical binary and EMI terminology

### Setup & Troubleshooting
- `docs/setup.md` — toolchains, PsyQ, extraction, and verification
- `docs/troubleshooting.md` — doctor, build, Ghidra, and extraction failures

### Reverse Specs
- `docs/specs/index.md` — entry point for reverse-engineering knowledge
- `docs/specs/formats/emi.md` — EMI container and entry layout
- `docs/specs/runtime/runtime-layout.md` — executable and overlay model
- `docs/specs/runtime/emi-loader.md` — payload dispatch
- `docs/specs/runtime/module-map.md` — confirmed executable targets
- `docs/specs/recovered-layouts.md` — evidenced structure offsets
- `docs/specs/assets/index.md` — archive-family roles

### Workflow
- `docs/reverse-engineering.md` — repeatable target and function loop
- `docs/matching.md` — function matching and result interpretation
- `docs/tools.md` — supported tool roles and evidence authority
- `.agents/skills/decomp-loop/SKILL.md` — function lifting and matching
- `.agents/rules/decomp.md` — C89 coding conventions (always-loaded rules)

## When to read each
- **Setup environment?** → `docs/setup.md`
- **Build/diff failing?** → `docs/troubleshooting.md`
- **Understanding target identity?** → `CONTEXT.md`, `docs/specs/runtime/runtime-layout.md`
- **What's verified?** → `docs/specs/index.md`, then the owning compact spec
- **Working on decomp?** → `docs/reverse-engineering.md`, `docs/matching.md`, `.agents/skills/decomp-loop/SKILL.md`
- **Binary format questions?** → `docs/specs/formats/`
- **Coding conventions?** → `.agents/rules/decomp.md`
