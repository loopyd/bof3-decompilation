# Function metadata

Use this branch to maintain a function's identity, behavior and progress. Read the
[target contract](target-contract.md) before target work and use the
[editing and recovery guidance](editing-recovery.md) before edits.

`bin/harness source metadata list TARGET --limit 20 --json` lists attached records
by target and address. Each record separates the authored claim from its measured
result. Measurements are absent, stale or fresh cached evidence. A listing never
builds or records a successful match. A truncated listing is not a full audit.

Resolve each selected `TARGET@0xADDRESS` through the current manifest claims,
`@source`, map and reviewed Splat boundary. Filenames do not establish identity.
Keep one `@source` and one nonempty `@behavior` attached to each function. In a
grouped source, preserve every other function's record and implementation.

For a macro-generated function, put its complete block immediately above the
standalone invocation. Keep one block per emitted function, including in a source
that also contains ordinary definitions. Resolve the emitted symbol through the
target and address, never through a macro argument's position. The configured
preprocessor must attribute exactly one reviewed function to each invocation;
empty, multiple or ambiguous output blocks acceptance. Complete assembler member
checks also reject extra functions outside the annotated invocations. Preserve
each member's effective compiler settings when grouping or moving sources.

Private templates may use a continued top-level `#define`. Keep continued lines
inside that directive. Source conditionals and directives inside implementations
remain unsupported. Balanced GNU prefix attributes retain their source offsets;
they do not replace compiler-profile and native checks.

Naming lists inspect generated arguments and locals from that configured output.
Their lexical facts do not prove the meaning of a name or authorize a metadata
update. Recheck all members after changing a shared definition.

Investigate behavior from original bytes, callers, data and other available
evidence. Record unknown behavior honestly. A native byte match does not prove a
chosen name, type or explanation. Obtain the independent semantic review required
by the work before retaining those conclusions.

Run `bin/harness lift byte-match TARGET@0xADDRESS` under the owning compiler and
settings before recording success. Require complete finite original-range and
current-size equality, required placement and bindings, and all grouped members.
Cached status and instruction percentages do not satisfy this live gate.

Edit the selected record directly after measurement. Exact progress uses
`@status exact`, `@match 100.00` and `@residual none`. Partial progress uses its
current measured percentage and a specific remaining discrepancy. Use
`@match unavailable` when measurement is unavailable; never copy a prior score as
current evidence. The existing tag parser requires status, match and residual
together when any progress tag is present.

Run `bin/harness source metadata check TARGET TARGET@0xADDRESS --json` after the
edit. It freshly compares current inputs and checks every function sharing the
selected source. Omit the selector for a full-target metadata check. Exit 0 means
the checked claims agree with complete current measurements. Exit 1 reports
findings or unavailable evidence; exit 2 rejects an invalid request. Checks never
repair records or grant semantic acceptance. Inspect partial residuals against
the actual diff; the tool cannot establish their meaning.

If inputs change, run a new live comparison before retaining an exact conclusion.
After a failed gate or review, confirm writer termination and restore only owned
edits to PRE. Preserve unrelated dirty work and the index. Report uncertain
cleanup or restoration as a blocker.
