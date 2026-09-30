---
name: bof3-test
description: Resolve BOF3 pytest ownership, run scoped/unit/full gates, or apply reviewed test-suite optimizations. Owns tests, test recipes and pytest configuration; no production behavior changes.
---

# BOF3 test units

For file/module ownership or unit selection, read [unit map](references/unit-map.md)
before resolving scope; use known mappings rather than rescanning. Unmapped files
are gaps, not guessed ownership. `run-check`/`run-all` need recipe inspection,
not unit-map loading unless diagnosing a file/unit failure.

## Select mode

| Mode | Action / completion |
| --- | --- |
| `unit-map FILE...` | Resolve every file/module via unit map; report unmapped entries |
| `run-check` | `just check`; recipe-owned repository gates, no pytest units |
| `run-unit UNIT` | `just check-unit UNIT` or `sh .pi/skills/bof3-test/scripts/check-unit.sh UNIT`; unit tests plus shared gate |
| `run-all` | `just check-all`; every unit undeselected plus shared gate |
| `optimize` | Read [optimization protocol](references/optimization.md); reviewed changes, preserved assertions and measured before/after |

`justfile` owns exact gate membership. Inspect recipe when reporting what ran;
never infer a pass from an earlier stage. Collection:
`pytest --collect-only -q tools/python/tests`; verify nothing deselected, report
actual count rather than stale expected totals.

## Ownership

`tools/python/tests/<unit>/` owns tests. Shared fixtures remain at test root.
`pyproject.toml` supplies cross-test `pythonpath` and overrides `norecursedirs` so
`build` unit is collected. Unit map owns folder/module details.

Reviewed optimization scope: `tools/python/tests/**`, `check`/`check-unit` recipes
in `justfile`, pytest configuration in `pyproject.toml`. No production behavior/
output changes, dependencies or weakened assertions. New regression tests require
explicit user request; running existing checks grants no test-addition authority.

Return tool count, wall time, method, selected/deselected/skipped counts and each
gate's status. Suite PASS never implies recipe PASS; name unreached stages and
uncovered invariants. Parent archives measurements; reusable rules belong here.
