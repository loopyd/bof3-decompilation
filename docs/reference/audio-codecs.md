# Rust audio delivery codecs

The approved dependency graph pins
`oxideav-mp3 =0.1.3` and `oxideav-vorbis =0.0.12`, with the exact transitive
versions below. License notices are included in the source package.
Conformance, fidelity, timing, and publication remain separate acceptance
gates. The production command surface exports WAV only.

## Reviewed graph

The MP3 package depends on `oxideav-core`; Vorbis depends on that core and
`oxideav-ogg`. The core adds `bytemuck`, `thiserror` and the existing
`serde_json`. `thiserror-impl` requires Syn 3, in addition to the existing Syn 2
used by Serde. Syn 3 reuses the already-locked `proc-macro2 1.0.106`,
`quote 1.0.46` and `unicode-ident 1.0.24`. Do not silently upgrade existing
packages or enable optional Bytemuck derive/rustversion features.

| Reviewed package | License in published archive | SHA-256 of reviewed archive |
| --- | --- | --- |
| [oxideav-mp3-0.1.3](https://crates.io/crates/oxideav-mp3/0.1.3) | MIT | `8583bbbe119a19c0b88ee5b0f76c90493395350878e7c770ce6402287b164477` |
| [oxideav-vorbis-0.0.12](https://crates.io/crates/oxideav-vorbis/0.0.12) | MIT | `08878d1ec255f403363cad0a42273cbea90c01b817d49e45ad8ec13416b9e913` |
| [oxideav-core-0.1.36](https://crates.io/crates/oxideav-core/0.1.36) | MIT | `a78b502f679c580ceb7cc834d23f80821673b0a393a3083d54ef82668bb9c028` |
| [oxideav-ogg-0.1.8](https://crates.io/crates/oxideav-ogg/0.1.8) | MIT | `8a582af48d51a2a8d2d87958ca22a3a21f9029dcc828c1b4a5865fa00f952d3a` |
| [bytemuck-1.25.2](https://crates.io/crates/bytemuck/1.25.2) | Zlib OR Apache-2.0 OR MIT | `95832e849adfb21180ccb6826a99da14e5d266ae5c2e668e1602cf234f153797` |
| [thiserror-2.0.21](https://crates.io/crates/thiserror/2.0.21) | MIT OR Apache-2.0 | `09e52cb86a36cede5cb101bf8908837b3e4c6e5e59fe7fd85c23fb56200d189e` |
| [thiserror-impl-2.0.21](https://crates.io/crates/thiserror-impl/2.0.21) | MIT OR Apache-2.0 | `fe5197923287db20a58125f0bc85c062f7f2c892de97b18c356f9efb14b28524` |
| [syn-3.0.6](https://crates.io/crates/syn/3.0.6) | MIT OR Apache-2.0 | `8593e8e72159ed2257d083c7a454a85cbf854f37a0966d8d483aff8c8a3ebcee` |

`Cargo.lock` records the resolved graph and archive checksums. Optional and
development dependencies are not approved runtime additions.

No reviewed archive contains C/C++ sources or native libraries. The four
OxideAV packages, Bytemuck, Syn and thiserror-impl have no build script. The
thiserror build script writes an `OUT_DIR` module and probes the configured
Rust compiler; it does not build native code, fetch data or run audio tools.
Bytemuck's two `extern "C"` occurrences implement traits for function-pointer
types; they do not link or invoke a native library. The scan is source evidence,
not a claim of full functional or security verification.

The OxideAV packages declare Rust 1.80, thiserror/thiserror-impl 1.77 and Syn
1.71. Bytemuck's manifest declares no minimum compiler. The project requires
Rust 1.88 or newer. See the
[audio README](../../tools/rust/bof3-audio/README.md) for build commands and
the scoped toolchain configuration.

## Integration and acceptance work

Use the direct Rust encoder APIs with explicitly validated PCM rate, channels,
quality and sample count. Keep synthesis and pre-lossy WAV comparisons separate
from encoding. A temporary verified file may be published only after structural,
timing and independent-consumer checks; no native or external production
encoder is permitted. FFmpeg may remain an independent test consumer.

The MP3 release exposes Xing/Info frame counts, byte counts, TOC, and quality,
but `XingTagSpec` does not expose delay or padding. The Rust adapter supplies
those fields. Vorbis uses Ogg framing and terminal granule positions. Verify
presentation length independently, including short clips and partial frames.

Required evaluation includes mono/stereo, silence, impulses, noise, short and
non-frame-aligned clips, representative BOF3 music/SFX/voice PCM, malformed
settings and failed publication. Compare pre-lossy quality, decoded duration,
encoder delay/padding and deterministic output on the pinned compiler. Confirm
independent MP3/Vorbis decoding and package builds before enabling delivery
formats. Fix or reject unsuitable codec behavior explicitly; dependency approval
alone is not production acceptance.

## Explicit PCM rate conversion

`codec::rate::convert` converts nonempty mono/stereo PCM16 between 18900, 32000,
37800, 44100 and 48000 Hz. Callers supply both rates. Neither encoder calls it
implicitly. The development `encoding INPUT_WAV NEW_OUTPUT_DIR [TARGET_RATE]`
example accepts an explicit target rate and retains the original `source.wav`,
the encoder's `reference.wav`, and a `bof3.audio.delivery-evaluation/v3` report.
Without a target rate it preserves the input rate.

The converter uses a symmetric Blackman-windowed sinc with 32 zero crossings
on each side and a cutoff at 94% of the lower Nyquist frequency. Each rational
phase has unity DC gain. Output frame `n` samples input time
`n * input_rate / output_rate`; the output has
`ceil(input_frames * output_rate / input_rate)` frames. Matching rates bypass
filtering and preserve the PCM exactly.

Finite clips use zero extension without an added delay or filter tail. Boundary
samples can change. PCM16 quantization rounds to nearest, with ties away from
zero, and reports saturated samples. Reports also retain both rates, channel
count, frame counts and cutoff. The PCM API does not carry container metadata
or rebase loop coordinates. This delivery filter does not model PSX CD
resampling. Perceptual quality, broader content coverage and floating-point
bit identity across platforms remain unaccepted.

## MP3 timing and consumer limits

The encoder has a 481-frame filterbank delay plus one 576-frame granule. The
[FFmpeg 6.1.1 MP3 reader](https://github.com/FFmpeg/FFmpeg), `libavformat/mp3dec.c`,
requires a following frame while probing and recognizes delay fields for
specific encoder identifiers. Its recognized tags add 529 decoder frames to
the reported encoder delay. Tag writers must identify the actual encoder.
Do not impersonate another implementation to satisfy a consumer.
The MP3 adapter supports 32000, 44100, and 48000 Hz; other rates require
verified Rust conversion or an explicit unsupported-rate error.

`src/codec/mp3.rs` adds 1152 zero input frames to flush filter history,
validates emitted frame boundaries/counts, and writes an Info carrier with
frame/byte counts and the 36-byte gapless extension. It records 528 encoder
delay frames, separately reports 529 decoder frames, and derives end padding
from the original PCM length. The extension identifies the actual codec as
`OxAV0.1.3`; its audio/tag CRCs use CRC-16/IBM. The binary layout follows the
[LAME tag writer](https://github.com/lameproject/lame), `libmp3lame/VbrTag.c`.
No third-party C implementation is included in production.

The Rust-only ignored consumer test dynamically loads the already installed
mpg123 library on Linux. Its [tag reader](https://github.com/libsdl-org/mpg123),
`src/libmpg123/parse.c`,
accepts this encoder identifier. A published-source defect in `stream_encoder.rs`
allocates 4508/5660 bits per granule at 32000 Hz mono, 256/320 kbit/s, exceeding
the 4095-bit `part2_3_length` field. `main_data.rs` does not reject overflow;
the side-information writer truncates it. Registry sources remain unchanged.
The Rust adapter rejects both configurations before encoding, reporting
the requested settings and bit budget without substituting a bitrate.

Boundary-impulse fidelity remains unresolved for 48000 Hz stereo at 128 kbit/s.
Exact decoded lengths and configuration guards do not establish PCM quality.
FFmpeg 6.1.1 still ignores the truthful encoder's gapless extension; this is a
consumer limitation, not permission to impersonate another encoder.

The Rust MP3 adapter validates coded-data bounds as well as frame headers
before returning output. `codec/mp3/stream.rs` checks constant rate/channel/
bitrate, complete MPEG-1 frames, available and nonoverlapping reservoir history,
total granule lengths, scalefactor lengths and complete big-value Huffman symbols
within each granule/channel's declared budget. The published decoder's big-value
loop does not enforce that last boundary; the adapter checks its consumed bit
position explicitly. This catches a Rust-generated reproduction of the original
12-bit overflow even when bypassing the separate configuration guard.
Count1 padding, gapless metadata and PCM fidelity remain separate checks;
this is not an independent complete decoder.

## Vorbis framing and consumer limits

The [Vorbis specification](https://xiph.org/vorbis/doc/Vorbis_I_spec.html) permits
multiple modes. A consumer warning about three modes does not establish an
invalid stream. The Rust Vorbis adapter checks single-stream Ogg
capture/version, flags, serial/page sequence, lacing, CRCs, header signatures,
identification fields, monotone granules and final presentation length using
the [Ogg framing specification](https://xiph.org/ogg/doc/framing.html). This
inspector does not decode setup/audio packets or establish perceptual quality.
Broader corpus quality and delivery publication remain open.

All executable tests and oracle adapters in the audio crate are Rust. Python
is confined to harness integration tests. mpg123/FFmpeg/Xiph are independent
test consumers only; none is a production dependency or fallback renderer.

## Primary sources and alternatives

- [OxideAV MP3 source](https://github.com/OxideAV/oxideav-mp3) and
  [Vorbis source](https://github.com/OxideAV/oxideav-vorbis) describe pure-Rust
  encoders. Published archives above, rather than mutable branch claims, own
  this review's version-specific findings.
- [Shine-RS](https://docs.rs/shine-rs) is another Rust MP3 encoder. The previously
  retained 0.1.4 archive declares LGPL-2.0; it is not the proposed dependency.
- [rusty_vorbis](https://docs.rs/rusty_vorbis/0.1.1/rusty_vorbis/) advertises a
  dependency-free Rust encoder. Its source/provenance has not been audited here,
  and it is not selected or approved by this review.

The repository's [dependency authorization rule](../../AGENTS.md) requires
explicit approval for specific additions. Approval covers exactly the graph
above; further dependency changes require their own authorization. Build success
does not waive the remaining conformance, quality, timing and publication checks.
