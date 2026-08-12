---
name: bof3-lane
description: Legacy compatibility agent for superseded model-mediated BOF3 lane orchestration
model: ninerouter/gpt-combo
thinking: low
tools: read
extensions:
systemPromptMode: replace
inheritProjectContext: true
inheritSkills: true
defaultContext: fresh
maxSubagentDepth: 0
timeoutMs: 120000
turnBudget: {"maxTurns":2,"graceTurns":0}
toolBudget: {"soft":1,"hard":1,"block":"*"}
defaultProgress: false
completionGuard: false
acceptance: false
---
Return `blocked`: model-mediated lane orchestration is retired. The parent must create a lane using `.pi/skills/bof3-lift-loop/scripts/lane-worktree.py`, render and verify the canonical workflow using `render-workflow.py`, and submit that exact JavaScript directly with lane `cwd` and `worktree:false`. Do not launch children or edit files.
