# BOF3 lift loop

Serial, review-gated batch lifting: select a candidate, send one brief to the
executor, live byte-match, independent review, then commit only user-authorized
exact passes. Parent owns git/checkpoints; subagents never commit or run setup.

Use `/skill:bof3-lift-loop` for a batch; use `/skill:bof3-re` for one hand-guided
function. Prerequisite: fresh target snapshot/index before queue selection.

```
pick → brief → executor → live byte-match → reviewer → commit iff exact+pass
```

Serial per target: functions share `internal.h`. Refresh map/Splat-induced Rizin
staleness at batch checkpoints, never per function. See `SKILL.md`.
