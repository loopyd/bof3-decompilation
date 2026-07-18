# Setup and tools

## Setup

```sh
just setup
just doctor
just binaries
```

`just setup` initializes retained submodules, prepares the PSX compatibility
toolchain, and builds first-party tools. User media belongs under
`inputs/disc/`; never commit it or generated `build/`, `out/`, or `toolchains/`.

PsyQ signature evidence additionally requires:

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

## Commands

The canonical ordered command guide is [tool usage](usage.md). This page owns
installation and verification only.

Repository recipes:

```text
just setup doctor binaries build check format index clean
```

Focused tools:

```text
bin/build       bin/splat      bin/bof3-disk    bin/emi-ex
bin/emi-target
bin/str-media   bin/symbols    bin/psyq-import  bin/psyq-find
bin/rz-project  bin/rev-query  bin/m2ctx        bin/m2c
bin/asm-diff    bin/byte-match bin/permute      bin/flag-search
bin/promote     bin/decomp-status
```

`bin/build` compiles every lift through CMake, selecting Ninja when available.
`bin/build TARGET` compiles all authored objects owned by one independently
loaded binary (a target without authored sources is a successful no-op), and
`bin/build TARGET@0xADDRESS` compiles one authored function. The compatibility
Makefile produces the same objects under `build/src/`; neither frontend links
separate BOF3 images together.

Use `--help` or `--example`. Commands write results to stdout and diagnostics
to stderr. Python analysis commands use `0` for success, `1` for a valid
negative match, and `2` for usage/configuration failure; native and upstream
tools may return their own codes. Mutations require `--write` or `--apply`.

## Verification

- `just check`: Python checks, symbol maps, and a full compile/link/compare of
  every retained C lift.
- `bin/asm-diff` and `bin/byte-match`: one function while iterating.
- `bin/decomp-status [TARGET...]`: complete exact/partial/invalid lift report.

See [matching](matching.md) and [Rizin evidence](reverse-engineering.md).
