# Reverse-engineering tools

> Each tool has one role; original bytes and canonical Splat assembly remain
> authoritative.

| Tool | Role | Required |
| --- | --- | --- |
| Splat/spimdisasm | Split PSX binaries and produce canonical assembly | Yes |
| historical GCC | Reproduce the BOF3 compiler family | Yes |
| MASPSX/binutils | Produce inspectable matching ELF objects | Yes |
| Rust `emi-ex` | Canonical BOF3 EMI extractor through `bin/emi-unpack` | Yes |
| Rust `bof3-disk` | Canonical disc extraction/checksum tool used by setup | Yes |
| asm-differ | Interactive instruction comparison | Yes |
| m2c | Produce a matching-oriented C seed | Optional |
| Rizin/rz-ghidra | Fast local CLI analysis | Optional |
| Ghidra | Optional transient deep analysis and manual review | Optional |
| decomp-permuter | Search credible compiling source variants | Optional |

Pinned repository tools live under `third_party/`; generated installations and
SDKs live under `toolchains/`. See `tools.lock.toml` for the local role of each
submodule.

The canonical command binaries are pure Rust and retain the names `bof3-disk`
and `emi-ex`. Setup builds them with Cargo's lockfiles into
`build/tools/rust/`. `emi-ex` has byte-identical extraction evidence across all
880 archives on the BOF3 disc. `bof3-disk` is canonical for extraction and
checksums; its `rebuild` command is not yet a parity replacement for the legacy
mkpsxiso path and fails explicitly rather than producing an unverified image.

## Historical PsyQ toolchain

The default toolchain uses GCC 2.7.2, MASPSX configured for ASPSX 2.56
behavior, and staged PsyQ 4.7 headers and libraries. These versions
represent separate compiler, assembler, header, and library choices; “PsyQ
4.7” does not prove that BOF3 originally used the complete 4.7 SDK.

Before changing the compiler or SDK, compare candidates against several
small known BOF3 functions and linked PsyQ routines. Check instruction output,
calling convention, library signatures, object layout, and relocations. Keep
the current toolchain until an earlier available SDK produces stronger binary
evidence. PsyQ 3.x or 4.0 are candidates for investigation, not verified BOF3
facts. Use `PSX_CC_DRIVER` for a one-off compiler driver or `PROFILE=...` when
selecting the supported compatibility profile in Make. The original PsyQ
profile entries remain disabled until their native compiler, assembler, and
wibo runtime are staged.

## Comparative projects

- [`sozud/mmx4`](https://github.com/sozud/mmx4) is a work-in-progress
  decompilation of Capcom's *Mega Man X4* for PlayStation. Its repository,
  extraction, splitting, build, and diff patterns are a useful nearby reference
  when evaluating BOF3 workflow or source organization.

Treat comparative projects as design and tooling references only. Their binary
layout, compiler flags, SDK assumptions, symbols, and runtime behavior are not
evidence for BOF3 unless BOF3's own binaries independently confirm them.
