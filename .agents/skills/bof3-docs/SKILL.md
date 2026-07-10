---
name: bof3-docs
description: Project documentation for BOF3 reverse engineering. Use when you need to understand the project structure, setup steps, specs, or troubleshooting. Loads the relevant doc file from docs/ on demand.
---

## How to use
Read the doc you need with the Read tool. Don't pre-load all docs.

## Doc index

### Project
- `docs/README.md` — docs overview
- `docs/REPO_LAYOUT.md` — canonical directory names and file layout
- `docs/plan.md` — living migration plan (completed, in-progress, next)
- `docs/specs/status.md` — current project frontier

### Setup & Troubleshooting
- `docs/SETUP.md` — setup steps: toolchains, PsyQ, submodules, extraction, Ghidra bootstrap, configure, build
- `docs/TROUBLESHOOTING.md` — common issues: doctor failures, build errors, Ghidra, extraction

### Reverse Specs
- `docs/specs/index.md` — entry point for reverse-engineering knowledge
- `docs/specs/glossary.md` — shared terms across specs (EMI, overlay, slot table, etc.)
- `docs/specs/status.md` — per-module reverse status
- `docs/specs/runtime/` — runtime behavior specs
- `docs/specs/formats/` — binary format specs (EMI, PSX-EXE, etc.)
- `docs/specs/sources/` — known source patterns and idioms
- `docs/specs/content/` — game content analysis

### Workflow
- `docs/DECOMP_WORKFLOW.md` — repeatable decomp loop
- `.agents/skills/harness/SKILL.md` — harness workflow
- `.agents/rules/decomp.md` — C89 coding conventions (always-loaded rules)

## When to read each
- **Setup environment?** → `docs/SETUP.md`
- **Build/diff failing?** → `docs/TROUBLESHOOTING.md`
- **Understanding code structure?** → `docs/REPO_LAYOUT.md`, `docs/specs/glossary.md`
- **What's been done?** → `docs/plan.md`, `docs/specs/status.md`
- **Working on decomp?** → `docs/DECOMP_WORKFLOW.md`
- **Binary format questions?** → `docs/specs/formats/`
- **Harness workflow?** → `.agents/skills/harness/SKILL.md`
- **Coding conventions?** → `.agents/rules/decomp.md`
