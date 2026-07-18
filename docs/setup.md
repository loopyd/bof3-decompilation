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

Repository recipes:

```text
just setup doctor binaries build check format index clean
```

Focused tools:

```text
bin/splat       bin/bof3-disk  bin/emi-ex       bin/emi-target
bin/str-media   bin/symbols    bin/psyq-import  bin/psyq-find
bin/rz-project  bin/rev-query  bin/m2ctx        bin/m2c
bin/asm-diff    bin/byte-match bin/permute      bin/flag-search
bin/promote     bin/decomp-status
```

Use `--help` or `--example`. Commands write results to stdout and diagnostics
to stderr. Exit codes are `0` success, `1` valid negative result, and `2`
usage/configuration/tool failure. Mutations require `--write` or `--apply`.

## Verification

- `just check`: Python checks, symbol maps, and every retained C lift.
- `bin/asm-diff` and `bin/byte-match`: one function while iterating.
- `bin/decomp-status [TARGET...]`: complete exact/partial/invalid lift report.

See [matching](matching.md) and [Rizin evidence](reverse-engineering.md).
