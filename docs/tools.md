# Reverse-engineering tools

> Each tool has one role; original bytes and canonical Splat assembly remain
> authoritative.

| Tool | Role | Required |
| --- | --- | --- |
| Splat/spimdisasm | Split PSX binaries and produce canonical assembly | Yes |
| historical GCC | Reproduce the BOF3 compiler family | Yes |
| MASPSX/binutils | Produce inspectable matching ELF objects | Yes |
| `emi-ex` | Extract BOF3 Capcom EMI containers | Yes |
| asm-differ | Interactive instruction comparison | Yes |
| m2c | Produce a matching-oriented C seed | Optional |
| Rizin/rz-ghidra | Fast local CLI analysis | Optional |
| Ghidra | Persistent deep analysis and manual review | Optional |
| decomp-permuter | Search late-stage source variants | Optional |

Pinned repository tools live under `third_party/`; generated installations and
SDKs live under `toolchains/`. See `tools.lock.toml` for the local role of each
submodule.

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
facts. Use `PSX_CC_DRIVER` for a one-off compiler driver or `PSX_C_COMPILER`
when configuring a separate CMake build directory.

## Comparative projects

- [`sozud/mmx4`](https://github.com/sozud/mmx4) is a work-in-progress
  decompilation of Capcom's *Mega Man X4* for PlayStation. Its repository,
  extraction, splitting, build, and diff patterns are a useful nearby reference
  when evaluating BOF3 workflow or source organization.

Treat comparative projects as design and tooling references only. Their binary
layout, compiler flags, SDK assumptions, symbols, and runtime behavior are not
evidence for BOF3 unless BOF3's own binaries independently confirm them.
