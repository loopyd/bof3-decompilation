# Owned audio guest driver

This target builds authored little-endian MIPS I assembly and its linker layout.
It needs GNU Make, `sha256sum` and the approved `mipsel-none-elf` GNU binutils.
The full repository defaults to its existing `toolchains/psyqo/bin/` tools.
An extracted source package uses the same target with an explicit tool prefix:

```sh
make -C tools/rust/bof3-audio/driver bof3-audio-driver \
  CROSS_COMPILE=/path/to/bin/mipsel-none-elf-
```

If those tools are already on `PATH`, use `CROSS_COMPILE=mipsel-none-elf-`.
Command-line `AS`, `LD`, `OBJCOPY` and `NM` values override the corresponding
tools individually. Building does not install tools or read a game executable,
BIOS, disc image or extraction cache.

Outputs default to `out/audio/driver/` beneath the repository or package root.
`OUT` can select another directory; keep it relative to the driver directory
when its containing path has spaces. The target produces `kernel.elf`,
`kernel.map`, `kernel.bin`, `kernel.sym` and `kernel.json`. The runtime needs the
last three files together. The `bof3.audio-driver/v1` manifest declares
`mips1-le-o32` and hashes the exact binary and defined-nm symbol bytes.

The low image, mutable state and stacks occupy the reserved
`0x80020000..0x80050000` arena. The initializer copies the packed 628-byte
scheduler to `0x8016C548..0x8016C7BC`; reserve that execution range separately.
The linker checks both allocations. Source comments retain original US function
ranges, data identities and provenance hashes.

The Rust [`machine::driver::Image`](../src/machine/driver.rs) validates artifact
hashes, required symbols and allocation bounds, then starts services in zeroed
RAM with explicit event, exception, timer, DMA and SPU service bindings. It uses the
1,024-byte initialization stack and separate 4,096-byte exception stack.
Startup also owns RAM `0xA0..0xC0` for the A0/B0 entry vectors and
`0x100..0x128` for kernel descriptors. A0/72 and B0/07..0D,17,18,20 execute those
vectors and relocated service tables before their owned guest bodies. The
loader requires the current vector symbols, checks their eleven table bindings
and verifies the installed bytes. Direct body guards remain active.
After service initialization, the host selects GP zero, foreground SP
`0x801FFF00` and status `0x401`, with automatic PAD acknowledgement disabled.
Sound initialization, layout selection and host-fed bank/SEP transfer follow
as separate guest calls. No original ROM or game installation is required.

B0/0A `WaitEvent` validates the handle against the live event allocation before
entering its guest body. READY becomes BUSY and returns 1. Initial BUSY waits on
the selected record until READY; other initial states return 0. The loop keeps
that record pointer across IRQ save and return, including after descriptor
relocation, disable or close. Delivery still requires an enabled matching event.

OpenEvent, CloseEvent, TestEvent, EnableEvent, DisableEvent and UnDeliverEvent
execute guest instructions against the live event allocation. OpenEvent scans
for the lowest free slot through its guest helper and preserves the original
record write order and 48-byte frame. Its caller supplies four writable argument
words above SP. Entry guards reject malformed descriptors, out-of-range handles,
unsupported open modes and invalid frames or return addresses before the first
body instruction. The instruction that consumes an argument determines which
side of an incoming delayed load the guard validates.

`Image::start_scheduled` accepts device configuration before the first startup
instruction and a resolver for audio waits. The resolver orders producer and
consumer work without restarting the guest call. `Execution::call_scheduled`
uses the same resolver for later bounded calls. Each wait poll counts toward
the call's transition limit. These APIs require the caller to supply a schedule;
they do not establish native startup timing.

`Image::start_cd` also attaches a reset CD host and the caller's paused `Drive`.
Its optional `Media` input attaches raw backing before any scheduled work.
The automatic controller starts at the owned execution clock's current tick.
The method executes `audio_cd_initialize(1)` with a 3,000,000-transition limit,
running guest callback registration, Nop, Init, Demute, IRQ draining and SPU CD setup.
It returns `CdStarted` with the service startup and CD initialization call records.
An active drive, failed guest call or unsuccessful initializer returns an error.
The supplied drive declares an authenticated, closed, spinning disc context;
startup does not establish authentication. XA decoder history and media LBAs
require separate explicit bindings. The image supplies the US selector tables
and their pointers. PCM output is not enabled during
startup, and these functional calls do not establish native timing.

The `audio_xa_select_cue` leaf takes a packed cue ID in A0. The signed upper
value `A0 >> 12` must be 0, 1 or 2; the low 12 bits select the cue. The three live
`audio_xa_table_pointers` start bound to complete US halfword tables supplied by
`audio/tables.s`. Before calling, populate `audio_xa_file_lbas` at each selected
file slot. A low-level caller that replaces a table pointer owns its full walk.
The adjacent `audio_xa_file_slots` words default to 681, 434 and 682. Slots
may change within the allocated 0..682 index range when their LBA is supplied.
Every halfword reached by the original high-bit table walk, including the
selected pair, must exist. Tables may alias live state; the guest reads them
in original instruction order. Zeroed pointers or LBA storage do not configure
a source, and the leaf does not validate these caller preconditions.

