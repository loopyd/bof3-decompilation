# CD/XA audio processing references

These sources describe disc/drive processing; game commands,
seek state and timing require separate original-runtime evidence.

- [psx-spx XA interpolation](https://psx-spx.consoledev.net/cdromformat/#cdrom-xa-audio-adpcm-compression):
  seven 29-coefficient phases and a six-input/seven-output schedule for 37.8 kHz.
  The description explicitly does not establish exact hardware rounding or a
  complete 18.9 kHz algorithm.
- PCSX-Redux Agentic's [CD controller](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc),
  [XA decoder](../../third_party/pcsx-redux-agentic/src/core/decode_xa.cc) and
  [SPU CD feed](../../third_party/pcsx-redux-agentic/src/spu/xa.cc) provide the
  current native comparison for decoding, resampling, gain and drive state.
  Native implementation facts require separate runtime and hardware evidence;
  no emulator implementation is imported into the Rust component.
- [psx-spx drive registers](https://psx-spx.consoledev.net/cdromdrive/): ATV0–3
  staging, ADPCTL apply/mute bits and incomplete saturation above double gain.

The retained published 37.8 kHz table contains 203 coefficients. In phase-major
little-endian signed-16 encoding their SHA-256 is
`0ea87420ec5d82b35aa651a26622572b422f3d330afbf78f166941db7035ca80`.
Both Rust 37.8 kHz models index history from `p-1` and floor each product before
summation. Native `zigzag` reads `ring[(xaRingPos-i)&31]` for `i=1..29`, shifts
each product by 15 and saturates the sum. Bounded comparisons of original VOICE
mono and S_XA00 stereo 4-bit sectors confirm `CombinedRounded` decoding, mono
expansion and this alignment with the native gain stages.
They cover consumed output under the captured settings, including distinct stereo
channels. Remaining queued output, other formats and histories, and hardware
fidelity require separate evidence.

Rust's emulator-reference 18.9 kHz path follows native `FeedXA`: insert the signed
arithmetic midpoint of the previous and current sample, then the current sample,
through the shared 37.8 kHz zigzag filter. It retains previous samples across
chunks, sectors and coding changes. Mono fills both rings. Bounded synthetic
stereo intervals agree with native decoded PCM, generated frame counts and
consumed output at unity gain for 4-bit 18.9 kHz input and 8-bit input at both
encoded rates. The 8-bit intervals consume all generated frames.
The fresh-process construction context does not directly observe native
interpolation rings at capture start.
Other formats and histories, clipping, queued output from other intervals and
hardware fidelity need separate evidence.

Rust `Volume::apply_xa` follows native XA gain: sum each output's matrix
products in signed 64-bit arithmetic, multiply by 33617, shift by 22, saturate to
signed 16-bit, then multiply by 31880 and shift by 15. The queue applies this
gain at sector admission. A bounded original stereo capture confirms unity and
an asymmetric crossed matrix, including old queued output consumed after a
latch. Clipping cases and other matrices need separate native evidence.

Native CD-DA and Rust `Volume::apply_cdda` use 31130 before saturation and
31880 afterward. The native feed also applies the configured host volume divisor. That host setting and the capture stage remain explicit
comparison inputs, separate from drive gain.

[Rust implementation and checks](../specs/formats/audio.md#rust-cdxa-sample-processing)
retain startup phase, history, gain limits and timing boundaries. Emphasis, drive
mechanical/media scheduling, mute effects on decoder state and hardware comparison
remain open; encoded-rate extraction keeps its existing separate contract.

## CD decoder host interface

[psx-spx host-register descriptions](https://psx-spx.consoledev.net/cdromdrive/#cdrom-controller-io-ports)
cover bank selection, parameter/result FIFOs, response readiness, interrupt
mask/acknowledgement and volume ports. Interrupt numbers occupy a bitmasked
field; they are not independent one-hot flags. Response availability and IRQ
delivery can occur separately. The Rust host retains numeric mask/partial-clear
behavior; the native comparison below covers full acknowledgements. Decoder
reset remains unsupported.

The [native queue owner](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
retains two response elements. A third candidate is dropped when both are full.
FIFO service promotes an acknowledged head's tail, even over unread head bytes,
and accepts a command only when no response cause or tail remains. Insertion can
also promote an acknowledged head before appending a new tail. BUSY follows the
pending command, so acceptance clears it before the command's response. Result
readiness compares cursor and payload size; reading beyond the end reasserts it,
while the separately latched drained flag remains set.

Without an attached controller, `service_cd_fifo` applies a caller-supplied MCU
boundary. Bounded guest backlog, drop, pending-Pause, acknowledgement/drain and
result-wrap observations agree with this host consumer.

`Interconnect::configure_cd_controller` attaches an explicit paused drive and CPU
cycle origin. Its unit-scale FIFO clock converts 797 microseconds to 26,993 cycles.
The first pending opcode write installs a deadline; replacing that opcode keeps
it. Result reads, full acknowledgements and completed command handlers replace
the deadline only when the two response slots are not both full. `Execution`
dispatches due callbacks after branch delay slots and before the next IRQ
admission. Output-clock subdivisions do not dispatch them. Reference execution
uses its absolute access cycles; raw execution retains its two-tick instruction
model.

The controller executes Getstat, Setloc, Mute/Demute, Setfilter/Setmode, Getparam,
GetlocL/P, GetTN/TD, Pause, Init, SeekL/P and ReadN/S through the functional drive.
Automatic
seek timing requires explicit TOC metadata. FIFO, command-continuation and read slots
use the native pending-slot snapshot, visit order and current-target test. Idle
Pause waits 200 microseconds, active-read Pause 70/35 milliseconds at single/double
speed, and Init 120 milliseconds. Seek waits 15
milliseconds before its destination/distance delay and any 650-millisecond speed
settling. Effective speed and its sticky settling latch are separate from raw
mode: Init resets them without changing the Getparam mode byte.

`configure_cd_media` attaches owned raw 2,352-byte backing extents before scheduled
work. Data delivery also needs the data FIFO; selected XA needs the audio queue.
ReadN/S wait 20 milliseconds before their located seek, then combine the seek,
speed settling and first-sector period into one delay. Periodic reads use 13,333
or 6,666 microseconds. Data replacement resets the cursor while retaining BFRD;
response queue fullness does not block it. Withheld XA preserves the old data
buffer. Read can take over an unfinished SeekP, and old read callbacks cancel
after Pause/Init. Delay conversion truncates the complete duration once and
schedules from the callback's actual cycle.

Attached IRQ assertions occur at native head insertion/promotion and mask-write
events. Init inserts its ACK using the old mask, then resets the mask silently.
Reading results or writing unrelated registers does not reassert the interrupt;
repeating an unmasked mask write does. The manual API retains its line model.
Automatic Play, unsupported mechanical overlaps and partial IRQ clears fail
explicitly. Manual acceptance, response publication and data-IRQ delivery are
unavailable while the controller owns them.

Captured native MMIO, all three deadlines and branch dispatch agree for bounded
ReadN/S, Pause/Init/Seek and immediate-command histories. These include active-read
Pause, requested-buffer overwrite despite response backlog, SeekP takeover,
TOC errors and XA withholding. The same guests' reference execution agrees in
MMIO values, relative access cycles and result memory. Subcapacity XA submission
and frame counts also agree, without accepting audio-worker phase or PCM.
Actual backing-read failure, active DMA3 overlap, pending audio host waits,
shared-pass stale continuations and media errors during explicit seek preparation
remain unsupported. Automatic Play, other scheduler interactions, broader IRQ
timing and hardware fidelity retain separate obligations. Init does not reconstruct
an unknown retained GetlocL header in a fresh ready context.

[Implementation and original-driver checks](../specs/formats/audio.md#rust-cd-decoder-host-registers)
use explicit host state and response boundaries. This reference does not supply
drive firmware, BIOS state or hardware timing; no upstream
source or dependency is imported.

## Sector data and DMA3

- [psx-spx DMA channels](https://psx-spx.consoledev.net/dmachannels/) describes
  manual burst counts, trigger/busy bits, priority enable and interrupt handling.
  For non-chopped bursts, visible MADR/BCR stay unchanged; zero count denotes
  65,536 words. Address low bits do not affect word access, and DMA addresses
  wrap at 24 bits. Bus timing and CPU stalls need separate validation.
- [psx-spx CD data ports](https://psx-spx.consoledev.net/cdromdrive/) documents
  2,048-/2,340-byte data blocks and byte/halfword reads. Its stated overread
  behaviour repeats bytes near the end of the block.
- The [native CD data-port handler](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
  returns zero when its FIFO has no data. Clearing BFRD resets the cursor and
  visible size; consuming the block clears the data request. Reasserting BFRD
  preserves the cursor. These are native comparison facts requiring separate
  physical-drive and timing evidence.

[The Rust data/DMA3 contract](../specs/formats/audio.md#rust-cd-sector-data-and-dma3)
names its emulator reference model and rejects overreads rather than choosing
between conflicting descriptions. Original US `CdGetSector` instructions verify
the transfer configuration and copying of supplied blocks; they do not establish
drive queue behaviour, media scheduling or clock timing.

## Drive commands and routing

[psx-spx command descriptions](https://psx-spx.consoledev.net/cdromdrive/)
separate Setloc from seek/read, identify two Pause responses, describe read resume
at the last received sector, and distinguish realtime XA routing from data
delivery. GetlocL reports the newest buffered header, including sectors outside
the selected audible channel. Data delivery has a documented deferred-retry
filtering difference; the Rust functional path covers the first delivery only.

The [native read callback](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
updates the latest header before routing. With realtime or subheader filtering
enabled, realtime audio sectors are withheld from host data regardless of the
selected file/channel; realtime mode may pass them to XA decoding. The Rust path
retains its documented first-delivery policy and rejects unimplemented overruns
and retry scheduling. Native routing and scheduled guest behavior still require
comparison; source inspection is not a hardware trace.

Native Read/Seek checks use SubQ track identity, including pregaps. Read and
SeekL reject audio with INT5 `[status | 4, 0x04]`; positions at or beyond
lead-out reject with `[status | 4, 0x10]`. SeekP permits audio and checks the
unfudged target after adjusting current position. Read errors perform no media
read and retain position, cached header and data. Explicit Seek completion
invalidates GetlocL on either outcome. Rust applies these rules when supplied
an explicit TOC; data-only contexts without metadata remain unchecked. Actual
failed data reads remain unsupported.

[Implementation, original SDK checks and raw-disc evidence](../specs/formats/audio.md#rust-cd-drive-commands-and-sector-routing)
keep command semantics separate from timing, drive initialization and physical
audio acceptance. The raw data source is a 2,352-byte-sector disc image; extracted
STR files contain 2,336-byte sectors and cannot supply original position headers.

## Serialized data selection

The [native CD controller](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
copies each delivered data sector into one FIFO, resets its cursor and publishes
readiness separately from queued responses. A withheld audio sector leaves the
previous data buffer intact. This is not an eight-sector delivery ring.
[psx-spx read documentation](https://psx-spx.consoledev.net/cdromdrive/#readnreads)
distinguishes overwriting unrequested sectors from preserving a requested data
block. These sources guide comparison of the Rust serialized policy, not hardware timing
acceptance; no external source is imported as a production dependency.

[The Rust serialized delivery path and original XA scheduler fixture](../specs/formats/audio.md#original-xa-scheduler-with-drive-responses)
require each prior host transaction to drain before selecting the next block.
They report displacement of unrequested data and reject active-read replacement.
They do not claim an eight-buffer implementation, overrun scheduling or BIOS
interrupt execution. Two explicit sector/callback schedules reach the game's
idle state with different decoded frame counts, leaving clock and audible-end
verification open.

## XA queue admission and output

The [Rust queue](../../tools/rust/bof3-audio/src/machine/cd_queue.rs) follows
native stream-1 whole-batch admission: queued plus incoming frames must be less
than the 16,384-frame ring size. Decoding and interpolation advance before the
batch waits. A full queue returns `Admission::Pending`; the caller supplies
`WaitOutcome::Retry` after consumption or `WaitOutcome::TimedOut` at an observed
deadline. Available space admits the whole batch even at that deadline. A timeout
while full discards only the pending output, retaining both advanced histories.
Another sector cannot enter until the pending submission finishes or resets.
The model does not measure the native host's 200 ms wait or infer hardware timing.

Mute advances ADPCM predictors but skips interpolation and submission, regardless
of queue occupancy. Unmuted decoding applies active drive gain before waiting;
later latches leave queued and pending frames unchanged. Output supplies the
stored frame, or zero when empty. Explicit reset discards queued and pending
output and clears decoder state.

The Rust output clock consumes one configured CD frame every 768 CPU ticks
through SPU capture, routing and gain. No CD source or an empty queue supplies
zero. Consumption follows successful SPU output; a full 4,096-frame output queue
retains the next CD frame. Callers drain output and supply the activation phase,
sector deliveries and pending-submission deadlines. This connection does not
establish native CPU phase or schedule drive transitions.

Native `playXASector` decodes accepted sectors before deciding whether mute
permits a feed. `FeedXA` advances interpolation and applies ATV gain before
submission; the [ring](../../third_party/pcsx-redux-agentic/src/support/circular.h)
waits for whole-batch capacity. Failed submission leaves those histories advanced.
The [audio callback](../../third_party/pcsx-redux-agentic/src/spu/sdlaudio.cc)
zero-fills stream underruns. Rust exposes the wait boundary through
`Interconnect::finish_cd_audio_submission`; callers provide its timing.

[The Rust queue and SPU checks](../specs/formats/audio.md#rust-xa-audio-queue-and-spu-consumption)
cover admission, retry, timeout, mute and malformed-input rollback.
Explicit steady-rate raw-disc tests and the original scheduler fixture establish
functional routing only.
The [XA interleave discussion](https://psx-spx.consoledev.net/cdromformat/)
supports the relationship between encoded mono/stereo rates and sector cadence;
it does not establish initial phase, game tick timing or audible cue endpoints.

## Selected CDDA input

`Queue::cdda_sector` accepts one already-selected 2,352-byte raw sector as
588 little-endian stereo frames at 44.1 kHz. It applies the active drive matrix,
scales by `31130 >> 22`, clips to signed 16-bit, then scales by `31880 >> 15`.
It bypasses XA interpolation and preserves XA predictors, filter history and
selection. Command mute suppresses CDDA; the XA-only mute register does not.
CDDA shares whole-batch admission, pending completion, output and reset with XA.

`Interconnect::enqueue_cd_cdda` supplies command mute and host attenuation to this
path. The caller supplies selected audio sectors and service boundaries. Native
stereo, clipping and a gain latch have a bounded consumed-output comparison;
the retained queued tail is counted separately.

[Functional Rust playback](../specs/formats/audio.md#rust-cdda-playback)
uses explicit data/audio track kind, pregap, program start/end and lead-out
metadata. It supports Play, GetlocP, reports, pregap autopause and motor-off
disc end. Located Play retains Idle/old position until a supplied seek-start
boundary, then enters Seeking before a second boundary starts Playing.
Immediate Play starts at the supplied command cycle. The caller supplies the
retained peak-channel phase, raw sectors and all CPU-cycle/response boundaries.

The [native handlers](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
and [image owner](../../third_party/pcsx-redux-agentic/src/cdrom/cdriso.cc)
select SubQ tracks at pregap start and physical audio at program start. Reports
settle for 750 ms, alternate absolute/relative time and measure unattenuated
channel peaks, including while muted. Autopause stops before the first changed
SubQ track; lead-out returns End before turning off the motor. Rust follows
these functional rules under supplied cycles. CUE parsing, integrated pending
feed ordering, automatic mechanical scheduling and hardware timing remain open.

## Decoder reset and command mute

The [native command handlers](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
reset XA predictors, first-sector, format-ready and ended state on every
accepted ReadN/ReadS and when Setmode enables realtime decoding. Pause,
SeekL/SeekP and Init preserve XA predictors. None of these commands clears the
SPU interpolation ring or queued audio. They also preserve selected data and
its read cursor. Mute/Demute change a flag separate from ADPCTL mute. CdlInit,
opcode `0x0A`, resets current and seek position to `00:02:00` and clears pending
Setloc. Native reset and scheduled-work cancellation still contain unresolved
behavior; source facts do not establish mechanical or IRQ timing.

[Rust command application and original cue restart](../specs/formats/audio.md#rust-cd-command-lifecycle-and-cue-restart)
use predictor-only resets at those Read and realtime-enable boundaries. Pause,
seek and Init preserve queued audio, interpolation and data. Init also preserves
mode, mute, filter and XA ended state. The caller
must complete a pending audio submission before servicing another command.
Successful Init, seek transitions and located Play reject active DMA3 before
mutation; this is an unsupported Rust overlap case. Full `Queue::reset()`
creates an explicit fresh context and is not a command effect.

The [functional drive](../specs/formats/audio.md#rust-cd-drive-commands-and-sector-routing)
restarts every Read at retained seek, even during an active read. Its supplied
Read and explicit Seek boundaries follow native state phases; SeekP adjusts
current position while retaining the exact seek target. Pause becomes Idle
and invalidates GetlocL immediately. Cached header bytes survive; Init restores
their validity, and a new sector replaces them before routing. Native guest
responses and phase observations qualify these rules under supplied cycles.
ReadN/S can replace either unfinished SeekP phase. The old seek completion is
cancelled, the exact seek target survives, and Read acknowledges Idle before its
normal two supplied phases. Native observations confirm both takeover phases and
the absence of a later SeekP INT2. Pending audio and active DMA3 remain rejected
by the interconnect. Automatic scheduling, other command overlaps and hardware
timing remain open.
Coding follows the first-success latch documented below.

Bounded native Init observations agree on position, pending Setloc, mode, mute
and filter. During an active read, a partially consumed 2,048-byte requested FIFO
retains its contents, cursor, request flag and latest header through Init.
Guest loads continue from the retained cursor after INT3/status `0x22` and
INT2/status `0x02`. These observations establish response order and the selected
data boundary. Joined native capture also observes queued disc frames after
Init acknowledgement, accounts for their later consumption without new feeds
or recorder loss, and retains the observed mono ADPCM predictor and first-sector
flag.

With explicit caller registers, the original reset routine installs its own
interrupt state and returns success. Its Init ACK reaches the SDK CD handler
inside an existing VBlank exception. A separate completion capture records INT2,
the CD-line assertion, CPU hardware IRQ admission and entry into that handler.
These bounded calls do not establish natural game startup, wider decoder
histories, signal parity, exact response timing or mechanical latency.

## XA selection and EOF

The [native XA routing](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
uses the configured filter directly and rejects channel 255 even when explicitly
filtered. A routed sector with both EOF and EOR stops XA without an interrupt;
either bit alone continues. ReadN/S and realtime enable clear ended state.
Setfilter, Pause, SeekL/P and Init preserve it.

[Rust routing and EOF](../specs/formats/audio.md#rust-xa-selection-and-eof)
follow these rules at supplied boundaries. The queue receives already-selected
identities without an automatic latch or release operation, preserving decoder
and interpolation histories and queued output. Same-coding native observations
qualify routing; original VOICE uses an explicit file 1/channel 0 filter. Decode
failures, feed timing and independent signal acceptance remain
separate obligations.

## XA coding transitions

The [native XA decoder](../../third_party/pcsx-redux-agentic/src/core/decode_xa.cc)
reads coding only when the caller marks the first sector. The CD controller
clears that flag after successful decoding, so later coding changes are not
automatically admitted as new formats. Native mono decoding leaves the right
predictor unused, while the CD feed duplicates mono into both interpolation
rings. Mute skips the feed after decoding.

The Rust runtime queue follows this first-success latch, including muted and
pending output. ReadN/S and realtime enable reset predictors and reopen the latch;
Pause, SeekL/P, Init and Setfilter retain it. A changed first-sector frequency,
depth or channel count resets both predictors. Interpolation rings, cursor, phase,
previous samples and queued output survive predictor resets. Later coding bytes
are ignored entirely. Strict export decoding still checks the selected stream.

Bounded native guest observations confirm changed and reserved later coding bytes
use the first format, and Read restart permits a different format. Captured decoded
PCM agrees with Rust decoding, and independent interpolation of those native
samples agrees with the runtime queue. Caller-supplied draining avoids a claim
about host admission timing. Native malformed-first failure behavior, emphasis,
automatic clocks and independent original-game audio remain open.

[Rust coding validation](../specs/formats/audio.md#rust-xa-coding-transitions)
covers all eight supported first formats with 192 synthetic vectors generated by
a separate integer model. Numeric FIR coefficients are shared; this does not
establish hardware-filter fidelity. Initial emphasis and actual format changes
after reset under `Published37800` remain explicitly unsupported.
