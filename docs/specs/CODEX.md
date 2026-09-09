# Codex configuration and Pi migration

Read [the documentation index](../INDEX.md) first. Project skill definitions and
Python owners are described there and in [HARNESS.md](HARNESS.md).

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
digest. The original [execution budget](HARNESS.md#bounded-continuation)
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
policy `never`, no apps/plugins/multi-agent/skill-triggered MCP installation, and
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
