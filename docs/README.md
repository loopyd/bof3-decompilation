# Documentation

The root [README](../README.md) is the command overview. This directory keeps
the focused operating references and durable BOF3 findings.

| Need | Read |
| --- | --- |
| Set up tools, inspect media, or learn command contracts | [Setup and tools](setup.md) |
| Lift, compile, match, or audit current lift status | [Matching one function](matching.md) |
| Reproduce Rizin evidence or query cross-target facts | [Rizin and reverse index](reverse-engineering.md) |
| Identify Psy-Q object signatures and their callsites | [Psy-Q signatures](reverse-engineering.md#psy-q-signatures) |
| Binary, archive, entry, and target terminology | [Context](../CONTEXT.md) |
| Durable format, runtime, program, and data evidence | [Specs](specs/index.md) |
| Reusable reverse-engineering gotchas | [Lessons](../LESSONS.md) |
| Retained/removed C migration evidence | [Source retention audit](specs/migration.md) |

Tracked configuration owns target facts; `out/` is disposable generated
evidence. Do not treat an EMI archive as one executable target.
