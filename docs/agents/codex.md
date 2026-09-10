# Codex configuration and Pi migration

Read [the documentation index](../INDEX.md) first. Project skill definitions and
Python owners are described there and in [harness.md](harness.md).

## Bounded native review

Bounded read-only review is available; the broader production pipeline is unfinished.
Repository evidence and writer leases use filesystem-native permissions, without
requiring POSIX `0600`/`0700` modes or a separate state store on NTFS. Ownership,
file/root identity, single-link and locking checks remain; filesystem modes do not
guarantee confidentiality. Empty/partial files after creation/write failure remain
unresolved evidence, never successful publication. Removing mode checks does not
reset an existing allowance or authorize retry.

`bin/agent-run review REQUEST --budget BUDGET --consumption INITIAL ...` dispatches
one read-only `bof3-reviewer` through the installed `codex exec`. It is not the
lift/naming/cleanup scheduler, a writer transport, a Pi runtime fallback or semantic
acceptance. `harness.context.{dispatch,codex,journal}` owns role policy, native
configuration/events and immutable dispatch evidence; `commands.agent_run` owns
the CLI. The `bof3-re` skill's `scripts/dispatch.py` delegates to that same CLI.

Requests contain exactly:

```json
{"schema":"bof3.codex-review/v1","selector":"emi/battle/battle/15@800a3638","capabilities":"native-read-only","task":"Review the specified source without editing or claiming unavailable gates passed.","adopted_baseline":"CURRENT_BASELINE_DIGEST"}
```

