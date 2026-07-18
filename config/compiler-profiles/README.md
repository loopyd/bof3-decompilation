# Compiler profiles

Each profile under `config/compiler-profiles/<name>/` describes the compiler
configuration for one source group or target family. The build system selects
a profile with `PROFILE=<name>` (default: `compat/capcom97`).

## File layout

```
profile/
└── default.mk      # Flags applied to every source in the profile.
```

Per-source-group overrides (`exe.mk`, `psyq.mk`, `overlays.mk`, `exceptions.mk`)
are not yet wired. The Makefile supports them via `CFLAGS_exe`, `CFLAGS_psyq`,
`CFLAGS_emi`, and `CFLAGS_exceptions` target-specific variables once the
corresponding `.mk` files exist; until then every source uses `default.mk`
flags.

## Adding a new profile

1. Copy an existing profile directory, or create from scratch.
2. Set flags matching the evidence for that source group in `default.mk`.
3. Test with `make PROFILE=<name>`.

## Current profiles

### `compat/capcom97`

Capcom PS1 (BOF3) configuration. Applied via PsyQ-GCC 2.7.2 compatibility
wrappers (`bin/cc`, `bin/as`). Default flags:

| Flag | Value |
|------|-------|
| Optimization | `-O2` |
| Small data | `-G0` |
| Char signedness | `-funsigned-char` |
| Floating point | `-msoft-float` |
| Debug format | `-gcoff` |
| ASPSX version | 2.56 |

All source groups currently inherit these defaults. Split into per-group
overrides when evidence requires different flags for a specific source group.

## Evidence requirements

Before splitting a new profile or adding per-file exceptions, document:

- compiler version
- optimization level
- `-G` value
- `signed-char` / `unsigned-char`
- `split-address` behavior
- COMMON-section behavior
- ASPSX version
- MASPSX transformations applied
- special assembler flags

Record the evidence source (decomp.me preset, known project, object metadata,
debug strings, controlled compilation experiment).
