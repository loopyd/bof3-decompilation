<!-- bof3.plan/v1 -->
# Autonomous BOF3 lift, naming and cleanup

## Goal and scope

Design the loop by recovering and streamlining our project-specific `bof3-*`
agent definitions and domain skills. **Do not use or copy built-in Pi subagent
profiles or home-directory agents.** Pi subagents is the execution mechanism,
not the source of agent roles. The owner approved implementation and requested
cleanup additions before execution; campaign scope remains a finite, explicit queue.

The preceding cleanup removed `.pi/agents/`. Recover its latest BOF3 definitions
from retained source reads/mutation history, not invented prompts. The roles were
`bof3-lifter`, `bof3-namer`, `bof3-cleaner`, `bof3-reviewer`,
`bof3-lift-batch-coordinator` and `bof3-lift-batch-finalizer`. Git HEAD contains
older `bof3-reverse.md`, `bof3-cleanup.md` and `bof3-review.md`; compare these but
do not assume they are the latest versions. Generic classifier/scout/planner/
worker/reviewer definitions imported from home are not recovery targets.

The current user explicitly authorizes narrowly targeted integration regression
tests for functions, shared variables/data, structs/types and macro opportunities,
and worker/reviewer implementation iterations. Generic roles implement/review
tooling only; the four project BOF3 roles still own domain execution. No copying
generic profiles into the project. Installed-extension edits and dependency
installation remain unauthorized. Preserve unrelated dirty work and
original media. Git writes/publication remain separately authorized. Existing
domain validation and transaction rollback stay; no campaign backup framework,
loader collectors, handshakes, mirrored state or custom receipt reconstruction.

## Current Codex continuation

