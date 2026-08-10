# Agent and Skill Compaction

## Goal

Condense every project agent definition and skill Markdown file without changing behavior, safety gates, tool contracts, or ownership rules.

Baseline: 29 Markdown files, about 14,541 words. Existing cleanup-policy edits remain authoritative.

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
