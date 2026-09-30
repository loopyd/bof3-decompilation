<!-- bof3.plan/v1 -->
# PSX emulator skill capabilities

## Goal and scope

Maintain a generic PCSX-Redux skill for bounded PS1 experiments, then close the
inspection, execution, replay and device-observation gaps assessed on 2026-09-24.
The user requested a dedicated plan and movement of emulator skill work out of
other plans, without cross-links or forwarding tasks in those source plans.
This plan owns reusable actions, their Lua implementations and operational
references, the Pi agent specification, and supporting harness runtime transport.
It does not authorize executing this backlog merely by recording it.

Keep `SKILL.md` broad and directive with an action/reference reading-order table.
Each capability has a focused procedure under `references/` and maintained Lua
under `scripts/`; extend existing actions before adding overlapping runners.
References contain purpose, procedure and application; keep chronology, temporary
validation results and implementation progress in this plan and evidence artifacts.
The canonical tree remains `.pi/skills/psx-emulator`, with relative discovery
symlinks in `.agents/skills` and `.codex/skills`, matching `psx-rizin`.

## Evidence baseline and boundaries

The source plan records working setup/status, staged Lua modules and inputs,
bounded process execution, hash checks and versioned receipts. Existing actions
cover startup/targets, saved-state schema/query/export/byte comparison/restore,
CPU registers, memory edits, function calls, selected-PC tracing, Exec/Read/Write
watches, Vsync advance/digital input/screenshots, VRAM/CLUT export/upload, GTE
inspection/editing/command execution, SPU state/ports and CD media/controller access.
These are historical implementation observations, not new acceptance decisions.
Migration preserved A8 statuses; subsequent reviewed decisions appear below.
Refresh owning evidence before work.

| Area | Main gap | Owner below |
| --- | --- | --- |
| States | Structured field/range differences and measured replay repeatability | E1 |
| CPU/debugging | General stepping, bounded execution/branch/function coverage, exception diagnosis | E2.01 |
| Inputs/display | Scheduled inputs, conditional stops and checkpoint comparisons | E2.02 |
| DMA/IRQ/timers | Decoded system state and timestamped transfer/completion observations | E3 |
| SPU/audio | Continuous register history and actual emulated PCM capture | E4.01 |
| CD-ROM/XA | Complete command/response, sector/DMA and streaming lifecycle | E4.02 |
| GPU | GP0/GP1 and ordering-table/DMA history tied to VRAM/display changes | E5 |
| Peripherals | Memory-card/SIO workflows, MDEC, analog input and disc switching | E6 |

Saved-state byte equality does not prove deterministic continuation. Watchpoints
observe before selected CPU accesses and do not constitute a complete DMA/bus
trace. SPU traces currently sample at Vsync, not every DSP tick or PCM sample.
CD commands stop at the first observed IRQ, which may only acknowledge a command.
Native MMIO probes restore scratch/GPR/PC but advance device time. Full histories
and native fidelity remain open. Execution coverage alone never proves unused
code or authorizes pruning. The current API's independent host audio preview is
not a capture of emulated sound.

## Ownership and implementation contract

- `.pi/skills/psx-emulator/scripts/` owns reusable Lua actions and state decoding
  through Redux's native schema/bundled libraries; no generated Rust emulator
  scripts or duplicate protobuf parser. `.pi/agents/psx-emulator.md` owns delegated
  scope, evidence handoff and escalation. The harness never launches models.
- `tools/python/harness/runtime/` owns literal inputs, process bounds, cleanup,
  isolated staging, receipts and capture publication. Use single-noun modules,
  existing CLI/process/deadline owners and fresh output directories. Lua is
  trusted host code, not a sandbox. No legacy/fallback transport or emulator.
- `harness.toolchain.pcsx` owns setup/build/environment; doctor only diagnoses.
  Retain the pinned `third_party/pcsx-redux` submodule and SDL3 source identities.
  Existing approvals do not authorize new dependency installs or changes to
  audited dependency sources. If Lua lacks an API, first document the exact
  bounded native binding needed; obtain specific authorization where required.
- Keep generic arguments free of named-game assumptions. Project-specific
  profiles, scenarios, comparisons and semantic/source acceptance stay with
  consumers. No production renderer fallback, audio-driver pruning, BIOS removal,
  game source edits, or hardware-fidelity claim is owned by this plan.
- Retain input/tool hashes, explicit observation boundaries, timing units, stop
  reason, overflow/truncation and failed-run evidence. No silent retries, dropped
  events or masks that conceal comparison failures. Keep proprietary captures,
  media, BIOS and generated products local and outside source/packages.
- Use existing checks and authorized harness integration coverage. Audio-project
  tests/reference models remain Rust; Python is only their harness integration.
  This planning move does not itself approve dependencies or unrelated tests.

## Migration provenance

Source: `docs/plans/bof3-audio-rust-migration.md`, SHA-256
`547978d8eebc6278693b04a7c2233b2c9a5943a06b2035c878e90a644becd472`,
3436 lines. Original bytes/mode and candidate review are retained externally for
recovery; the permanent mapping below records ownership without source-plan
cross-links. No source plan is deleted, no unfinished task is declared complete,
and stable A8/A8.01–A8.05 IDs survive here. No emulator-skill actions were found
in the other inventoried plan, `autonomous-bof3-decompilation.md`.

| Original source scope | Destination / disposition |
| --- | --- |
| Lines 249–256, workflow extension | Goal/ownership contract and inherited A8; audio evidence gates remain with A4/A6 |
| Lines 1917–1933, reference setup | Historical setup checkpoint below; BIOS identity and runtime diagnostics remain audio evidence |
| Line 1934, BIOS-presence transition | Wording updated in audio A4 after moving the setup paragraph; the same prepared BIOS identity and diagnostics remain there |
| Lines 2457–2467, reference build | Historical build checkpoint below; source/build review never becomes parity acceptance |
| Lines 3198–3243, A8/A8.01/A8.02 | Same IDs, states, dependencies, setup/runtime obligations below |
| Lines 3244–3293, A8.03 | Same ID/state and generic missions/checkpoints below; music/SFX/voice scenario acceptance retained in audio A4 |
| Lines 3294–3309, A8.04 | Same ID/state, generic skill/agent and discovery obligations below |
| Lines 3310–3320, A8.05 | Same ID/state, harness validation and independent capture handoff below; Rust audio comparison obligations retained in A4 |
| Lines 3321–3356, tooling/generic checkpoints | Historical checkpoints below, with consumer parity outside this plan |
| Lines 3357–3375, independent runtime comparison | Relocated within audio A4; including the unresolved 27-byte mismatch and all evidence/acceptance limits |
| All remaining source lines | Retained in the audio plan; only adjacent wording updated to remove stale A8 ownership references |

The later E phases extend the inherited backlog; they do not reset its status.
Start E1 after the accepted build baseline and E2 after E1, rather than depending
on all of A8.03, whose remaining history/audio requirements these phases help deliver.
Close inherited requirements only with fresh evidence for their full scope.

## Historical reference setup and build

Runtime reference setup checkpoint (2026-09-24): the user authorized the supplied
Archive.org BIOS collection and selected PCSX-Redux as the behavior reference for
the audio runtime, explicitly requiring a `third_party/pcsx-redux` Git submodule.
The gitlink pins `28438546c781fbe372a06399c82bed43ca2c6f4d`; recursive setup verified
43 repositories. `just setup --component pcsx-redux` prepares source and records
revision provenance; it does not claim a working emulator binary. The focused
BIOS action initially prepared all 24 archives. Subsequent user steering narrowed
it to US `ps-30a.bin` only: setup now downloads and verifies that single pinned
archive/ROM, and the other 23 prepared images and caches were discarded after
hash verification. The final user contract embeds the US SHA-256 and download
pins in the BIOS tool; no collection configuration or inventory is needed. Setup
names the ROM `inputs/external/bios/scph5501.bin`. `just doctor` scans this BIOS
directory for matching bytes regardless of filename, without downloads or
repairs. Reuse is offline; failed refresh or
publication preserves existing assets. See
[reference setup](../reference/audio-runtime-setup.md).

