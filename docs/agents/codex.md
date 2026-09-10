# Codex session integration

Start at [docs/INDEX.md](../INDEX.md). The active session owns agent execution;
the project harness owns deterministic domain tools, not Codex/model launchers.

## Skill-only operator

[`bof3-lift-loop`](../../.codex/skills/bof3-lift-loop/SKILL.md) defines the bounded
mission protocol and lift → review → naming → types/macros → cleanup/final review
sequence. Delegate only through actual tools exposed by the active session.
Parallelize independent reads/reviews; serialize shared-checkout writes, native
target outputs and index refreshes. Missing delegation or independent review is a
host-capability blocker, never permission for a CLI/SDK/Pi fallback.

Standing authorization covers relevant project evidence/reviews and safe-checkpoint
index refreshes; do not repeatedly request user consent. The host session owns
its own approvals and configuration. The harness must not invoke `codex`, launch
a model SDK/API, discover MCP via Codex CLI or rewrite child permission profiles.
No detached controller, background model session or second campaign database.
Local native compiler processes retain existing ownership/deadline/cleanup guards.

## Retired transports

The former `agent-run review`, `lift` and `review-lift` commands, their CLI
configuration/process helpers and dispatch script are removed. Historical receipts,
thread IDs, budget pins and failures remain immutable history, not live mission
state or permission to replay. The abandoned auto-review child-configuration
experiment is superseded by skill-only operation. `agent-run diagnose` and `audit`
remain model-free measurement tools, not a replacement scheduler or acceptance
shortcut. Independent review and final parent acceptance still bind actual source.

## Native compiler execution

The installed canonical GCC driver, `cpp` and `cc1` are static i386 ELF programs.
On this host, even `gcc --version` receives `SIGSYS` (shell exit 159) under the
Codex command sandbox; the same compiler completes native comparisons through
reviewed escalated execution. This is an execution-capability failure, not a C
error. Nested `bwrap` cannot remove the outer inherited syscall restrictions.

