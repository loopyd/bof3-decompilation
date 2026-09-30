# CD/XA audio processing references

Inspected 2026-09-24. These sources describe disc/drive processing; game commands,
seek state and timing require separate original-runtime evidence.

- [psx-spx XA interpolation](https://psx-spx.consoledev.net/cdromformat/#cdrom-xa-audio-adpcm-compression):
  seven 29-coefficient phases and a six-input/seven-output schedule for 37.8 kHz.
  The description explicitly does not establish exact hardware rounding or a
  complete 18.9 kHz algorithm.
- [DuckStation CD implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp):
  inspected decoder/resampler alignment, separate 18.9 kHz interpolation and drive
  gain application. Moving branch, not an immutable hardware oracle. No source
  implementation or dependency is imported; only numeric coefficient facts are
  retained.
- [psx-spx drive registers](https://psx-spx.consoledev.net/cdromdrive/): ATV0–3
  staging, ADPCTL apply/mute bits and incomplete saturation above double gain.

All 203 coefficients of the published 37.8 kHz table agree with the inspected
emulator. In phase-major little-endian signed-16 encoding their SHA-256 is
`0ea87420ec5d82b35aa651a26622572b422f3d330afbf78f166941db7035ca80`.
The pseudocode indexes history from `p-1`, while the emulator indexes from `p`;
Rust preserves both as named choices. Each product is floored before summation.

The emulator's separate 18.9 kHz table has 175 coefficients, SHA-256
`02f3c9597f8e4b8f961e7981ac758034a2ff8eb5200b01ba410106e3460bffa1`.
Its own comment identifies uncertain coefficient provenance. This path sums
products before shifting and uses a different phase schedule. It is an explicit
emulator reference, not verified half-rate hardware behavior or duplicated input
fed through the 37.8 kHz filter.

[Rust implementation and checks](../specs/formats/audio.md#rust-cdxa-sample-processing)
retain startup phase, history, gain limits and timing boundaries. Emphasis, drive
commands/FIFO scheduling, mute effects on decoder state and hardware comparison
remain open; encoded-rate extraction keeps its existing separate contract.

## CD decoder host interface

Inspected 2026-09-24: [psx-spx host-register descriptions](https://psx-spx.consoledev.net/cdromdrive/#cdrom-controller-io-ports)
cover bank selection, parameter/result FIFOs, response readiness, interrupt
mask/acknowledgement and volume ports. Interrupt numbers occupy a bitmasked
field; they are not independent one-hot flags. Response availability and IRQ
delivery can occur separately. Overlapping commands and decoder reset have
unresolved behaviour and are not approximated by the Rust host component.

[Implementation and original-driver checks](../specs/formats/audio.md#rust-cd-decoder-host-registers)
use explicit host state and response boundaries. This reference does not supply
drive firmware, sector scheduling, BIOS state or hardware timing; no upstream
source or dependency is imported.

## Sector data and DMA3

Inspected 2026-09-24:

- [psx-spx DMA channels](https://psx-spx.consoledev.net/dmachannels/) describes
  manual burst counts, trigger/busy bits, priority enable and interrupt handling.
  For non-chopped bursts, visible MADR/BCR stay unchanged; zero count denotes
  65,536 words. Address low bits do not affect word access, and DMA addresses
  wrap at 24 bits. Bus timing and CPU stalls need separate validation.
- [psx-spx CD data ports](https://psx-spx.consoledev.net/cdromdrive/) documents
  2,048-/2,340-byte data blocks and byte/halfword reads. Its stated overread
  behaviour repeats bytes near the end of the block.
- [DuckStation CD implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp),
  inspected as a moving upstream source, instead zero-fills overreads. It rewinds
  a partial block when BFRD is cleared and clears BFRD/readiness on consumption.
  Repeated assertion preserves the cursor. This is emulator comparison evidence,
  not a physical-drive trace or a dependency imported into the project.

[The Rust data/DMA3 contract](../specs/formats/audio.md#rust-cd-sector-data-and-dma3)
names its emulator reference model and rejects overreads rather than choosing
between conflicting descriptions. Original US `CdGetSector` instructions verify
the transfer configuration and copying of supplied blocks; they do not establish
drive queue behaviour, media scheduling or clock timing.

## Drive commands and routing

Inspected 2026-09-24: [psx-spx command descriptions](https://psx-spx.consoledev.net/cdromdrive/)
separate Setloc from seek/read, identify two Pause responses, describe read resume
at the last received sector, and distinguish realtime XA routing from data
delivery. GetlocL reports the newest buffered header, including sectors outside
the selected audible channel. Data delivery has a documented deferred-retry
filtering difference; the Rust functional path covers the first delivery only.

[DuckStation's drive implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp)
updates the latest header before routing. Its routing contains unresolved filter
behaviour and its queue model is comparison evidence, not a hardware trace. The
Rust path follows the explicitly documented first-delivery rules and rejects
unimplemented automatic XA selection, overruns and retry scheduling.

[Implementation, original SDK checks and raw-disc evidence](../specs/formats/audio.md#rust-cd-drive-commands-and-sector-routing)
keep command semantics separate from timing, drive initialization and physical
audio acceptance. The raw data source is a 2,352-byte-sector disc image; extracted
STR files contain 2,336-byte sectors and cannot supply original position headers.

## Serialized data selection

Inspected 2026-09-24: [DuckStation's asynchronous interrupt delivery](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp)
selects its current written sector buffer when delivering INT1. Its eight-buffer
ring, delayed interrupts and retry rules are separate from that selection step.
[psx-spx read documentation](https://psx-spx.consoledev.net/cdromdrive/#readnreads)
distinguishes overwriting unrequested sectors from preserving a requested data
block. These sources guide a named emulator-reference path, not hardware timing
acceptance; no external source is imported as a production dependency.

[The Rust serialized delivery path and original XA scheduler fixture](../specs/formats/audio.md#original-xa-scheduler-with-drive-responses)
require each prior host transaction to drain before selecting the next block.
They report displacement of unrequested data and reject active-read replacement.
They do not claim an eight-buffer implementation, overrun scheduling or BIOS
interrupt execution. Two explicit sector/callback schedules reach the game's
idle state with different decoded frame counts, leaving clock and audible-end
verification open.

## XA queue admission and output

Inspected 2026-09-24: [DuckStation's CD audio implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp)
drops incoming XA when more than ten frames remain buffered, before committing
decoder history. For accepted muted audio it decodes ADPCM but skips resampling
and queue insertion. Its output routine applies current drive-matrix gain after
popping a frame and supplies zero when empty. These are explicit emulator
reference choices; neither the watermark nor lifecycle/timing behaviour is
independent hardware evidence.

[The Rust queue and SPU checks](../specs/formats/audio.md#rust-xa-audio-queue-and-spu-consumption)
retain those ordering choices, expose drops/underruns, and additionally reject
malformed sectors even when they would be dropped. Explicit steady-rate raw-disc
tests and the original scheduler fixture establish functional routing only.
The [XA interleave discussion](https://psx-spx.consoledev.net/cdromformat/)
supports the relationship between encoded mono/stereo rates and sector cadence;
it does not establish initial phase, game tick timing or audible cue endpoints.

## Decoder reset and command mute

Inspected 2026-09-24: [DuckStation's command and drive transitions](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp)
clear audio decoder/queue state on seek/read startup and Pause. Seek clears data
buffers too; Pause does not perform that data-buffer reset. Mute/Demute change a
separate flag from ADPCTL mute. A read already continuing at the requested next
sector avoids restart. Reset releases XA file/channel selection and zeros
predictor/filter history. These are emulator comparison facts; mechanical and
IRQ timings require separate verification.

[Rust command application and original cue restart](../specs/formats/audio.md#rust-cd-command-lifecycle-and-cue-restart)
connect these effects at explicit command boundaries. They reject active-DMA
reset. Selection release and emulator-reference coding transitions are
implemented separately below.

## XA selection and EOF

Inspected 2026-09-24: [DuckStation's Setfilter and ProcessXAADPCMSector](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp)
separate current file/channel selection from decoder history. Setfilter clears
selection; a selected EOF clears it before the backlog check. Unselected EOF
cannot do so. Automatic selection skips channel 255 unless explicitly filtered.
These comparison behaviours do not establish physical timing or audio fidelity.

[Rust selection and EOF](../specs/formats/audio.md#rust-xa-selection-and-eof)
retain decoder history across same-coding handoffs, including dropped/muted EOF.
Five synthetic checks and the complete raw VOICE scan cover the implemented
boundary. VOICE has one selected EOF but no subsequent cross-channel handoff;
handoff evidence is synthetic. Coding-format transitions are covered below.

## XA coding transitions

Inspected 2026-09-24: [DuckStation's XA decoder and resamplers](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp)
read coding per admitted sector and retain device history across changes.
Mono leaves the right predictor and interpolation ring untouched; output
duplicates interpolated left. Both rates share cursor/phase state. Mute skips
resampling, while backlog drop skips decoding as well.

[Rust transition validation](../specs/formats/audio.md#rust-xa-coding-transitions)
covers all eight supported coding formats with 192 synthetic vectors generated
by separate Python arithmetic/state. Numeric filter coefficients are shared;
this is not an independent hardware-filter or game-audio oracle. Emphasis and
dynamic changes under `Published37800` remain explicitly unsupported.
