# External references

Retained research notes and source provenance. External specifications describe
their own formats; BOF3 behavior requires the target-qualified evidence linked
from [game specifications](../specs/INDEX.md).

- [Standard MIDI Files](standard-midi-files.md): specification locations, framing
  rules, channel messages, controller assignments, BOF3 SEP differences and
  current Rust interchange coverage, with a reusable timing example and source map.
- [SoundFont](soundfont.md): inspected specification, table/sample requirements
  and independent-consumer checks for the Rust writer.
- [Rust delivery codecs](audio-codecs.md): pinned MP3/Vorbis source and license
  review, proposed dependency graph, and timing/conformance gates.
- [SPU envelopes](spu-envelopes.md): arithmetic-source differences, retained
  boundary cases and the limits of current ADSR validation.
- [SPU sample playback](spu-samples.md): interpolation ROM provenance, pitch and
  rounding differences, voice-state evidence and unresolved hardware timing.
- [CD/XA audio processing](cd-audio.md): resampling coefficients/alignment,
  drive gain routing and unresolved hardware/command timing.
- [PSX CPU and BIOS execution](psx-kernel.md): COP0, critical sections and
  CD-driver removal sources, with explicit runtime-evidence limits.

- [Audio runtime reference setup](audio-runtime-setup.md): pinned PCSX-Redux
  submodule, BIOS collection preparation and local provenance inventories.

Return to the [documentation index](../INDEX.md).
