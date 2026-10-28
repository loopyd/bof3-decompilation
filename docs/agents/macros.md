# Macro facts and checks

The harness reports current macro definitions, lexical uses, C repetition and
original-byte similarity. The [macro skill](../../.agents/skills/bof3-macros/SKILL.md)
owns selection, semantic review and direct edits.

```sh
bin/harness macros list exe/logo --kind definitions --limit 10 --json
bin/harness macros list exe/logo --kind uses
bin/harness macros list exe/logo --kind repetitions
bin/harness macros list exe/logo --kind byte-repeats
bin/harness macros check exe/logo exe/logo@801CE758 --json
```

Every action requires one explicit target. Check selectors are canonical
`TARGET@ADDRESS` function IDs in that target. Omitting selectors checks the whole
target. `--limit` is positive and only truncates list records; counts and findings
retain complete scope. `--kind all` combines definitions, uses and C repetitions.
Byte-repeat discovery is separate because it reads the global reviewed corpus.
It retains all members of a relationship that touches the selected target.

Listings read a fresh existing index without refreshing it or building source.
Repeated tokens and instruction shapes are leads. They do not establish a useful
macro, safe arguments or shared ownership. Definition facts include parameter
mention counts. Current native checks follow indexed transitive macro uses and
check full reviewed original ranges, including grouped members. Foreign consumers,
unknown identities, unsupported definitions or stale inputs remain explicit
findings. Shared edits require separate checks of every owning target.

Text and JSON report the same scope and findings. Exit code 0 means a complete
passing result, 1 means findings or unavailable evidence, and 2 means an invalid
request. Checks may use disposable native outputs. Neither action edits reviewed
source, maps, metadata or progress. Record successful matches through the
[metadata owner](harness.md#function-metadata) after the existing live byte gate.