The earlier independent Redux build lacked development prerequisites, including
SDL3 with no candidate in the local APT metadata.
The [reference-build proposal](../reference/audio-runtime-setup.md#reference-build-dependency-proposal)
now pins the eight-package APT addition and SDL3 3.4.16 source/hash, with license
inventories and a zero-upgrade/removal simulation. Specific installation
authorization was granted by the user. Reinspection found all eight packages
already installed at exactly the approved versions; this agent made no APT
changes. SDL3 built and installed under the approved project prefix with audio
and video backends enabled. The pinned Redux build is running. Source/build
review is not an emulator comparison; A8 owns the reusable execution workflow.

## 1. [A8] (in-progress) Establish reusable emulator evidence tooling

- Owner: parent
- Depends: none
- Blocker: none
- Evidence: user extension; pinned PCSX-Redux source and approved dependency review in `docs/reference/audio-runtime-setup.md`
- Acceptance: bounded harness execution, reusable skill missions, reproducible independent captures and applicable integration checks; no production fallback

1. [A8.01] (done) Build and identify the reference emulator
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: user-approved dependency names; parent-reviewed setup-chain, live smoke/boot and doctor receipts under `out/audio-migration/redux-review/`; all three Redux/SDL doctor rows pass
- Acceptance: approved dependencies only; pinned recursive source, executable and linked SDL identity; a successful isolated CLI smoke run
   - Keep setup/build/invocation environment in `harness.toolchain.pcsx`.
     Per the user's subsequent simplification, system dependencies are identified
     by package name and availability, not pinned or recorded package versions.
     Wire `just setup` and `bin/harness setup --component pcsx-redux` through
     pinned SDL3 download, extraction, configuration/build/install and Redux
     build. Doctor separately checks build dependencies/source integrity, SDL3
     build identity and Redux built status; it never repairs or builds. Setup
     does not implicitly install system packages. Reject stale build receipts,
     failed linkage, altered source/archive inputs and missing prerequisites.
     Preserve source changes; reject mismatched revisions. Record compiler,
     build options, dependencies and resulting hashes. Never install packages
     implicitly while running a runtime mission.

2. [A8.02] (done) Add the Python harness runtime command node
- Owner: parent
- Depends: A8.01
- Blocker: none
- Evidence: independent reviewer `/root/plan_review` accepted the current transport and 15 pinned source hashes; `out/psx-coverage/harness-audit/` retains 58 passing runtime/layout checks, four passing process-ownership checks, current build identity and delegated live success/failure receipts; the index-writing file-mode case remains excluded, not passed
- Acceptance: status plus bounded command/script execution through PCSX-Redux; versioned receipts; failures/timeouts/output limits retain evidence and fail explicitly
   - Use single-noun modules, shared CLI/process/deadline primitives and the
     toolchain-owned executable/environment. Isolate each run in a fresh output
     directory; disable external debug servers and automatic updates by default.
     Pin BIOS, game/overlay, script and emulator inputs before launch and recheck
     afterward. Preserve stdout/stderr, terminal status and captured artifacts.
     Arbitrary Lua missions are trusted code, not a sandbox; never interpolate
     user text into shell commands or pretend an exit code proves a capture.
   - Keep the mission interface generic: `--script` selects the experiment,
     repeatable `--argument NAME=VALUE` supplies literal parameters, and
     `--executable` is optional. Capture supplied BIOS identities without tying
     execution to the audio project's US profile; its setup/doctor policy and
     Rust runtime profile checks remain separate. No legacy schema fallback.

3. [A8.03] (in-progress) Add repeatable boot, trace and capture missions
- Owner: parent
- Depends: A8.02
- Blocker: none
- Evidence: maintained smoke/target/call Lua scripts under `.pi/skills/psx-emulator/scripts/`; live startup, BIOS-only target, EXE-target and bounded function/selected-PC trace missions pass; device/audio histories remain open
- Acceptance: bounded BIOS/EXE handoff, target-qualified breakpoints, registers/RAM and audio capture with explicit completion markers and validated sizes/identities
   - Skill scripts dispatch to the harness; maintained Lua assets use the pinned
     Redux API. Separate emulator commands, capture validation and Rust comparison
     of audio/runtime semantics. Retain failed/mismatched evidence; no generated
     Rust-owned emulator runner, silent retries or alternate emulator fallback.
   - Extend generic device, DMA/IRQ and audio histories with explicit timing
     models and coverage bounds. Project-specific scenarios and semantic
     comparisons stay with consumers; missing device/audio evidence remains a gate.
   - Generic capability phases: (1) research official Lua/debugger APIs against
     the pinned build; (2) stage/hash Lua modules and state inputs in the harness;
     (3) implement state schema/query/export/compare/restore, CPU inspection/edits,
     bounded Exec/Read/Write observations, memory inspection/patching, counted
     Vsync/controller missions and display capture; (4) exercise real and invalid
     missions and publish concise routing plus detailed references. Generic
     protobuf interpretation belongs in Lua using Redux's schema and bundled
     decoder; Rust consumes evidence for audio conformance, not emulator scripting.
   - Implementation checkpoint: maintained Lua actions and module/input transport
     now exist. Live state export/restore and refactored function-call captures
     reproduce all 2 MiB of the prior RAM capture; its 24 trace observations are
     retained. A four-byte patch is identified exactly by a saved-state comparison.
     CPU inspection, Exec watch, controller override and counted Vsync execution
     pass; a BIOS display mission captures a 640x478 black buffer and valid PPM.
     A synthetic PS-X EXE validates Read/Write watches: the pre-store capture has
     zero at the destination, the pre-load capture has the written `0x1234`.
     Two restored-PC device-watch attempts and an unreached reset target timed
     out and remain failed evidence; no device-history success is inferred.
     See `out/audio-migration/redux-actions-*` and
     `out/audio-migration/redux-review/actions-*`. Generic actions do not close
     this step's still-required device/audio histories.
   - Device-action checkpoint: generic VRAM rectangle/CLUT export and native GP0
     uploads, COP2 inspection/editing/native command stepping, SPU port writes and
     Vsync DSP-state histories, CD state/media reads and bounded controller commands
     are implemented. Native MMIO probes use caller-selected scratch RAM because
     Redux's debugger memory-file writes bypass device handlers. They restore
     scratch bytes/GPRs/PC but advance device time; pending delayed loads/branches
     reject. CUE/BINARY mounting pins every declared track and isolates sidecars.
     Live evidence in `out/psx-devices/` proves a four-colour upload and indexed
     palette export, NCLIP MAC0=100 with CU2 enabled, SPU pitch=4096 and advancing
     ADSR fractional state, Getstat IRQ3, and original-disc sector/file bytes.
     Device bounds, disabled COP2 and missing media reject. These scoped actions
     do not establish full bus histories, PCM fidelity or production driver closure.
     Skill references are now organized per action with a broad reading-order table;
     `.agents`/`.codex` discovery links retain the same `.pi` owner layout as
     `psx-rizin`. Both linked skill paths validate.

4. [A8.04] (done) Publish the psx-emulator skill and agent spec
- Owner: parent
- Depends: A8.02
- Blocker: none
- Evidence: independent reviewer `/root/plan_review` accepted generic skill/spec routing, both validated discovery links and installed Pi discovery without diagnostics; delegated PC/IRQ queries and RAM export pass, invalid schema query fails without completion, and parent verified capture hashes under `out/psx-coverage/harness-audit/`; documentation checks pass; this proves neither Pi provider execution nor E7's complete action-surface acceptance
- Acceptance: usable scripts/references, concise skill routing, Pi agent schema and discovery links; scoped documentation and agent checks pass
   - Keep runtime mission selection, evidence interpretation and handoff to
     `psx-rizin` in the skill. The agent spec defines scope, tools, bounded runs,
     output contract and supervisor escalation; it does not grant source/map
     acceptance or launch models through the harness. References document the
     exact API, commands, scenario schemas and known limitations.
   - The skill and agent spec are generic PS1/PCSX-Redux tools, with no BOF3
     assumptions or named-game parameters. Execution to a target and state capture
     is one capability, not the definition of every mission. Audio-specific
     scenarios and Rust comparisons belong to the consuming projects' workflows.

5. [A8.05] (in-progress) Validate the harness boundary and capture handoff
- Owner: parent
- Depends: A8.03, A8.04
- Blocker: none
- Evidence: generic runtime/layout suite passes 48 checks (index-writing file-mode check excluded), including CUE/track identities, sidecar isolation, media drift and capacity failures; malformed state/query/arguments reject in live Redux; consumer-specific comparison results remain outside generic tooling acceptance; broader histories remain open
- Acceptance: malformed/missing inputs, revision drift, timeout/process cleanup, output limits, failed/incomplete captures and publication conflicts reject; representative live captures validate and hand off to an independent consumer
   - Harness-level Python coverage remains authorized for this integration.
     Audio-project tests stay Rust. Run focused harness/agent/docs checks and
     validate the evidence interface with independent consumers. Each consumer
     owns semantic checks and acceptance; a passing boot is not fidelity proof.

Runtime tooling checkpoint (2026-09-24): setup now owns the SDL3 download/hash,
source check, CMake build/install and pinned Redux Release build. Both
`bin/harness setup --component pcsx-redux` and the `just setup` chain use that
lifecycle; no implicit system-package install is introduced. Per user direction,
package names/availability replace version capture/pinning. Read-only doctor
checks prerequisites/recursive source, SDL3 built identity and Redux built
identity/linkage separately. `bin/harness runtime status|run`, maintained Lua
missions, the Pi skill/agent and discovery links replace the short-lived Rust
emulator runner (removed). Command registration moved to `harness.registry` to
keep dispatch modules below the existing 600-line ceiling.

Receipts are under `out/setup/` and `out/audio-migration/redux-review/`; independent
captures are in `redux-smoke/`, `redux-boot/` and `redux-boot-verified/`. The boot
capture stops at US EXE entry `0x8014aa0c` after original BIOS execution. It is
not a RAM/device parity result, reference audio, pruning proof or BIOS-free driver.
Doctor repair subsequently passed all 9 checks on the host. The sandbox rejects
the pinned 32-bit GCC with SIGSYS; diagnostics now identify that constraint.
The wrapper probe uses a temporary C file supported by the current compiler
adapter instead of unsupported stdin input. Documentation drift is split into
docs domain modules with its command at `harness.commands.docs.drift`; registry
inspection follows the declarative owner and reports zero findings. The repair
suite passed 117 checks with the unrelated file-mode/index-writing check deselected.

Generic mission checkpoint (2026-09-24): user clarified that `psx-emulator` serves
arbitrary PS1 runtime questions. Skill/agent instructions, scripts, environment
variables and runtime schemas now use generic PSX terminology. `target.lua`
replaces the boot-only mission, with an explicit target argument or supplied EXE
entry default. Harness arguments are recorded literal strings; stale inherited
mission variables are removed. EXE loading is optional and supplied BIOS hashes
are captured independently of the US-only setup/doctor policy. No compatibility
fallback was added. The runtime/layout suite passes 31 checks with one unrelated
check deselected. Live smoke, BIOS-only target and EXE-target missions pass under
`out/audio-migration/redux-generic-{smoke,bios,exe}/`; they capture RAM/registers,
not complete machine state. Consumer parity remains its project's obligation.
Broader capture/trace scenarios and full agent mission validation remain required;
A8 stays in progress.

## 2. [E1] (done) Compare structured states and measure replay
- Owner: skill implementation owner
- Depends: A8.01
- Blocker: none
- Evidence: independent reviewer `/root/plan_review` accepted E1.01 and E1.02; strict native state checks, exact byte-change evidence, repeated continuation, changed schedules and timing-only fault cases pass under `out/psx-coverage/`; 48 runtime harness tests pass; settings and observation limits are explicit
- Acceptance: exact field/range differences and repeated checkpoint comparisons retain identities and report divergence without hiding differences

1. [E1.01] (done) Add structured state differences
- Owner: skill implementation owner
- Depends: none
- Blocker: none
- Evidence: independent reviewer `/root/plan_review` accepted implementation after overflow and predecode budget fixes, with the required bounds documentation applied; `state-overflow-fixed` passes 14 native Lua checks, `state-equal` compares 21119854 bytes, and `state-patch-origin` identifies exactly four changed bytes at offset 1048576 under `out/psx-coverage/`
- Acceptance: scalar/repeated/message changes identify exact paths; bytes identify changed ranges; incompatible/absent fields and output limits are explicit

Extend `state.lua`, `snapshot.lua` and `references/STATES.md`. Select schema paths,
depth and output bounds; distinguish absence from zero, preserve large integers,
and record schema compatibility. Never silently skip fields or claim equality
from truncated output. Check known scalar/array/byte edits, equal states, missing
paths, incompatible states and limit failures with retained synthetic fixtures.

2. [E1.02] (done) Compare repeated continuation from a saved state
- Owner: skill implementation owner
- Depends: E1.01
- Blocker: none
- Evidence: independent reviewer `/root/plan_review` accepted bounded replay, origin validation and explicit settings provenance; `replay-cycles` matches two CPU checkpoints and start/stop cycles, `replay-alternate` reports changed buttons, `replay-timing-{start,stop,checkpoint}` detects timing-only drift, and `timeline-checks` passes six native Lua groups under `out/psx-coverage/`; all 48 runtime harness tests pass
- Acceptance: finite repeated runs compare equivalent checkpoints and report the first observed divergence with input/tool/state identities

Add a focused Lua replay action and `references/REPLAY.md`. Pin settings, state,
media, input schedule and checkpoint definitions. Bound repeat/frame/event counts;
capture selected CPU/device/memory checkpoints, cycles and stop reasons. Distinguish
guest state from host scheduling/audio queues. Check changed schedules and
state/media mismatches. Integrate E4/E5 sound/display observations when available;
report their absence in earlier results rather than claiming complete determinism.

## 3. [E2] (done) Trace execution and schedule interactive missions
- Owner: skill implementation owner
- Depends: E1
- Blocker: none
- Evidence: `/root/plan_review` accepted E2.01 and E2.02 after native-boundary, display, schedule and failure checks; all 101 focused harness checks pass with one index-writing case excluded; explicit native-emulator limitations remain documented
- Acceptance: general execution observations and input/checkpoint schedules have explicit bounds, ordering and provenance

1. [E2.01] (done) Add stepping, coverage and exception diagnosis
- Owner: skill implementation owner and native-binding reviewer
- Depends: none
- Blocker: none
- Evidence: `/root/plan_review` accepted current implementation and retained bounds/native limitations; approved into and exception patches pass setup/runtime identity checks (`setup-exceptions.log`); 13-step delay/call, 16-step nested/indirect/tail, 62-step recursion/five-entry and injected unchanged-context exception cases pass; both SYSCALL/BREAK slot indices preserve BD/EPC/exception PC; failed limit/target/step timeout reports retained under `out/psx-coverage/`; STEP/TRACE/NATIVE procedures explicitly exclude unverified hardware and other exception semantics
- Acceptance: bounded instruction/branch/function observations and hit counts identify their exact boundary; exception context and unsupported semantics are explicit

Extend `watch.lua`/`call.lua` or add focused `trace.lua` and `step.lua`, with
`references/TRACE.md` and `STEP.md`. Audit pinned native stepping before claiming
step-into/over/return support. Handle branch/load delays, indirect calls, tail
calls, exceptions and bounded recursion. Capture PC/opcode, selected register
deltas, cycles, qualified executable/overlay identity and pre-execution versus
retired observations. Report function/branch/address hit counts. Capture exception
PC/EPC/Cause/Status, available fault address, delay-slot state and nearby code;
retain diagnostic context on timeout. Check synthetic branches, delayed loads,
nested calls and faults. A temporary breakpoint is not automatically one retired
instruction. If Lua lacks the needed API, document a minimal native binding and
its authorization before implementation. Trace absence never proves unused code.

The user approved the guarded into-only patch and requires automatic setup
customization before compilation. Keep the registered CLI in `harness.commands.patch`
using the shared command runner; validation and framework belong under
`harness.patches`. Provide `list`, `apply`, `check`, `revert` and discovery in
`inputs/patches/<target>/`. Plain listing requires no target checkout; optional
`list --status` reports applied, pristine or unresolved state with diagnostics.
Operations select all patches by default, or explicit
targets/patches; ordering and dependencies must reject unsafe partial scope.
PCSX-Redux is the sole current target and its policy belongs in `harness.runtime`.
Setup and doctor must call the same target implementation. Preserve exact patch/base/postimage identity, reject
unrelated tracked/untracked source edits, and validate generated-source provenance
before incremental reuse. No compatibility patcher or parallel implementation.

2. [E2.02] (done) Schedule controller input and compare display checkpoints
- Owner: skill implementation owner
- Depends: E1.02
- Blocker: none
- Evidence: `/root/plan_review` accepted full E2.02 code/docs and current hashes; `timeline-current` passes six groups, `replay-release-current` observes same-frame release before sampling with equal cycles/registers and explicit button divergence; early-stop/unmet-condition/unavailable/metadata/bounds checks remain applicable; `display-visible-defaults` verifies exact native red/green 320x240 pixels and all 153600 changed bytes; `replay-visible` repeats visible checkpoints exactly under `out/psx-coverage/`
- Acceptance: finite press/release schedules, conditional stops and checkpoints preserve declared order and report pixel/state differences

Extend `frames.lua`, `references/FRAMES.md` and replay procedures. Stage a validated
schedule with frame-indexed transitions, slots, checkpoints and event/frame bounds.
Define simultaneous-event order and press/release semantics. Add bounded PC,
register or memory conditions without evaluating input as Lua. Preserve native
pixel metadata and compare dimensions, format, pixels and unavailable displays.
Record physical-input isolation or its limits; clear overrides on termination.
Reject unknown buttons, malformed conditions, nonmonotonic schedules, overflow
and unmet conditions explicitly. Analog modes remain E6.03.

## 4. [E3] (done) Inspect and trace DMA, interrupts and timers
- Owner: skill implementation owner
- Depends: E2.01
- Blocker: none
- Evidence: `/root/plan_review` accepted E3.01 and E3.02 implementation and operational documentation; exact approved patch/setup identities, bounded Lua export, native byte/order/lifecycle/failure checks and 84 focused harness checks pass; emulator/hardware boundaries remain explicit
- Acceptance: decoded system state and timestamped histories distinguish CPU observations, DMA transfers, completion and IRQ/timer transitions

1. [E3.01] (done) Expose decoded system-device state
- Owner: skill implementation owner
- Depends: none
- Blocker: none
- Evidence: `/root/plan_review` accepted serialized inspection against current source hashes and pinned native implementation; `devices-current` passes seven native/precision/wrap/malformed groups, all three configured CLI inspections pass, and invalid channel selection fails under `out/psx-coverage/`; raw mirrors, native-read side effects and history limitations remain explicit in DMA/IRQ/TIMERS procedures
- Acceptance: DMA address/direction/count/control, IRQ pending/mask and timer mode/count/target agree with known configurations

Add focused Lua actions with `references/DMA.md`, `IRQ.md` and `TIMERS.md`.
Prefer side-effect-free serialized inspection; distinguish raw mirrors from live
counters. Explicit native reads/writes must report side effects and elapsed time,
using maintained MMIO procedures rather than debugger-memory writes. Validate
channel/address/width/alias bounds and known device configurations.

2. [E3.02] (done) Capture native transfer, interrupt and timer histories
- Owner: skill implementation owner and native-binding reviewer
- Depends: E3.01
- Blocker: none
- Evidence: user-approved 19-file history patch SHA-256 `c9e3329c1b37c926eec7f8ae2b26d78cdc14d65b7e2c639325046daba414c7ed` is installed through setup; build/runtime identities pass (`out/psx-coverage/proposals/history/setup.log`); `/root/plan_review` accepted E3.02 implementation after raw-byte/order checks for OTC, GPU block/list/readback, SPU read/write wrap, synthetic CD wrap/zero-BCR, deferred/partial/overlapping/pre-capture MDEC, surviving callbacks and reset/cancel, timer gates/overflow/read-clear and CPU IRQ acceptance; terminal evidence is under `out/psx-coverage/history-*-checked*`, with ten ABI/lifecycle/reset groups in `history-recorder-v3`, PIO/MSAN/cyclic and event/byte/stop/timeout failures retained; four Lua field-label defects were corrected without native changes; standalone journal ASan/UBSan includes wrong-thread coverage, LeakSanitizer excluded by sandbox ptrace; 84 harness checks pass, one index-writing case excluded; reviewer also accepted all 23 compacted references and E3.02 operational contracts (HISTORY SHA-256 `d8c0f40812f490a831e4ee05e1f0be917415f01708d0d628d05f64999dbe4034`); GPU acknowledgment uses a pre-capture fixture latch and does not validate GP0 IRQ assertion; hardware fidelity and later capabilities remain outside this acceptance
- Acceptance: known transfers reconcile bytes/counts/addresses and completion/IRQ order; missing hooks, overflow and dropped events cannot report success

Audit native transfer/IRQ/timer boundaries and document the smallest hooks needed
for Lua capture; implement source changes only within specific authorization.
Record channel, source/destination, mode/size, cycles, completion, IRQ assertion/
acknowledgment and timer rollover/target events. Label level, edge and sampled
observations. Bound event/byte counts and retain overflow evidence. Exercise block
and linked-list DMA, IRQ masking/acknowledgment and timer transitions. Redux timing
is reference-emulator evidence, not independent physical-hardware validation.

## 5. [E4] (in-progress) Capture emulated sound and complete CD transactions
- Owner: skill implementation owner and native-binding reviewer
- Depends: E3
- Blocker: none
- Evidence: `/root/plan_review` accepted E4.01 native PCM/SPU capture below; E4.02 complete CD lifecycle capture remains open
- Acceptance: bounded PCM/register and CD lifecycle captures correlate samples, sectors, transfers and explicit terminal states

1. [E4.01] (done) Capture actual emulated PCM and SPU register events
- Owner: skill implementation owner and native-binding reviewer
- Depends: none
- Blocker: none
- Evidence: `/root/plan_review` accepted the approved installation, native PCM/SPU/CD mixing, capacity/lifecycle/settings and stop/timeout evidence below; ASan/UBSan store checks pass with leak detection disabled, TSan initialization failed with unexpected memory mapping
- Acceptance: PCM count/rate/channels, capture stage and cycle boundaries are explicit; ordered register events and overflow handling are validated

Extend `spu.lua`, `references/SPU.md` and focused `AUDIO.md` procedures. Locate the
native mixer/output boundary, document a minimal capture binding, and obtain
required dependency-source authorization. Capture raw PCM/WAV with explicit
pre/post-volume, reverb, XA/CDDA mixing and resampling provenance. Bound samples
and events; report latency, start/end/drain behavior and lost data. Capture SPU
register address/width/value and cycle order; do not relabel Vsync observations as
continuous history. Check silence, voices, key-on/off, pitch/envelopes, tails and
CD mixing, including exact sample counts/bytes at the chosen native stage.
Physical-audio fidelity and game-specific acceptance remain consumer gates.

Native patch checkpoint: the 17-file `sound.patch` candidate under
`out/psx-coverage/proposals/audio/` now implements dispatcher access ancestry/read
results, SDL controls and queue observations, lifecycle/settings invalidation and
owning Lua copies. `/root/plan_review` accepted authorization readiness at SHA-256
`4a91f64250339847fd6319faee2f6e504e5ae0fc2e6ea4b55d4c7edc9b48fc87` after
fixing pre-guard CPU reads, mid-capture MSAN/dynarec detection and missing XA
setting provenance. Relevant translation-unit syntax checks, updated ASan/UBSan
store checks and read-only patch applicability pass. No native source is installed;
specific authorization, setup/build, maintained Lua export/action integration,
operational references and live acceptance remain required. Preserve float values
beyond ±1, separate callback sample indices from observed CPU publications, and
never imply capture stop drains native queues or ends a DSP tail. Keep proposal
progress outside references.

Lua integration checkpoint: `spu.lua action=capture` now delegates to maintained
`sound.lua`, with bounded frame/PC stops, guest tail intervals, deferred native
freeze and explicit missing-binding failure. `pcm.lua` validates the named native
API and exports exact float bytes, WAV and separate block/access journals;
`wave.lua` preserves samples beyond ±1. `support.requireIdleCapture` rejects
maintained host edits during history or audio capture. Standalone Lua contract
checks in `tests/pcm.lua` and `tests/sound.lua` cover exact uint64 ordering,
payloads, bounds, loss, lifecycle, timeouts, tails and publication failure; these
are simulated API checks, not native acceptance. Independent WAV inspection
identifies stereo `pcm_f32le`, 44100 Hz and two fixture frames. Procedures remain
in `references/AUDIO.md` with the entrypoint/SPU routes updated. Specific patch
authorization, setup/build, live validation and final integration review remain
open; no native source or dependency was changed.

Lua review/verification: `/root/plan_review` accepted integration readiness after
independently rerunning both standalone suites and reviewing the shared guard,
scripts and procedures. The parent added a pre-save empty-recorder/idle-capture
check and reran the lifecycle suite. Current evidence under
`out/psx-coverage/proposals/audio/` records both contract suites, byte-exact
independent float WAV decoding, 58 passing focused harness tests with the existing
index-writing case deselected, clean documentation checks and plan parsing.
`out/psx-coverage/audio-lua-recorder/` passes all 10 installed native history checks
with the renamed guard. `out/psx-coverage/audio-lua-binding/` records the expected
missing-`PCSX.Audio` failure without a completion marker. This is Lua integration
evidence only; actual PCM capture and live/failure E4 acceptance remain unproven
until the specifically authorized native patch is installed and validated.

Approved installation checkpoint: the user authorized the exact reviewed
`sound.patch` above and GPU `video-draft.patch` below. Their bytes are integrated
as `inputs/patches/pcsx-redux/{sound,video}.patch`; the combined series passes
check and setup/build. `out/psx-coverage/proposals/audio/setup-approved.log` and
`status-approved.json` identify binary SHA-256
`f5bd3fb95ff443f78886579d488c40e70d806232df3070808123786258ce8974` with both
patches applied and SDL linkage verified. All 84 focused runtime/patch/layout
checks pass, one index-writing case excluded. This supersedes the pending-source
approval/install gates only; live PCM/SPU validation remains open. Lua audio
capture now enforces the documented state-origin receipt requirement, with its
focused lifecycle checks passing; native patch bytes are unchanged.

Native validation checkpoint: `/root/plan_review` independently verified capture
hashes, exact WAV/float payloads, silence and guest voice observations in
`out/psx-coverage/approved-audio-{silence,loop}/`. Silence contains 21888 stereo
frames; the looping voice contains 21120 frames, 330 blocks and 51 ordered SPU
events, with measured 2:1 channel amplitude, doubled pitch crossing counts and
silent post-key-off samples. The initial `approved-audio-voice` fixture used
flags 7, which this pinned mixer treats as one-shot; the corrected flags-3 fixture
uses the explicit repeat register. Read32 observes the hardware mirror without
invented halfword-handler reads. Frame/block/event capacity failures under
`approved-audio-limit-*` retain bounded prefixes, dropped counts and no completion
markers; the reviewer verified failure bits 1/2/4. `approved-audio-lifecycle`
passes eight native groups. The later `approved-audio-settings-named` adds eleven
setting mutations with sticky discontinuity failures, independently verified by
`/root/plan_review`; its earlier failed fixture
used C++ names instead of the exposed Lua names. No native patch changed.
The subsequent mixing and failure checkpoint below closes E4.01; neither queue
drain nor physical-audio fidelity is established.

E4.01 acceptance: `/root/plan_review` verified synthetic media identities,
receipt hashes, raw/WAV equality, channel levels and queue accounting in
`approved-cdda`, `approved-mix` and `approved-xa`. CDDA captures 21056 stereo
frames; simultaneous SPU/CDDA captures 21120 with both steady disc levels and
the varying voice contribution. XA captures 23296 frames and two accepted
37800 Hz feeds, each converting 2016 source frames to 2352 output frames;
4704 fed minus 3696 dequeued equals 1008 remaining. `tests/disc.lua` generates
local media; `tests/signal.lua` independently checks signal levels and RIFF
payload/count. XA's zero EDC is explicit: decoder evidence, not mastered-disc
conformance. Native location-change delay leaves an observed silence gap.
`approved-audio-limit-{stop,timeout}` retain native captures while failing mission
completion for an unmet PC stop or wall timeout. All 59 Lua files parse and six
focused contract suites pass. The existing standalone store checks pass against
the installed capture header with ASan/UBSan; wrong-thread, SDL-submission and
streaming-loss fault injection remain standalone coverage, not live device-fault
validation. Callback/CPU-publication timing is not per-sample execution timing.
The reviewer accepted E4.01 on this combined evidence, with no queue-drain,
hardware-fidelity or complete CD-lifecycle claim; E4.02 and phase E4 remain open.

`out/psx-coverage/proposals/audio/doctor-approved.log` records seven of nine
doctor checks passing, including Redux dependencies/build, SDL3 and BIOS.
Legacy GCC/tool-wrapper checks fail because the execution environment rejects
the pinned 32-bit compiler with SIGSYS; no sandbox policy was weakened.

2. [E4.02] (in-progress) Follow CD responses, sectors and streaming through completion
- Owner: skill implementation owner
- Depends: E4.01
- Blocker: none
- Evidence: approved native patch is installed through setup; `/root/plan_review` accepted the bounded command/PIO/DMA, joined XA/CDDA, missing-media and capture-limit batch below; broader native semantics and edge-path acceptance remain open
- Acceptance: acknowledgment/data/completion/error differ explicitly; bounded response/sector/DMA/XA histories preserve order and timing

Extend `cdrom.lua` and `references/CDROM.md`. Declare controller ownership and FIFO
consumption/acknowledgment: parked diagnostics must not race a guest handler;
observation-only missions must not consume guest data. Follow multi-response
commands, sector arrival/read/DMA, cancellation, timeout and errors. Record LBA,
sector mode, file/channel filters, XA coding transitions and sample/cycle linkage.
Validate reads, seeks, pause/stop, missing media and failed commands. First IRQ or
an ISO file read cannot prove completed controller streaming or XA playback.

Preparation audit: `out/psx-coverage/proposals/cdrom/` pins 13 source/proposal
inputs and maps missing CD port, response, sector and audio-feed boundaries.
Independent reviewer `/root/plan_review` identified shared mutable command
parameters, partial transfer-buffer changes, response/IRQ separation, surviving
callbacks/DMA after stream stop, synthesized CDDA silence and suppressed feed
paths; these requirements are retained in the audit. No dependency source or
skill action changed at that checkpoint. E4.01 is now accepted. The refreshed
`installed-inputs.json` and `design.md` in the same evidence directory define
explicit CD-profile recording, command/scheduler/stream identities, revised
response builders, partial-buffer provenance and optional Audio epoch/event joins.
`/root/plan_review` verified the original twenty installed-source pins and accepted
the in-progress preparation transition, identifying zero-byte probe compatibility,
response-builder revision, joined-recorder lifecycle and repeated-command ownership
gaps. The revised design addresses those points and adds scheduler/binding pins;
complete design and exact source-patch review remain required before authorization.
No new dependency source is installed. Full lifecycle capture remains unimplemented.

Design preparation review: `/root/plan_review` verified all 23 refreshed pins and
accepted the corrected design for exact layout/source drafting, including valid
native helper nesting. The reviewer also accepted standalone `provenance.h`
origin-range and response-builder helpers plus their ASan/UBSan checks as
preparation only; integration must emit each returned revision/displacement
immediately. `out/psx-coverage/cd-controller-data` passes 13 guest response IRQs,
filter bytes, eight PIO/DMA bytes and an invalid-command error, with independently
verified script/media/capture hashes. The initial mixed-disc fixture's read error
remains in `cd-controller-baseline`: its 75-sector data track falls within the
native 150-sector next-track rule. The first positive zero-byte transfer check
did not establish offsets. `/root/plan_review` subsequently accepted
`cd-controller-offsets`, verifying all 300 patterned sectors, source/media/capture
hashes and thirteen IRQ responses: PIO reads `00 01 02 03`, then DMA reads
`04 05 06 07`. Synthetic EDC/ECC remains zero; this is a diagnostic baseline,
not mastered-media conformance. Native hook and provenance integration remain
required. Proposal source copies remain separate from
the installed submodule; no additional native source authorization is inferred.

Partial native proposal: `/root/plan_review` verified 28 installed-source pins and
accepted the separate CD profile, guarded port/callback scopes, response publication,
command/scheduler ownership, Audio epoch checks and buffer/media integration for
continued work. Review fixes preserve builder IDs at IRQ assertion, command owners
across delayed phases, guards before metadata reads and backing-selection ancestry
across compressed CDDA/cache reads. Media records distinguish absolute MSF addresses,
adjusted data sectors, track-relative CDDA sectors, byte swapping and backend layout;
unowned sector reads and media replacement invalidate capture. Exact mutation ranges,
PIO readiness, DMA wrap behavior and retained trailing bytes remain explicit.

The reviewer independently accepted `scripts/sectors.lua` and `tests/sectors.lua`
as buffer-only correlation preparation: synthetic checks cover partial headers,
zero-fill, CDDA derivation, exact uint64 IDs and adjacent native DMA byte joins.
Whole-journal order/cycle/scope/media/stream/Audio checks remain the caller's duty.
The proposal's `layout.md` describes records; affected translation units pass syntax
checks, and standalone identity/provenance checks pass ASan/UBSan with leak detection
disabled. All 62 skill Lua files parse; documentation checks report no findings or
broken references. These checks do not exercise native joined capture.

Stream/feed preparation: `/root/plan_review` accepted scheduler ancestry, separate
read/play stream identities, feed suppression/queue outcomes and exact Audio
epoch/event joins. A canceled CDREAD owner survives until dispatch or replacement:
the CPU can still dispatch a callback from its earlier pending-bit snapshot.
Native stream stop does not imply callback cancellation, queue drain or audible
onset. The reviewer also accepted pre-activation environment bounds, allocation-free
CD-only settings/clock/scale checks, SPU/SDL lifecycle invalidation and paired
mode/filter/attenuation records. Transient unhooked host setting edits remain outside
the observer guarantee; native playback arithmetic and scheduling are unchanged.
All 30 installed-input pins remain unchanged. Eight affected translation units
pass syntax checks; existing provenance/identity ASan/UBSan checks (leak detection
disabled), buffer correlation checks and all 62 skill Lua parses plus the proposal
binding pass. `layout.md`, `integration-draft.diff` and `draft-checkpoint.json`
retain the 21-file source preparation. Complete journal/export validation and Lua
capture orchestration, final exact-patch review and authorization, build and live
acceptance remain open. No new dependency source is installed; E4.02 remains in
progress.

Wire export preparation: `/root/plan_review` accepted additive CD fields in
`events.lua`, including phase-specific reserved scheduler words and buffer-boundary
labels. `tests/records.lua` checks every variant, exact uint64 IDs, copied payloads
and malformed layouts; existing GPU export and buffer-correlation suites pass.
`out/psx-coverage/cd-wire-recorder-current` passes all ten installed recorder groups
with verified current input/capture hashes. This exercises the existing recorder,
not the proposed CD hooks. The original `events.lua` bytes are retained as
`events-before.lua` beside the proposal; the other 29 baseline pins remain unchanged.
`wire-checkpoint.json` pins the maintained changes separately. Raw export does not
validate command/response, scheduler, media, stream or Audio relationships; complete
correlation/orchestration and all native installation/live gates remain open.

Controller preparation: `/root/plan_review` accepted `scripts/controller.lua`
for ordered scopes, shared parameters, repeated command owners, revised response
publication and FIFO/IRQ observations. CPU writes retain their full operand while
handlers consume its low byte; each FIFO/data read requires one observation.
Exported bytes use hex or retained-payload references. `tests/responses.lua`
passes positive and corruption cases, including wrapped cursors with ready clear,
nested helpers, parameter changes, mask writes and acknowledgments. Existing wire,
GPU export and buffer checks pass; all 65 skill Lua files parse.
`controller-checkpoint.json` pins this component and its checks under the CD
proposal. These are synthetic component checks, not complete scheduler/media/
stream/Audio correlation, capture orchestration or native live acceptance. E4.02
remains in progress with those gates open.

Scheduler preparation: `/root/plan_review` accepted `scripts/scheduler.lua` for
native/CD schedule adjacency, float32 target calculation, replacement ancestry,
nested helpers and initial/final slot reconciliation. Dispatch requires native
due-target and pending eligibility; canceled reads can retain same-cycle CDR
snapshot eligibility, independently of owner retention. Later native dispatches
expire that eligibility, including non-CD devices. `tests/scheduler.lua` covers
positive/corrupt records and composition with the controller. Five focused suites
pass and all 67 Lua files parse. `scheduler-checkpoint.json` retains component
identities/checks under the CD proposal. Stream/media/Audio relationships,
orchestration and all native authorization/build/live gates remain open; E4.02
stays in progress.

Stream/media preparation: `/root/plan_review` accepted `scripts/streams.lua`
for initiating/trigger command identities, native active flags, stop/cancellation
and surviving callback ancestry. The reviewer accepted `scripts/media.lua` for
lookup/cache/backend and buffer-source correlation, including backing selection
changed by compressed CDDA, mutable cache success, signed/short backend returns
and synthesized silence. Seven focused suites pass; all 71 Lua files parse.
`correlation-checkpoint.json` under the CD proposal pins both components and their
checks. Mock relationship fixtures and empty composed captures do not establish
native or full integration. Raw transition metadata, track-table semantics, Audio
gates/outcomes/joins, complete journal/export integration and orchestration, final
exact native review/authorization/build and live acceptance remain open. The native
proposal and installed baseline are unchanged; E4.02 remains in progress.

Audio correlation preparation: `/root/plan_review` accepted `scripts/feeds.lua`
for early suppression, native frame calculation, queue outcomes and exact forward
Audio epoch/event joins, requiring a complete frozen Audio export when joined.
The reviewer accepted `scripts/eligibility.lua` and the `sectors.header()` accessor
for XA gates, automatic filters, decoder control flow and CDDA stop/mute/feed order.
Observed decoder metadata persists across callbacks; failed parsing preserves it,
continuations ignore new coding bytes, and known tuple changes reset sample counts.
Pre-capture coding can remain unknown. Nine focused suites pass; all 75 Lua files
parse. `audio-checkpoint.json` under the CD proposal pins these components, checks
and decoder evidence. These acceptances do not prove PCM fidelity or audible timing.
Raw transition/track-table semantics, whole-journal integration and orchestration,
final native review/authorization/build and live acceptance remain open. No native
source was installed; E4.02 remains in progress.

Journal/capture preparation: `/root/plan_review` accepted `transactions.lua`,
shared wire decoding in `events.describe`, and the `transport.lua` lifecycle
behind `cdrom.lua action=capture`. Every record reaches all seven consumers;
buffer bytes remain references into `payload.bin`. Synthetic composed captures
cover read delivery, response publication, next-sector lookup/rescheduling,
explicit transfer enable/PIO, muted XA and optional forward Audio joins. Encoding
and size failures retain raw evidence and emit failed correlation reports.
Capture starts Audio before History, freezes in reverse order outside callbacks,
and forbids state serialization while either remains active. Failed starts,
freezes, exports, limits and stops cannot publish mission completion.

Eleven focused Lua suites and 58 selected harness checks pass; all 79 Lua files
parse. The Git-index-writing harness case remains excluded. The installed recorder
passes ten groups in `out/psx-coverage/cd-integration-recorder-current`, exercising
the shared exporter without CD hooks. The installed build rejects
the absent CD binding in `out/psx-coverage/cd-capture-unavailable` before mission
execution, with exit 1 and no completion marker. `integration-checkpoint.json`
under the CD proposal retains component/check identities and unchanged native
proposal pins. Original `cdrom.lua` bytes are retained as `cdrom-before.lua`;
the original baseline pins remain authoritative. References describe procedures
and explicit limits, not a live-support claim. Complete native callback-transition
and track-table semantics remain open: successful correlation is not proof that
every required callback event occurred. Final exact native review, authorization,
setup/build and positive/failure live capture acceptance remain required. No new
dependency source is installed; E4.02 remains in progress.

Native installation: `/root/plan_review` accepted the exact 21-file CD patch
`267231d0955116bf7a4a5d6a0f7e1929be819923e507ba40a5dc724919b99c90`
for authorization/build testing; the user approved those bytes and setup integration.
`inputs/patches/pcsx-redux/cdrom.patch` retains that checksum. Initial setup failed
before native mutation because alphabetical reconstruction put CD before its
prerequisites. Target-owned explicit ordering fixes this while retaining generic
discovery, exact reconstruction and scoped dependency guards; 44 focused harness
checks and Ruff pass. `/root/plan_review` accepted the ordering fix and additional
Lua reserved-field and PIO/DMA cursor/readiness joins, including DMA wrap.

Setup then passed and all 21 installed files match the approved draft. Binary
`d61595623b9c9b4eb132b3ff4b3a6e9548ee3633b59f9b200e790fb71c31f19a`
passes runtime identity/linkage checks. `out/psx-coverage/cd-approved-binding`
passes eight native groups: bounds, ownership, ordinary-profile compatibility,
unjoined/joined export and no-feed Audio stop/restart invalidation. This is empty
profile evidence, not CD playback acceptance. `cd-approved-controller` passes
thirteen guest IRQ responses, filter bytes, PIO/DMA offsets and invalid-command
error using synthetic media and guest polling waits. These independently checked
terminal values precede native-journal replay validation. Artifacts remain under
`out/psx-coverage/proposals/cdrom/`; E4.02 stays in progress with full callback/track
semantics, multi-response/stream capture, positive/failure and playback gates open.

Live validation checkpoint: `/root/plan_review` accepted the bounded evidence in
`cd-approved-data`: 19,895 events, thirteen response IRQ/payload pairs matching
the independent guest diagnostic, and PIO offsets 12–15 followed by DMA offsets
16–19 yielding `00 01 02 03 04 05 06 07`. The maintained state-export action in
`cd-approved-data-ram` also matches those values in this capture's own final RAM.
These are command/transfer observations, not whole-driver or hardware fidelity.

Independent reviewer `/root/plan_review` accepted `cd-approved-xa` and
`cd-approved-cdda`: raw journals,
correlation, exact Audio joins and independent WAV/signal checks. XA records
25,235 events, four mutations and one 37800 Hz stereo feed converting 2016 source
frames to 2352 output frames; its PCM has 20,928 frames. CDDA records 22,806 events,
62 mutations and 31 feeds of 588 frames; its PCM has 18,624 frames. No queue drain
or complete XA coding/filter-transition coverage is inferred. `cd-approved-missing`
records two no-image lookup failures and IRQ5 with no feeds; observation success
does not mean the command succeeded. `cd-approved-limit-{events,mutations,reads,stop,timeout}`
all withhold completion: event overflow sets failure bit 1; consumer limits retain
complete raw journals; unmet stops and wall timeout remain mission failures.

`installation-checkpoint.json` under the CD proposal pins fifteen runs and 459
verified input/staged/capture/tool identities, plus retained failed-run artifacts.
Eleven Lua suites, all 80 Lua parses and 58 harness checks pass; one index-writing
case remains excluded. Documentation checks and `git diff --check` pass.
The reviewer independently verified the batch, including 132 retained capture
hashes and 106 source pins. Complete callback/track semantics, backend/cancellation
edge paths and remaining streaming transitions are still required. E4.02 remains
in progress.

Required-flow audit: `/root/plan_review` demonstrated that removing both delivered
READ scheduling observations and reconciling final scheduler metadata still passed
relationship correlation. `delivery.lua` now checks mandatory response construction,
delivery-source lookup, next-read scheduling, next-sector lookup, cleared transfer
readiness and conditional DataReady; error-zero requires terminal DiskError without
rescheduling. The existing composed-journal suite rejects removed schedules/next
lookups. Callbacks without delivery mutations remain unclassified; exact hidden
delay state, inactive/retry branches, command/play paths and DMA obligations remain
open. `/root/plan_review` accepted the scoped implementation after requiring the
native READ/ROTATING/SEEK response bits and exact uint8 MSF carry/wrap behavior;
status corruption and equal-LBA/different-packed-address cases now reject.
The reviewer verified 82 identities and required event order in final-module
`cd-delivery-final-data` and `cd-delivery-final-missing`: delivered data includes
schedule/DataReady/next lookup; error-zero terminates with DiskError without a
reschedule. `delivery-checkpoint.json` pins these runs and accepted module/test
hashes. Earlier `cd-delivery-{data,xa,cdda,missing}` runs retain the pre-refinement
module as historical evidence. The composed suite passes with the final fixes;
eleven suites and 81 parses passed before those two narrow refinements.

Transfer semantics: `/root/plan_review` accepted `controller.lua` transfer-enable
and mode-specific cursor/readiness checks plus `transfers.lua` DMA issuance and
emitted completion validation. Zero BCR uses the native mode-dependent size;
DMA continues after readiness clears. Not-ready starts require immediate nested
completion. Orphan DMA scopes, missing schedules, wrong byte counts/addresses,
unknown current-request identities and duplicate completion trios reject. Dispatch
owner and current channel request remain separate; DICR masking controls assertion,
not the presence of completion records. Arbitrary scheduled-callback busy
eligibility remains open until CHCR state is observed sufficiently.

The reviewer independently accepted `cd-transfers-{pending,wrap,data}`: a
4096-byte transfer's callback survives a newer immediate request and later runs
without another completion; wrap plus zero-BCR transfers reproduce all 2056
patterned bytes and destinations; the command-driven capture preserves four PIO
and four DMA bytes. `transfers-checkpoint.json` pins five runs, including seeds,
and 150 verified identities. Eleven Lua suites and 82 parses passed before the
three narrow review fixes; the expanded controller/transfer suite and fresh live
captures pass with those fixes. References retain the remaining CHCR boundary.

Native media metadata also needs a bounded authoritative descriptor before exact
track/pregap/file/offset derivation can be accepted. Controller track selection and
backend CDDA selection use different rules; nonnegative short data reads succeed,
whereas CDDA requires 2352 bytes. Do not infer missing prehistory values or promote
relationship consistency to full native behavior. Any further native source change
requires a separately reviewed exact patch and specific authorization.

Next observer preparation is in `out/psx-coverage/proposals/cdrom/context/`, with
nineteen pinned source inputs and verified recovery copies. Its design adds bounded callback state, actual evaluated
predicate results and native media descriptors, enabling full READ/command/PLAY/
lid/decode branch checks and scheduled-completion busy eligibility. It forbids
extra MMIO/media reads or host-time evaluations. Stable media ordinals must bind
to staged identities; unsupported sidecar effects must reject. Design review,
exact source drafting/review, specific authorization, setup/build, coordinated Lua
profile cutover and fresh live acceptance remain required. Installed native source
and the approved patch are unchanged.

Context proposal: `/root/plan_review` accepted the corrected design for drafting,
including all 100 allocated track slots, observer-only SubQ validity, dynamic
availability and preactivation allocation. The separate twelve-file draft adds
profile-2 callback contexts and evaluated predicates, bounded file/slice descriptors,
geometry checks and SBI provenance. CUE SubFile offsets must bind through parent
ordinals to staged media; filename-only mapping is insufficient. An absent SBI
file can leave the native count unknown: observing a later CheckSBI invalidates
capture before its original evaluation, without changing native behavior.
The reviewer accepted the partial callback/predicate portion for integration and
corrected its minimum payload calculation. Final review accepted the twelve-file
patch for specific authorization at SHA-256
`f7ff24919749fae02a22e6ae24de8ecbb5060fefb0d872317e29914cdf0b5cc6`,
after fixing cached subtree depth, strong media ownership, state-changing PosixFile
size queries and pre-guard metadata reads. Media identity now uses weak ownership;
file roots are limited to UvFile and exact path bytes require consumer validation.
The reviewer verified all nineteen source pins and exact patch reconstruction.
Nine affected translation units pass syntax checks, eleven existing Lua
suites pass and all 82 Lua files parse. Lua checks exercise the installed profile,
not the proposal. Existing journal/provenance/Audio identity checks pass ASan/UBSan
with leak detection disabled; read-only patch applicability, runtime identity and
documentation checks pass. Exact patch authorization, setup/build, maintained profile-2
cutover and fresh live acceptance remain required; E4.02 remains in progress.

The user subsequently approved the exact context patch above. Its bytes are
installed as `inputs/patches/pcsx-redux/context.patch`; setup passed and all twelve
installed postimages match the reviewed draft. `context/setup-approved.log` and
`runtime-approved.json` retain build/status evidence. The rebuilt executable is
SHA-256 `f0c1b7a89043bed0c2e3e5561f2c1933d90ad0174ba4075f4d768fc1da98470d`;
SDL and recursive source revisions are unchanged. Independent Lua preparation
adds `image.lua` and extends the existing media suite for profile-2
descriptor bounds, all slots, exact uint64/raw path bytes, file/slice ordering,
receipt association and native table helpers. A reviewed getPregap fallback
mismatch was corrected and covered; focused checks pass, with unusual MSF and
uint32-underflow values also checked against the native header. `/root/plan_review`
accepted this helper's preparation, without native-capture acceptance.

Strict Lua profile-2 cutover now includes callback contexts, predicate wire/owner
checks and decoded identical begin/end image descriptors with boundary ordering.
Earlier CD context layouts reject. Review found and prompted fixes for callback
response-byte/identity joins and same-cycle state triples; response construction
may change bytes beyond its published prefix, so full-buffer continuity resumes
from the observed after-context. Migrated component/full-journal fixtures pass,
including old-version and corrupted context/image rejection. The fresh native
`cd-context-binding` mission passes empty-profile/lifecycle checks; its image
composition predates the latest integration. `cd-context-controller` independently
passes thirteen guest response stages, filter response, PIO/DMA bytes and command
error checks on the rebuilt binary. Full capture review/live validation, staged
receipt binding, predicate/branch reconstruction and complete begin/end semantic
reconciliation remain open; E4.02 is not complete.

`/root/plan_review` accepted the limited profile-2 consumer cutover and live
integration after rechecking its three controller findings. Fresh
`cd-context-boundaries` records 53 events/11130 payload bytes;
`cd-context-data` records 21054 events, 572 paired callback contexts, thirteen
predicates and identical image descriptors. Independently decoded same-run final
RAM matches all thirteen IRQ/response tuples and eight PIO/DMA bytes `00–07`.
`cd-context-overflow` fails at 53 events, retains its prefix and writes no
completion marker. Review verified 164 hashes and twelve approved native
postimages; `context/lua/review.json` retains attribution and scope.
`context/installation.json` additionally pins current sources and 120 receipt
identities, explicitly retaining the older binding run's staged transaction
module. Eleven focused suites pass, all 83 Lua files parse, and 58 harness checks
pass with the index-writing case excluded. Documentation/diff checks pass.
Synthetic full-journal fixtures pair an empty descriptor with fabricated
successful media records: they prove composition, not backend/descriptor
compatibility. Receipt binding, branch validation and E4.02 completion remain open.

Staged-media binding implementation: `harness.runtime.disc` encodes ordered
track keys, byte sizes and hashes as literal environment rows; the session owner
verifies staging before launch and replaces inherited media/directory variables,
including explicit empty media. `mount.lua` enforces exact rows, 99 ordered unique
tracks, canonical sizes/hashes and existing capacity bounds, then derives exact
absolute staged path bytes. Capture binds both image descriptors to these entries;
unknown roots and size disagreements reject, unused staged tracks remain explicit.
No executable data, additional manifest/parser, dependency or basename fallback
is introduced. Final passed-receipt checks retain authority over input stability.
`/root/plan_review` accepted implementation readiness after four focused Lua
suites, fifty harness tests and an independent failed-export probe that preserved
raw evidence. `/root/plan_review` subsequently accepted live receipt binding and
profile-2 path integration after 428 hash checks across five captures and two
seeds. `cd-binding-xa` has one bound root/one feed; `cd-binding-cdda` has two
bound roots (ordinals 1/3), owning slices 2/4, and 31 feeds. Both descriptor
endpoints match exact staged paths, sizes, digests and final passed receipt keys.
`cd-binding-guards` rejects missing bindings, unmatched roots and altered sizes.
The early empty/data captures retain old authority wording; empty also precedes
the added guard assertions. Current XA/CDDA/guards use the reviewed current
binding source. This closes the association gap, not native media arithmetic or
branch validation. Evidence/recovery inputs are in `context/binding/`.

Callback CHCR snapshots now drive `transfers.lua` completion eligibility: native
`dmaInterrupt()` completes exactly when bit 24 is set, clears only that bit and
otherwise leaves CHCR unchanged. Completion records must match the before-value;
missing required work and contradictory after-values reject. Callback owner stays
separate from the channel's current request. The result retains active and idle
callbacks. `/root/plan_review` accepted the component and fresh integration after
248 hash checks with no current-source drift. `cd-busy-pending` completes request
2 immediately, then observes older owner 1 idle without another completion;
`cd-busy-wrap` requires both completions for exact 8/2048-byte transfers;
`cd-busy-data` completes four DMA bytes and preserves combined PIO/DMA RAM `00–07`.
Raw callback snapshots/completion records and same-run RAM agree. The unsupported
`cdwrap` seed attempt remains failed without a completion marker; the corrected
maintained `cd` scenario used a fresh output. Twelve focused Lua suites, 85 parse
checks and sixty harness checks pass, with the index-writing case excluded.
Ruff, documentation and diff checks pass. Recovery/attributed reviews are in
`context/binding/` and `context/busy/`. Next, complete READ inactive/busy/IRQ-delay
branches and exact delays using captured predicates and context; command, PLAY,
lid/decode and native media-selection semantics still require validation.
E4.02 remains unfinished.

READ branch validation now derives inactive/busy/IRQ-delay paths from initial
contexts and evaluated predicates, requiring exact unscaled delays and stable
unrelated state. Delivered/error paths bind the first lookup and actual buffer
predicate, preserve unused response bytes, and enforce status, sector/cache
movement, required work and IRQ mask/latch transitions. Unknown body records
reject; media/SubQ and XA observations stay in delegated windows. Native code is
unchanged. Maintained `tests/reading.lua` seeds controller/scheduler/hardware fields
through the native saved-state schema; these are host-edited branch experiments,
not evidence of reachable game initialization.

`/root/plan_review` accepted the scoped implementation and live evidence after
692 hash checks. Standard-clock captures cover inactive, busy (256), IRQ-delay
(225792), missing-media DiskError and speed (225792). Fresh guest-command data
capture demonstrates location delay (13547520), thirteen response tuples and eight
PIO/DMA RAM bytes; XA capture validates three delivered callbacks and joined Audio.
The separate host-edited location capture exited 245 with empty logs and no journal
or completion. Its exact-source diagnostic replay passed, including location
`1→0`, sector advancement and delay 13547520; the original failure remains
unexplained and retained. A successful replay does not resolve that investigation.

Odd/low clocks, null-buffer success, trailing prefetch failure, status preservation,
malformed events and uint8 MSF carries have component coverage, not live claims.
The first inactive seed failed on Lua argument expansion without a completion
marker; corrected source used fresh output. The passing inactive capture predates
final IRQ checks but contains no IRQ work. Thirteen focused Lua suites and all
87 Lua parse checks pass; documentation/diff checks pass. Recovery, source pins,
receipts and attributed review are in `context/read/`. E4.02 remains unfinished:
command, PLAY, lid/decode, media-selection/SubQ semantics, remaining live edge
cases and the unexplained native exit still require work.

Native exit investigation recovered the exact location run's system core. Its
SIGSEGV occurs in LuaJIT `check_call_unroll` during trace recording, with a
synchronous Pause event reentering Lua from `pauseEmulator()` inside the target
breakpoint. This establishes the failing host path, not the precise upstream
LuaJIT defect. Maintained missions now load `support` before native calls: it
uses `jit.off()` and `jit.flush()` to exclude host trace compilation, rejects
re-enabled entry and disables and flushes tracing before native failure cleanup.
Guest CPU mode, dependency source and approved native patches are unchanged.

`/root/plan_review` accepted this mitigation for maintained execution after
251 final receipt/source/staged/capture/tool hash checks and four current source
pins. Callback checks remove an observed preexisting trace, complete 4096 native
Run/Pause pairs and reject deliberate re-enablement with exit 1/no completion.
Fresh location/data/XA captures pass; guest cycle bounds, CD event counts
(89/21054/25314) and READ records match earlier captures. Mixed Audio remains
complete, with 327 versus 326 baseline blocks; host scheduling and PCM equality
are not inferred from this comparison. All three sessions finish within their
existing bounds. Thirteen focused suites and 88 Lua parse checks pass;
documentation/reference/diff checks pass. Core, source recovery, comparisons,
receipts and attributed review are retained under
`out/psx-coverage/proposals/runtime/jit/`. The original failed receipt remains
negative evidence; support is for the maintained interpreter workflow, not a
repaired or validated LuaJIT tracing mode. E4.02 remains in progress with command,
PLAY, lid/decode, media/SubQ semantics and remaining edge cases open.

Command-callback checkpoint: `command.lua` and `opcode.lua` now enforce busy
preservation, exact uint64 delayed-repeat selection, opcode-local state/response
writes, ordered schedules/stream effects, drive overrides and IRQ publication.
Shared parameter bytes and all sixteen response bytes remain checked; GetTD/Test
unwritten tails survive. ID predicates must agree with play state, position and
image type, while lid openness remains an evaluated input. Media/SubQ arithmetic
and submission-time effects remain separate obligations.

`/root/plan_review` accepted the scoped callback consumer after correcting ID
predicate consistency. Fifteen focused Lua suites and 93 parse checks pass.
`command-data` observes eleven executed and 61 busy callbacks; `command-xa`
observes three executed callbacks. Host-seeded `command-{retry,late,tail,test,
notready}` exercise repeat deferral, overdue execution, stale tails and retained
scheduled work despite NOTREADY. These captures stage the earlier opcode module,
whose only subsequent change is the ID check. Final `command-id-{data,play,missing}`
validate data/play status, the complete PCSX ID response and missing-media error;
seeded callbacks do not establish submission reachability. Source recovery,
checks, receipt/source/staged/capture hashes and review are retained under
`out/psx-coverage/proposals/cdrom/context/command/`. E4.02 remains in progress:
submission, PLAY/lid/decode, media/SubQ arithmetic and remaining edge cases are
still required; this checkpoint does not claim complete command coverage.

Submission checkpoint: `submission.lua` checks all write1 register banks,
submission snapshots, repeated/delayed queue retention, replacement with a stale
repeat flag, exact 2048-cycle scheduling, stored BCD parameters regardless of
count, the >16-sector seek threshold, mode changes and ordered stream stops.
Controller/scheduler/stream consumers retain identity and cancellation ownership;
all other context and parameter/response/SubQ bytes must remain unchanged.
`/root/plan_review` accepted the observable-state model and scoped live integration.
Sixteen focused Lua suites and 96 Lua parse checks pass. `submission-all` executes
24 commands, fifteen repeats and two inferred reset conditions through guest
stores. `submission-{mode,keep,bits}` distinguish entering exact mode 0x40,
remaining there and entering 0xc0. Data/XA integration preserves callback, stream
and Audio correlation. Recovery, source pins, raw captures, receipts and review
are retained under `out/psx-coverage/proposals/cdrom/context/submission/`.

XA predictor writes remain unverified: profile 2 records neither channel's reset
state. The pinned `spu/freeze.cc` saves both channels into `XAADPCMLeft`, overwriting
the left with the right; `sstate.cc` restores both from that field. SPU's saved XA
pointer also need not be current CD decoder state. Record reset conditions as
inferred, not captured execution; full predictor/reset acceptance requires direct
evidence and resolution of this serialization limitation through reviewed,
specifically authorized native changes. No native source changed here. E4.02
remains in progress with those obligations, PLAY/lid/decode callbacks, media/SubQ
arithmetic and remaining edge coverage open.

PLAY checkpoint: `playback.lua` enforces seek/busy returns, endpoint and
autopause ordering, report construction from raw signed16 sector peaks, mute/feed
control, native uint8 MSF movement, unscaled delays and publication/state
preservation. Media/SubQ arithmetic, attenuation and feed-content fidelity remain
separate obligations. Seventeen focused Lua suites pass; the final focused rerun
and all 101 Lua parse checks pass. Missing/reordered work, corrupted state/response
bytes, both report channels, peak clamping and carry boundaries have component
coverage.

`/root/plan_review` accepted the scoped implementation and native captures:
`playback-cdda` has 31 exact Audio joins; `playback-data` completes its seek-only
callback. `playback-{busy,queued,seekplay}` prove busy preservation, seek response
without IRQ and location-delay playback. Corrected `playback-{published,endauto,
report,right,endreport}-fixed` retain a nonzero published response through
DataEnd, exercise endpoint double stops and report-after-stop, and validate raw
left/right peaks and relative-time selection. Raw left peak -32768 is clamped
before half-volume attenuation; the right peak occurs in the final frame.

Earlier endpoint/right/publication fixtures did not reach their named branches:
native restore replaces the endpoint and refreshes SubQ/track state via
`readTrack(++prev)`. Those original receipts remain diagnostic evidence, not
endpoint/autopause/right-channel acceptance. Corrected seeds and post-restore
contexts establish the intended paths. Source recovery, focused checks, pinned
receipts/captures and attributed review are under
`out/psx-coverage/proposals/cdrom/context/playback/`. Native sources are unchanged.
E4.02 remains in progress: lid/decode callbacks, media/SubQ and audio arithmetic,
XA predictor/reset evidence and serialization repair, plus remaining edge cases
remain open.

Decoded-buffer checkpoint: `decoded.lua` checks playback, SPU control and IRQ
address gates, IRQ 0x200 before rescheduling, floor-before-multiply delay and
unchanged controller/parameter/response/SubQ bytes. The command and scheduler
consumers retain global IRQ-latch and native scheduling checks. Eighteen focused
Lua suites and all 104 Lua parse checks pass; malformed/missing work, exact delay
and unchanged-context checks have component coverage.

`/root/plan_review` accepted the scoped consumer and native
`decode-{inactive,disabled,outside,edge,asserted,latched}` captures: address 255
asserts and reschedules, 256 returns, and a preexisting IRQ latch still records
assertion without losing other bits. Native rescheduling requests 196608 cycles.
Seeds use the native save schema and account for restored control from its port
mirror. These host-edited experiments do not prove guest initialization or DSP
buffer timing. SPU reads can clear wait state outside the unchanged CD context.
Recovery, checks, source pins, raw journals and attributed review are retained in
`out/psx-coverage/proposals/cdrom/context/decode/`. Native sources are unchanged.

Lid source review identified a remaining observation gap: closed-lid `check()`
clears CD ID/label outside the context and, with mounted media, reaches the ISO
`readSectors()` path that invalidates current CD capture. Missing media can return
earlier and still enter RESCAN. Keep mounted-media closure unsupported until a
reviewed, specifically authorized native boundary is installed and validated.
E4.02 remains in progress with lid callbacks, media/SubQ and audio arithmetic,
XA predictor/reset evidence and serialization repair, plus remaining edge cases.

## 6. [E5] (done) Trace GPU packets and display effects
- Owner: skill implementation owner and native-binding reviewer
- Depends: E3
- Blocker: none
- Evidence: `/root/plan_review` accepted E5/E5.01 after independently checking the approved build, exact native ingress/effects, failure boundaries, CPU-stored readback, receipt hashes and current clean documentation checks below
- Acceptance: GP0/GP1 and ordering-table DMA history correlates bounded raw/decoded packets with observed VRAM/display changes

1. [E5.01] (done) Decode GPU commands and ordering-table DMA
- Owner: skill implementation owner
- Depends: none
- Blocker: none
- Evidence: `/root/plan_review` accepted supported native capture and decoder semantics with positive/failure evidence below; source audit and current checks are under `out/psx-coverage/proposals/gpu/`
- Acceptance: known packet streams retain boundaries/order/identity and explain captured effects; malformed/cyclic/oversized streams reject

Extend `vram.lua` or add `gpu.lua`, with `references/GPU.md`. Capture CPU and DMA
command ingress through verified APIs/hooks; retain raw packets and decoded
commands with timestamps. Follow ordering-table links with RAM alias validation,
cycle detection and packet/word bounds. Decode draw environment, texture/CLUT
references, transfers and display settings; associate before/after VRAM and
screenshots. Check primitives/uploads, linked lists, truncated packets, cyclic
pointers and overflow. Packet replay needs separately validated state inputs;
do not infer hardware equivalence from matching images.

Decoder preparation checkpoint: maintained `packets.lua` and `gpu.lua` now inspect
immutable GP0/GP1 streams and bounded 2 MiB RAM ordering tables. Raw words,
packet/node boundaries and RAM aliases survive; malformed/truncated streams,
cycles and unsupported parser cases reject. `tests/packets.lua` exercises
primitive/texture/transfer/control fields, crossing node boundaries and failures.
The source audit in `out/psx-coverage/proposals/gpu/` distinguishes existing native
DMA spans from missing CPU GP0/GP1 ingress, and GUI frame logging from persistent
capture. No native source was changed. Native command hooks, initial parser
context, cross-port ordering, timestamps and before/after VRAM/display correlation
remain required for E5 acceptance; offline packet decoding does not satisfy them.

`/root/plan_review` accepted the in-progress transition after finding and
rechecking polyline terminator-pattern command/first-coordinate cases. Both reject
explicitly with focused coverage. Native-buffer-sensitive malformed polylines and
zero-area uploads remain declared unsupported. Current live harness decode/chain
receipts preserve exact bytes, cross-node packet framing, mirrored RAM identity
and bit-23 termination. These are input-inspection checks, not executed-command
or rendered-effect evidence. Source/parser checks and command tests must remain
separate from final E5 capture acceptance.

Native proposal checkpoint: seven-file `video-draft.patch` under
`out/psx-coverage/proposals/gpu/`, SHA-256
`a30da9e17d1ae30aee993698704ea7b3f4d2fc9f2c31d0a5b467854890ba30b1`,
adds bounded GPU context/input/result records to History. Scope IDs, DMA owners,
cycles, raw CPU operands, actual read returns, parser readiness and environment
availability remain explicit. `/root/plan_review` found no source-review blocker
to requesting specific authorization after rechecking DMA count/FIFO bounds and
GUI-edit invalidation. Native arithmetic is unchanged; malformed native paths
may still abort or time out after capture is marked incomplete. All seven source
identities and combined read-only applicability after pending `sound.patch`
were checked. Eight translation units pass syntax checks, including both GPU
backends, GUI and save states. Maintained additive `events.lua` export and packet
checks pass; `out/psx-coverage/gpu-lua-recorder/` passes the installed recorder
regression. Mock export and old-ABI checks do not validate the new native hooks.
Specific source authorization, setup/build, live failure/ingress checks, maintained
capture orchestration and VRAM/display correlation remain required. No dependency
source was changed and E5 remains in progress.

Lua capture preparation: `gpu.lua action=capture` delegates to `video.lua` for
bounded target/frame/PC capture, deferred freeze and before/after raw state,
VRAM and display comparison. `ingress.lua` correlates exact input/result scope
IDs, request ownership, visited chain headers and DMA payloads with structural
packet decoding; split GP0 buffers and intervening GP1 words retain source
sequences/cycles. Missing native context, partial boundary packets, resets
interrupting packets and lost associations fail rather than inventing history.
`tests/ingress.lua` and `tests/video.lua` exercise synthetic records/lifecycle;
these are preparation, not native or rendered-effect acceptance. Source approval,
live capture/failure checks and independent integration review remain open.

Lua integration review: `/root/plan_review` accepted readiness after identifying
and rechecking complete-prefix/partial-tail ordering across an intervening GP1
command. Packets now sort by their final contributing buffer's result sequence,
retaining within-buffer order and exact source associations. Both standalone
suites pass independently. `out/psx-coverage/gpu-lua-binding/` records the expected
missing-native-GPU-binding failure without completion; `gpu-lua-decode/` passes
the offline decoder regression with exact input bytes. All 56 Lua files parse,
skill validation passes and documentation checks report no findings. These
checks cover Lua integration and explicit missing support, not native GPU
execution or rendered-effect acceptance. The subsequent approved installation
checkpoint under E4.01 records the combined rebuild; fresh native GPU captures
must validate ingress/effects and failures before E5 acceptance. Older origin
receipts remain bound to their original binary.

Native validation checkpoint: `/root/plan_review` verified the complete VRAM
effects and four-pixel DMA readback in `approved-gpu-dma`, exact transfer checks
in `approved-gpu-transfers`, and all ten lifecycle groups in
`approved-history-recorder` under `out/psx-coverage/`. In
`approved-gpuview-bounded`, both full VRAM images and both 320×240 screens match
independently constructed expected canvases: initial red; final green with red
fill, two uploaded pixels and a white rectangle. Environment commands and the
interleaved GP1 completion 48 precede split-fill completion 57, which retains
CPU buffer 7 and DMA buffer 9/request 1 ancestry. The initial `approved-gpuview`
hit its 16384-event capacity; the explicit 32768-event run completes. That failure
and `approved-gpu-limit` retain incomplete evidence without completion markers.
`approved-gpu-partial-{start,end}` reject incomplete parser boundaries with raw
native records retained. `approved-gpuports-checked` matches native GP0 read
returns `0x001f001f` and zero against guest-stored RAM. `approved-gpureset`
rejects reset-interrupted packets; `approved-cyclic` reports native bounds failure.
`approved-gpu-limit-{words,packets,stop}` reject decoder limits and frame exhaustion
before the requested stop. Every failed mission retains evidence without a
completion marker. Current packet checks pass, documentation reports zero broken
links/findings and receipt hashes validate. `/root/plan_review` accepted E5/E5.01
on this evidence. Exclusions remain explicit: CPU GP1 status reads, complete bus
tracing, packet replay, per-command rendering causality and hardware equivalence.

## 7. [E6] (open) Extend peripheral and media workflows
- Owner: skill implementation owner
- Depends: E2, E3, E4, E5
- Blocker: none
- Evidence: coverage assessment identified these lower-priority gaps; no maintained action claims support
- Acceptance: each action has explicit inputs/lifecycle/bounds, references and positive/failure evidence before publication

1. [E6.01] (open) Inspect SIO and run isolated memory-card experiments
- Owner: skill implementation owner
- Depends: none
- Blocker: none
- Evidence: generic SIO state queries exist; card workflows are unimplemented
- Acceptance: pinned card inputs, bounded serial transactions and resulting card exports preserve original input bytes

Add `sio.lua` and `references/SIO.md`. Inspect controller/card protocol state and
capture bounded transactions. Stage card copies for guest reads/writes; export
results to fresh outputs, leaving source cards intact. Exercise save/load,
rejected/truncated cards and interrupted transactions. Guest card contents and
emulator save-state serialization remain distinct formats.

2. [E6.02] (open) Inspect MDEC and capture decode transactions
- Owner: skill implementation owner
- Depends: none
- Blocker: none
- Evidence: serialized MDEC state exists; no maintained decode mission
- Acceptance: bounded command/input/output/DMA identities explain a known decode, with unsupported formats and malformed streams rejected

Add `mdec.lua` and `references/MDEC.md`: inspect status, quantization and buffers;
capture command/DMA input and raw decoded blocks with layout/pixel metadata.
Validate known data and malformed/truncated streams.

3. [E6.03] (open) Schedule analog controllers and explicit disc changes
- Owner: skill implementation owner
- Depends: none
- Blocker: none
- Evidence: digital overrides and initial CUE/BINARY mounting only
- Acceptance: supported analog ranges/modes and eject/insert lifecycle are explicit and reproducible within measured limits

Audit analog APIs before extending frame schedules/references; do not substitute
digital buttons for analog trajectories. Extend `runtime/disc.py` and CD actions
for a bounded explicit set of staged discs, timed eject/insert, media identities
and controller transitions. Reject unsupported modes/tracks/sidecars and state/
media conflicts. Verify authorization and source identities before new bindings.

## 8. [E7] (open) Validate and publish the complete action surface
- Owner: parent and independent reviewer
- Depends: A8, E1, E2, E3, E4, E5, E6
- Blocker: none
- Evidence: historical baseline has 48 passing runtime/layout checks with one index-writing case excluded; new gates remain open
- Acceptance: inherited and new actions pass bounded live/failure validation and independent review; references/discovery/receipts match capabilities

1. [E7.01] (open) Refresh references and exercise the agent workflow
- Owner: parent and independent reviewer
- Depends: none
- Blocker: none
- Evidence: per-action references and discovery links exist; full agent mission validation is still an inherited open requirement
- Acceptance: concise skill routing leads to executable harness missions and correctly bounded evidence handoff

Update `SKILL.md`, the agent spec and owning reference with every capability
change, compacting directly without weakening contracts. Preserve one skill tree
and both discovery links. Exercise representative scoped agent missions with
reproducible inputs and retained receipts. No harness model launcher, duplicate
parser, hidden input or game-specific default. Independent review checks both
obligation preservation and observed behavior, not process success alone.

## Validation and acceptance procedure

Before each implementation phase, refresh pinned API/source evidence, select a
bounded synthetic positive case and a relevant malformed/unsupported/timeout
case, retain receipts/captures, and run existing focused checks. Missing APIs or
reference inputs remain unresolved work. Preserve historical failures/exclusions.
Native binding review includes capture boundaries and threading/ordering, not
just Lua output shape. Documentation-only work does not require audio-domain runs.

```sh
bin/harness runtime status
bin/harness plans status psx-emulator-capabilities.md
# link and anchor hygiene is manual now: the harness Markdown transport was retired
env PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=tools/python .venv/bin/python -m pytest -q -p no:cacheprovider tools/python/tests/commands/test_runtime.py tools/python/tests/commands/test_harness_dry.py -k 'not test_required_untracked_file_modes_are_normalized'
bin/harness source docs --json
git diff --check
```

The excluded case writes Git objects/index; exclusion protects unrelated dirty
work and is not a passing result. Run applicable existing skill/agent checks
through both discovery paths. Native/API changes must pass owning build/checks
before live experiments. Final acceptance requires every inherited obligation,
E1–E7, attributable independent review and explicit limitations. Recording or
parsing this plan does not complete an implementation phase.
