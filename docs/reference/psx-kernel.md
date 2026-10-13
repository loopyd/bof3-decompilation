# PSX CPU and BIOS execution references

These are external hardware/BIOS
references, not proof of BOF3 execution or timing. Original US observations and
implementation limits live in the [audio specification](../specs/formats/audio.md#rust-system-control-and-critical-sections).

| Source | Retained use | Limits |
| --- | --- | --- |
| [psx-spx CPU specification](https://psx-spx.consoledev.net/cpuspecifications/#cop0-exception-handling) | COP0 status stack, exception codes/vectors, EPC, branch-delay information, BadVaddr and RFE | No complete CPU or hardware-timing acceptance from implementing these rules. |
| [psx-spx BIOS specification](https://psx-spx.consoledev.net/kernelbios/#bios-interruptexception-handling) | Critical-section syscall semantics; BIOS exception chains and CD removal limitations | A service name does not establish its effects or permit a success stub. |
| [DuckStation CPU register definitions](https://github.com/stenzek/duckstation/blob/master/src/core/cpu_types.h) | Independent comparison of status and cause writable-bit masks | Implementation evidence; no DuckStation code is linked or used as a production fallback. |
| [OpenBIOS CD driver](https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/openbios/cdrom/cdrom.c) | `_96_remove` comparison: critical section, five event closures, handler removal | Replacement BIOS behavior is not original Sony BIOS equivalence. |

RFE changes the status stack; it does not select a return PC. Critical-section
entry tests and clears saved interrupt-enable and interrupt-mask bits; exit
sets them without returning a value. CD-handler removal also depends on event
and chain state. The BIOS reference describes broken non-head chain removal;
the Rust machine must not silently replace that with a corrected list operation.

The [BIOS event reference](https://psx-spx.consoledev.net/kernelbios/#bios-event-functions)
and [control-block layout](https://psx-spx.consoledev.net/kernelbios/#bios-control-blocks)
describe the guest RAM descriptor and `0x1C`-byte EvCB records, allocation,
polling/consumption and callback modes. [OpenBIOS event code](https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/openbios/kernel/events.c)
provides a separate implementation comparison. Invalid handles can reach beyond
the table in BIOS implementations; the Rust model rejects that unresolved case.
Its pending-wait state is functional scheduling support, not a reproduction of
the documented timing-dependent WaitEvent race.

The event references describe ordered table scanning with immediate callback
execution. The Rust continuation captures the table range but reads later
records after each handler returns, so handler changes affect subsequent matches.
Its reserved return trap and separate O32 argument home area are HLE mechanisms;
neither establishes the original BIOS stack frame, volatile registers or timing.
See [callback implementation boundaries](../specs/formats/audio.md#rust-bios-event-callbacks).

The Rust tests use synthetic instruction streams and optional original US
wrappers. [Event-state coverage](../specs/formats/audio.md#rust-bios-event-state)
does not establish original BIOS allocation or initial contents. Whole-game
initialization, complete event/callback lifecycles, handler dispatch, cycle timing
and independent audio comparison remain separate acceptance gates.

## Exception-chain registration

The [published priority-chain layout and removal warning](https://psx-spx.consoledev.net/kernelbios/#priority-chains)
define four priorities and node links with two handler pointers. A handler's
second function depends on a nonzero first-function result. Non-head removal is
documented as reading an uninitialized stack location; a correct general list
removal would change that behavior.

[OpenBIOS registration code](https://raw.githubusercontent.com/grumpycoders/pcsx-redux/main/src/mips/openbios/handlers/setup.c),
inspected 2026-09-24, provides a separate comparison: enqueue returns zero;
successful head removal returns the removed pointer and leaves its link intact.
Its non-head search is repaired, so it is not evidence for original bug behavior.
The [Rust implementation](../specs/formats/audio.md#rust-bios-exception-chain-registration)
supports bounded registration/head removal and explicitly rejects unresolved
contexts. No BIOS code, payload or dependency is imported. The original game
wrappers are tested against a synthetic RAM table, not a claimed BIOS boot image.

## Thread exception contexts

Inspected 2026-09-24: the [BIOS control-block layout](https://psx-spx.consoledev.net/kernelbios/#bios-control-blocks)
and [exception-return contract](https://psx-spx.consoledev.net/kernelbios/#bios-interruptexception-handling)
describe the PCB's current-thread pointer and TCB register image. Exception entry
records GPRs other than zero/K0, return PC, HI/LO, status and diagnostic cause;
return restores the current thread's saved state without reloading cause.
[OpenBIOS vector assembly](https://raw.githubusercontent.com/grumpycoders/pcsx-redux/main/src/mips/openbios/kernel/vectors.s)
provides a separate comparison for register offsets, K0 as return target and RFE
status restoration. It also exposes a COP2 interrupt-PC adjustment, which the
Rust capture path rejects pending GTE evidence. Replacement-BIOS assembly is not
an original Sony BIOS image or independent timing evidence; no source was copied
or linked. [Implementation limits](../specs/formats/audio.md#rust-bios-thread-exception-contexts)
retain boot allocation, driver state and general nested exception handling as
separate obligations.

## Priority-chain execution

Inspected 2026-09-24: the [priority-chain reference](https://psx-spx.consoledev.net/kernelbios/#priority-chains)
describes priorities zero through three, verifier-dependent second handlers and
early `ReturnFromException` exits. [OpenBIOS vector assembly](https://raw.githubusercontent.com/grumpycoders/pcsx-redux/main/src/mips/openbios/kernel/vectors.s)
provides implementation comparison: capture both handler pointers before calling
the verifier, pass its nonzero result in A0 to the second handler, then read the
next link. Later priority heads are read after earlier handlers run. Its final
hook restores a setjmp buffer and resumes with V0 equal to one. These observations
do not establish Sony BIOS volatile registers, stack frames or cycle timing.

The [Rust continuation](../specs/formats/audio.md#rust-bios-priority-chain-execution)
uses independently written host control flow and executes handlers as guest code.
Explicit stack/GP and missing-hook policy avoid inventing boot state. Original US
handler checks use a synthetic table and device inputs; they establish neither
BIOS initialization nor independently verified audio.

## BIOS ROM and CD-removal evidence

Inspected 2026-09-24: the [memory map](https://psx-spx.consoledev.net/memorymap/#memory-map)
places the normal 512 KiB BIOS at physical `0x1FC00000`, with cached and uncached
aliases `0x9FC00000` and `0xBFC00000`. The Rust mapping implements these windows;
larger ROMs, bus timing and store behavior remain unsupported. A raw RAM image
does not supply CPU pipeline, device or clock state.

The [BIOS CD functions](https://psx-spx.consoledev.net/kernelbios/#bios-cdrom-functions)
identify `_96_remove` as A0 services `56`/`72` and document broken handler removal.
[OpenBIOS CD driver code](https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/openbios/cdrom/cdrom.c)
provides a separate comparison: enter a critical section, close ACK/DNE/RDY/END/ERR
events, then dequeue CD handlers. It does not establish Sony BIOS globals, stack
contents or bug effects. Implementing a success-only return would omit these
effects. The [ROM probe and original-game caller evidence](../specs/formats/audio.md#rust-bios-rom-execution-probe)
retain this unresolved boundary. No BIOS payload or upstream implementation is
distributed or linked.

## Original SDK CD interrupt hook fixture

The [interrupt controller reference](https://psx-spx.consoledev.net/interrupts/)
separates the edge-latched I_STAT source, I_MASK, COP0 pending bit and CPU enable
bits; acknowledging a controller bit does not replace device acknowledgement.
The [original US hook integration](../specs/formats/audio.md#original-sdk-cd-interrupt-hook-integration)
exercises that route through the SDK dispatcher and BIOS context restore.
Its hook is installed by original executable instructions, while the empty
priority chain and live thread allocation are supplied fixture context.
The fixture does not establish complete game initialization. The
[US BIOS setup](audio-runtime-setup.md) supplies the required ROM; physical
timing and independent audio remain open.

## Reference reset and original BIOS memory setup

The pinned PCSX-Redux revision
`28438546c781fbe372a06399c82bed43ca2c6f4d` initializes CPU PC/SR to
`0xBFC00000`/`0x10900000` in `src/core/r3000a.cc:56–83`. SR includes bit 23,
which the [CPU specification](https://psx-spx.consoledev.net/cpuspecifications/#cop0r12-sr-system-status-register-rw)
marks unused. Its adjacent source comment does not correctly describe the
constant's BEV/TS bits. Rust exposes this literal seed as `pcsx_redux_reset`,
retaining normal masked guest SR writes and labeling the preset as emulator
reference state. It is not independently verified hardware reset state.

The [memory-controller reference](https://psx-spx.consoledev.net/memorycontrol/)
describes the BIOS's initial `DEV2=0x0013243F`, `DRAM_CTRL=0x0B88` writes,
standard expansion base/delay registers and `COM_DELAY` readback. Rust now retains
these supported configurations and maps the configured 8 MiB RAM aperture onto
the 2 MiB physical image for ordinary and masked accesses. Other mappings and
partial configuration accesses fail explicitly. Expansion device reads remain
unsupported. Bus delays are recorded; CPU/device clocks are not modeled by these
register latches. Common-delay upper bits read zero, including after the original
BIOS writes `0x31125`; the initial CD bus setting `0x20843` is retained separately
from the SDK's later `0x20943` setting.

## BIOS cache clearing and debug initialization

The functional cache now separates instruction fetches from data accesses. The
[published cache observations](https://psx-spx.consoledev.net/memorymap/#i-cache)
describe physical tags, per-word validity, forward refill and stale instructions
after ordinary RAM writes. Rust implements these effects, including two-word
refill configuration and full refill on an invalid word with a matching tag.
KUSEG/KSEG0 aliases share entries; KSEG1 bypasses them. This is functional state,
not measured cache-miss timing. PCSX-Redux supplies the reset/BIU sequence reference;
its bulk invalidation/refill shortcuts are not hardware timing evidence.

The [BIU/isolation description](https://psx-spx.consoledev.net/memorycontrol/#fffe0130h-bcc-biu-cache-configuration-register-rw)
guides separate tag and instruction-word stores during BIOS clearing. Isolated
cached stores leave physical RAM unchanged; uncached accesses retain normal RAM
behavior. Scratchpad access follows the supported BIU configurations and
scratchpad instruction fetches fail. CPU isolation is scoped to CPU transactions,
so host/DMA access is not accidentally redirected to cache SRAM. Partial isolated
accesses, masked cache writes and unsupported BIU modes fail explicitly.

A hash-gated ROM test seeds dirty cache and nonzero RAM, executes the BIOS through
1,528 instructions, and verifies cleared tags/code words, unchanged main RAM,
SR zero and BIU `0x1E988`. COP0 also retains disabled-breakpoint address/mask
registers and accepts DCIC zero. Active breakpoint control stays unsupported.
The BIOS's zero TAR write is accepted only while TAR is already zero: both the
documented read-only register and Redux's writable storage yield that same state;
other TAR writes remain unresolved and rejected.

Cache behavior checks do not establish complete BIOS boot, device clocks, or
sound fidelity.

## BIOS POST and absent expansion ROM

The Rust bus accepts byte writes to retail POST `0x1F802041` through its three
physical aliases and exposes the last byte to the host boot probe. Reads,
other widths, masked stores and adjacent dev-board ports remain unsupported.
The diagnostic sink has no interrupt effect. This follows the register purpose
in [psx-spx expansion ports](https://psx-spx.consoledev.net/expansionportpio/#exp2-post-registers)
and the pinned PCSX-Redux byte-write handler in `src/core/psxhw.cc`.

After EXP1 base/delay configuration, reads within its 512 KiB window return all
ones for an absent cartridge. This adopts the emulator's empty-ROM convention
(`src/core/psxmem.cc`); it does not establish electrical open-bus behavior.
Other windows, unaligned reads and expansion writes still fail. The BIOS itself
checks the missing cartridge signature and chooses its normal continuation.

The hash-gated original-ROM test exercises these behaviors through the guest
exception handler without HLE interception. Two synthetic checks cover POST
width/alias/error behavior and absent EXP1 mapping, alignment and RAM
preservation. These checks do not establish full boot, clock scheduling,
independent trace agreement, or audio fidelity.

## BIOS SPU initialization and transfer clock

The boot probe now accepts optional `spu` choices for sample/envelope arithmetic,
disable behavior and reverb, plus an optional `transfer_clock`. It records these
choices and the final SPU RAM hash, FIFO occupancy and transfer address. Existing
contexts without models retain their prior behavior. Unknown choices, missing
required arithmetic selections and extra SPU fields are rejected. The transfer
clock requires the explicit PCSX-Redux CPU reset profile. The bounded probe cap
is now 100 million transitions; a numeric trace ring avoids allocating JSON on
every instruction. Neither the larger cap nor model selection declares a BIOS
revision supported for production rendering.

Key-on/off MMIO now preserves each written halfword for readback separately from
pending frame events. Reads retain unused upper bits and survive event consumption;
they do not report voice activity or retrigger previously consumed keys. This
matches [psx-spx's voice-flag readback notes](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-voice-flags)
and the pinned PCSX-Redux `src/spu/registers.cc` register array. The BIOS reaches
`0x800543B4` and reads key-on state after configuring voices; the previous
write-only model rejected that valid access.

`machine::spu_clock` advances FIFO/RAM transfers at an explicitly selected
16 system ticks per halfword, with retained partial progress and no accumulated
idle-time credit. Its source is the `TRANSFER_TICKS_PER_HALFWORD` behavior in
[DuckStation's SPU implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/spu.cpp),
inspected 2026-09-24 on a moving branch, not an immutable hardware oracle.
Control applies at an advance call; DMA FIFO handshakes still use the existing
untimed interconnect. The probe supplies two base ticks per executed instruction
or entered syscall, following pinned Redux interpreter `BIAS=2`; memory stalls,
voice frames, timer/CD clocks and DMA arbitration are absent. Redux itself writes
manual SPU data directly; this FIFO model does not claim identical Redux timing.
The clock establishes bounded, reproducible device progress, not final PSX timing
or PCM fidelity. No reference implementation code is imported or executed.

The hash-gated ROM fixture exercises initialization under this clock model;
it is regression evidence, not an independent trace. Clock
checks exercise split-step equivalence, exact transfer boundaries, idle/stopped
behavior, invalid control and DMA-read completion. Key-latch checks distinguish
readback from consumed events. Full boot and independent audio acceptance
remain separate gates.

## BIOS shell handoff and original sound initialization

Pinned PCSX-Redux has a PS-X EXE loading path separate from its FastBoot option.
`src/core/r3000a.h::executionFlowTrace` signals shell entry at `0x80030000`;
`src/core/ui.cc::shellReached` can load a requested executable there. Its
`src/supportpsx/binloader.cc::loadPSEXE` copies only the declared payload and sets
PC plus nonzero stack base. It does not assign the EXE GP field, zero BSS or
reset devices. The separate FastBoot branch returns to RA and enables the display;
that branch is not required for EXE loading. `MemoryAsFile::writeBlock` changes
backing memory without applying guest cache isolation or clearing unrelated RAM.

`machine::boot::load_us` now reproduces this EXE-load handoff for the exact US
BIOS/executable hashes. The original ROM runs to the shell entry, then its RAM
and device state are retained while the EXE payload is loaded. Unsupported hashes
and exhausted/invalid transition limits fail. The BIOS itself has cleared its
cache before this handoff. No boot-animation code or HLE BIOS service runs on
this path. This is a reference-emulator executable-launch contract, not proof
that full disc boot, video, CD or sound scheduling has completed.

The handoff test compares all RAM outside the payload and all inherited GPRs against a
separately executed pre-handoff ROM context, checks the BIOS-created PCB/TCB and
executes the first original EXE instruction.

`examples/runtime_probe.rs` accepts `BIOS EXE entry|callback-init|sound-init`.
`entry` executes the original EXE startup. The other modes are explicitly marked
diagnostic calls: they retain handoff GPR/COP0 state, select the original routine,
set zero arguments/HI/LO and a return sentinel. They do not claim that the game
has run its full startup. All modes select the existing emulator-reference SPU
models and transfer clock; BIOS services execute through guest vectors and ROM,
without the host `Kernel` dispatcher or synthetic kernel tables.

ENDX writes now have transient readback separate from actual voice loop-end
latches, consistent with [psx-spx's read/write notes](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-voice-flags).
The next scheduled frame refreshes readback from voice state. That boundary is
an explicit functional approximation: exact sub-frame overwrite timing still
requires independent evidence. Tests prove writes do not fabricate or erase
voice end flags. Redux's register storage also accepts these writes; its backend
readback behavior is not used as a hardware timing oracle.

BIOS shell loading does not establish complete disc boot or hardware timing.
C retirement remains gated on complete device scheduling and independent
runtime/PCM comparisons.

## Original VAB transfer and SEP opening through the BIOS

`machine::execution::Execution` runs bounded guest calls while retaining CPU,
BIOS kernel and device state. Syscalls enter the guest exception vector; pending
interrupts use the CPU exception path. Each instruction advances root counters
and the explicit emulator-reference SPU transfer clock by two base ticks.
Memory stalls, audio frames, beam/CD clocks and DMA arbitration remain unmodeled;
this is functional bank-loading evidence, not playback timing evidence.

Fresh Rizin disassembly of the US executable payload (SHA-256
`677754d0d22c88151a5022cd98b8e89af1b0882177d9850faf62676eb7089eff`)
connects the game loader to these routines:

- The type-7 dispatcher calls VAB head open `0x80173C50` at `0x8016292C`.
  Layout-table slots at `0x8014677C` have stride 20 and supply the SPU base,
  header pointer and VAB ID.
- The body callback calls partial transfer `0x80174354` at `0x801634D8`,
  using a 2048-byte CD staging buffer and the selected bank ID. Completion
  waits call `0x80174598`.
- The SEP loader calls `0x8016B38C` at `0x801635D0` with four sequence slots.
- DMA setup at `0x80168B6C` shifts the byte count by six, rounds upward when
  low bits remain, and programs 16-word blocks. The final DMA therefore reads
  archive padding beyond the declared VB data when its length is not a
  multiple of 64 bytes. Padding cannot be synthesized as zero.

`examples/bank_probe.rs BIOS US_EXE BGM_EMI` boots the verified US ROM, loads
the executable at the shell handoff, runs original sound/layout initialization,
stages the archive buffers, and executes the original head/body/SEP routines.
The real BIOS handles DMA interrupts; no host BIOS service replaces it. The
probe checks SPU RAM against the immutable archive including final DMA padding,
rejects failed completion waits and invalid SEP handles, and records bounded
call outcomes. Host staging does not establish original CD loader execution.

| Archive | Declared VB bytes | DMA bytes | Guest IRQs | Outcome |
| --- | ---: | ---: | ---: | --- |
| BGM000.EMI | 233504 | 233536 | 115 | Complete |
| BGM021.EMI | 244768 | 244800 | 120 | Complete |
| BGM068.EMI | 239168 | 239168 | 117 | Complete |

BGM000's final 32 bytes
are `0x5F`; the media regression also verifies unchanged SPU RAM outside the
transfer, intermediate/final transfer results, completion waits, guest IRQ
execution and SEP handle zero. Default synthetic checks cover preserved call
state, invalid entry/fuel requests and guest IRQ acknowledgement/RFE.

Bank-transfer checks do not establish independent PCM fidelity or permit
retirement of the host C tool.

## Original sequence scheduler through VBlank interrupts

Bounded disassembly and execution of the same verified US executable establish
this path. Addresses below are in `exe/slus_004_22`; offsets include the PS-X EXE
header.

| Runtime address | Full EXE offset | Observed role |
| --- | --- | --- |
| `0x8015CE70` | `0xC6E70` | Game start wrapper: starts tick callbacks and sets initial sound volumes/routing. |
| `0x8016C498` | `0xD6498` | Passes zero to tick registration at `0x8016C210`; called by the game at `0x8015CE78`. |
| `0x8016C210` | `0xD6210` | Selects callback registration from tick mode and start argument. |
| `0x8016C548` | `0xD6548` | Global scheduler, initially stored at `0x80184448`. |
| `0x8016CDE0` | `0xD6DE0` | Sign-extends handle/sequence arguments and calls per-sequence scheduling at `0x8016CE0C`. |
| `0x801706A4` | `0xDA6A4` | Voice-state/register service called before sequence dispatch. |
| `0x8016B9CC` | `0xD59CC` | Sequence play entry used by the game wrapper; accepts handle, sequence, mode and repeat count. |
| `0x8016D534` | `0xD7534` | Original sequence stop/reset routine. |

Game sound initialization passes tick mode 1; the NTSC video-mode path stores
mode 5 at `0x80184440` and nominal rate 60 at `0x80190300`. With start argument
zero, mode 5 sets byte `0x80184450` to one and installs the scheduler through
the SDK VBlank callback entry at `0x80174974`. The alternative start wrapper
`0x8016C478` passes one and selects a different registration path; it has no
direct game call in the inspected executable. The diagnostic uses the game's
`0x8015CE70` wrapper, including its additional sound setup.

The global scheduler guards reentry with `0x8018E248`, services voice state,
then scans allocated sequence handles using `0x8018E7D0`, the handle count at
`0x80190B88`, and per-handle sequence count at `0x80190B8A`. Records are 172
bytes. Flags at record `+0x90` select play, stop and other transitions; active
play reaches the existing verified per-sequence scheduler. The guest code,
not the host, advances event cursors and writes SPU key-on registers.

`bank_probe BIOS US_EXE BGM_EMI SEQUENCE TICKS` now optionally starts one parsed
SEP sequence and pulses VBlank for 1–10000 bounded iterations. Before each
bounded `GetVideoMode` leaf call (`0x801753DC`), it asserts VBlank. Execution
delivers the pending interrupt to the real BIOS, which reaches the SDK callback
and original scheduler before returning to the leaf. Each iteration requires
one IRQ and records instructions, sequence cursor/delay/flags and key-on
readback. Missing sequence indices are rejected before boot. Ticks are injected
without video timing or SPU output frames; register activity is not PCM evidence.

| Archive / sequence index | IRQ iterations | Iterations with key-on readback |
| --- | ---: | ---: |
| BGM000 / 0 | 120 | 10 |
| BGM000 / 1 | 120 | 17 |
| BGM021 / 0 | 120 | 21 |
| BGM068 / 0 | 120 | 9 |

The BGM000 media regression now verifies game callback registration, IRQ
acknowledgement, cursor advancement, key-on writes and byte-identical inactive
records. It stops sequence zero through original code, settles its stop flag,
then plays sequence one and verifies that sequence zero and all other inactive
records remain unchanged. No event cursor, guest routine or BIOS service is
patched. Independent runtime/PCM comparison remains required. The
reference-emulator constants must remain explicit: pinned PCSX-Redux uses
33,868,800 system ticks per second, integer 60 Hz / 263-line NTSC counter setup,
and VBlank start line 243 in `src/core/psxcounters.{cc,h}`. Those constants
alone do not prove hardware cadence, CPU stall timing or audio fidelity.


## Clocked SPU output with the original sequence runtime

`machine::output_clock` adds an explicit `PcsxReduxNtsc` model to
`machine::execution::Execution`. Activation chooses a new scanline/sample phase
at the caller's current guest boundary. It does not reconstruct the video phase
of the preceding untimed BIOS bootstrap.

SPU output occurs every 768 system ticks. The
[psx-spx logic-analyzer account](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-internal-state-machine-and-spu-ram-timing)
reports that period for the 44.1 kHz SPU work cycle. The implementation retains
the existing per-frame voice, interpolation, ADSR, mixing, reverb and capture
models; it does not claim cycle-level SPU RAM arbitration. This source was
consulted on 2026-09-24; its hardware observations and remaining uncertainty
must not be conflated with the emulator models used here.

The NTSC line clock follows PCSX-Redux commit
`28438546c781fbe372a06399c82bed43ca2c6f4d`,
`src/core/psxcounters.cc::init/calculateHsync` and the constants in
`src/core/psxcounters.h`: 33,868,800 / (60 × 263), truncated to 2146 system ticks
per line, with VBlank starting at line 243 and ending on line wrap. Each line
also supplies a pulse for timer 1's HBlank clock. Root counters, the existing
SPU FIFO/DMA transfer clock, scanline events and output frames advance on the
same clock. Guest instructions retain the explicit two-tick base cost; IRQs
enter the guest BIOS at the next instruction boundary.

The BIOS leaves timer 1 mode `0x0507`, using HBlank pulses and synchronization
with VBlank. The implementation services this clock source. Timer 0 HBlank gating and
dot-clock modes are rejected before advancement because HBlank duration and
GPU dot clocks are unimplemented. CPU stalls, GPU command/display timing, CD
sector/input scheduling and hardware DMA arbitration remain separate gates.
Coincident line events precede SPU output in this model; hardware sub-frame
ordering and the chosen initial phase remain unverified.

Output buffering is bounded to 4096 stereo frames; callers drain it through
`take_audio_frames`. Overflow and unsupported device execution fail explicitly.
Errors can leave elapsed device work committed and are terminal for that run.
Default execution without an output clock retains the earlier call behavior.

The diagnostic now accepts:

```text
bank_probe BIOS US_EXE BGM_EMI SEQUENCE TICKS WAV
```

With a WAV path, `TICKS` counts clocked VBlank events rather than injected
interrupts. The host repeatedly calls the original `GetVideoMode` leaf while
instruction time drives devices; it does not call the sequence scheduler or
write event cursors. Real BIOS IRQ handling reaches the original game callback
and produces SPU writes. The WAV is written to a temporary sibling, synced,
parsed back and atomically linked into place without overwriting an existing
file. The mode without a WAV retains the earlier injected-IRQ diagnostic.
These are bounded diagnostics. Use the renderer below for duration, loop-count,
and release-tail controls. Diagnostic frame totals include guest IRQ execution
through return after the last VBlank, so they vary with sequence workload and
do not represent exact requested durations.

Default tests prove partition-invariant output, root-counter/transfer clock
advancement, exact model VBlank boundaries, interrupt acknowledgement,
unsupported timer modes and bounded-output failure. The expanded BGM000 media
test checks nonzero clocked PCM, frame accounting, and repeated-enable rejection.

Independent execution traces, reference audio, and broader song/SFX/XA coverage
remain required. The
approximate C renderer is not an audio oracle and C retirement stays blocked
on the complete migration acceptance gates.


## Duration-limited PSX music render command

`machine::music::prepare` now owns validated single-VAB/SEP preparation for the
verified US executable and BIOS. It runs original initialization, header/body
transfer, completion waits, SEP open and the game callback-start wrapper.
The source archive stays immutable. Preparation rejects ambiguous entry sets,
missing sequence indices, VH/VB size disagreement, capacity overflow, failed
SDK returns and differing SPU transfer bytes, including final DMA padding.

The selected layout's next higher SPU bank base is an upload boundary; the
initialized guest reverb work area is also protected. This is a conservative
single-bank loading contract, not proof that all game contexts reserve those
banks simultaneously. Fresh disassembly confirms direct base loads from
`0x80182348`; the sole direct executable call at `0x8014EAAC` passes layout zero
in its delay slot. Cue-specific alternate-layout selection in overlays remains
unverified.

`psx_render` and `render --mode music --engine psx` expose this runtime with
`--archive`, `--executable`, `--bios`, explicit `--sequence`, and optional
`--duration` or `--loops` (mutually exclusive). The renderer tries game layouts 0, 1, then 2 and selects the first
whose VH/SEP capacities and SPU interval fit the archive's bank slot and aligned
VB DMA size. No cue IDs or archive filenames participate. `--layout 0|1|2`
overrides automatic selection and retains capacity validation. The identity
report records both `requested_layout` (null for automatic) and resolved `layout`.
This host fit policy does not infer the original game's scene-specific selection.
Output is fixed at 44100 Hz. `--tail` defaults to two seconds and
`--timeout` is an audio-duration safety limit, positive and at most 600 seconds.
Every guest call also has an instruction budget, so a stuck callback fails.
PC-only MIDI/SoundFont/repeat options, invalid selections and unsupported modes
are rejected instead of ignored.

The renderer requests one original SEP play and retains encoded controller
loops. The explicit duration is the body cutoff; then the original stop/reset
routine runs at a safe guest-call boundary while clocks continue. The tail
includes that possible boundary delay and the game's deferred key-off/voice
flush. PCM is trimmed to exactly the requested body plus tail. Reports retain
executed frames, stop-call start/return frames and clock state so overshoot is
visible. `--repeats` is not reinterpreted as game-loop traversal counting.
Loop-count stopping and default two-traversal behavior are implemented below.

`render.wav` and versioned `bof3.psx-render/v1` `render.json` are verified in a
staging directory before publication. The report qualifies each component by
archive SHA-256 and entry, retains VH header ID separately from game bank ID,
and retains encoded sequence ID separately from sequence index. BIOS/executable
hashes, initialization/play/stop call evidence, explicit model limitations and
`independent_pcm_validated: false` accompany the output. Unsupported execution
fails without publishing a successful rendering directory.

Production command availability does not close independent fidelity, complete
runtime coverage, codec, or C-retirement gates. The
[audio README](../../tools/rust/bof3-audio/README.md) provides runnable commands.

## Original-runtime loop-count stopping

`psx_render` defaults to two traversals of an infinite controller loop (`--loops
N` overrides this); an end marker stops a non-looping or finite-loop sequence
once. `--duration` retains the exact fixed body cutoff. Finite encoded repeat
counts are unchanged. The renderer requests one original SEP play, so these
controls do not restart the whole song. Tail frames count toward the audio
safety limit; failure to reach a loop/end boundary before the reserved tail
starts rejects publication. A zero-delay callback that never returns fails its
instruction limit even if a boundary has been observed.

`Execution::step_observed` invokes a read-only observer after IRQ admission and
before the selected instruction. This avoids counting a PC that was interrupted
before execution. `music_progress` observes these verified US instructions:

| Purpose | Runtime address | Full US EXE offset | Selection evidence |
| --- | --- | --- | --- |
| Save controller-loop start cursor | `0x8016A308` | `0xD4308` | `s0` equals selected record; delay-slot store of `v1` to `+0x0C` |
| Infinite-loop jump | `0x8016A374` | `0xD4374` | `s0` equals record, count `+0x28` is 127, `v0` equals saved cursor; delay-slot store to `+4` |
| End marker handler entry | `0x8016CF1C` | `0xD6F1C` | Low halfwords of `a0/a1` select handle and sequence |

Finite-loop jumps at `0x8016A354` are not infinite-loop traversals. No event,
loop counter or original instruction is rewritten. The first selected boundary
latches frame/instruction/PC/cursor evidence; stopping runs at the next safe guest
call return. Tail timing includes that delay and original deferred voice flush.
Reports add `requested_body` and `progress`, including loop/end counters and the
latched reason. Independent PCM fidelity remains unverified.

Preparation passes the parsed sequence count (at most four)
to original `0x8016B38C` (EXE offset `0xD538C`). Its `a2` count controls the loop
calling `0x8016B4B8`; it is not a capacity argument. Original four-sequence assets
retain four, and a valid single-sequence archive no longer parses zero padding
as additional headers or divides by a zero tempo.

Sequence and renderer tests cover finite/infinite/end observers, unselected-handle
isolation, admitted IRQs and delay slots, loop limits, exact body/tail frames,
reduced SEP sets, layout selection, timeouts, and nonreturning callbacks.
These are original-runtime execution and host-policy checks; independent
reference-audio agreement remains unverified.
