# Rust audio delivery codec review

Source review and authorized integration dated 2026-09-24 for migration A1.04.
The user approved the audited eight-package graph; the project now pins
`oxideav-mp3 =0.1.3` and `oxideav-vorbis =0.0.12`, with the exact transitive
versions below. Downloaded archives match the reviewed checksums, existing
dependency versions are preserved, and license notices are included in the
source package. Conformance, fidelity and timing remain separate acceptance
gates; the production command surface still exports WAV only.

## Reviewed graph

The MP3 package depends on `oxideav-core`; Vorbis depends on that core and
`oxideav-ogg`. The core adds `bytemuck`, `thiserror` and the existing
`serde_json`. `thiserror-impl` requires Syn 3, in addition to the existing Syn 2
used by Serde. Syn 3 reuses the already-locked `proc-macro2 1.0.106`,
`quote 1.0.46` and `unicode-ident 1.0.24`. Do not silently upgrade existing
packages or enable optional Bytemuck derive/rustversion features.

| New registry package | License in published archive | SHA-256 of reviewed archive |
| --- | --- | --- |
| [oxideav-mp3-0.1.3](https://crates.io/crates/oxideav-mp3/0.1.3) | MIT | `8583bbbe119a19c0b88ee5b0f76c90493395350878e7c770ce6402287b164477` |
| [oxideav-vorbis-0.0.12](https://crates.io/crates/oxideav-vorbis/0.0.12) | MIT | `08878d1ec255f403363cad0a42273cbea90c01b817d49e45ad8ec13416b9e913` |
| [oxideav-core-0.1.36](https://crates.io/crates/oxideav-core/0.1.36) | MIT | `a78b502f679c580ceb7cc834d23f80821673b0a393a3083d54ef82668bb9c028` |
| [oxideav-ogg-0.1.8](https://crates.io/crates/oxideav-ogg/0.1.8) | MIT | `8a582af48d51a2a8d2d87958ca22a3a21f9029dcc828c1b4a5865fa00f952d3a` |
| [bytemuck-1.25.2](https://crates.io/crates/bytemuck/1.25.2) | Zlib OR Apache-2.0 OR MIT | `95832e849adfb21180ccb6826a99da14e5d266ae5c2e668e1602cf234f153797` |
| [thiserror-2.0.21](https://crates.io/crates/thiserror/2.0.21) | MIT OR Apache-2.0 | `09e52cb86a36cede5cb101bf8908837b3e4c6e5e59fe7fd85c23fb56200d189e` |
| [thiserror-impl-2.0.21](https://crates.io/crates/thiserror-impl/2.0.21) | MIT OR Apache-2.0 | `fe5197923287db20a58125f0bc85c062f7f2c892de97b18c356f9efb14b28524` |
| [syn-3.0.6](https://crates.io/crates/syn/3.0.6) | MIT OR Apache-2.0 | `8593e8e72159ed2257d083c7a454a85cbf854f37a0966d8d483aff8c8a3ebcee` |

Archive checksums match retained registry metadata. Every package includes
license text, retained with the published source and source-commit identity
under `out/audio-migration/codec-review/`. The review inventory records each
license-file hash, Cargo dependencies/features and source-scan findings in
`review.json`. Optional and development dependencies are not proposed runtime
dependencies. Cargo resolution still needs to confirm this exact graph.

No reviewed archive contains C/C++ sources or native libraries. The four
OxideAV packages, Bytemuck, Syn and thiserror-impl have no build script. The
thiserror build script writes an `OUT_DIR` module and probes the configured
Rust compiler; it does not build native code, fetch data or run audio tools.
Bytemuck's two `extern "C"` occurrences implement traits for function-pointer
types; they do not link or invoke a native library. The scan is source evidence,
not a claim of full functional or security verification.

The OxideAV packages declare Rust 1.80, thiserror/thiserror-impl 1.77 and Syn
1.71. Bytemuck's manifest declares no minimum compiler. All-target checks pass
on this project's Rust 1.88 minimum and pinned 1.98.1. Default tests pass
(362 passed, 144 media tests ignored), as do Clippy with warnings denied and
an independent locked build from the source package. All 16 audio harness
checks pass with the verified local Cargo cache, including packaged release
build and tests without proprietary media. Installation receipts,
feature inventory and logs are retained beside the source review.

## Integration and acceptance work

Use the direct Rust encoder APIs with explicitly validated PCM rate, channels,
quality and sample count. Keep synthesis and pre-lossy WAV comparisons separate
from encoding. A temporary verified file may be published only after structural,
timing and independent-consumer checks; no native or external production
encoder is permitted. FFmpeg may remain an independent test consumer.

The inspected MP3 release exposes Xing/Info frame counts, byte counts, TOC and
quality, but `XingTagSpec` does not expose encoder-delay or end-padding fields.
Do not infer gapless output from the current upstream README. Measure delay,
implement the required signaling/reporting, and verify exact presentation
length including very short clips and final partial frames. Vorbis uses its
own Ogg framing and terminal granule positions; verify those independently.

Required evaluation includes mono/stereo, silence, impulses, noise, short and
non-frame-aligned clips, representative BOF3 music/SFX/voice PCM, malformed
settings and failed publication. Compare pre-lossy quality, decoded duration,
encoder delay/padding and deterministic output on the pinned compiler. Confirm
independent MP3/Vorbis decoding and package builds before enabling delivery
formats. Fix or reject unsuitable codec behavior explicitly; dependency approval
alone is not production acceptance.

## Initial independent consumer probe

The initial development-only `examples/encoding.rs` retained input PCM, encoded bytes,
settings, hashes, timings and errors. It uses the MP3 outer loop with the High
preset at 192 kbit/s and Vorbis quality 0.7. It does not implement delivery
publication or claim timing acceptance. FFmpeg 6.1.1 is only a test consumer.
The first probe covers mono/stereo clips of 1, 100, 1152 and 1153 frames plus
8193-frame silence, impulses, tones and seeded noise, all at 44100 Hz:

- All 16 Vorbis outputs decode to the original frame count. Non-silent longer
  fixtures align at zero delay. Stereo impulse and tone cases emit a consumer
  warning about three Vorbis modes; successful decoding does not resolve that
  compatibility warning. Independent container/mode validation remains open.
- Six MP3 outputs containing only one audio frame fail this consumer's input
  probing. Longer signals show a 1057-frame offset. For 8193 input frames the
  output contains 9216 decoded frames, leaving only 8159 after alignment: the
  final 34 input frames cannot be compared. Encoder tail flushing and timing
  metadata therefore need implementation before delivery acceptance.
- Aligned MP3 noise error is substantial at these settings: about 17.8 dB SNR
  for mono and 7.6 dB for stereo. These measurements cover only the overlapping
  samples and do not excuse the missing tail. Quality selection remains open.

The reviewed encoder's analysis/MDCT checks describe a 481-frame filterbank
delay plus one 576-frame granule, consistent with the measured 1057. The
[FFmpeg 6.1.1 MP3 reader](https://github.com/FFmpeg/FFmpeg/blob/n6.1.1/libavformat/mp3dec.c)
requires a following frame while probing and recognizes delay fields for
specific encoder identifiers. Its recognized tags add 529 decoder frames to
the reported encoder delay. A future tag writer must identify the actual
encoder; do not impersonate another implementation to satisfy a consumer.
Retained fixtures, decoder logs, PCM and metrics are under
`out/audio-migration/codec-review/probe-*` and `probe.json`.

Three further 8193-frame clips use retained BOF3 music, SFX and XA voice PCM.
Their Vorbis outputs decode to the input lengths. Music and SFX reproduce the
MP3 length issue; the 37800 Hz voice input is explicitly rejected by the MP3
encoder. Delivery requires a verified Rust rate conversion or a precise
unsupported-rate diagnostic. `game-probe.json` binds each source hash and clip
offset. These probes are not a runtime fidelity oracle or full codec acceptance.

## Rust timing adapter and consumer checks

`src/codec/mp3.rs` now adds 1152 zero input frames to flush filter history,
validates emitted frame boundaries/counts, and writes an Info carrier with
frame/byte counts and the 36-byte gapless extension. It records 528 encoder
delay frames, separately reports 529 decoder frames, and derives end padding
from the original PCM length. The extension identifies the actual codec as
`OxAV0.1.3`; its audio/tag CRCs use CRC-16/IBM. The binary layout follows the
[published tag writer](https://github.com/lameproject/lame/blob/master/libmp3lame/VbrTag.c).
No third-party C implementation is included in production.

The Rust-only ignored consumer test dynamically loads the already installed
mpg123 library on Linux. Its [tag reader](https://github.com/libsdl-org/mpg123/blob/master/src/libmpg123/parse.c)
accepts this encoder identifier. At 192 kbit/s, all 36 combinations of three
rates (32000/44100/48000), mono/stereo, and six lengths (1/100/576/1152/1153/8193)
decode to exactly the requested length with both boundary impulses retained.
The expanded matrix exposed a published-source defect: `stream_encoder.rs`
allocates 4508/5660 bits per granule at 32000 Hz mono, 256/320 kbit/s, exceeding
the 4095-bit `part2_3_length` field. `main_data.rs` does not reject overflow;
the side-information writer truncates it. The retained one-frame 256-kbit/s
fixture produces a mpg123 dequantization error. The user chose to keep the
registry source unchanged; the proposed vendoring patch remains unapplied.
The Rust adapter now rejects both configurations before encoding, reporting
the requested settings and bit budget without substituting a bitrate.

The updated 144-case matrix rejects those 12 rate/channel/length combinations
and decodes the remaining 132 to their exact lengths. One amplitude gate still
fails: 48000 Hz stereo, 8193 frames, 128 kbit/s, final channel-1 impulse is 1905
against a threshold greater than 8000 (input 16000). All other tested boundary
impulses pass. This is unresolved fidelity evidence; neither exact lengths nor
the configuration guard establish acceptance of all payloads. The retained
failure is `mp3-48000-stereo-8193-128.mp3`; the complete Rust test receipt is
`mp3-registry-matrix.log`, both under `out/audio-migration/codec-review/`.
FFmpeg 6.1.1 still ignores the truthful encoder's gapless extension; this is a
consumer limitation, not permission to impersonate another encoder.

The Rust MP3 adapter now validates coded-data bounds as well as frame headers
before returning output. `codec/mp3/stream.rs` checks constant rate/channel/
bitrate, complete MPEG-1 frames, available and nonoverlapping reservoir history,
total granule lengths, scalefactor lengths and complete big-value Huffman symbols
within each granule/channel's declared budget. The published decoder's big-value
loop does not enforce that last boundary; the adapter checks its consumed bit
position explicitly. This catches a Rust-generated reproduction of the original
12-bit overflow even when bypassing the separate configuration guard. Four Rust
tests cover that reproduction, malformed budgets/reservoirs, truncation, format
changes and valid raw/tagged streams. Count1 padding, gapless metadata and PCM
fidelity remain separate checks; this is not an independent complete decoder.

`examples/quality.rs` compares the unchanged registry's Fast, Standard, High and
Transparent presets against the installed mpg123 consumer, with an explicit
1057-frame alignment and no per-clip delay optimization. The retained impulse,
music, SFX and noise comparisons produce identical PCM and payload hashes across
the four presets at each bitrate. Thus a preset change does not resolve the
boundary failure. At 192 kbit/s, the short music clip measures 10.63 dB SNR per
channel, the active SFX channel 14.55 dB, and stereo noise 7.62/7.59 dB. These are
measured errors against pre-encoding PCM, not quality acceptance thresholds or
proof of perceptual transparency. The original runtime clips are not independent
PSX fidelity oracles. Reports, complete settings and hashes are retained as
`quality-{impulse,music,sfx,noise}.json` under the codec-review output directory;
the stricter payload-validated impulse repeat is `quality-validated.json`.

The installed Xiph `oggdec 1.4.2` / libVorbis 1.3.7 independently decodes all
19 retained Vorbis synthetic/game clips to the expected lengths. The
[Vorbis specification](https://xiph.org/vorbis/doc/Vorbis_I_spec.html) permits
multiple modes; the earlier FFmpeg three-mode warning does not establish an
invalid stream. The Rust Vorbis adapter additionally checks single-stream Ogg
capture/version, flags, serial/page sequence, lacing, CRCs, header signatures,
identification fields, monotone granules and final presentation length using
the [Ogg framing specification](https://xiph.org/ogg/doc/framing.html). This
inspector does not decode setup/audio packets or establish perceptual quality.
The Rust Xiph consumer matrix passes all 60 combinations of 18900/32000/37800/
44100/48000 Hz, mono/stereo and the six lengths above at quality 0.7, with exact
decoded lengths and both boundary impulses retained (`ogg-tests.log`). Broader
corpus quality and delivery publication remain open. Earlier receipts are `timing-rust-tests.log`,
`xiph-probe.json`, and the earlier 36-case timing result under
`out/audio-migration/codec-review/`.

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
above; further dependency changes require their own authorization. The original
`review.json` remains a historical pre-installation record; `installation.json`
records the authorized checksum-verified graph. Build success does not waive
the remaining conformance, quality, timing and publication checks.
