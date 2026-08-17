# Agent and Skill Compaction

## Goal

Condense every project agent definition and skill Markdown file without changing behavior, safety gates, tool contracts, or ownership rules.

Baseline: 29 Markdown files, about 14,541 words. Existing cleanup-policy edits remain authoritative.

## Status (active, incomplete)

- [x] Phase 1–4 structure defined; targeted contract fixes applied in the
      documentation-refresh effort.
- [x] Absolute gate 1: `.pi` context files ≤ **69,000 bytes** (enforced by
      `tools/python/tests/test_bof3_lift_loop_acceptance.py::test_agent_and_skill_context_files_stay_compact`,
      measuring `.pi/agents/*.md` + `.pi/skills/*/SKILL.md` +
      `.pi/skills/bof3-re/references/*/*.md`). Current 68,973 (measured
      2026-08-16; margin 27); `pytest` gate passes.
- [x] Absolute gate 2: `python3 .pi/skills/bof3-re/scripts/test-skill-scripts.py`
      passes (exit 0, all cases ok). Full selector payloads for
      `emi/battle/battle/15@0x80096E90` are below the 100,000-byte ceiling:
      reverse 99,960 / review 97,827 / agents 71,728 (measured 2026-08-16).
      Static-prefix contexts before the selector-evidence section: reverse
      77,432 / review 75,299 (defined as the byte offset of the first
      `===== config/targets/...` marker; selector evidence adds 22,528 bytes).
- [ ] Independent semantic review confirms no dropped/weakened contract, link,
      command, selector, path, address, or front-matter field.

## Style

```text
long prose -> short rules -> decision diagram -> command table
```

- Prefer terse verbs and fragments.
- Drop implied subjects and repeated context.
- Use Mermaid only for branching logic or relationships.
- Keep commands, paths, selectors, schemas, front matter, and hard prohibitions exact.
- Preserve every normative requirement; remove examples only when a rule or table fully replaces them.

## Phases

1. **Agent definitions**
   - Compact `.pi/agents/*.md`.
   - Preserve front matter and execution contracts.
2. **BOF3 skills**
   - Compact `.pi/skills/bof3-lift-loop/` and `.pi/skills/bof3-re/` Markdown.
   - Preserve cleanup, reverse, review, matching, and validation gates.
3. **PSX Rizin skill**
   - Compact `.pi/skills/psx-rizin/**/*.md`.
   - Preserve provenance, address, analysis, runtime, and evidence rules.
4. **Cross-file review**
   - Check links, commands, policy terms, front matter, and generated ordered context.
   - Compare word/byte totals against baseline.

## Validation

```sh
python3 .pi/skills/bof3-re/scripts/agent-context.py agents
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=tools/python .venv/bin/python -m pytest -q -p no:cacheprovider tools/python/tests/test_bof3_cleanup_agent.py
git diff --check
```

Required absolute gates (both must pass before this plan closes):

```sh
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=tools/python .venv/bin/python -m pytest -q -p no:cacheprovider tools/python/tests/test_bof3_lift_loop_acceptance.py::test_agent_and_skill_context_files_stay_compact
python3 .pi/skills/bof3-re/scripts/test-skill-scripts.py
```

Also verify:

- all Markdown links resolve;
- every agent front matter remains valid;
- required command strings and selector forms remain present;
- independent review finds no dropped or weakened contract.

## Boundaries

- No code, config, target metadata, lift, or generated-state edits.
- No semantic policy changes.
- No commit, stage, push, or external mutation.
- Existing unrelated worktree changes stay untouched.
