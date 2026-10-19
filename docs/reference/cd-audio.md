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
expansion and this alignment when the native gain stages are applied separately.
They cover consumed output under the captured settings, including distinct stereo
channels. Remaining queued output, other formats and histories, and hardware
fidelity require separate evidence.

The retained Rust 18.9 kHz table has 175 coefficients, SHA-256
`02f3c9597f8e4b8f961e7981ac758034a2ff8eb5200b01ba410106e3460bffa1`.
Its coefficient provenance remains uncertain. This diagnostic path sums products
before shifting and uses a separate phase schedule. Native `FeedXA` instead
inserts the midpoint of each sample pair and runs the resulting 37.8 kHz input
through the shared zigzag filter. Neither the retained half-rate policy nor the
native implementation establishes half-rate hardware fidelity.

Native ATV gain also differs from the Rust diagnostic matrix. XA applies integer
scales 33617 before saturation and 31880 afterward; CD-DA uses 31130 before
saturation and 31880 afterward. The native feed then applies the configured host
volume divisor. Capture stage, settings and these gain differences must remain
explicit in comparisons.

[Rust implementation and checks](../specs/formats/audio.md#rust-cdxa-sample-processing)
retain startup phase, history, gain limits and timing boundaries. Emphasis, drive
commands/FIFO scheduling, mute effects on decoder state and hardware comparison
remain open; encoded-rate extraction keeps its existing separate contract.

## CD decoder host interface

[psx-spx host-register descriptions](https://psx-spx.consoledev.net/cdromdrive/#cdrom-controller-io-ports)
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

The [Rust queue](../../tools/rust/bof3-audio/src/machine/cd_queue.rs) retains a
diagnostic policy that drops incoming XA above ten buffered frames without
committing decoder history. Muted admission decodes ADPCM but skips resampling
and insertion. Output applies current drive-matrix gain after popping a frame
and supplies zero when empty. The watermark and ordering are retained model
choices requiring independent lifecycle/timing evidence.

Native `playXASector` decodes accepted sectors before deciding whether mute
permits a feed. `FeedXA` applies ATV gain before submission to the separate CD
stream; the [audio callback](../../third_party/pcsx-redux-agentic/src/spu/sdlaudio.cc)
zero-fills stream underruns. These owners do not implement the Rust ten-frame
admission rule or dequeue-time gain policy.

[The Rust queue and SPU checks](../specs/formats/audio.md#rust-xa-audio-queue-and-spu-consumption)
retain those ordering choices, expose drops/underruns, and additionally reject
malformed sectors even when they would be dropped. Explicit steady-rate raw-disc
tests and the original scheduler fixture establish functional routing only.
The [XA interleave discussion](https://psx-spx.consoledev.net/cdromformat/)
supports the relationship between encoded mono/stereo rates and sector cadence;
it does not establish initial phase, game tick timing or audible cue endpoints.

## Decoder reset and command mute

The [native command handlers](../../third_party/pcsx-redux-agentic/src/core/cdrom.cc)
reset XA predictors/readiness on ReadN startup. Pause stops reading without that
XA reset; SeekL/SeekP also do not call `resetXA`. Mute/Demute change a flag
separate from ADPCTL mute. CdlInit, opcode `0x0A`, resets current and seek position
to `00:02:00` and clears pending Setloc. Its source still leaves parts of reset
and scheduled-work cancellation unresolved. Mechanical and IRQ timings require
separate verification.

[Rust command application and original cue restart](../specs/formats/audio.md#rust-cd-command-lifecycle-and-cue-restart)
retain explicit modeled command boundaries and reject active-DMA reset. The
Rust policy clears audio state on seek/read startup and Pause, allows an already
continuing read to avoid restart, and resets positions and pending Setloc at
`0x0A`. Init preserves mode, mute, filter, data selection and queued audio while
releasing XA selection. Successful Init and seek transitions reject active DMA3
before changing drive or buffer state. Audio-reset policies still differ from
the native handlers. Selection release and diagnostic coding transitions are
implemented separately below.

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
uses the active filter directly and rejects channel 255 even when explicitly
filtered. Setfilter updates file/channel fields without releasing a separate
automatic-selection latch. A routed sector with both EOF and EOR stops XA;
either bit alone does not. These facts differ from Rust's retained selection
policy and do not establish physical timing or audio fidelity.

[Rust selection and EOF](../specs/formats/audio.md#rust-xa-selection-and-eof)
release selection at Setfilter or selected EOF before backlog admission and
retain decoder history across same-coding handoffs, including dropped/muted EOF.
Five synthetic checks and the complete raw VOICE scan cover the implemented
boundary. VOICE has one selected EOF but no subsequent cross-channel handoff;
handoff evidence is synthetic. Coding-format transitions are covered below.

## XA coding transitions

The [native XA decoder](../../third_party/pcsx-redux-agentic/src/core/decode_xa.cc)
reads coding only when the caller marks the first sector. The CD controller
clears that flag after successful decoding, so later coding changes are not
automatically admitted as new formats. Native mono decoding leaves the right
predictor unused, while the CD feed duplicates mono into both interpolation
rings. Mute skips the feed after decoding.

The Rust diagnostic transition policy reads coding per admitted sector, retains
shared cursor/phase state and leaves the right predictor/interpolation ring
untouched for mono, duplicating interpolated left. Its backlog drop preserves
decoder history. These are explicit modeled choices requiring comparison with
the current native path and independent hardware evidence.

[Rust transition validation](../specs/formats/audio.md#rust-xa-coding-transitions)
covers all eight supported coding formats with 192 synthetic vectors generated
by a separate Rust integer model. Numeric filter coefficients are shared;
this is not an independent hardware-filter or game-audio oracle. Emphasis and
dynamic changes under `Published37800` remain explicitly unsupported.
