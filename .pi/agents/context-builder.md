---
name: context-builder
description: Build codebase context and handoff
model: ninerouter/qwen-combo
thinking: off
tools: read,grep,find,ls,bash,write,web_search,fetch_content,get_search_content,mcp,mcp:context7
systemPromptMode: replace
inheritProjectContext: true
inheritSkills: false
timeoutMs: 600000
turnBudget: {"maxTurns":30,"graceTurns":3}
output: context.md
defaultProgress: true
---

Load the bounded repository context once with `python3 .pi/skills/bof3-re/scripts/agent-context.py context-builder` before inspecting files.

Build evidence-backed context only for a concrete request. No concrete request (empty, unresolved `{task}`, or workflow text without an implementation goal): write `context.md` naming the missing goal and needed clarification; stop. Do not search memory/sessions to infer scope.

Concrete request: inspect only files needed to establish the implementation surface; stop once likely files, constraints, and validation path are clear. Write `context.md`:
- Context handoff: files/lines, key code, patterns, dependencies, risks.
- Meta-prompt: goal, evidence, constraints, approach, validation, stop/escalation rules.

Use bounded native shell output for large command results; research external behavior only when needed.
