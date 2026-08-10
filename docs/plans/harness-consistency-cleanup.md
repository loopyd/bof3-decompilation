# Harness consistency cleanup — completed

Completed through 2026-08-04, including `9eb47059` (`refactor(harness): decompose oversized modules and share CLI plumbing`).

## Result

- Agent context generation excludes the specs tree and remains size-bounded.
- `bof3-cleanup` front matter is valid and tested.
- Generic `bof.chain.json` has one repository-recon handoff and routes lifts to the BOF3 lift loop.
- Exact lifts require cleanup, fresh byte-match, and fresh review before eligibility.
- Lift-loop status fails closed on a dirty worktree.
- Commits require explicit user authorization.
- Shared CLI plumbing owns common root/example handling; decomposed harness modules remain protected by the module-size regression test.

## Superseding source ownership

`6fd4e4a1` removed the former `source_dir/func_XXXXXXXX.c` ownership assumption. Authored lifts now live under `src/bof3/`; explicit manifest claims and function metadata establish target ownership. See [the migration completion record](source-tree-classification-cleanup.md).
