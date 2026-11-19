# Disk cleanup

`bin/harness cleanup` removes recognized disposable outputs. Preview the plan and
estimated disk savings before a live run:

```sh
bin/harness cleanup --dry-run
bin/harness cleanup
```

Every planned operation includes its path, category and selection reason. Skipped
candidates include the protection that kept them. Dry runs write no files,
indexes, worktree registrations, Python caches or locks.

## Selection

| Scope | Eligible outputs | Retention |
| --- | --- | --- |
| Default | Rust incremental caches under `tools/rust/*/target/` and `build/tools/rust/*/`, in debug and release profiles | Unmodified for 24 hours |
| Default | Superseded hashed ELF executables in debug `deps/` and `examples/` | Older than seven days; keep the newest per family and binaries with retained hardlinks |
| Default | Recognized root GCC pass dumps such as `*.c.rtl` and `*.c.combine` | Unmodified for 24 hours |
| Default | Retired root `ledger/` | No age threshold |
| `--with-evidence` | Children of `out/source-refinement/`, `out/matching/`, `out/asm-diff/` and `out/lift-batches/` | Unmodified for 24 hours |
| `--with-session-history` | Children of `.pi/sessions/`, `.pi-subagents/sessions/`, `.pi-subagents/chain-runs/` and subagent `artifacts/{outputs,progress,structured-output,handoffs}/` | Unmodified for 24 hours |
| `--with-worktrees` | Registered linked worktrees inside this repository | Unmodified for 24 hours; clean, merged, unlocked and inactive |

Options combine. `--all` enables all three optional categories; it retains every
protection and age threshold.

```sh
bin/harness cleanup --with-evidence --dry-run
bin/harness cleanup --with-worktrees --with-session-history --dry-run
bin/harness cleanup --all --dry-run
```

Evidence and session history may be needed to explain earlier work. Select these
categories only when those records can be discarded. Saved worktree diffs,
acceptance records, missions and unknown subagent state remain outside history
scope. Unknown output roots, original inputs, installed toolchains, current tools,
indexed or committed paths and recovery/PRE/POST trees remain protected.

## Execution and space reports

Live runs acquire the repository writer lease and existing Cargo profile locks.
They recheck Git ownership, file identities, directory membership and process use
before removal. Symlinks cannot redirect deletion outside the checkout. Nested
repositories, nested filesystems and special files are skipped.

Worktrees must have no tracked or untracked changes, and their HEAD must be an
ancestor of the primary checkout's current commit. Removal uses `git worktree
remove` without force; branches, the primary checkout and the invoking checkout
remain. External worktrees are skipped.

The final estimate counts allocated storage, including directories, rather than
logical file sizes. It counts hardlinked storage once only when all links are
selected. Per-operation estimates can therefore sum to less than the combined
estimate when operations share hardlinks. Live runs also report the observed
filesystem free-space change; other writers and filesystem metadata can make it
differ from the estimate.

Process inspection requires Linux and covers visible current-user processes.
Unreadable process IDs are reported as a visibility gap. Age checks and writer
and Cargo locks still apply; a report does not prove that every process is idle.
A changed or unsafe selection stops a live run with nonzero status. Earlier
removals remain deleted, and only completed operations contribute to its estimate.

## Tests

`just check-unit cleanup` runs fixture-based cleanup tests and the repository's
shared gate. Live deletion tests use temporary Git repositories, so current
evidence and session history are preserved.