The latest discovery slice adds reviewed within-function assembly blocks under the
[macro specification](../specs/MACROS.md#assembly-blocks). An explicit experimental
floor of eight instructions yields 1,568 global four-use groups, 675 involving
battle/15; the largest displayed lead spans 67 instructions at four sites. Reports
are `out/reviews/codex-macro-blocks-battle15.json`,
`out/reviews/codex-macro-block-account.json`, and
`out/reviews/codex-macro-block-description.json`. Account replay and descriptor
fingerprints agree; 222 existing checks, exhaustive small-sequence/disposable
preparation probes, and independent code/docs review pass. No lifted source,
index, frozen membership, or proof changed. Human-value evidence is required for
block artifacts; AI reranking, bounded top-N execution, and actual source acceptance
remain unfinished. The experiment does not establish a project-wide size default.

The latest macro consumer-coverage correction closes the independently reproduced
literal-include omissions; its current contract lives in
[MACROS.md](../specs/MACROS.md). Six disposable omission probes now reject while
covered scopes pass, and their dependency sets agree with native C89 preprocessing.
Default dependency lists remain unchanged for all 977 manifest source/support
inputs. All 343 scoped source/macro/type/naming checks pass; reviewer
`01a084c1-85d3-7b01-8298-e63590cd5e6a` accepted the final code/docs correction.
No lifted source, index, or frozen proof changed. This closes the reviewed tooling
gap, not macro acceptance, ranking-loop requirements, or a campaign phase.

Macro-tooling continuation: reverse-index v13 repairs unique target-owned
source/function associations for absolute and relative paths; ambiguous and
generated owners remain unlinked. Native rebuild retains all 6,580 macro uses and
links 1,010; an in-memory comparison preserves every other use fact. Read-only
`bin/macro-audit describe ID [--target TARGET]` exposes member sources, existing
file-level lexical macro context, and review gates without narrowing the global
candidate fingerprint. The pilot report
`out/reviews/codex-macro-resolution-panel.json` retains ten members (three focused,
seven external) and identifies `PANEL_ADVANCE_X`; global/focused/account
fingerprints agree. All five opportunity kinds pass native inspection probes;
238 existing macro/index/status/plan checks pass. Independent tooling review
accepted the multiline-context correction. No source or frozen proof changed;
semantic guards, independent private exact proofs, and shared promotion remain
unresolved. This advances discovery/inspection, not candidate acceptance or phase
completion.

The subsequent impact lookup follows global indexed consumers and target-qualified
definition-body dependencies, including cycles, without claiming preprocessor
binding or expanding write scope. `bin/macro-audit impact DEFINITION_ID` and the
descriptor's impact section expose fourteen `PANEL_ADVANCE_X` uses across three
targets: ten with limit 320, four with limit 17. This is wider than both the focused
pilot and its exact group. Evidence is retained in
`out/reviews/codex-macro-impact-panel.json` and
`out/reviews/codex-macro-resolution-panel-impact.json`; the candidate fingerprint
and frozen membership are unchanged. All 176 existing scoped checks and disposable
transitive/cycle/target-isolation probes pass; independent tooling review found no
blockers. Real macro resolution still owes use-site/semantic review, native private
proofs, and safe coverage of external consumers before any shared change. The
impact report remains incomplete lexical evidence, never an `all_use_sites` proof.

The current user goal requires the lifting, evidence-based naming, indexing and
type/macro cleanup pipeline to work in Codex and ultimately produce byte-faithful,
readable source across the codebase. Codex's available tools now execute the
repository commands and independent reviews; this supersedes the earlier Pi-only
execution assumption, not domain ownership, review, rollback or acceptance gates.
Historical Pi recovery is not proof of Codex recovery. Keep the frozen five-entry
pilot and all broader unfinished obligations; subprocess probes alone cannot
complete S3.5, S3.2, S4 or whole-game reconstruction.

This continuation repairs reproduced tooling failures: workspace inspection's
optional Git-index refresh rejected otherwise valid type/macro transactions;
duplicate mission selection queried a removed `hash` column instead of the
reviewed hash/size identity; owner-group cancellation killed the cleanup supervisor;
native-check interruption lacked cleanup; partial index responses and blocked
writes could exceed their deadlines. No source, identity, type or macro promotion
follows from these repairs. Existing checks and disposable process probes provide
implementation evidence; independent review and full validation remain separate.

Codex reviewer `01a08459-58dc-7ac3-86db-08057e5bbbec` independently accepted this
tooling slice, including the existing environment-fixture adaptation; no tests
were added. All 1,910 collected cases have outcomes across split runs: 1,908
passed and two skipped. The Git-object mode check and five historical-compiler
checks passed separately with approved sandbox escalation. Final naming/process/
root checks passed 68 cases; lint, format, maps and plan checks passed. The selected
`passesBytePairGate` still matches 20 instructions and 80 bytes. The initial source
audit was interrupted (exit 130); the performance follow-up below resolves it.

The performance follow-up reuses proven manifests/SDK space within each operation
and filters impossible whitespace-delimited tokens before CMake's unchanged
missing-source regex. Every manifest load still resolves and hashes each claim;
comparison reloads ownership after building. Reviewer
`01a08476-b44e-7be3-9e0b-3595295e57d1` accepted the retained changes and typed-error
repair. Parent-directory caching was rejected for a symlink race and removed;
its interrupted exploratory profile is not validation evidence.

The 20-lift GAME01 audit fell from 25.834s to 13.063s after manifest reuse
(102 to 42 loads). Complete CLI profiles then measured 13.158s before and 11.610s
after CMake scan filtering; all reports are identical, with native build products
warm and status summaries bypassed for the repeats. The scan retained all 977
paths while falling from 1.080s to 0.249s. Warm full-report profiles remain about
2.5s; no meaningful warm-cache speedup is claimed. Profiles and JSON live under
`out/reviews/codex-decomp-status-*`; `docs/usage.md` gives reproduction commands.

Final uncached `decomp-status` independently recomputed all 920 lifts across 23
targets: 817 exact, 103 partial, zero invalid, with complete JSON equality to
`out/reviews/codex-source-audit.json`. The final report is
`out/reviews/codex-source-audit-optimized.json`. Existing checks again cover all
1,910 cases: 1,908 passed across split runs, two skipped for missing disposable
forensic inputs. Five sandbox-sensitive compiler failures passed on approved
retries; the Git-object mode check and compiler-pipeline group passed separately.
Lint, formatting, maps and live `passesBytePairGate` instruction/byte checks pass.
No tests, source lifts, symbol maps or frozen pilot evidence were added or changed
by this performance follow-up; audit totals do not establish whole-game completion.

The subsequent frozen battle15 member cleanup corrects only comments in
`func_800B2218.c`, `func_800B22AC.c` and `func_800B250C.c`: canonical
`@residual none` replaces an invalid exact-status residual, and original-backed
behavior replaces UNKNOWN. All three add 32 modulo 65536 to panel x, interpret
the result as signed, then clamp to 320 and clear state only above 320.
Executable source is unchanged. Parent and independent BOF3 reviewer
`01a08494-61ba-7353-9aa8-aaf82c0984f3` verified each 16-instruction/64-byte match;
parent compiler failures passed with approved sandbox escalation. Existing
source/index/macro checks passed 139 cases; whitespace checks pass. No installed
formatter was available; no dependencies were installed.

All 23 snapshots remained fresh. After source acceptance, parent ran `bin/index`
once: all three lifecycle rows now read exact, and the selected
`exact_group:8e1ad03b4ba92303` query clears its registry-level no-exact-member
blocker. `out/reviews/codex-panel-macro-after-index.json` still reports blocked
source-shape/independent-member/semantic acceptance. Seven cross-target members
remain report-only. Frozen report and pilot-input hashes are unchanged; no names,
types, shared implementations or historical receipts were promoted or rebound.
This supersedes historical invalid-status statements for these three only, not
the frozen macro obligation, full-target naming or any phase acceptance.

Retain the review's limits: initialization uses separate bounded send/read
allowances, noisy startup can exhaust a write deadline, and the naming evidence
collector's native timeouts discard partial output. Actual Codex cancellation/resume, escaped-session descendants and
cancellation-time transaction rollback remain unproven. Existing coverage-only
inspection rejects nested Splat metadata and reports unmanifested STR files;
analysis readiness does not establish coverage or semantic naming completion.
Next: finish native recovery before claiming unattended
operation, then discharge the frozen live naming/type/macro and broader source
obligations through their existing owners. No phase status changes here.

The latest Codex integration follow-up moves macro/type/naming query parsing and
adapters into each domain's `cli.py`; `rev-query` now composes those owners.
Existing parser contracts and 176 focused checks passed. All 93 domain/common
modules satisfy formatting and the 450-line ceiling; formatting preserved ASTs.
Four `dispatchWorkTable69*` wrappers received only canonical exact-status residual
metadata. Independent review and live native checks confirm 32 instructions and
128 bytes each; the refreshed index recognizes all four as exact. The refreshed
macro ranking in `out/reviews/evidence/codex-macro-dispatch-ranking.json` has pin
`v1:60c90358ce826d520543986e1986ed33d78bf7f6193be267e346e793b82c9966`:
one defer, four rejections, zero selected. A generic macro must not hide the
local `REGISTER_PIN`; no extraction or type declaration was accepted.

Application/revalidation runtime preparation now shares naming postapply's
bounded process owner with macro/type gates: 120 seconds and 2 MiB of native
output per command, failed timeout/overflow receipts, and owned-tree cleanup.
Injected-runner APIs and receipt schemas remain unchanged. The existing gate
suite passed 287 checks during implementation; direct native probes cover text
compatibility and clipped-UTF8 timeout/overflow behavior after review fixes.
This does not establish actual Codex cancellation-time transaction rollback.

The **broader pipeline remains the acceptance target**:
- **Recovery/S3.5:** the user authorized the bounded `/tmp`-only rehearsal.
  Native cancellation exposed the missing durable recovery mapping described
  below; this is still an implementation gap, not an authorization blocker.
- **Naming/S3:** caller/role/session-bound terminal reservation and the frozen
  live naming obligations remain unfinished; inventory is not semantic closure.
- **Types/macros/S7–S8:** reviewed private-to-shared application, independent
  live consumer proof and the macro bounded-attempt accounting remain unfinished.
- **Lifting/finalization/S4–S9:** the gated lift-to-name-to-cleanup sequence,
  pause/resume/batching and broader coverage still require end-to-end evidence.
No phase is promoted by this integration work. Frozen reports, historical
receipts and original parent approvals are not rebound to changed tooling.

The authorized rehearsal used `/tmp/bof3-codex-recovery-prhacsqm`, one writer at
a time, within the 08:13:41–08:23:41 UTC window. Native Codex tool cancellation
ended sessions 79316 and 17461 without executing Python exception rollback:
the fixture header remained POST, no application was published, and child
heartbeat activity stopped. Reusing the stale PRE manifest rejected before edits.
Explicit parent recovery through the production rollback/quarantine primitives
restored PRE bytes and recorded modes; displaced POST evidence was retained.
A fresh invocation then passed all four fixture-native gates and integrity replay
with external pin `v1:9a2b08a09e0bc74def3ffa549b5207bfd56523d1314d1fac932b3961873ccfe7`.
`out/rehearsal/{cancellation-observation,recovery,result}.json` under that fixture
retain the observations. Live repository status, index and frozen report stayed
unchanged. The fixture used no mocked production callables, but its binary/native
outputs are synthetic, the application is legacy/non-context-bearing, and no
Git-index recovery, parent source acceptance or Codex agent-session resume is
proven. No phase is complete on this evidence.

The first implementation layer now persists root/manifest/run-bound PRE images,
mode/inode facts, intended POST and reserved quarantine destinations before moving
source files. Capture failure aborts before source writes; new directory links
are synced and post-rename mode drift rejects. This is recovery evidence, not
restoration authority or authenticated proof that a writer has terminated.

A follow-up bounded fixture at `/tmp/bof3-codex-recovery-ecmduidg` completed within
08:33:55–08:43:55 UTC. Recovery record pin
`v1:f33a5977df86fea2874d2c1e284a05bddb6ed2363870df3e3c803fd11b5053cf`
was observed before native cancellation of session 64316. The mapping survived;
explicit parent verification/recovery restored the original inode, bytes and
mode. A fresh invocation passed four synthetic gates and integrity replay at
`v1:3c3027f2b075c4661d737e3c7026d23fa00ba980081e8bb86fb473a476bf9f43`.
The fixture's `out/rehearsal/journal-recovery.json` retains the results. No source
acceptance, Git-index recovery, automatic recovery or agent-session resume is
proven; original live inputs remain unchanged.

Read-only recovery inspection is now exposed by both owner CLIs with an external
recovery digest pin. It validates private record/root/owner bindings and PRE
material, reports current source/quarantine/publication observations without
source images, and explicitly leaves manifest re-derivation, POST inode binding,
writer termination/exclusion and workspace/Git verification unproven. This is an
inspection checkpoint, not automatic recovery or pipeline acceptance.

The inspection checkpoint passes 210 existing file/domain/history/shared-PRE/CLI
and harness checks plus the separately escalated file-mode check. Disposable
POST/PRE, CLI, read-only and malformed-record probes pass; independent review
accepted the strict root-identity correction. Ruff, 128 local documentation
links/anchors and all eight migrated skill metadata checks pass. The broader
suite and guarded restoration are not completed by these results. The live index
and frozen naming report remain unchanged, and plan phase states are unchanged.

The user authorizes checkpoint commits after feature completion, without pushing,
then continued goal work. Local commit `88d6925b` contains only the seven reviewed
panel/dispatch metadata corrections; fresh native asm-diff and byte-match pass
for every member. Other source/header/map changes and the broader tooling refactor
remained uncommitted at that checkpoint. Review of the deleted
`test_agent_context.py` identified active compatibility, read-only, bounds and
transport coverage alongside obsolete Pi protocol checks.
Do not silently restore that deleted file or count its missing coverage as passed.
The user subsequently approved a tooling/docs/skills staging scope explicitly
preserving the existing `test_agent_context.py` deletion. That resolves the
checkpoint disposition without restoring the file; applicable lost coverage
remains a follow-up. Local settings, media, generated evidence and unreviewed
source/header/maps stay outside the tooling checkpoint.
Retained proofs are not rebound to the new checkout state.

Next implementation: explicit, restartable owner recovery requiring writer
termination/exclusion, rejecting unexplained drift and reconciling application
publication. Owned-file recovery must not claim coverage for unrelated workspace
or Git-index mutations without durable backing and corresponding validation.
Macro/type application and check-only revalidation now hold a common nonblocking
writer lease across canonical manifest re-derivation, source application, native
gates, publication and exception rollback. Root/path/inode drift and nested
writers reject; the private persistent lock is never removed on release. Naming
report-set locks and nonparticipating commands remain separate. This closes the
cooperating-owner exclusion gap, not guarded restoration, cross-domain campaign
scheduling or prior-worker termination. Native handles remain the termination
evidence; stored PIDs alone are insufficient across tool PID namespaces.

The lease rehearsal at `/tmp/bof3-codex-recovery-2eb_jrgx` ran within the bounded
09:11:21–09:21:21 UTC window and closed at 09:14:26 UTC. Native session 31625
held the lease while type, macro and revalidation contenders rejected. The parent
retained recovery pin
`v1:f0697e02fa67b85d2efbad6af2c06f8cbf59653d272067430802990f13f97f16`
before native cancellation (terminal exit 1), confirmed the child heartbeat
stopped, reacquired the lease and explicitly restored the original inode, bytes
and mode. A fresh invocation passed four synthetic gates and integrity replay at
`v1:05d56f3c8f8b333edee8c220575dce95b858d52085681f1b2e2ede1edbe1e1ce`.
The fixture's `out/rehearsal/lease-recovery.json` retains the result. No automatic
restoration, BOF3 source acceptance, Git/workspace recovery or Codex agent-session
resume is proven. The live reverse index and frozen naming report are unchanged.

Lease validation passes 260 existing file/domain/history/review/shared-PRE/harness
checks, 56 revalidation/CLI checks and the separately escalated file-mode check.
After the final per-primitive guards, 96 overlapping boundary checks pass again.
Twelve disposable rejection probes cover native contention, owner death,
close-on-exec behavior, persistent inode, substitution and unsafe file types.
Injected lease-loss probes preserve unexpected POST or missing-source state and
the original PRE quarantine rather than continuing publication/restoration.
Independent code/docs review reports no blockers; Ruff, decomposition, whitespace,
98 documentation links/anchors and unchanged plan-phase checks pass. Full
`just check` and broader pipeline acceptance are not claimed.

Recovery v2 now stages POST images in fresh private directories before any source
mutation and binds exact hash/device/inode/mode plus staging and reserved
PRE/POST quarantine names. Verified no-replace installation preserves the prepared
inode; rollback rejects same-content substitutions. New-file mode binds the
actual umask result. `common/directory.py` owns descriptor traversal with direct
verb-named imports; `common/images.py` owns staging/installation. Existing fault
injections move to the installation owner without changing their assertions.
Historical v1 inspection stays content-only and is not upgraded.

Independent review found that subsequent Git-backed workspace rollback rewrote
already-restored owned files. It now skips byte-identical PRE paths, preserving
their restored identity/mode. A full-owner failing-gate Git fixture with explicit
baseline adoption verifies this, including reserved POST retention and absent
application publication. This does not establish durable unrelated-workspace or
Git-index recovery; those remain separate requirements.

The v2 native rehearsal at `/tmp/bof3-codex-recovery-jb9069uf` ran within the
09:32:49–09:42:49 UTC window and closed at 09:38:03 UTC. The parent retained pin
`v1:e5f5b7a364adbc35ff163c4b7d2c4a769f6d6671f16ee635ceed4cdb64f21de0`
before cancelling session 60787 (terminal exit 1). With the writer stopped and
lease reacquired, a deliberate same-content/mode clone was reported as drifted
and preserved by refused rollback. Restoring the known POST inode then allowed
explicit parent recovery of original PRE inode/mode/bytes, with POST at its
reserved quarantine. A fresh invocation passed four synthetic gates and integrity
replay at
`v1:c24d20f53f7b94aa76988495c67038b078bdba6332ff278b99b1c8a64c1b57e9`.
The fixture retains `out/rehearsal/v2-recovery.json`. Disposable probes also verify
new-file umask/identity, capture-before-source-mutation, staged-clone rejection and
unchanged historical v1 inspection. No native BOF3 source acceptance or Codex
agent-session resume is claimed; the live index and frozen report are unchanged.

Recovery v3 now reuses the owner's captured workspace/index snapshots to retain
private untouched PRE images/metadata and exact index bytes/path/state alongside
v2 source identity evidence. Guard inspection reports scoped additions, removals,
content/metadata drift and index agreement/locks without exposing archived bytes.
This inventory follows tracked/unignored workspace-backup policy and excludes
generated-artifact roots, not every filesystem path. Non-Git or unsupplied
snapshots are explicitly unavailable; v1/v2 records are not silently upgraded.
Durable backing and observational guards close prerequisites but do not grant
restoration authority or establish atomic, complete workspace/Git verification.

Remaining automatic recovery must independently establish prior-worker
termination, reconcile publication and implement guarded restoration using the
retained source/workspace/index backing. Neither snapshots nor a newly acquired
lease supplies those authority and lifecycle facts.

The v3 Git-backed rehearsal at `/tmp/bof3-codex-recovery-sizl4nt_` ran within
09:59:51–10:09:51 UTC and closed at 10:02:48 UTC. Session 38144 reached a native
gate with source POST installed and matching workspace/index guards. The parent
retained pin
`v1:e14a62937c7675737a8d1713c49345f36d3cbdd3197a508afa6d8f076dc8c2f8`
before cancellation (terminal exit 1). Guard matching and archived bytes survived;
controlled untouched-content drift, an added file and an index lock were detected.
Explicit parent recovery under the reacquired lease restored owned PRE, and a
fresh invocation passed four synthetic gates plus integrity replay at
`v1:a5481ea4252e40daf6e664c5570deff2b8c18a1893005ed9bcf7a720a5973dce`.
The fixture retains `out/rehearsal/v3-recovery.json`; no automated restoration,
native BOF3 source acceptance or Codex agent-session resume is proven.

A separate native Git probe detected staging of an owned edit even though that
path is excluded from untouched-file comparison. Reconstructing an index snapshot
from archived bytes lets the existing primitive restore index bytes/mode/ownership,
but its new inode remains distinct from the original exact guard. Future recovery
must record this verified identity transition, not rewrite the original record or
pretend its guard still matches. Exact index backing is not a complete Git-metadata
backup. The live reverse index and frozen naming report remain unchanged.

V3 validation passes 196 existing owner/file/harness checks and 120 existing
history/review/shared-PRE/revalidation checks. Independent malformed-image and
scope/redaction probes pass; no new persistent tests were added. Archival reuses
the owner's existing reads, but increases storage: the observed live inventory
contained 2,068 files and about 14.85 MB of PRE data, roughly 19.8 MB in base64
before JSON/metadata overhead. This is a measured cost, not a new size limit or a
claim that the full pipeline is complete.

The v2 checkpoint passes 196 existing owner/file/harness checks, 120 existing
history/review/shared-PRE/revalidation checks and the escalated file-mode check.
After final verb-name migration, 36 overlapping file/decomposition/import checks
and the Git-backed full-owner rollback probe pass again. Independent review
accepts the inode-preservation correction and documented scope. Ruff, formatting,
98 local links/anchors, whitespace and unchanged phase-state checks pass. No new
regression tests or dependencies were added; broader acceptance and context
coverage remain open. No additional commit or push at that v2 checkpoint.

## Proposed roles and sequence

`select TARGET@ADDRESS → bof3-lifter → bof3-reviewer → bof3-namer →
bof3-reviewer → bof3-cleaner → bof3-reviewer → record result → next function`

| Project role | Responsibility | Boundary |
| --- | --- | --- |
| bof3-lifter | Reconstruct readable C and obtain an evidence-backed match | One target-qualified function; no speculative identity promotion |
| bof3-namer | Gather/corroborate naming evidence and propose identities | Evidence preparation does not authorize application |
| bof3-cleaner | Apply reviewed canonical identity transactions; byte-safe cleanup | Apply once, validate and roll back on failure; semantic changes return to lifting |
| bof3-reviewer | Inspect actual changes and independently run existing checks | Inspection/check tools, no source edits; reused for final acceptance |

Recover the BOF3 reviewer's actual check-running capability; do not inherit the
built-in reviewer's tool limitations. Document allowed validation side effects.
Review fresh evidence, not a writer's PASS assertion. Naming or cleanup may be a
justified no-op; never manufacture changes to satisfy a stage.

Retain only these four core profiles unless a retained definition proves another
role necessary. Remove coordinator/finalizer wrappers that merely relay calls;
move any unique target freshness or acceptance obligation into its owning skill
or final review stage. One writer, one selector and one serial loop initially.

Agent files should contain role, tools/model, input, skill route, output and stop
conditions. Skills own domain procedures. Put shared rules in one owning place,
not every prompt. Use one small workflow definition with native `runs.run`, a
finite queue and bounded repair rounds. No default generic framework or separate
scheduler. Repairable findings return to the owning stage; failed/unknown native
execution stops for recovery, never silently replays mutations. Record blocked
items honestly and proceed to independent items only within the approved scope.

## Autonomy and cleanup acceptance

- Audit live entrypoints, role names, removed-script links and stale plan anchors
  (including `docs/INDEX.md`'s missing `#N1`). Repair only references owned by
  this implementation; historical records are not active instructions.
- Preserve the starting dirty work. Use one writer in this checkout; require a
  clean source checkout before any managed worktree allocation. Never reset,
  stage or commit unrelated changes to satisfy a launch prerequisite.
- Select a finite queue from live target-qualified evidence; exclude accepted,
  duplicate and already-owned work. Record selector, baseline and stage outcome
  in native run artifacts, not a second campaign database.
- Require explicit structured accepted/no-op/repair/blocked outcomes bound to
  the selector and actual changed content. Missing, malformed, stale or failed
  results stop advancement; writer self-approval cannot satisfy review.
- Reviewer acceptance authorizes only the evidenced identity transaction within
  the approved scope, not speculative names or new privileges. No-op naming and
  cleanup require reasons. Limit repair rounds and total runtime/spawns.
- Resume only after inspecting native run state and current content; never
  blindly repeat identity application. Infrastructure failure stops the loop;
  domain-blocked items may yield to independent approved items. No silent CLI
  fallback or permission widening. Native recovery must be demonstrated, not
  inferred from a successful happy path.
- Compact changed project agent/skill Markdown without weakening its contracts;
  remove duplicate instructions, not unique evidence gates. Delete temporary
  outputs only when proven owned by this implementation and no longer needed;
  preserve earlier recovery copies, session evidence and unrelated `tmp/` work.
- Handoff one documented launch/resume path with finite defaults, supported
  inputs, stop conditions and limitations. Run existing relevant checks and
  independent review; report unexercised paths as NOT RUN. Full autonomy means
  unattended work inside approved boundaries, not unbounded execution.

Recovery exposed two required scope corrections: `audit-target TARGET` is a
target-wide audit, not a function-only naming request; reuse validated target
reports rather than auditing an entire target afresh for every function. The
former reviewer's lesson-edit exception moves to a scoped writer transaction;
review itself must remain no-edit. The previously deleted compaction skill is
absent despite its stale session advertisement; do not silently restore retired
machinery or claim that unavailable skill ran. The owner explicitly approved
replacing that obsolete requirement with direct Markdown compaction and existing
checks; `AGENTS.md` now records the replacement. This was the S1.1 unblock; its later acceptance is recorded below. Current
AGENTS.md and the plans skill were read: direct compaction is the policy, the
retired skill is physically absent, and no separate compaction tool was run.

These requirements extend S1–S4 acceptance; implementation and demonstration
must satisfy them before the corresponding phase is marked done.

## 1. [S1] (done) Recover and streamline project BOF3 agents
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: latest BOF3 role names retained in native session history; older BOF3 definitions available in Git HEAD.
- Acceptance: latest source bodies identified, four minimal project-owned profiles proposed, no imported generic or built-in profiles.

1. [S1.1] (done) Compare retained definitions and preserve only unique domain responsibilities.
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: recovered complete read results from native sessions: 2026-09-07 ef728133/run-0 (lifter/reviewer/coordinator/finalizer), 2026-09-06 0397af18/run-0 (namer/cleaner); later namer read eac7279d/run-0 matches SHA-256 21e12fd3d88a593cbdb9f1b893ac8ab388cdacb1c2736397b58243760089f4c5; Git HEAD bof3-review.md inspected as older baseline.
- Acceptance: restore and simplify bof3-lifter, bof3-namer, bof3-cleaner and bof3-reviewer under .pi/agents after approval; retain reviewer check execution without source-edit authority; remove redundant coordinator/finalizer profiles and stale orchestration requirements without losing domain checks.

## 2. [S2] (done) Streamline the project skills around those roles
- Owner: parent with bof3-reviewer
- Depends: S1
- Blocker: none
- Evidence: six project skills remain; their actual instructions require a focused overlap/reference audit.
- Acceptance: concise entrypoints, explicit routes, one owner per rule, valid links and existing relevant checks; no weakening byte or naming evidence.

1. [S2.1] (done) Align lift, naming-evidence and identity-maintenance contracts.
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: .codex/skills/bof3-re, .codex/skills/bof3-naming-evidence and .codex/skills/bof3-identity-maintenance.
- Acceptance: consistent target-qualified inputs/results; separate naming proposal, approval and application; remove duplicated prompt/orchestration text; retain byte-match and rollback requirements.

2. [S2.2] (done) Reconcile supporting skills and project instructions.
- Owner: parent
- Depends: S2.1
- Blocker: none
- Evidence: .codex/skills/psx-rizin, .codex/skills/repo-documentation-repair, .codex/skills/plans, AGENTS.md and docs/INDEX.md.
- Acceptance: preserve explicit psx-rizin opt-in and documentation-only scope; eliminate stale imported-agent/default-loop references; no home/package skill edits or global skill rewrite.

## 3. [S3] (in-progress) Prove machine integration, native recovery and one bounded loop
- Owner: parent
- Depends: S2
- Blocker: none
- Evidence: accepted bounded owner/handler/accounting slices below; accepted native preapply interrupt/read-only retained recovery cell recorded below; remaining recovery matrix and loop execution NOT RUN.
- Acceptance: independently accepted S3.4 machine integration, S3.5 actual native recovery and S3.2 bounded loop; project bof3-* profiles, finite budgets and no custom infrastructure. Fail-closed blocked handling proves machinery, not S4 live closure; separation contract below governs.

1. [S3.1] (done) Exercise one direct target-qualified handoff before automation.
- Owner: parent and BOF3 roles
- Depends: none
- Blocker: none
- Evidence: workflow 3a519b7f lifted emi/battle/battle/15@0x800A3638; reviewer 5e97e6aa accepted 20/20 instructions and all 80 bytes with no matching aids. Parent refreshed battle15 snapshot and rebuilt the index. Naming workflow f58d0343 and independent reviewer 459539e5 reproduced inventory failure (249 report rows versus 250 live rows), canonical report SHA-256 39fbb754e5b8a0f3b7e30a92bf611cc7689768d9be60c9e54e1b5f5a8e6d2856. Audit remains blocked, not no-op; cleanup unproven. Existing initialize(root,target) can generate current blocked rows in memory, but neither CLI initializer merges preserved rows.
- Acceptance: restored role discovery/tool availability verified; lift/naming/cleanup handoffs need no invented permissions or duplicate skill instructions; blockers attributed to actual domain evidence.

2. [S3.3] (done) Freeze indexed pilot coverage and owner routing.
- Owner: worker with independent reviewer
- Depends: S3.1
- Blocker: none
- Evidence: independent S3.3 review-0.md accepted the declared-global repair, all five frozen payloads and fingerprints, exact macro membership and 294 checks; checkpoint below. Historical selected FUNCTION verify is not rebound.
- Acceptance: first implementation slice: characterize existing index/query behavior in test_reverse_index.py, test_type_index.py and test_macro_opportunities.py; minimally repair owning index/query omissions only if reproduced. Freeze the five pilot entries below with current IDs, fingerprints, owners and overlap exclusions in native run inputs; all five classes route or explicitly block, never disappear. Reviewer independently runs focused tests and inspects inventory equality; no target/report writes.

3. [S3.4] (done) Accept concern-owned machine integration and frozen accounting.
- Owner: worker with independent reviewer; parent owns scope acceptance
- Depends: S3.3
- Blocker: none
- Evidence: whole S3.4 independently accepted by 2dc9dec6 (715 passed, 2 skipped), attributed in the superseding acceptance record below. S3.4 checkpoint below records independently accepted duplicate-key rejection and the independently accepted native-gate prerequisite repair and accepted execution-context/publication prerequisite and accepted parent-envelope slice; accepted reviewed shared-input migration; accepted no-drift check-only revalidation including B1; accepted fresh no-drift parent acceptance; accepted internal historical validation prerequisite; accepted same-target exact private transition; accepted distinct-target common-PRE integration; accepted shared PRE/POST integration; accepted current-byte owner CLI transport (b295c173); accepted bounded terminal CLI/API (e2920f90, 605 passed/two skipped); accepted all-handler fixture sequence (11fcb490) and bounded frozen-five accounting (efb1b118), not native execution or live closure; frozen local-data control BLOCKED (d740aabe), not accepted exhaustion/no-op. naming/postapply*.py supports selected target-local FUNCTION only; existing type/macro digest pins do not establish independent final review.
- Acceptance: independent reviewer decides whole machine readiness from accepted owner, all-handler and frozen-accounting coverage: proposal versus approval, dirty-baseline adoption, scope/storage/layout preservation, gate rejection, digest-bound final verification and all-five accounting. Supported fixture positives must be labeled synthetic; unsupported/live-blocked routes fail closed. Actual Pi recovery belongs to S3.5; live evidence/application/full-target closure moves to S4.3/S4.4 below. Never route data/types/macros through FUNCTION-only postapply or invent a framework; no writer self-approval.

4. [S3.5] (in-progress) Prove failure, rollback and resume before unattended execution.
- Owner: worker with independent reviewer and parent
- Depends: S3.4
- Blocker: none
- Evidence: parent accepted reviewer 9342ff6b's bounded preapply interrupt/read-only resume cell and reviewer 12b72400's retained active-tool timeout/closure cell below; no transaction rollback or independent hard-stop/process-tree acceptance.
- Acceptance: run the failure matrix below against owner transactions and the small native workflow; inspect actual run state/tool outputs, not prompt text or exit zero alone. Reviewer verifies rollback bytes/modes/absence, unchanged unrelated dirty work/index and no duplicate apply; parent owns freshness recovery and attestation. Report unavailable native capabilities as blockers, not mocked acceptance.

5. [S3.2] (blocked) Encode the proven sequence with bounded repair and native recovery.
- Owner: parent
- Depends: S3.5
- Blocker: whole S3.5 remains unaccepted despite its bounded preapply cell; no bounded native loop is demonstrated. Full-target naming blocks production, not controlled fixture stages or read-only blocked accounting.
- Evidence: S3.1 handoffs and bounded efb1b118 accounting accepted; neither is a native loop. Parent freshness/attestation checkpoints remain explicit and cannot be delegated implicitly.
- Acceptance: native syntax validation, existing applicable checks and independently reviewed finite execution with fixed queue/budget. May select/account blocked entries and simulate controlled fixture stages, never advance a failed/unknown mutation or claim live campaign completion. Production cannot pass failed unfiltered full-target naming complete:true or missing separate identity approval; final review binds actual selector/change. Accepted work skips; uncertain apply stops for recovery; no budget reset on resume.

## 4. [S4] (open) Demonstrate useful autonomy and close accepted scope
- Owner: parent with bof3-reviewer
- Depends: S3
- Blocker: none
- Evidence: none
- Acceptance: real target-qualified results, existing owning checks and independent review; no claim of whole-game coverage from samples.

1. [S4.1] (open) Complete one function through lift, naming and cleanup.
- Owner: bof3-lifter, bof3-namer, bof3-cleaner and bof3-reviewer
- Depends: none
- Blocker: none
- Evidence: historical passesBytePairGate selected-row success accepted by 94f37f72 in workflow 93ed23ed; current tooling closure is stale per efb1b118. Remains open under S3 dependency; fresh gates/review without rename replay and automated final cleanup/review are required.
- Acceptance: instruction/byte checks as applicable; validated identity application or justified no-op; cleanup preserves final bytes; final reviewer reports actual residual risks.

2. [S4.2] (open) Run a small approved queue and measure intervention cost.
- Owner: parent
- Depends: S4.1, S4.3, S4.4
- Blocker: none
- Evidence: finite pilot and closure matrix below; no queue execution accepted yet
- Acceptance: finite queue/budget, native stop/resume without duplicate writes, honest completed/blocked/deferred outcomes; assess elapsed effort and parent interventions before scaling.

3. [S4.3] (blocked) Close the frozen five live domain obligations.
- Owner: parent and BOF3 evidence/application roles with independent reviewer
- Depends: none
- Blocker: all five frozen entries remain blocked; linked storage/layout same-envelope closure is incompatible with current concern rules; exact private/shared domain evidence is absent.
- Evidence: efb1b118 accepted accounting: accepted 0, noop 0, blocked 5, historical_skip 1 (overlapping), linked pair blocked; campaign_complete false and production_complete false.
- Acceptance: discharge the S3.4 live obligations explicitly mapped below through current concern-owned evidence, separate approval/application, native gates and independent final verification. No speculative names, receipt rebinding or fixture substitution. Resolve linked storage/layout schema design only with explicit approval and independent review; preserve concern isolation, serialization and both IDs. Every frozen entry must be accepted or independently justified no-op/exhaustion for finite closure; blocked/deferred remains unfinished.

4. [S4.4] (blocked) Restore and satisfy full-target naming production closure.
- Owner: parent with bof3-namer and bof3-reviewer
- Depends: none
- Blocker: unfiltered report rejects func_800A3638 binding_locations: missing=[] invented=['config/targets/emi/battle/battle/15/symbols.txt']; full-target complete:true is absent.
- Evidence: efb1b118 actual unfiltered verifier failure; selected FUNCTION history and frozen accounting do not override it.
- Acceptance: separately authorize evidence-preserving report/provenance recovery if needed; obtain successful unfiltered full-target complete:true and separate identity approval before production advancement. Never narrow the report gate, rewrite historical receipts or treat selected acceptance as target completion.

## Milestone separation and superseding obligation map

The user explicitly authorized separating machine integration/recovery/loop from
live campaign closure. This section supersedes earlier **current-readiness** claims
that live S3.4 closure/full-target naming must precede any S3.5 rehearsal or S3.2
machine loop; historical findings and their evidence are retained, not rewritten.
No existing ID or done status changes. S3.4 remains in-progress for an independent
whole-machine verdict; this writer does not infer readiness from slice acceptance.
S3.5 still depends on accepted S3.4, S3.2 on accepted S3.5, and S4 on accepted S3.
No unit edge or implicit launch permission bypasses those dependencies.

| Old obligation / ID | Retained or new owner ID | Acceptance preserved / attribution |
| --- | --- | --- |
| S3.4 owner gates, all-handler integration, five-entry accounting | S3.4 | 11fcb490 accepts handlers only, not subprocess/Pi or every positive replay placement; efb1b118 accepts the read-only frozen consumer only. Reviewer must decide combined machine readiness; fail-closed handling is admissible, not live success. |
| S3.4 current FUNCTION closure | S4.3; S4.1 retains end-to-end function demonstration | Fresh owning gates/review without rename replay or rebinding; historical FUNCTION skip is informational and overlaps blocked, not current acceptance. |
| S3.4 data terminal acceptance | S4.3 | D_80096994 remains blocked: capability integrity/historical exhausted row is not accepted semantic exhaustion/no-op. Retain name, report and unresolved findings; no peripheral research merely to account blocked. |
| S3.4 fixed-RAM/storage and aggregate evidence/application | S4.3 | Owners/access/base/extent/role and layout/alignment/padding/semantics remain unproven. Same-envelope positive requirement cannot currently satisfy concern isolation: deferred live/schema design obligation, not erased or weakened into different-envelope acceptance. |
| S3.4 real type/macro private members and useful sharing | S4.3 | Two independently exact private members, corroborated equal contracts and useful common body still required. Frozen three invalid macro lifts are not exact; seven external observations remain report-only. |
| S3.2/S3.4 full-target production naming prerequisite | S4.4; S3.2 retains enforcement | Exact binding failure above remains; production still needs unfiltered complete:true plus separate identity approval. A machine loop may stop/account it, never advance through it. |
| S3.4 closure-matrix recovery; S3.5 failure matrix | S3.5 | Actual native stop/cancel/timeout, rollback and state-inspected resume still mandatory; fixtures cannot replace native evidence. |
| S3.2 bounded orchestration; S4.2 useful finite campaign | S3.2 / S4.2 respectively | Machine loop may account blocked and simulate controlled fixture stages; S4.2 owes live accepted/no-op closure and intervention measurement, not five-blocked success. |

The integration/closure matrix below is read with this mapping: S3.4 owns machine
routing, supported owner fixtures and fail-closed accounting; S3.5 owns actual
native recovery; S4.3/S4.4 own live positives and production closure. All five
blockers, linked pair and full-report failure survive. The frozen consumer's
unchanged-envelope ceiling needs a separately reviewed fresh baseline for later
mutations; do not silently replace the five or fabricate current positive proofs.
Finite ceilings remain five entries, one writer, two repairs/entry, 60 spawns and
two hours, with stricter native timeouts and no reset on resume. Machine completion
never implies full finite campaign or whole-game completion.

Latest independent accounting acceptance is
`.pi/sessions/subagent-artifacts/outputs/efb1b118-5597-4a95-a40e-991f244688c6/review-frozen-accounting.md`:
161 checks and actual five-blocked accounting preserving 435 byte/mode states.
It supersedes only the pending accounting verdict below; its whole-S3.4 blocker
was against the former combined scope, not approval of this new machine scope.
The all-handler attribution remains 11fcb490 as recorded below. No native runtime
has been exercised by this plan revision; full source-audit timeout is not a pass.

After independent plan/machine review, smallest S3.5 rehearsal is one frozen
selector, one writer, a short parent-frozen budget and **actual native stop before
apply**, then inspection of native run state, owned bytes/modes/absence and
unrelated/index preservation before a nonmutating resume/skip decision. This is
only the first failure-matrix cell, not S3.5 acceptance. Later cells still require
actual cancellation during mutation/checks, after apply before review, and after
acceptance, plus timeout/tool/partial-write/verification failure, owned rollback
and no duplicate apply. Any mutating rehearsal needs separately scoped safe owned
inputs and authorization; controlled fixtures label synthetic domain evidence.
Verify current native runs.run, stop/cancel and state/resume protocol/tool exposure
first (historical 0.66.0 discovery is not current runtime evidence), four project
role discovery/check tools, one-writer ownership, frozen baseline/budget and parent
freshness/attestation/recovery availability. Unavailable native tools block rather
than trigger CLI fallback, installs or extension edits. Worktree allocation still
requires a clean source checkout; no unauthorized Git writes to clean this one.

## S3.4 whole-machine acceptance (superseding status record)

Parent-authorized recording of independent reviewer `2dc9dec6`'s **whole S3.4
acceptance**, not an inference from slice passes:
`.pi/sessions/subagent-artifacts/outputs/2dc9dec6-1d6e-4561-a518-fa9ad96db172/whole-machine-s34-review.md`.
The combined review ran **715 passed, 2 skipped** and real five-blocked accounting;
synthetic owner/handler/positive-consumer fixtures prove the machine boundary,
not live domain closure or native recovery. Before this status-only edit, the
reviewed plan SHA-256 was verified as
`bd7dd15e04a952cd5978598b88ffbdeb4c5cb228a469fe3036ebe93cd6505d0f`
and the sorted 220-file harness manifest SHA-256 as
`6c0aebe1780db49ecb31c5d5efae9bf146b2d8a4dd64961f5c38304d800bc82d`
(sorted `tools/python/harness/**/*.py`, each `SHA256  repository-relative-path\n`).

Only S3.4 becomes done. This supersedes earlier pending whole-machine/readiness
claims, including the separation map's pending verdict, not historical evidence
or obligations. S3.5 remains open with its S3.4 dependency now satisfied; native
capability, baseline, budget and authorization prerequisites still precede any
first pre-apply interruption cell. No fixture preparation or native launch occurs
here. S3 stays in-progress, S3.2 blocked, S4 open and S4.3/S4.4 blocked unchanged;
all live five-entry, linked same-envelope, full-target naming, full native failure
matrix and source-audit limitations remain. Machine acceptance is neither launch
permission nor campaign/production completion.

## S3.5 preapply cell (accepted bounded scope; superseding status record)

Parent accepted independent reviewer `9342ff6b`, finalized by retained continuation
`41d69a9a`, at 2026-09-09T00:22:09.448Z:
`.pi/sessions/subagent-artifacts/outputs/9342ff6b-5a71-40de-98f0-5ff48cebe0fe/review-native-preapply.md`.
Only preapply native interruption, retained read-only recovery/skip and captured
preservation close. S3.5 becomes in-progress, not done; this supersedes earlier
NOT RUN/open claims for this cell only. S3.2 stays blocked; S4 stays unchanged.

Workflow `1f1751d3` remains failed/paused-after-interrupt, not relabeled success.
Original child `a0d25e00` printed the no-transaction checkpoint then its bash tool
returned `Command aborted` (00:13:43.752Z–00:13:52.999Z). Paused state and observed
runner closure establish resumable termination, not task success. Recovery
`43b8ea26` appended to the same persisted session: exactly one checkpoint execution
across 16 tool calls; zero applications/repairs, no historical FUNCTION replay.
Its runner closure and canonical-session lease release were observed.

Independent captures under `/tmp/bof3-native-preapply-eb2b18c2/` compared 26,030
states and all nine Git evidence files with zero drift, including union membership
and five owner-root enumerations. Preserved main/HEAD, raw index, empty staged set,
starting dirty status/binary diffs, captured Git-operation absence, plan/pilot,
historical FUNCTION absence/current renamed source and recursive report/index roots.
This is sampled preservation, not continuous monitoring: arbitrary unseeded ignored
paths, symlink referent contents, all metadata, transient restored changes and active
native artifacts are not globally covered. No rollback/restoration or independent
OS process-tree sweep occurred. Remaining native matrix: interruption during
mutation/checks, after apply/before review and after acceptance; timeout/tool failure,
partial-write/verification failure, owned rollback and no duplicate application.

Original reviewer `9342ff6b` failed with `Subagent timed out after 180000ms`;
its incomplete inspection was not acceptance. Recovery took 137.536s, exceeding the
requested 120s target; its status has no deadline. Original three-launch/600s
rehearsal allowance is exhausted (deadline 2026-09-09T00:22:36.394Z); the separately
authorized five-minute reviewer continuation did not reset it. Original campaign
budget remains unknown. No further native launch or domain authority follows.

Read-only timeout investigation traced actual parent calls and installed native
0.66.0 owners: `43b8ea26`'s resume supplied 120s only in message text, no API
`timeoutMs`; direct revival forwards only an explicit timeout/internal deadline,
not the stored source deadline or normal launch default. Its absent status field
therefore accompanies an absent run-deadline timer, not just serialization loss.
`41d69a9a` supplied API `timeoutMs:300000`; status and recovery descriptor retain
absolute deadline 1788913575510, with 117.179s completion before expiry. Native
launch passes that deadline to the runner's real timeout/abort path. This proves
configured propagation plus early completion, not expiry enforcement on resume.
No repository-owned native caller defect was reproduced; no code/helper is needed.
Use explicit native `timeoutMs`, never task text as the timer or a fresh relative
allowance as an implicit ledger reset. Investigation/evidence and the concrete
parent-executable next timeout cell (NOT RUN), including absolute budget requirements:
`.pi/sessions/subagent-artifacts/outputs/db9bd640-9c16-401d-a08f-4e6224e049c7/record-and-advance-recovery.md`.
That proposed cell requires separate bounded native execution authorization; this
continuation permits reconciliation/development only. Full source-audit timeout
remains unresolved, not passed; independent review of this recording is required.

## S3.5 retained-timeout cell (accepted bounded scope)

Independent review `12b72400` accepted retained-resume timer expiry during an
active tool, captured preservation and observed runner closure/lease release:
`.pi/sessions/subagent-artifacts/outputs/12b72400-9be4-4049-a2f4-b63f5ed52f9d/review-retained-timeout.md`.
Actual run `974d6e09-8987-4d5d-a651-84ab6a4dec36` supplied API
`timeoutMs:120000`; native timeout occurred 1 ms after deadline 1788914432546,
followed by `Command aborted` and closure 4,060 ms after timeout. The run remains
failed/timedOut/acceptance rejected, not successful. Review verified 26,030 states
and ten Git/inventory files unchanged, raw index
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`,
empty staged paths, absent new timeout output and preserved prior report.

This supersedes NOT RUN/expiry-unproven claims only for this retained-timeout cell.
S3.5 stays in-progress; S3.2 blocked and S4 unchanged. Rollback, during-check,
after-apply/before-review, after-acceptance and independent hard-stop/process-tree
cells remain unfinished. Old rehearsal budgets remain exhausted and original
campaign allowance unknown; this acceptance resets neither and grants no launch
authority. Current separately scoped disposable failure demonstrations are
synthetic machine evidence, not live source/report/index fault authority.

## Refreshed selected-row baseline and finite pilot

Selected `emi/battle/battle/15@0x800A3638` is now `passesBytePairGate`,
APPLIED and independently accepted by reviewer `94f37f72` in workflow
`93ed23ed`. Parent actually completed postapply-review and final verify (exit 0,
`applied:true`, `rows:1`). This plan-only refresh repeated **read-only verify**
with the retained bundle and explicit evidence root; it returned the same result.
Frozen report `out/reviews/plan-audit-naming/emi__battle__battle__15.json` retains
SHA-256 `bc6c9cabcb1dfea5cbf3746598cf7e966b58742fe92876cf4e4d53cbf5f2a500`.
Evidence root is `out/reviews/evidence-access-selection-regeneration`; bundle is
`postapply-ae216471b00c2e8f9e615c11a4cc3d29/reviewed-postapply-809d3534b4a012ce2f5c40c4c85e15e5.json`
under that root; parent attestation is root-level `parent-review-d8be8adb.json`.
No gates, report, source, target or index regeneration occurred in this refresh.
Future tooling edits can invalidate this closure: retain the historical success,
then obtain fresh gates/review rather than rewriting or rebinding old receipts.

This resolves the historical missing proposal/application blocker for **one
function**, not full-target audit, automated loop, type/macro acceptance or S4.
The older 249/250-row failures and hashes below remain historical evidence;
current inventory after rename need not equal the frozen preapply report.

Pilot selection below comes from live `rev-query` type-candidates,
macro-opportunities and variables queries plus the verified function binding.
These are five work entries, not five promised mutations. S3.3 freezes current
fingerprints and full membership before execution; new entries or replacement
candidates require parent approval. Already accepted entries are read-only controls,
not repeat applications. Cross-target observations do not expand write authority.

| Pilot | Live identity / owner route | Current outcome / exact next evidence |
| --- | --- | --- |
| Function | `emi/battle/battle/15@0x800A3638`, frozen `function:func_800A3638`; naming prepare/postapply/verify | Accepted control above; prove skip/replay detection and final loop accounting without renaming again. |
| Local data | `emi/battle/battle/15@0x80096994`, `data:D_80096994`; naming evidence/validation | BLOCKED per independent review d740aabe below: retained capability validates, semantic exhaustion/no-op is not accepted. Preserve historical exhausted report row/name; parent may scope read-only handler review through bof3-reviewer, not row dispatch to bof3-namer. |
| Shared fixed RAM | `emi/battle/battle/15@1F800044:storage`; type candidate, mapped `g_battle_work = 0x1F800044` | Width 4 lead; base/extent/semantics unresolved. `variables g_battle_work` returns no rows despite map/type presence: S3.3 must establish whether intentional query scope or coverage gap and route fixed RAM explicitly. Corroborate real owners/accesses before any shared change. |
| Struct/layout | `emi/battle/battle/15@1F800044:aggregate_region`; type-audit | Indexed blocked lead: extent/alignment/padding/semantic role unproven. Shares address with storage entry: one serialized concern transaction, linked accounting, not two conflicting applications. Two independent observations required; no inferred struct identity. |
| Macro opportunity | `exact_group:8e1ad03b4ba92303`, target scope `emi/battle/battle/15`; macro-audit | Members `@800b2218`, `@800b22ac`, `@800b250c` were invalid at freeze; the reviewed Codex metadata cleanup above establishes exact bytes and indexed status. Macro acceptance remains BLOCKED: cross-target private/shared proof and semantic/source-shape gates are unclosed. Membership is frozen below; no shared promotion follows. |

Initial implementation was **S3.3**, not a production rename: worker adds
narrow index/routing characterizations and only reproduced owner fixes; reviewer
checks actual queries and tests. Follow with S3.4, S3.5, S3.2, S4.1, S4.2, one
writer/reviewer iteration per slice. Parent launches these iterations; BOF3 roles
retain all domain skill gates. No worker/reviewer profiles are installed/copied.

## S3.3 accepted implementation checkpoint

Independent reviewer `30a418da` accepted the prerequisite plan-only refresh in
`.pi/sessions/subagent-artifacts/outputs/04f0918b-70b7-4249-afca-a4b77958d75e/review-autonomy-plan.md`;
46 plan checks and read-only selected FUNCTION verification passed there. This
accepts the plan, not S3.3 runtime or subsequent transactions.

Worker run `e423967f` reproduced `variables g_battle_work = []`: symbol insertion
classified every non-`D_` map name as a function despite the target-owned global
declaration. The repair uses existing `header_claim` global type usages, rejects
function-entry/prototype collisions, preserves unproven fallback and target
isolation, and bumps the disposable index schema to v12. No domain identity or
storage authority changed. Parent ran `bin/index --recover` (exit 0); subsequent
live queries return `g_battle_work` as data. `describe` still reports unknown
storage authority outside the payload: indexing is not fixed-RAM ownership proof.
Battle15 snapshot is fresh; binary SHA-256 remains
`77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140`.

Frozen native inputs:
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/pilot-input.json`,
SHA-256 `bb68a07c5fe271005251d11634b6ed4496a28418d961193304ad39f00719c218`.
Exactly five entries retain owner routes, source/binary/index fingerprints,
blocked/control outcomes and storage/layout serialization. These are review
inputs, not launch or application approval. Function remains a historical accepted
skip control; local-data exhaustion needs receipt revalidation. Both type concerns
remain blocked. Tooling changed: old final-state bundle is historical only; fresh
owning gates/review are required, never receipt rebinding.

Macro membership was equal before/after recovery: the selected three battle15
members in the pilot table are a target-filtered subset of ten reviewed-byte
members, not a missing-member defect. The other seven are report-only:
`emi/etc/game/00@801996fc` and `emi/etc/shop/00@801e31c4`, `@801e3774`,
`@801e3bf8`, `@801e3d4c`, `@801e438c`, `@801e4540`. All ten have invalid lift
status; reviewed SHA-256 is
`b5a0227129c99631166270ac80b2bfc769b0c330181a0be7b9ca7e97adcaee78`,
size 64. No macro query repair or cross-target write expansion was needed.

Focused reverse/type/macro opportunity/index and harness decomposition checks:
116 passed; 46 plan tests and plan status passed; scoped Ruff and `git diff --check`
passed. Bounded `just check` timed out at 110 seconds during pytest (past 81%, no
failure shown); full-suite/lint/maps/source-audit completion is not claimed.
Tests cover refresh inventory
equality, stale headers, classification/collision/fallback/isolation, distinct
blocked fixed-RAM concerns and complete target-filtered/global macro membership.
Independent review now accepts S3.3 as recorded below; S3.4 is the next slice. Native recovery/application and full-target
naming closure were not exercised and remain later gates.

Independent read-only reviewer accepted S3.3 in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/review-0.md`:
174 routing/index/plan/harness tests and 120 owner tests passed (294 total).
All five frozen payloads, 437 source fingerprints, three binary/snapshot pairs,
index/report hashes and ten-member macro inventory were independently equal.
No source/report/index mutation or fresh historical FUNCTION closure was claimed.
This acceptance does not complete S3, full-target naming or native recovery.

## S3.4 accepted trust-boundary sub-slice

Supervisor approved limiting this iteration to duplicate-key rejection at existing
reviewed type/macro JSON boundaries. Their previous last-value parsing accepted
contradictory top-level and nested reviews while the final parsed digest remained
valid. Reuse the naming parser's existing `unique_object`, moved to `harness/io.py`
with direct importer updates to avoid an analysis-to-naming dependency. Apply it
to type candidates/private proofs and macro opportunities/exact proofs. No schema,
valid-input policy, domain identity, transaction authority or full-target gate
changes. Ordinary `read_json` and other parsing boundaries are unchanged.

Eight added parameterized owner cases plus one extended shared-macro proof case
cover duplicate schema/reviewer/verdict/semantic-guard/concern rejection; prepare
rejections preserve all fixture file bytes. Last-value parser substitution makes
all nine cases fail to reject; strict parsing passes the 128 owner checks. An additional
194 naming/plan/harness checks passed, two skipped; scoped Ruff and plan status
passed. `just check` timed out after 110 seconds at 81% pytest, no failure shown;
full-suite/lint/maps/source-audit completion is not claimed. These
are fixture integration results, not native pilot application or independent
review. Snapshot/index/report and historical proof artifacts remain untouched.

Independent read-only review accepted this duplicate-key sub-slice in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/review-1.md`:
439 tests passed, two skipped; scoped lint/format and five-class live payload and
fingerprint comparisons passed. No blockers or domain mutations. This accepts
only the prior repair, not S3.4 final review or native recovery.

S3.4 remains in progress. Next after this sub-slice's independent acceptance:
add concern-owned final review ingestion and verification, keeping the order
`execute native gates → distinct reviewer inspects → parent attests → final verify`.
The approved local trust model requires explicit parent attribution of distinct
implementation/reviewer run IDs and retained reviewer artifact, bound to the exact
request/application/final state and native gates. Existing writer-created digest
pins remain integrity evidence only, never relabeled independent acceptance.
Use owner-specific review input/envelope additions; no generic framework or new
credentials. Unsupported local-data/shared closure stays fail-closed and storage/
layout stays serialized. Remaining all-class coverage, fresh native domain gates,
parent attestation and S3.5 cancellation/recovery are NOT RUN; no live promotion
or stale receipt rebinding is authorized by this repair.

## S3.4 accepted native-gate prerequisite

While preparing parent-review ingestion, the worker traced exact type/macro
checks to the native asm-diff/byte-match payload owners: exit zero alone accepted
warning, malformed and missing-status JSON. Supervisor approved repairing this
shared owner seam first rather than layering final authority on insufficient
gates. Both exact gates now require their closed native schema, positive exact
and byte-match booleans, selected source/function/address, target-owned binary,
equal positive sizes and coherent asm counts/no mismatch. Splat/build retain their
plain-output exit contract; non-exact transactions retain pinned partial evidence.
Strict duplicate parsing applies to native JSON and retained command receipts.
Exact stdout remains complete and separate from retained stderr, so receipt replay
cannot mistake diagnostics or a truncated JSON tail for the command payload.

Targeted type/macro fixture integrations cover exit-zero rejection, rollback and
failed receipts, plus successful receipt replay with large stderr and rejection of
rehash-consistent warning receipts. These are not live BOF3 gates or final review.
No request/application authority, shared promotion, full-target naming gate,
source/map/type/macro identity, index or native pilot artifact changed. New owner
parent envelopes/context capture are NOT IMPLEMENTED in this coherent prerequisite
slice; old application pins remain integrity-only. Validation: 225 owner/naming/plan/harness checks and 117 routing/index checks
passed; scoped Ruff, plan status and diff whitespace passed. All 437 frozen
source fingerprints, three binary/snapshot pairs and index/report hashes remain
equal; staged paths are empty. `just check` timed out at 110 seconds during pytest
(past 53%, no failure shown); full-suite/lint/maps/source-audit completion is not
claimed. Evidence is in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/implement-2.md`;
independent review accepted this prerequisite in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/review-2.md`: 342 checks passed; no blockers. This accepts native payload/receipt integrity only, not S3.4 completion.

Next ready work remains S3.4: approved owner-specific review/final-verify APIs,
caller-retained parent-envelope digest, explicit distinct implementation/reviewer
runs and supervising parent attestation, retained reviewer bytes, native execution
context across gates/publication (inputs, modes/absence, tooling/environment and
index), and shared-promotion rejection of legacy integrity-only pins. Preserve
`gates → reviewer → parent → final verify`; unsupported data/shared pilot closure
stays blocked. No S3.5 or live application is authorized by this prerequisite.

## S3.4 execution-context slice (implementation pending independent review)

Type/macro `run --implementation-run-id` now binds an owner-derived closure
before edits, around each native check and application publication. Macro reuses
the type-owned runtime; naming delegates its existing closure to that same input
owner without changing FUNCTION scope. Context retains request/manifest/adopted
baseline, sources/headers/config/binary/tooling/catalog presence, modes, index,
resolved native tools/environment digest, explicit edits and native receipt/build
transitions. Ninja is the supported build ceiling; configured variants must
already exist and environment toolchain overrides reject. Legacy omitted-ID run
and verify remain integrity-only; context itself is not independent review.

Mode capture reproduced an existing application defect: quarantine followed by
new-file publication reset nondefault permissions. Supervisor approved forwarding
the original mode through validated optional `atomic_write(mode=...)`; omitted
mode/new-file behavior stays unchanged. Owner fixtures check successful and failed
applications preserve mode 0750, native drift stops subsequent gates, publication
and replay drift reject, and malformed context/duplicate CLI JSON reject.
No live application, report/index recovery, identity promotion or historical proof
rebinding occurred. Tooling changes invalidate old proof closure. Current slice
is NOT independently accepted; S3.4 remains in progress and S3.5 stays unready.
Validation: 251 owner/naming/plan/harness and 117 routing/index checks passed;
26 new targeted cases are included. A single 600-second `just check` completed
1653 tests (two skipped), repository Ruff and maps, then timed out during the
source audit; audit completion is not claimed. All 1124 frozen source/header/
config/report/reverse-index hashes remained equal. Raw Git index bytes changed
with no staged paths (stat refresh is suspected, not proven); no staging, reset
or raw-index restoration was attempted. Scoped format/lint and plan status passed.

Independent read-only review blocked this context slice in
`.pi/sessions/subagent-artifacts/outputs/0e915fbc-2892-41c2-9738-ca2720278f57/owner-review-0.md`:
368 focused checks passed, but native Splat launcher/runtime and Ninja-executed
VerifyGlobs inputs were omitted, and context rejection preempted Git-index and
owned-source rollback. Repair and independently re-review this same slice before
parent-envelope ingestion. No execution-context acceptance or S3.5 readiness is
claimed; the earlier raw-index discrepancy remains historical and unresolved.

Independent re-review remained blocked in
`.pi/sessions/subagent-artifacts/outputs/0e915fbc-2892-41c2-9738-ca2720278f57/owner-review-1.md`:
380 checks passed and launcher/build/index rollback repairs were accepted, but
CLI application publication still validated context outside owned-source rollback.
The publication-boundary repair remains pending independent review; no context
slice acceptance, parent-envelope readiness or S3.5 completion is claimed.

Next after that independent acceptance: owner parent envelopes with distinct actual reviewer/implementation IDs,
parent identity and retained accepted reviewer bytes, externally pinned final
verification, then shared-promotion migration rejecting legacy integrity pins.
Native pilot/recovery and unsupported data/shared closure remain NOT RUN.

## Integration test and closure matrix

Use existing owners: `analysis/index_build.py`, `index_validation.py`,
`rev_queries.py`, `type_index.py`, `type_transactions.py`,
`type_transaction_checks.py`, `macro_index.py`, `macro_opportunities.py`,
`macro_accounting.py`, `macro_transactions.py`, and `naming/prepare.py`,
`postapply.py`, `postapply_review.py`. Read actual owning tests before adding
coverage; no generic framework, new database, dependency, or scheduler.

| Coverage | Required positive and rejection evidence |
| --- | --- |
| Inventory/routing, all five classes | Target-qualified selectors, source/binary fingerprints, complete finite membership, ownership and overlap exclusions survive index refresh; missing/stale rows fail closed. Indexed opportunities are not approvals. Cross-target same-address/byte coincidences do not merge owners. |
| Function/local data/shared variables | Evidence → authored proposal or justified exhaustion/no-op → independent approval → scoped application → native gates → separate review → final verification → accounting. Test unchanged function body/ABI/range; data address/storage/width, all declarations/references and fixed-RAM ownership. A blocked evidence ceiling never becomes no-op just to advance. |
| Struct/types | Existing type-audit account/validate-account/prepare/run/verify: widths, signedness, field offsets, extent, padding, alignment and semantics need corroboration; conflicting consumers reject. Shared promotion requires two independently verified private applications with identical contracts, no target-address-bearing shared contract. |
| Macros | Existing macro-audit account/validate-account/prepare/run/verify: every selected opportunity ID once; automatic safe application count stays zero. Check evaluation count, side effects, precedence, lvalues, integer promotions, aliasing, volatile/control flow; shared template requires independently exact members and explicit owners. Generated/helper exclusions survive routing. |
| Trust/freshness, every owner | Missing/duplicate/unknown JSON fields, stale report/row/source/binary/index, malformed receipts, wrong root/digest/selector, replay and writer self-approval reject before writes. Failed mandatory gates and exit-zero actionable warnings never advance. Approvals bind current changed content, not author assertions. |
| Native recovery | Cancel/stop before apply, during mutation/checks, after apply before review and after acceptance; timeout/tool failure, partial write and verification failure retain evidence. Restore only transaction-owned bytes/modes/absence from adopted baseline, preserving concurrent/unrelated changes. Inspect native state and current hashes before resume; accepted work is skipped, uncertain application stops for parent/cleaner recovery, never blindly reapplied. |
| Final accounting | Each frozen entry has accepted application, independently justified no-op/exhaustion, or explicit blocked/deferred owner/reason/next action. Preserve cross-entry dependencies; missing/duplicate entries and unsupported routes fail campaign completion. Requery freshness after sanctioned parent recovery and independently verify final applications with trusted retained digests. |

Focused validation: existing `test_reverse_index.py`, `test_type_index.py`,
`test_macro_index.py`, `test_macro_opportunities.py`, `test_type_transactions.py`,
`test_macro_transactions.py`, `test_macro_accounting.py`, `test_naming_audit_check.py`
and relevant naming lifecycle tests, then `just check` when practical. Tests
assert behavior/parsed structures, not Markdown strings. Fixtures may demonstrate
rejection/recovery but cannot substitute for actual native stop/resume and live
owner-gate evidence. List NOT RUN paths explicitly. Plan-only checks are
`bin/plans status autonomous-bof3-decompilation.md` and existing `test_plans.py`.

Proposed finite launch ceilings: five entries above, one writer, two repair rounds
per entry, 60 total spawns, two hours campaign wall time; preserve stricter native
command timeouts. Parent freezes these limits with queue fingerprints before
launch. On exhaustion stop with remaining obligations; no implicit budget reset
on resume. Record elapsed time, stage attempts, cancellations, blocked/deferred
counts and each parent intervention/reason (refresh, attestation, recovery,
permissions) in native artifacts. No claim of zero-intervention autonomy while
parent checkpoints remain required. Worktrees require a clean source checkout;
this dirty checkout cannot be made clean via unauthorized Git writes.

Integration acceptance means all five classes have checked routing, transaction,
recovery and accounting behavior with independent review. **Full finite campaign
closure additionally requires every entry accepted or independently justified
no-op/exhaustion, with no blocked/deferred entry silently counted done.** A
blocked pilot can prove fail-closed integration, not campaign completion. Neither
level implies target-wide semantic completeness: retain the existing full-target
`complete:true` naming gate unchanged; no blocked naming gate can advance a
production target audit. S4 stays open until its declared scope is accepted.

## Current prerequisite implementation

Infrastructure scripts accepted by reviewer `891ecdeb`: snapshot status global
JSON flag repaired, bootstrap import lint repaired without suppression, live
snapshot/mission JSON checks and two existing tests passed.

Reviewer `8e026bce` corrected the route analysis: `prepare-transaction` already
supports authored proposed FUNCTION rows independently of exact-capability
`conclude`. The implemented `--candidate JSON --expected-sha256 HASH` extension
replaces exactly one blocked initializer in memory before readiness, provenance,
full validation and atomic publication; `--evidence-root` retains explicit receipt
context. Independent reviewer `3761d476` verified success, preservation and fault
paths; duplicate-key repair accepted by `8e86d774` with production fixture replay.
Seventy existing checks passed locally. That was the pre-application baseline; the selected FUNCTION has since been prepared, applied and independently accepted as recorded below.
Hashes and reviewer labels are integrity/coordination evidence, not authentication;
parent-controlled semantic review and separate identity application remain required.

## Full-autonomy readiness blocker (historical finding, corrected above)

Independent reviewer `5d19eb7d` (workflow `6b5af217`) inspected current domain
implementation and native pi-subagents 0.66.0. Review:
`.pi/sessions/subagent-artifacts/outputs/6b5af217-1585-4f62-9e10-103e7f91fba8/loop-contract-check.md`.
`terminalize_report` is a no-op; the importer rejects conclusions without trusted
capabilities; current capabilities exclude functions and semantic proposals.
Thus `function:func_800A3638` cannot close through the existing importer. A
fail-closed workflow shell is possible, but cannot fulfill S4 under the retained
full-target `complete:true` gate. Do not narrow that gate or mark collection as
semantic completion.

Semantic prerequisite review `0f3efd96` (workflow `94ba66ba`) examined the
complete selected body and all three caller sites against original bytes. Caller
use supports a suppressing predicate for requested mask updates; a narrow
descriptive name remains plausible. Exhaustion is not justified. Consequently,
do not implement the suggested exhaustion-only function pilot merely to fit the
current importer. The missing route must support independently reviewed semantic
proposals, or this function must remain blocked without changing its identity.
Evidence: `.pi/sessions/subagent-artifacts/outputs/94ba66ba-941e-4833-831a-879cee08ea7a/function-semantic-prerequisite.md`.

Historical required scope decision (selected FUNCTION resolved below; general
classes still need owner-specific integration): design a trusted, evidence-backed function/general
conclusion route with independent approval, current report/row binding and
serialized import/revalidation. Expert JSON alone is not authority. Retain
existing receipt validation and evidence-preserving recovery. Define explicit
ownership for post-lift snapshot/index refresh without granting raw scripts host
execution or silently widening a role. This historical dependency record authorized no tests, dependencies or installed
extension edits; only the current targeted test authorization above supersedes it.

## Reconciliation recovery evidence

Reviewer `3721cee2` (workflow `a2e7d74f`) accepted the exact battle15 additive
operation. Parent verified all five reviewed code hashes and report hash, then
ran `bin/naming-audit reconcile emi/battle/battle/15 --apply`. Only missing
`function:func_800A3638` was appended; all 249 prior ordered rows, top-level
values, mode 0770 and 23 other report-set files were preserved. Report SHA-256
is `42451b711ee48eff8cc35641345d27f2cdd519a63341d2854454de7bdb1558af`.
Full validation passes with 250 rows, `complete:false`; repeated preview is a
no-op. This resolves inventory staleness, not naming completion. Prepared
whole-report proposal receipts may still reject additive changes as stale;
reconciliation never rewrites receipts to bypass validation.

One-row closure: reviewer `42ea1461` accepted corrected exhaustion envelope for
`data:D_80096994`; parent imported its exact JSON via `conclude` with the explicit
regeneration evidence root. Only that row changed; all 72 evidence files stayed
hash-identical. Report digest became
`ca8b82bdc1e48933632483b890bacc69ea40efaa72222e385ace20ec36a5dd85`.
Plain full validation rejected absolute receipts without root context. Parent
added explicit `validate --evidence-root` using existing canonical-root/context
helpers; live full validation now passes with that root, 250 rows, complete:false.
112 existing tests passed, 2 skipped; independent validation-context review is
pending. This is one exhausted row, not semantic renaming or target completion.

Direct handoff acceptance: cleanup reviewer `2aad3a20` (workflow `003d2e1c`)
accepted justified no-op for `emi/battle/battle/15@0x800A3638`: live 20/20
instructions and 80 bytes, symbols/Splat checks passed, 2,008 authored files
unchanged. Naming collection/root-generation integration was accepted by
`14c95adb` (workflow `e10aad04`); canonical report remains one exhausted and
249 blocked. Together with the independently accepted lift these complete
S3.1's role/handoff demonstration, not S4's end-to-end naming acceptance.
S3.2 must preserve this distinction: collection progress cannot satisfy a full
naming gate or silently advance the production loop through blocked naming.

## Accepted implementation evidence

S1/S1.1 and S2/S2.1/S2.2 accepted by independent project `bof3-reviewer`
run `317d0a41-fe6b-407b-a61b-7eb6101d0875`, workflow `9222db22`.
Review: `.pi/sessions/subagent-artifacts/outputs/9222db22-4148-4989-b0c3-8ff7cf1ead7f/contracts-recheck.md`.
167 existing tests and both skill-script checks passed; emitted context and
native 0.66.0 discovery verified; 30 scoped Markdown files had valid references.
Acceptance applies to these contracts, not S3/S4 runtime behavior. Initial S3
candidate: `emi/battle/battle/15@0x800A3638`, subject to fresh mission checks.

The owner-requested performance detour is complete: capture-local source lookup
reduced profiled battle15 capture from 5.14s to 1.23s with identical snapshot JSON.
Reusing existing manifest/PsyQ inputs and batch source lookup reduced profiled
index rebuild from 555s to 10.0s; actual `bin/index` took 4.89s. All table contents
were equal and 105 existing focused tests passed. No new cache, dependency or
regression tests; these timings do not accept S3/S4 autonomy.

## Historical scope is not completion

This proposal replaces the former program at the owner's request. Its domain
work is not marked complete by deletion: F4 identity reconciliation, F5 duplicate
configured claims, denominator/LOGO interpretation and other retained findings
remain leads to reverify when relevant. Do not turn every historical lead into a
prerequisite for a single lift. Native sessions and original data are unchanged.
The previous program's verified plan-only recovery copy remains at
`/tmp/bof3-retired-plan-0axpva0h/autonomous-bof3-decompilation.md`, SHA-256
`dbe8c70630872f8e110274c5481c68fbd007f948b8f471c57d590569bb428982`;
it is history, not a workflow dependency.

## S3.4 parent-envelope slice (accepted bounded scope)

Independent reviewer accepted the execution-context/mode/publication prerequisite
in `.pi/sessions/subagent-artifacts/outputs/0e915fbc-2892-41c2-9738-ca2720278f57/owner-review-2.md`:
390 checks passed; no blockers. This supersedes the pending context verdicts
above, not historical index uncertainty or full S3.4 acceptance.

Owner `type_application_review.py` and `macro_application_review.py` now provide
`review_application`/`verify_reviewed_application`, with CLI `review` and
`final-verify`. Closed parent envelopes explicitly accept actual distinct parent,
implementation and reviewer runs, retained reviewer path/hash, exact original
application/manifest/request/context/post-state/native receipts and adopted
baseline preservation. Replay uses the externally retained envelope digest,
original owner integrity verification, current closure and unrelated adopted
workspace checks; omitted-ID legacy applications cannot become independent
acceptance. No prose-derived acceptance, credentials, new database or gate rerun.
Fixture integrations exercise actual owner preparation/application/receipts,
CMake/Ninja generation, ingestion and replay; exact BOF3 payloads remain synthetic.

Independent review accepted this bounded slice in
`.pi/sessions/subagent-artifacts/outputs/4d41995f-5a49-4cc6-b403-a7e53e13f5ec/final-review-check-0.md`:
329 tests passed in 205s; scoped lint/format, plan/diff and raw-index checks passed,
no blockers. Shared migration and full S3.4 were not accepted. S3.4 remains in progress; unsupported data/shared closure, live native
pilot gates and S3.5 cancellation/resume remain NOT RUN. No live application,
report/index recovery, historical proof rebinding, installs or checkout Git writes.

Validation for this slice: 327 parent/owner/naming/plan/harness checks passed;
a subsequent parent/routing/index run passed 173 checks (56 parent cases,
117 existing routing/index cases; two retained-receipt cases added after the
first run). Scoped Ruff/format, plan status and diff whitespace passed. Raw
checkout index remains `558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`,
with no staged paths. Full `just check`/source audit was not repeated: prior
`validate_sources` timeout remains an evidence limitation, not a pass.


## S3.4 reviewed shared-input migration (accepted bounded scope)

Type/macro shared preparation now requires closed `expected_envelope_digest`
pins and owner final verification of retained private envelopes; legacy integrity
pins reject. Existing target/private/exact-wrapper/contract/dependency guards
remain. Standalone integrity verification is unchanged. Shared parent review and
final verification explicitly reject until the owner transition below exists;
positive preparation is not shared acceptance.

Inspection found that current closure includes all source/header/config inputs,
Ninja state and unrelated adopted workspace: a later private mutation invalidates
the first private envelope. Existing `run` requires real changed paths and has no
check-only revalidation operation. Shared replay validates stored manifests but
has no historical prerequisite validator. Supervisor approved this fail-closed
bounded migration rather than weakening closure or manufacturing no-op proofs.
Tests use real temporary private transactions/envelopes to reach the existing
shared input guards, then execute a later private mutation and reject the original
pin without rebinding. Both owners reject shared parent acceptance before CLI
publication. Existing consumer tests retain uniqueness/contract/path/exact guards;
their stubbed positives are not two-private/shared execution acceptance.

Next dependency-ready S3.4 slice: add genuine owner check-only revalidation of
already-applied private results at one finalized pre-shared state. Freeze both
fresh accepted envelopes/external pins and common pre-state at shared preparation.
Shared execution must own the explicit pre/post delta, rollback, new native gates
and independent parent review. Final shared replay must validate retained private
prerequisite integrity, nested evidence and common historical PRE-state, plus the
current shared POST-state, rejecting all unrelated drift. Standalone private final
verification remains unchanged; no historical rebinding, inferred acceptance,
generic snapshot graph or new database. Implement and independently review that
transition before removing the shared acceptance guard. Positive mutating
sharing, unsupported data/shared pilot closure, live gates and S3.5 remain NOT RUN.

Validation: 333 owner/parent/naming/plan/harness checks passed in 213.24s.
Initial aggregate had 332 passes and one module-ceiling failure (452 lines);
relocating the lazy review import into the proof owner restored the 450-line
ceiling, then the complete bounded aggregate passed. Scoped Ruff/format, plan
status and diff whitespace passed; no staged paths. Raw index SHA-256 remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
No live source/map/promotion, report/index recovery, historical rebinding,
installs or checkout Git writes. Full `just check`/`validate_sources` was not
repeated; its known timeout is not a pass. Independent review is still required.


## S3.4 no-drift check-only revalidation (accepted bounded scope)

Independent read-only review accepted the preceding shared-input migration and
fail-closed guard in
`.pi/sessions/subagent-artifacts/outputs/4d41995f-5a49-4cc6-b403-a7e53e13f5ec/final-review-check-1.md`:
333 checks passed in 213.42s; no blockers. This supersedes that slice's pending
verdict above, not full S3.4 or shared execution acceptance.

Supervisor approved a bounded no-drift prerequisite after inspection: owner
`revalidate_application` requires a still-current independently reviewed private
envelope, external digest, explicit current baseline adoption and new execution
ID distinct from its original parent/reviewer/implementation. It executes new
native gates without application writes, retains separate owner revalidation
schema/receipts/context, and provides externally pinned `verify_revalidation`.
Original envelope, application and receipts remain unchanged. The result says
`checked`, never `applied` or independently `accepted`; shared consumers and
standalone final verification are unchanged. No CLI or new parent-acceptance
surface is added at this layer.

The no-drift ceiling also rejects build-graph regeneration that invalidates the
original envelope; an unchanged native Ninja build succeeds. Unexpected gate
mutations stop before further checks/publication and remain for explicit parent
recovery: this check-only operation has no source mutation/rollback authority.
Fixture tests use real private applications and actual Ninja execution, with
synthetic BOF3 exact-match payloads; they are not live pilot evidence.

Next dependency-ready layer after independent review: validate exact intervening
private-transaction deltas backed by retained independently reviewed envelopes
against original/current states, not blanket workspace adoption. Then issue fresh
independent revalidation acceptance/common PRE-state pins before explicit shared
PRE-to-POST execution/rollback/new parent review and historical prerequisite replay.
The shared guard stays until that transition is accepted. Mutating two-private
closure, unsupported data/shared pilots, live native gates and S3.5 remain blocked
or NOT RUN; S3.4 stays in progress. No historical proof rebinding, source/map
promotion, report/index recovery, installs or checkout Git writes occurred.

Validation: 347 owner/revalidation/parent/naming/plan/harness checks passed in
236.79s (14 new revalidation cases); scoped Ruff/format, plan status and diff
whitespace passed. Initial positive fixtures regenerated CMake and correctly
rejected stale original build closure (12 passed, two failed); using native Ninja
against the already-finalized graph passed all 14 in 27.32s. Initial test-fixture
import lint was corrected without suppression. No staged paths; raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Full `just check`/`validate_sources` was not repeated; known timeout is not a pass.


Independent read-only review accepted this bounded no-drift slice including B1 in
`.pi/sessions/subagent-artifacts/outputs/3f96bc49-9797-463f-bbfe-63f6a2e903a9/review-revalidation-0.md`:
264 checks passed in 281.72s; no blockers. Existing output/prerequisite aliases
reject before gates; native exclusive publication rejects late creation without
clobbering receipts, applications or envelopes. Original bytes/modes and replay
survive. This supersedes the pending verdict above, not full S3.4 acceptance.

Independent review accepted fresh parent acceptance of still-current no-drift
revalidation in
`.pi/sessions/subagent-artifacts/outputs/3f96bc49-9797-463f-bbfe-63f6a2e903a9/review-revalidation-1.md`:
all 273 collected focused cases have passing evidence across an interrupted
290-second aggregate and nine-case completion; lint/format, plan, whitespace and
unchanged raw index passed. No blockers; this accepts only that bounded layer.

## S3.4 internal historical validation (pending independent review)

Supervisor approved a mechanical prerequisite before the reviewed-delta producer:
separate retained private application/context integrity from live post-state,
execution-input/build and unrelated-workspace comparisons. Both standalone owner
verification paths still perform those live comparisons. The internal historical
validator requires the original external envelope pin, parent/reviewer identities
and retained artifact, original application/manifest/attestation/native receipts,
context transitions and retained manifest evidence. It neither rebuilds old context
from current files nor returns current validity, accepted drift or sharing authority.
Native receipt semantics still use existing owner resolution; target configuration
or path-identity drift can conservatively reject. Shared acceptance guard remains.

Two owner integrations use real temporary transactions/native Ninja and synthetic
BOF3 payloads. Historical integrity survives unrelated/source/build drift while
standalone final verification rejects; every retained manifest artifact, receipt,
attestation and reviewer file is tampered individually and rejects. No live domain
mutation, report/index recovery, installs or Git writes. This slice is pending
independent review, not S3.4 completion.

Remaining: exact independently reviewed intervening-private deltas and continuity,
genuine new native checks for both results at one common PRE-state, fresh parent
acceptance/external pins, then explicit shared PRE/POST execution/rollback/new
review and historical prerequisite replay. No blanket adoption authority, no-op
application fabrication, proof rebinding or removal of the shared guard.
Validation: new two-case test passed; 332-case aggregate timed out at 290 seconds
with 260 passing progress results and no failure printed. Bounded completion runs
passed 10 owner-context/publication cases and 64 accounting/plan/harness cases
(including overlap). Full `just check`/known source-audit timeout was not repeated.


## S3.4 exact sequential private transition (accepted bounded scope)

Independent reviewer accepted the internal historical prerequisite in
`.pi/sessions/subagent-artifacts/outputs/3f96bc49-9797-463f-bbfe-63f6a2e903a9/review-revalidation-2.md`:
210 focused cases passed; no blockers. This supersedes its pending verdict above,
not common-PRE, shared execution or S3.4 acceptance.

Both owner revalidation APIs now retain ordered externally pinned, independently
reviewed private envelopes and validate exact POST/PRE/POST/current continuity.
Nested history remains prerequisite integrity, not present acceptance. Only owned
reviewed edits and receipt-owned native build transitions explain drift; input,
workspace, index, mode and build gaps reject despite adoption. Original owned
post-state is preserved. Existing no-drift and standalone verification are unchanged.
Ten sequential fixture cases exercise real owner preparation/application, native
CMake/Ninja and synthetic BOF3 payloads: positive two-private fresh checks/parent
acceptance at a common state, plus independently accepted second transactions
with unevidenced workspace/input/build/mode gaps that reject before new gates.
Shared guard remains; shared historical PRE/current POST replay, live pilots and
S3.5 remain NOT RUN. This implementation is pending independent review; S3.4 stays
in progress. No live source/map/shared promotion, index recovery, installs or
checkout Git writes occurred.

Validation: 10 sequential cases passed in 32.25s; 50 history/revalidation cases
(including two sequential positives) passed in 109.44s; 98 owner/context/review
cases passed in 109.35s; 64 accounting/plan/harness cases passed in 15.74s.
Scoped Ruff/format, plan status and diff whitespace passed; staged paths remain
empty and raw index remains `558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Full `just check`/known oversized source audit was not repeated. The positive
sequence uses nonoverlapping private headers within one target, not two distinct
targets; differing target-specific input membership conservatively rejects.
Cross-target common-PRE and shared acceptance remain unproven, not silently
excluded from closure or counted complete.


## S3.4 distinct-target common-PRE integration (accepted bounded scope)

Independent reviewer accepted the preceding same-owner sequential transition in
`.pi/sessions/subagent-artifacts/outputs/88cf8e3a-6fc1-46fb-8d6d-9960b290ffd9/review-private-transition-0.md`:
220 checks passed, no blockers; this supersedes that slice's pending verdict only.

Distinct-target execution exposed missing historical binary/Splat/reviewed membership.
Supervisor approved explicit finite `participating_targets` capture on type/macro
Python `run_transaction` only: canonical sorted unique one/two-target set including
mutation targets, frozen before original execution and inherited unchanged by fresh
checks. Existing manifest owners derive each participant's closure; no write/check
authority expands, naming/default capture stays unchanged, and old incomplete or
mismatched histories reject. No historical reconstruction, exclusions or rebinding.

Both owner fixtures now execute two distinct private target transactions, retain
externally pinned synthetic parent envelopes, run distinct fresh native Ninja
checks at one common state, freshly parent-review both and replay nested evidence.
BOF3 exact payloads and reviewer identities remain explicitly synthetic. Missing/
mismatched scope, invalid sets, revalidation scope rebinding and participant input
drift reject alongside prior gap cases. Shared historical PRE/current POST execution,
rollback and prerequisite replay remain unimplemented; shared guard stays. S3.4 is
in progress and this slice requires independent review; no live promotion, index
recovery, installs/extensions or checkout Git writes occurred.

Validation: final sequential/accounting/plan/harness group 88 passed in 93.94s;
history/revalidation/parent group 108 passed in 184.21s; selected owner gates 64
passed in 55.17s. Scoped Ruff/format, plan status and diff whitespace passed.
Initial fixture-only failure came from Git's same-second DB stat cache after raw
index restoration; explicit fixture DB timestamp separation fixed it without
changing runtime checks. Staged paths remain empty; raw index SHA-256 remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Full `just check`/known oversized source audit intentionally NOT RUN.


## S3.4 shared PRE integration (accepted bounded scope)

Independent reviewer accepted distinct-target common-PRE in
`.pi/sessions/subagent-artifacts/outputs/88cf8e3a-6fc1-46fb-8d6d-9960b290ffd9/review-private-transition-1.md`:
356 checks passed, no blockers. This supersedes that slice's pending verdict only.

Inspection found shared consumers lacked reviewed-revalidation ingestion and
retained revalidation replay still requires private-current equality. Supervisor
approved integrating shared PRE first, keeping shared parent/final acceptance
blocked. Both owner preparations now consume externally pinned fresh accepted
revalidations, validate all nested current prerequisites and existing contracts,
exact wrappers and ownership, require two distinct fresh executions/targets with
identical captured participants, and freeze common native state/build/workspace
plus full prerequisites in the manifest. Run rederives this binding and compares
captured PRE before mutation/gates. Capture scope remains separate from authority.
Legacy reviewed-application preparation remains preparation-only.

Canonical absolute reviewer artifacts exposed a repository-input seed mismatch.
Supervisor approved schema-aware parent validation before excluding only those
reviewer references from repo-relative seeds; arbitrary absolute evidence still
rejects. Nested reviewer evidence remains mandatory at preparation and run entry.
Two real sequential owner fixtures exercise fresh native Ninja checks, shared
preparation, PRE/pin/evidence/drift rejection, actual failed-gate rollback and
successful bounded shared mutation with parent acceptance still rejected. BOF3
payloads/registry and reviewer identities are synthetic, not live domain approval.

Remaining boundary: retained historical revalidation integrity/common PRE replay,
new shared POST/context final validation, distinct fresh shared parent acceptance
and external envelope pin, full nested tamper/drift/rollback matrix and independent
review. No impossible private-current comparison may replace historical validation;
no original proof rewriting. S3.4 remains in progress; S3.5/live pilots remain unready.
No live promotion, report/index recovery, installs/extensions or checkout Git writes.

Validation: 28 shared/sequential/history checks, 106 revalidation/parent checks,
160 owner checks and 66 plan/harness/accounting/shared checks passed in complete
runs below 300 seconds each (358 distinct cases; shared cases overlap). Final
expanded shared cases passed again (2 in 25.65s). Scoped Ruff/format, plan status
and diff whitespace passed; staged paths are empty and raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Initial fixture candidate/registry setup and recursive reviewer-node handling
failures were repaired; an existing stub proof regression was fixed by preserving
verified target output. Full `just check`/known oversized source audit NOT RUN.


## S3.4 shared POST integration (accepted bounded scope)

Independent review accepted only shared PRE in
`.pi/sessions/subagent-artifacts/outputs/1007202e-7ecf-48f8-8ff9-ec09d8acc2cc/review-shared-transition-0.md`:
358 distinct checks passed in four complete groups below 300 seconds; no blockers.
This supersedes PRE's pending verdict only, not full S3.4 acceptance.

Both owners now replay retained private/revalidation histories against their frozen
common PRE, then validate the shared authorized delta and new current POST/context.
The existing parent review/final replay accepts only the fresh two-target branch,
with new shared execution/reviewer identities, retained fresh reviewer artifact and
external shared envelope pin. Legacy preparation-only branches remain guarded.
Standalone private final verification still rejects intended subsequent drift;
no old proof is rewritten or reconstructed from current files. Existing preparation
contracts, macro exact wrappers, address/ownership restrictions and capture-only
participation remain unchanged. No generic framework or CLI surface was added.

Expanded two-owner integration checks exercise all shared native gate failures,
publication failure and rollback bytes/modes/index, positive publication/new parent
acceptance/read-only replay, every retained nested artifact, historical record
mutation, wrong pins, reused reviewers and unrelated/source/build/mode drift.
Fixtures use actual temporary owner applications and native CMake/Ninja with
synthetic BOF3 exact payloads/registry/reviews and a shared header comment edit;
these are not domain approval or a demonstrated useful extraction. Full S3.4,
live pilots and S3.5 cancellation/resume remain incomplete. Independent acceptance of this bounded branch is recorded below.

Validation: 2 expanded shared checks (55.34s), 26 sequential/history (80.10s),
106 revalidation/parent (182.18s), 160 owner (112.38s) passed in complete bounded
groups. Initial fixture output-path rejection and an inert added receipt field
were corrected to use the canonical evidence directory and actual receipt status.
No live source/map/promotion, report/index recovery, installs/extensions or checkout
Git writes. Full `just check`/known oversized source audit intentionally NOT RUN.
Final plan/harness/accounting group: 64 passed (16.16s); 358 distinct cases across
all groups. Scoped Ruff/format, plan status and diff whitespace passed. Staged
paths remain empty; raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.


## S3.4 owner transport and obligation audit (accepted bounded transport)

Independent review accepted shared POST in
`.pi/sessions/subagent-artifacts/outputs/1007202e-7ecf-48f8-8ff9-ec09d8acc2cc/review-shared-transition-1.md`:
358 distinct checks passed in complete bounded groups; no blockers. This supersedes
POST's pending verdict only. Synthetic BOF3 payloads and comment edits prove owner
integration, not useful extraction or live domain acceptance.

| Remaining obligation | Existing evidence / smallest next action and owner |
| --- | --- |
| Function | Historical selected-row acceptance/skip control; BOF3 reviewer must obtain fresh naming postapply gates/review before present closure, never reapply or rebind. Full-target `complete:true` remains required. |
| Local data | Frozen `data:D_80096994` control is BLOCKED per d740aabe below; capability integrity is not semantic exhaustion/no-op. Parent owns bounded read-only bof3-reviewer follow-up; historical report row stays unchanged and FUNCTION-only `naming/postapply*.py` still rejects data application. |
| Fixed RAM/storage and aggregate | Indexed blocked `1F800044` concerns, not ownership/layout proof. BOF3 namer/lifter must corroborate owners/access width/base/extent and layout/semantics before serialized `type-audit` preparation; storage and aggregate remain linked, not competing writes. |
| Types/macros shared positive | Accepted two-private/fresh-check/shared PRE/POST tooling fixtures; BOF3 lanes still owe real corroborated contracts, two private native exact results and useful common body before live shared review. Macro pilot members remain invalid lifts, not exact extraction evidence. |
| Accepted/no-op/blocked accounting | Existing owner account validators and frozen five-entry routing are not final campaign acceptance. Parent must bind every entry to current independent acceptance, justified exhaustion/no-op or explicit blocked owner/reason/next action; blocked entries cannot count done. |
| API/CLI | Existing participant/revalidation owner transport independently accepted at current bytes by b295c173; no before-image provenance or all-CLI shared campaign acceptance. |

Both commands now expose `run --participating-targets` and owner revalidation,
integrity replay, parent review and final replay commands. One private command
helper shares only parsing/transport, not orchestration, state or authority.
Ordered intervening JSON retains full envelopes/external pins; strict duplicate
parsing and native output publication stay owner-controlled. No new domain route,
proof rebinding, recovery, installs/extensions or checkout Git writes. S3.4 stays
in progress; S3.5/native cancellation and S4 remain unready. Current checks and
transport limitations are recorded in this run's implementation artifact.

Validation: 8 CLI cases passed (7.47s); 160 transaction cases passed in the
166-case CLI/owner run (120.70s); 28 shared/history/sequential cases passed
(137.61s); 106 revalidation/parent cases passed (181.36s); 64 plan/harness/accounting
cases passed (14.76s): 366 distinct cases with complete groups below 300s.
Scoped Ruff/format, plan status and diff whitespace passed. Revalidation and fresh
review output collisions preserve existing bytes via native exclusive publication.
Initial fixture absolute-output and exception-class expectations were corrected;
no owner validation was weakened. Raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`, staged paths
empty. Full `just check`/known source-audit timeout and live/native cancellation
were NOT RUN.

Independent reviewer b295c173 accepted only current-byte CLI transport in
`.pi/sessions/subagent-artifacts/outputs/b295c173-77ad-4bae-b602-3ba902620990/cli-transport-independent-review.md`:
366 existing tests and 26 supplemental assertions passed; no tooling blockers.
No preserved before-images or reconstruction equivalence were verified. Synthetic
owner fixtures are not live domain or all-CLI shared campaign acceptance; S3.4
and all unfinished obligations above remain open in their existing states.

## Frozen data control (BLOCKED; plan update pending semantic review)

Independent read-only review d740aabe in
`.pi/sessions/subagent-artifacts/outputs/d740aabe-6583-40f5-990d-23d7553538eb/data-control-independent-review.md`
validated retained capability integrity and original data/consumer bytes, not
semantic exhaustion or current no-op completion. Handler behavior and independent
corroboration remain unproven; preserve the historical exhausted row and raw name.
Selected transaction CLI rejection reflects proposal-only routing; full-report
validation fails on separate historical FUNCTION binding state, not a new data
defect. No report/receipt repair or rebinding is authorized.

Parent may scope read-only `bof3-reviewer` review of the three retained handler
ranges to establish selector/state effects or a concrete static ceiling. Row-level
`bof3-namer` dispatch is unsupported: it accepts only `audit-target TARGET`;
reuse the existing reviewer route, without widening skills or launching collection.
The review's selector-offset/consumer-end specification correction remains a
separate writer obligation, not authorized here. BLOCKED accounting cannot satisfy
finite campaign closure or full-target `complete:true`. This plan-only recording
still requires independent semantic review; neither domain nor S3/S4 completion
is claimed.

## Bounded index ceiling (tooling pending independent review)

Domain review 707dab39 (`data-list-producer-review.md` in its retained output
directory) accepted conditional uniqueness preservation only, not a global
invariant or independent access to `D_80096994`; the data pilot remains BLOCKED.
No further peripheral gameplay investigation is required for its identity gate.

Owning `domain/mips.py::data_references` deliberately recognizes direct LUI/%lo
pairs within 12 instructions, not materialized-pointer chains or dynamic indexed
bases. Original `800A5424`, `800A5434` and `800A5590` omissions reproduce this
ceiling; exact `801463C4` xrefs remain empty, correctly without guessed aliases.
The fixed table-consumer owner separately proves all three loads in its reviewed
body; the index reports only its table-address materialization at `800AD278`.
Neither establishes a complete inventory of consumers. No propagation defect in
supported behavior was reproduced; runtime extraction/capabilities are unchanged.

The empty-access analyzer already leaves selected access open. Its allowlisted
positive structural capability can still say `exhausted` with a second independent
consumer explicitly missing: this is an unresolved terminal-accounting integration
obligation, not semantic ladder exhaustion/current no-op acceptance. Parent approved
characterization/docs only. Next: independently review this ceiling, then parent
scope the naming conclusion/accounting owner contract to distinguish structural
capability from reviewed terminal acceptance. Keep this pilot blocked, historical
report/receipts untouched, and the full-target `complete:true` gate unchanged;
blocked outcomes prove fail-closed routing, not finite campaign closure.

Validation: 433 reverse-index/facts/table-consumer tests and 148 plan/harness/
conclusion tests passed (two skipped), in groups under 300 seconds. Initial new
query-fixture foreign-key/row-factory setup failures were corrected; no runtime
check changed. Original image/table/consumer assertions passed. No live index or
snapshot regeneration, source/map/report promotion, installs/extensions or Git
writes. Full `just check`/source audit and native cancellation were NOT RUN.

## S3.4 terminal-accounting boundary (accepted bounded scope)

Independent review a4f82975 accepted the bounded index-ceiling characterization in
`.pi/sessions/subagent-artifacts/outputs/a4f82975-a4ff-455a-b8ce-59e8f21e5d0d/review-index-evidence-limit.md`:
581 checks passed, two skipped; no extractor repair required. This supersedes only
that slice's pending verdict. Structural single-consumer exhaustion is not semantic
ladder exhaustion; `data:D_80096994` remains BLOCKED with historical bytes unchanged.

Parent approved a bounded read-only `naming-audit terminal-verify` CLI/API as the
future loop's selected-row acceptance consumer; no existing autonomous consumer
was available to wire. It reuses naming row/capability/receipt and local review
primitives, current owner-derived scope/tooling, external parent digest and separate
retained preparation/semantic reviewer artifacts. Three distinct actual native run
identities are locally parent-attributed; old collection journals contain no run ID.
False ladders, unresolved executable leads, contrary evidence and stale bindings
reject. Evidence-backed static ceiling rationale must address the missing naming
fact; missing corroboration alone cannot prove exhaustion. Structural validation
and historical report/capabilities remain unchanged. Full-report blockers are
reported separately and `complete:true` remains additional mandatory production
acceptance, never replaced by a selected row. No current BOF3 attestation is issued.

Targeted synthetic owner/CLI tests cover accepted reviewed ceiling/replay, structural
only, open leads, contrary reviews, stale pins/row/report/tooling/source/evidence,
self-review, missing artifacts and separate fullreport failure. Synthetic attribution
is not live domain review. Independent review e2920f90 accepted this CLI/API
boundary (605 passed, two skipped):
`.pi/sessions/subagent-artifacts/outputs/e2920f90-54da-4c4c-80de-4c1a3d10466c/review-terminal-accounting.md`.
This does not accept loop accounting or complete S3.4. S3.5/native recovery, S3.2 and S4 remain unready.
No source/map/report/index recovery, receipts, installs/extensions or Git writes.

Validation: 237 naming/conclusion/CLI/plan/harness checks passed, two skipped;
392 terminal/facts/consumer/root/bulk checks passed (30 terminal cases, overlapping
first group). Scoped Ruff/format, plan status and diff whitespace passed. Raw index
remains `558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`,
staged paths empty; historical report retains `bc6c9cabcb1dfea5cbf3746598cf7e966b58742fe92876cf4e4d53cbf5f2a500`.
Full `just check`/known oversized source audit and live/native cancellation NOT RUN.

## S3.4 all-handler fixture sequence (accepted bounded scope)

Two new type/macro cases reuse the existing distinct-target private/common-PRE/
shared fixture and its rejection matrix through actual argparse/CLI handlers:
prepare → participating run → review/final verify for each private application,
ordered fresh revalidation → integrity replay → new parent review/final replay,
then shared prepare/run/review/final verify. Only native-gate runner injection is
substituted; actual CMake/Ninja runs, BOF3 payloads and parent/reviewer identities
are explicitly synthetic. No transport defect reproduced; runtime is unchanged.
This validates the existing supported-class obligation, not a new plan dependency.
Validation: 36 CLI/private/shared cases passed (200.06s); 128 terminal/plan/harness/
agent/skill checks passed (22.46s), plus both skill-script checks. Structural-only
terminal rejection remains covered without issuing a live data attestation.

Current AGENTS requires direct compaction, forbids restoring retired machinery;
`.pi/skills/agent-skill-compaction/SKILL.md` is absent. S1/S2 review 9222db22 above
records accepted compact contracts, 30-file reference inspection and checks; this
run edits no agent/skill Markdown and reran applicable existing checks.

Independent review 11fcb490 accepted this bounded all-handler sequence (164
checks plus both skill-script checks):
`.pi/sessions/subagent-artifacts/outputs/11fcb490-323d-468e-bb89-e31621ec7604/review-remaining-integration-gates.md`.
Handler reachability is not subprocess/Pi execution or positive private replay
at every lifecycle point. All-five accounting acceptance remains distinct. Native S3.5 stop/recovery is NOT RUN and still depends on
accepted S3.4. Live S3.4 prerequisites are unchanged: fresh FUNCTION gates/review,
BLOCKED local-data control, corroborated serialized fixed-RAM/layout ownership,
and real exact private type/macro members/useful shared body. No peripheral data
research, live recovery or prerequisite reclassification is authorized here.
S3.2 cannot encode the production loop before S3.5 acceptance and the retained
full-target naming `complete:true` gate; S4 remains open.

Next: independently review the bounded accounting slice below, then obtain an
explicit whole-S3.4 acceptance/blocker verdict, not infer completion from fixture
passes. Only after whole-S3.4 acceptance, scope an actual native
finite rehearsal: one writer, frozen selectors/baseline/budgets, stop before apply,
during checks, after apply/before review and after acceptance; inspect real native
state, owned bytes/modes/absence and unrelated/index preservation before resume;
accepted work skips, uncertain apply stops for parent recovery. Fixtures do not
prove native cancellation, and no such rehearsal was launched here. Full
`just check`/known oversized source audit remains NOT RUN, not passed.

## S3.4 frozen-five accounting (pending independent review)

Authorized design ea766bcf now has one read-only Python consumer,
`analysis/frozen_queue_accounting.py::account_frozen_queue`, and focused tests.
It requires the exact externally pinned S3.3 five, unchanged source/binary/snapshot/
index/report bytes before and after, current owner queries and direct verifier
calls. Closed proof references retain owner digest spellings and manifest identity;
both naming evidence/receipt contexts restore on success or failure. Storage/layout
positive closure requires the same verified envelope explicitly covering both IDs;
current type concern rules cannot produce that pair, so no positive live closure
is claimed. Macro owner uses `candidate_id`, global-account fingerprint and
repo-path owners; only the frozen three affected functions are permitted.

Live read-only accounting returns five accounted, zero accepted/noop, five blocked,
one overlapping historical FUNCTION skip and one blocked linked pair. Unfiltered
full-report validation independently reports the existing FUNCTION binding-location
blocker; report/receipts remain unchanged. Campaign is incomplete and bounded
`production_complete` is always false. Parent/BOF3 owners still owe fresh FUNCTION
checks/review without reapplication, accepted data ceiling, corroborated fixed-RAM/
layout authority and real exact private members/useful sharing. S3.4 stays in
progress; S3.5, S3.2 and S4 cannot advance from this summary.

Validation: 121 consumer/accounting/plan/harness checks and 40 terminal/owner-CLI
checks passed in complete groups below 300s. Synthetic consumer positives prove
routing/binding only; actual malformed owner proofs reject. One intermediate
relative-path check exposed RHS-before-key evaluation during hash capture; explicit
path validation now precedes hashing, and the entire group passed. Scoped Ruff/
format, plan status and whitespace checks passed. No live application, receipt
publication/rebinding, recovery, installs/extensions or checkout Git writes.
Full `just check`/known oversized source audit and native cancellation NOT RUN.