Use the shared canonical function-ID spelling and current `bin/type-audit baseline`
digest. The original [execution budget](harness.md#bounded-continuation)
must contain queue ID `review:SELECTOR` with fingerprint `digest(REQUEST)`. Supply
`--expected-budget-digest`, `--expected-checkpoint-digest`, `--expected-sequence`
and the complete ordered consumption chain through repeated `--consumption`.
After sequence zero, also supply the **externally retained**
`--expected-result-digest` from the preceding completed invocation. Do not discover
success by scanning receipt files or silently replace a frozen request/baseline.

One dispatch debit covers its context/capability preparation and one Codex job,
including that job's diagnostic subprocesses; it is not an unlimited token or
time allowance. The shared writer lease excludes cooperating source writers.
Exclusive `out/reviews/dispatch/BUDGET_DIGEST/SEQUENCE/consumption.json` is durable
before any Codex process. Its `dispatch.debited` JSONL event reports the new
checkpoint pin before launch; interrupted dispatches retain that debit. Missing,
failed, unpinned or changed previous completion evidence blocks another dispatch.
No retry/refund, recovery, restoration or automatic acceptance is provided.

The explicit capability policy is **child-only**: `--sandbox read-only`, approval
policy `never`, no apps/plugins/multi-agent variants/hooks/shell snapshots or
skill-triggered MCP installation, and
every enumerated MCP server disabled by override. A second native listing must
confirm the disabled inventory; failure stops before model dispatch. Unsupported
server-key spellings reject. Empty-table overrides do not disable inherited MCP
servers in the inspected CLI. Global settings, configured model/provider and
existing authentication remain inherited, not rewritten or copied. Normal Codex
home runtime/auth activity is not a promise of zero home writes. Installation,
credentials, network access and execution approvals retain their own gates.

The existing role body and complete canonical review prefill go over stdin, not
argv. Task text is at most 8 KiB; the complete prompt at most 128 KiB. The child
uses `--json --ephemeral`; no implicit `codex exec resume`. Native stdout/stderr
are incrementally flushed/fsynced into exclusive evidence files, with a combined
1 MiB cap and the original absolute work cutoff. Output can contain source
or diagnostics: never dump it, credential-bearing configuration or environment
values into public logs. Capability enumeration diagnostics are withheld.

Only exit zero plus a structurally valid ordered native thread/turn event stream,
nonempty proposal, unchanged baseline and retained writer lease can publish
`bof3.codex-dispatch/v1`. It binds hashes of all eight retained input/output artifacts
and records the emitted native thread ID. `dispatch.completed` prints its receipt
pin, not model text. `semantic_acceptance:false` and `parent_review_required:true`
remain mandatory; unavailable byte/build gates remain blockers. A lease/publication
failure can leave a receipt, but no successful CLI handoff or automatic advancement.

The original owner supervises and reaps descendants through `common.process`.
Saved supervisor PIDs are diagnostic, never future termination authority.
`ProcessCleanupError` remains distinct even if failure recording fails; pending
records require parent investigation. Retain a separate outer cleanup hard-stop.
Authentication, completed native review and production pipeline acceptance require
their own observed evidence; CLI discovery, fixture JSON events and green tests do
not establish them. See [official non-interactive Codex usage](https://developers.openai.com/codex/noninteractive/).

## Parent lift diagnosis

`bin/agent-run diagnose REQUEST --expected-request-digest PIN --output
out/reviews/lift-diagnosis/NAME --deadline ORIGINAL_MONOTONIC_CUTOFF` measures an
**existing claimed lift** before editing. The `bof3-re` dispatch script forwards
this command too. It launches no model, changes no source, and grants no review,
restoration, retry or campaign advancement. New unlifted functions still need the
original-byte/first-source mission route; this command cannot diagnose absent C.

The closed `bof3.lift-request/v1` request contains `schema`, canonical `selector`,
`source`, sorted unique explicit `paths` (at most twelve), nonempty `task` (at most
8 KiB), current `adopted_baseline`, and
`capabilities:"native-scoped-directory-write"`. That explicit scope describes the
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

The installed legacy 32-bit compiler could not run under the tested Codex command
sandbox. Parent gates therefore have their own explicit installed `bwrap` invocation:
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

After a [managed writer](#one-shot-lift-writer), also supply its externally retained
`--expected-writer-digest`. Audit verifies the diagnosis-owned slot, original debit,
completion pin and all eight writer artifacts before and after native checks.
Failed/incomplete writers reject. The candidate must still match the writer's POST,
and the proposal must be its original unmeasured report. A hand-guided edit with
no managed slot retains the existing route; a supplied pin without a slot rejects.

Default `--proposal-format mission` checks the measured two-fence report above.
For a writer unable to run the legacy compiler, explicitly use
`--proposal-format unmeasured`: the same report must contain
`status:"unverified"` and `match_percent:null`, with truthful attempts, changed files,
risks and refresh/restoration flags. Ordinary mission completion does not gain this
status. Parent gates supply measured outcomes separately; they never rewrite an
unmeasured writer claim into a claimed exact result or bypass unavailable tooling.

The source-read-only audit reruns cold native checks, verifies retained evidence
and proposal bytes before/after, and publishes `bof3.lift-audit/v2` with their pins,
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
`bof3-re`'s dispatch script forwards both commands unchanged.

## One-shot lift writer

```sh
bin/agent-run lift out/reviews/lift-diagnosis/BEFORE \
  --expected-diagnosis-digest DIAGNOSIS_PIN \
  --capabilities native-scoped-directory-write \
  --budget out/budget.json --consumption out/initial-consumption.json \
  --expected-budget-digest BUDGET_PIN --expected-checkpoint-digest CHECKPOINT_PIN \
  --expected-sequence 0
```

This explicitly authorized source-writing command connects diagnosis to retained
audit, not to automatic acceptance. Its original budget queue contains
`lift:SELECTOR` with fingerprint `digest(LIFT_REQUEST)`; use the same boot-bound
cutoff as diagnosis. Later sequence positions require the complete consumption
chain and previous read-only completion pin, as [review](#bounded-native-review)
does. It never creates or resets a budget, refreshes an index, or repins evidence.

Under the shared writer lease, fresh mission/policy checks precede one durable
dispatch debit and exclusive `DIAGNOSIS_DIRECTORY/writer.json` slot. One diagnosis
and queue entry permit only one writer job. Failure, cancellation or a leftover
slot stops replay; a completed writer cannot unlock generic next-dispatch logic.
Missing cleanup confirmation requires parent native-handle inspection, not rollback.
Only the separately pinned [audited-review step](#audited-lift-review) may consume
its planned review debit after a successful audit; that is not acceptance.

The child inherits configured model/provider/authentication, disables the review
capabilities above, and uses `--strict-config` with the pinned named permission
profile. It does not use workspace-wide writes or weaken the outer sandbox.
Existing siblings/child directories, Git and frozen evidence remain read-only;
declared source parents and generated outputs/scratch are writable, tool networking
is disabled, and home Codex/Pi paths are denied to tools. **Future names within a
writable parent are not confined**: post-audit rejects unexpected names, including
ignored files. This is cooperative scoped-directory execution, not a hostile-code
or credential-inventory guarantee. Codex's own authentication/runtime is separate.

The retained role and canonical reverse prefill accompany the pinned cold diff.
The writer is instructed to make one structural experiment or justified no-op,
then return the full two-fence `unverified`/null report. It cannot claim unavailable
compiler measurements; native gates stay with the parent. Prompt instructions do
not mechanically prove one semantic experiment or a human-quality improvement.
One job shares the original deadline, 128 KiB prompt bound and 1 MiB output cap.

Successful transport publishes `bof3.codex-lift/v1`: original bindings, native thread
ID, eight artifact hashes, actual POST inventory/owned images and the writer report.
Stdout supplies its receipt digest and proposal SHA-256 for `audit`, using
`--proposal-format unmeasured --expected-writer-digest WRITER_PIN`. Audit retains
that writer pin separately from native measurements. Neither completion permits
source acceptance, automatic restoration, retry or campaign advancement.

Installed Codex 0.153.4 applied this named profile through native `exec` in disposable
Linux/NTFS probes. Scripted loopback responses exercised shell writes and native
`apply_patch`: owned edits succeeded, protected sibling edits/deletions/moves failed,
and an unexpected ignored name remained visible to the rejecting post-audit.
A separate Git-backed fixture completed `diagnose` → native `lift` → `audit`,
using a real Codex patch/event stream but scripted provider responses and synthetic
gate payloads. Empty temporary Codex homes avoided credentials and live models.
These probes do not prove production authentication, BOF3 semantics, independent
review or whole-pipeline acceptance. No live BOF3 writer is claimed.

**Native writer reliability remains unresolved.** Two subsequent fresh fixtures
left new `.git`, `.codex` and `.agents` directories beside the owned source.
Both stopped at the unchanged membership guard after native turn completion,
retaining candidate/debit without a successful writer receipt. Codex's
[version-matched sandbox source](https://github.com/openai/codex/blob/rust-v0.153.4/codex-rs/linux-sandbox/src/linux_run_main.rs)
implements synthetic metadata mounts and cleanup; the precise residue cause is
not established. Do not ignore these paths, pre-create them in production, relax
sandbox rules, delete unexpected entries or treat a stopped writer as accepted.

## Audited lift review

```sh
bin/agent-run review-lift out/reviews/lift-audit/AFTER \
  --expected-audit-digest AUDIT_PIN --expected-diagnosis-digest DIAGNOSIS_PIN \
  --expected-writer-digest WRITER_PIN --expected-result-digest WRITER_PIN \
  --budget out/budget.json --consumption out/initial-consumption.json \
  --consumption out/reviews/dispatch/BUDGET_DIGEST/1/consumption.json \
  --expected-budget-digest BUDGET_PIN --expected-checkpoint-digest WRITER_CHECKPOINT \
  --expected-sequence 1
```

Plan queue ID `review-lift:SELECTOR` with fingerprint `digest(LIFT_REQUEST)` in the
**original** budget, before diagnosis. The complete consumption chain must end at
the immediate writer debit; the two writer-pin flags bind audit provenance and
that prior result, respectively. No inferred queue entry, new budget/cutoff or
receipt scan supplies authorization. This route requires a managed writer, not
an unbound hand-guided proposal.

Under the writer lease, the command verifies the pinned diagnosis, writer, audit,
all retained artifacts and unchanged audited POST/native inputs/Git index. It
renders the canonical tracked review prefill without refreshing or trusting the
post-edit reverse index. Its prompt separates original facts, parent measurements,
writer claims and review obligations. Parent native gates remain parent-owned;
the reviewer must disclose unavailable independent checks, never claim to run them.

One durable debit and diagnosis-owned `review.json` slot precede capability
preparation and a fresh native `codex exec --sandbox read-only` job. It inherits
the [read-only restrictions](#bounded-native-review), prompt/output bounds and
original boot-bound cutoff. Slot/debit drift, stale evidence, changed candidate,
failed/unfinished output or reuse of the writer's thread rejects. Empty/partial
publication or failure retains the slot/debit and candidate; no automatic replay.
Audit/candidate verification runs again before and after the native job.

`bof3.codex-lift-review/v1` binds the original queue/debit, diagnosis/audit/writer
pins, separate native thread, review text and eight input/output hashes. The
generated review-request digest binds these runtime facts; the planned queue
fingerprint still binds the original lift request. Stdout reports the receipt pin
and path, not source text. `native-audited-lift-review` is a distinct handoff kind:
generic dispatch cannot advance from it. `record_debit` only accounts/persists a
charge under a writer lease; its domain caller must validate the preceding stage.

Completion proves transport and retained scope, **not a parsed or accepted semantic
verdict**. Parent acceptance, affected-consumer checks, matching-aid approval,
reviewed partial/exhaustion handling, repair scheduling and safe recovery remain
unfinished. Separate threads do not alone prove independent review quality.

A controlled native fixture with metadata directories already present in PRE
completed writer/audit/reviewer transport; the reviewer's source write was denied
with `EROFS`, and audited source remained unchanged. Scripted loopback responses,
synthetic gates and injected prefills isolate transport, not live-model behavior or
BOF3 acceptance. This fixture does not resolve the missing-directory writer issue.

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
