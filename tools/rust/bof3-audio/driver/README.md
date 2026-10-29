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
RAM with explicit event, exception, timer, CD, DMA and SPU bindings. It uses the
1,024-byte initialization stack and separate 4,096-byte exception stack.
After service initialization, the host selects GP zero, foreground SP
`0x801FFF00` and status `0x401`, with automatic PAD acknowledgement disabled.
Sound initialization, layout selection and host-fed bank/SEP transfer follow
as separate guest calls. No original ROM or game installation is required.

The `audio_xa_select_cue` leaf takes a packed cue ID in A0. The signed upper
value `A0 >> 12` must be 0, 1 or 2; the low 12 bits select the cue. Before
calling, bind the three live `audio_xa_table_pointers` to readable guest
halfword tables and populate `audio_xa_file_lbas` at each selected file slot.
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

The production PSX music renderer uses this driver. Other production operations,
full SFX/vocal contexts, native timing and independent PCM acceptance remain open.
The authored sources use the project [MIT license](../LICENSE); dependency
notices remain in [`THIRD_PARTY_LICENSES.txt`](../THIRD_PARTY_LICENSES.txt).