For this known host/compiler combination, the active-session parent requests the
exact bounded native gate with `exec_command`'s `sandbox_permissions:
"require_escalated"` **before the baseline build**. Prefer the pinned diagnosis/audit
commands below, whose inner sandbox keeps source/Git/evidence read-only and limits
generated writes. Direct scoped asm-diff/byte-match commands also require that
reviewed execution route; a worker without it returns unverified, not another
sandboxed compiler attempt. Keep original scope, cutoff, evidence and cleanup.

Approval remains per reviewed action: no automatic self-elevation, command-rule or
Codex-policy changes, alternate wrapper after denial, emulator installation or
silent compiler substitution. A denied/unavailable native route blocks its gates;
continue only unaffected work. On a different host/compiler, establish capability
once before relying on this diagnosis. Ordinary Python inspection stays sandboxed.

## Parent lift diagnosis

`bin/agent-run diagnose REQUEST --expected-request-digest PIN --output
out/reviews/lift-diagnosis/NAME --deadline ORIGINAL_MONOTONIC_CUTOFF` measures an
**existing claimed lift** before editing. The `bof3-re` `scripts/mission.py` helper forwards
this local command. It launches no model, changes no source, and grants no review,
restoration, retry or campaign advancement. New unlifted functions still need the
original-byte/first-source mission route; this command cannot diagnose absent C.

The closed `bof3.lift-request/v1` request contains `schema`, canonical `selector`,
`source`, sorted unique explicit `paths` (at most twelve), nonempty `task` (at most
8 KiB), current `adopted_baseline`, and
`capabilities:"mission-scoped-source-edit"`. That explicit scope describes the
potential mission, **not a write grant from diagnosis**. Existing ownership must
match the selector; shared source/header changes reject. Retain the request digest
externally. Capture validates current mission/index freshness, original span and
PS-X header, source/configuration ownership, native inputs/tools/environment,
workspace metadata and exact raw Git index. Stale analysis stops; it is never
refreshed or replaced here.

The parent supplies its already-accounted original monotonic work cutoff; diagnosis
does not create/debit a campaign budget or infer permission to retry. Preparation,
each native command and publication retain that cutoff, with each command capped
at 120 seconds and 1 MiB combined output. Keep the separate parent cleanup hard-stop.
The shared writer lease excludes cooperating writers throughout; it neither proves
prior-worker termination nor excludes manual editors.

Parent gates use the [reviewed native execution route](#native-compiler-execution)
and an explicit installed `bwrap` invocation:
separate namespaces/network, read-only root/source/Git/evidence, writable generated
build/asm-diff/bindings/matching/Splat outputs and fresh dispatch scratch; home Codex
and Pi directories are masked. These gates neither install packages, alter Codex
policy, provide a fallback after denial, nor grant source-writing authority. Outer sandbox
restrictions still apply; unavailable native capabilities require approval or stop.
This is bounded cooperative execution, not proof against hostile filesystem aliases
or arbitrary privileged code. Other credential locations are not inventoried/masked.

Diagnosis performs fresh CMake/Ninja configuration, selectively removes the chosen
object, verifies its absence, then runs full JSON asm-diff and byte-match. It checks
native schemas, identity, original span, instruction/byte agreement and unchanged
PRE/native inputs around the gates. Exact **and valid partial** measurements are
useful diagnostics, never source acceptance. The owning candidate audit reuses these
same gates after edits and separately validates the mission/acceptance report.

The candidate-audit API's machine report uses two fences: mission `json`, then
JSON `acceptance-report`. The latter has exactly `pre_mission` (mission/baseline
digests), `commands`, `attempts`, `risks`, `retained_candidate` (source path),
`remaining_candidates`, `snapshot_index_refresh_required`, `staged_index_changed`,
`parent_restore_required`, and `matching_aid_approvals`. Commands, risks, candidates
and approvals are lists; the three flags are booleans. Each ordered attempt contains
`order`, `diagnosis`, `change`, `command`, `result`, `retained`, and `reason`.
This explicit JSON variant is not inferred from legacy prose. Writer claims are
compared with parent measurement; neither parsing nor matching bytes supplies the
independent semantic review needed for acceptance.

Only a fresh direct-child output directory is allowed. It retains the pinned
`mission.json`, `policy.json`, invocation cutoff, started/spawn/terminal records,
fsynced stdout/stderr and `bof3.lift-diagnosis/v1` result. Stdout reports only paths,
pins and measured status. The result pins its boot/original cutoff in `clock` and
hashes all 23 retained input/native artifacts. Evidence can contain source/PRE images; keep it out of
public logs and commits. Existing output rejects unchanged. Failure retains evidence
and records no restoration/retry authority; a leftover result alongside failure is
not a successful handoff. Native cleanup uncertainty remains distinct. Saved PIDs
are diagnostic only. No automatic rollback, model edit/retry loop, independent
acceptance or stale-index handoff is established by this prerequisite.

## Retained lift audit

```sh
bin/agent-run audit out/reviews/lift-diagnosis/BEFORE out/reviews/proposal.md \
  --expected-diagnosis-digest PIN --expected-proposal-sha256 SHA256 \
  --output out/reviews/lift-audit/AFTER