The 56-byte `audio_xa_state` preserves original field offsets. The selector
writes the start LBA at +0, stop threshold at +4, current LBA at +8, filter
file at +0x28 and channel at +0x29. Other bytes remain live and untouched.
The stop threshold is a scheduler input, not an accepted audible endpoint.
The `audio_xa_start` entry takes the same packed ID in A0 and requires the
same table/LBA bindings. It selects the cue, initializes the original control fields, writes the
start position at state +0x2C and installs `audio_xa_completion_callback` in the
shared `audio_cd_sync_callback` word. V0 returns the previous callback pointer.
START preserves the completion byte at +0x22 and poll halfword at +0x12; callers
must retain their live values across repeated calls. It also clears the separate
`audio_xa_start_clear_byte`, which state 7 sets to 3; other uses remain unresolved.

`audio_cd_int_to_position` takes a signed LBA in A0 and a writable three-byte
output in A1, returning that output pointer in V0. It preserves the original
unchecked 32-bit arithmetic and stores bytes at +2, +1, then +0. The host's
validated BCD domain is -150..449849; this guest entry does not enforce it.
`audio_cd_set_sync_callback` replaces the shared callback word with A0 and
returns its previous value without invoking either callback.

The completion callback takes the completion code in A0 and response pointer
in A1. Code 2 sets state +0x22 to 1 before copying eight live response bytes,
indices 7 through 0, into state +0x18..+0x1F. Other codes set +0x22 to 255
without reading A1. Overlapping response and state buffers follow that exact
order.

The standalone state entries `audio_xa_state_0`, `audio_xa_state_2` and
`audio_xa_state_7` take no arguments and use the same live state. State 0 clears
the timer and completion fields. States 2 and 7 treat completion as a signed byte and
on negative completion compare the old signed halfword retry count with six
while incrementing its stored halfword. State 2 advances positive completion to
state 3. On positive completion, state 7 clears state, timer and completion,
writes 2 at +0x23 and 3 to the separate byte, and exchanges the callback
with NULL; V0 then returns the previous callback pointer.
Zero completion leaves those state fields unchanged. These entries preserve
original register results on their other paths.

`audio_xa_response_has_bit4` returns Boolean bit4 of response byte +0x18.
`audio_xa_is_idle` returns true only when state +0x21 is zero.
`audio_cd_position_to_int` takes three readable bytes in A0 and returns the
signed LBA in V0, subtracting the 150-frame lead-in. It reads offsets +0, +1
and +2 in order, clobbers A0 and converts unchecked nibbles, including invalid
BCD. It performs no writes and preserves HI/LO.

`audio_xa_tick` dispatches all nine states through the live signed state byte.
The caller must keep that byte within 0..8. The guest preserves the original
unchecked table access, state transitions, completion polling and watchdog order.
`audio_xa_set_gain` writes the current gain and calls the CD mixer. The live
`audio_xa_gain_flag` selects the original gain matrix and starts at zero; its
producer and meaning remain unresolved. `audio_xa_ramp_gain` rereads gain after
each mixer call and performs the final mixer call even when already at its target.

The CD SDK entries own command retries, blocking and polling responses, IRQ
FIFO draining, callback dispatch, recovery, initialization and banked gain writes.
Their mutable tables share the callback word used by XA. Response copies read
live bytes in ascending order; the XA completion callback uses descending order.
Callers must provide valid command indices, parameter buffers and response storage
for the selected SDK entry. The build does not include the unrelated CD DMA helpers.

`audio_vsync` retains both positive waits and negative counter queries, including
GPU status and timer reads before the mode branch. It shares the callback manager's
VBlank counter and uses its PAD and root-counter service wrappers on timeout.
The source requires functional register and clock inputs. Linking these entries
does not establish native GPU timing, scheduled CD playback, XA rendering or vocal
roles.

Owned startup and original BIOS reference boot attach the NTSC software-GPU
status model in reset state. Raw RAM callers use `Interconnect::configure_gpu_status`
before starting their frame clock. GP1 reset, FIFO clear, IRQ acknowledgement,
display enable, DMA direction and NTSC display mode update its status. Reset
and IRQ acknowledgement clear the GPU interrupt latch. The output clock and
original reference clock supply VBlank transitions for field parity. GPU
status/control accesses require words. PAL mode, other GP1 commands, GP0, VRAM
and GPU DMA remain unsupported.

Original BIOS reference boot uses absolute CPU cycles for the NTSC root-counter
scheduler. Counter reads, target and mode writes, blanking gates and IRQs follow
the emulator's update order. Scheduling wraps rate products at 32 bits and
uses 64-bit absolute CPU cycles. This profile uses the default disabled
`RCntFix` setting and retains the mode-triggered timer-2 jitter rule.

Owned execution counts instruction fetch, data access and multiply/divide costs
with the same interpreter model. Host kernel services retain two functional
ticks per dispatch; their latency is unverified. Owned timers and PCM output
keep their separate device clocks. Production output starts before game sound
initialization, and capture retains its phase. CD service startup remains
unclocked. Instruction costs alone do not establish native timing or PCM equivalence.

Production PSX music, cold-bank SFX and cold XA cue renderers use this driver.
The XA host parks at `audio_xa_foreground` between its 60 Hz functional tick calls,
returning through interrupt continuations before the next call. Duration stopping
pulses the caller-owned state byte at +0x20 for one tick; the guest owns recovery,
gain ramp, Pause and callback teardown. Broader playback histories, other
production operations, native timing and independent PCM acceptance remain open.
The authored sources use the project [MIT license](../LICENSE); dependency
notices remain in [`THIRD_PARTY_LICENSES.txt`](../THIRD_PARTY_LICENSES.txt).