```

The parent uses this after a separately authorized edit and confirmed writer
termination. It reuses the original mission, policy and boot-bound cutoff, not a
new baseline/deadline; the CLI has no replacement-clock flag. Supply externally
retained diagnosis/proposal pins. Missing, failed, altered or legacy diagnostics
without clock/artifact coverage reject; never upgrade or silently repin old evidence.
The original manifest/native inputs, index, unrelated workspace and source-directory
guards remain active. Do not refresh analysis between diagnosis and audit; report
required refresh for the later parent checkpoint. A free lease is not quiescence.

Diagnoses owning retired CLI writer/reviewer slots reject as historical-only.
Do not reinterpret their receipts as active-session missions or strip their slots
to gain acceptance. Prepare fresh mission inputs at an authorized checkpoint.

Default `--proposal-format mission` checks the measured two-fence report above.
For a writer unable to run the legacy compiler, explicitly use
`--proposal-format unmeasured`: the same report must contain
`status:"unverified"` and `match_percent:null`, with truthful attempts, changed files,
risks and refresh/restoration flags. Ordinary mission completion does not gain this
status. Parent gates supply measured outcomes separately; they never rewrite an
unmeasured writer claim into a claimed exact result or bypass unavailable tooling.

The source-read-only audit reruns cold native checks, verifies retained evidence
and proposal bytes before/after, and publishes `bof3.lift-audit/v3` with their pins,
native artifacts and before/after comparison. A lower instruction score or loss of
byte exactness requests parent restoration **review**, without restoring anything.
The comparison is against this diagnosis, not proof of best-of-history or semantic
quality. Exact/partial results remain `needs-independent-review` unless a regression
or writer request requires parent restoration review; exit zero is not acceptance.
Only the selected function is measured: final approval still owes affected-consumer,
matching-aid, semantic and domain-owner checks.

Output must be a fresh direct-child audit directory. Failure retains evidence and
any candidate, with no retry/restoration/acceptance authority. The parent accounts
each invocation against the original campaign budget; neither command debits it or
implements a model writer, repair scheduler, independent acceptance or recovery.
`bof3-re`'s `scripts/mission.py` forwards both commands unchanged.

## Global MCP ownership

MCP server definitions and configured credentials belong in `~/.codex/config.toml`,
not this repository's `.codex` directory. The migration source is
`~/.pi/agent/mcp.json`, overlaid by `.pi/mcp.json` when it defines project servers.
The project file was empty at migration. Preserve unrelated Codex settings and a
private backup before changing global configuration; keep credential-bearing
configuration and backups mode `0600`. Never print tokens, connection strings or
credential-bearing command arguments in reports or commit them to the project.

The migrated servers are `sqlitecloud-mcp-server`, `github-mcp-server`,
`markitdown`, `context7`, `context-mode`, `playwright`, and `vercel`. Command
arguments and literal credentials are preserved. Pi's `directTools` flag has no
Codex configuration equivalent and does not grant approval. Expand Pi environment
references explicitly when translating; do not persist session-temporary PATH
entries. The Pi source remains untouched as recovery/reference material.

Codex successfully parses all seven global definitions. This proves configuration
loading, **not remote connectivity, credential validity or OAuth completion**.
No servers were launched or packages installed for that check. Stdio servers use
their configured environment/arguments; an OAuth status of `unsupported` from the
listing is not a failed API-key validation.

Vercel is configured as streamable HTTP; its OAuth status remains unverified.
Use `codex mcp login vercel` for Codex-owned authentication when needed. Do not
copy opaque Pi keyring records into Codex or substitute an expiring access token
for a complete OAuth lifecycle. Authentication may require user browser approval.
See [official MCP configuration](https://developers.openai.com/codex/mcp/).

## Extension equivalents

Installed Pi extensions are not modified. Use native Codex features before adding
another process or plugin. **Serena is explicitly excluded**, as are replacement
subagent, memory, VCC and compaction extensions. Existing Codex features/settings
are not silently disabled by excluding their Pi counterparts.

| Pi extension | Codex disposition |
| --- | --- |
| `pi-mcp-adapter` | Native MCP configuration; seven definitions migrated globally. |
| `pi-web-access` | Native web tools; existing MarkItDown/Playwright MCP definitions retain document/browser options. This does not claim parity with every provider or video feature. |
| `pi-intercom` | Native `codex agents` and `codex queue --thread ID --message TEXT` cover session discovery/messaging. No Pi broker or subagent bridge is ported. |
| `pi-semantic-edit` | Native `apply_patch` remains the edit mechanism; no fuzzy-match replacement tool or weakened uniqueness rules. |
| `@plannotator/pi-extension` | Publisher-supported [Codex integration](https://github.com/backnotprop/plannotator/blob/main/apps/codex/README.md): review/annotation skills and optional Stop-hook plan review. Not installed or enabled by this migration. Avoid the broad installer that also changes other agents. |
| `pi-lsp` | Non-Serena candidate: [MCP Language Server](https://github.com/isaacphi/mcp-language-server). Its documentation covers clangd, Pyright, TypeScript and Rust; the adapter is not installed or validated here. If adopted, scope workspaces and prefer read-only navigation/diagnostic tools; its rename/edit tools do not bypass BOF3 evidence gates. |
| `pi-session-search` | Use native `codex resume` for session selection; no semantic memory index or automatic import of Pi transcripts. This is not full semantic-search parity. |
| `context-mode` | No additional context/compaction extension or hooks. Its MCP definition was retained under the separate all-server migration request. |
| `pi-subagents`, `@samfp/pi-memory`, `@sting8k/pi-vcc` | Omitted as requested. |
| `pi-archon` | Not ported; its Pi entrypoint is excluded in the current package configuration. |

Plannotator and the LSP adapter are researched alternatives, not installed plugins.
New binaries, dependencies, hooks and global configuration changes require explicit
approval of the specific installation. A capability table or marketplace result is
not installation evidence. Keep project-specific language-server flags and trust
boundaries explicit rather than blindly copying them into global launch arguments.
