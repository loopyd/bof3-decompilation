# Commands

The compiler and binutils commands are thin, native-style PSX tool adapters:

```sh
bin/cc -O2 -Iinclude -c source.c -o source.o
bin/ld -T linker.ld source.o -o program.elf
bin/objdump -dr source.o
```

Repository-local tools are the defaults. Override a complete compiler driver
with `PSX_CC_DRIVER`, or individual tools with `PSX_GCC`, `PSX_AS`, `PSX_LD`,
`PSX_AR`, `PSX_RANLIB`, `PSX_NM`, `PSX_OBJCOPY`, and `PSX_OBJDUMP`.

The Makefile supplies the verified BOF3 compiler and assembler flags. `just build`
intentionally compiles serially because historical PsyQ compilation is not
output-race safe. Select a compiler profile with:

```sh
PROFILE=compat/capcom97 make all
```

The `original/psyq36` and `original/psyq40` entries are candidate metadata
only. They remain disabled until their native compiler, assembler, and wibo
runtime are staged; Make rejects those profiles rather than silently using the
compatibility compiler.

`bin/harness` is the BOF3 command surface:

```sh
bin/harness setup [options]
bin/harness discover
bin/harness targets [target] [--json]
bin/harness promote <archive#slot> --confirm-code
bin/harness reverse <target[@address]> [--run]
bin/asmdiff <source-or-function-id>
```

`reverse --run` resolves one function and launches a bounded OpenCode mission.
Without `--run`, it only previews the selected work. Focused workflows are
standalone entry points and do not require the command dispatcher:

```sh
bin/asmdiff src/exe/logo/func_801ce758.c
bin/permute src/exe/logo/func_801ce758.c --prepare-only
bin/check-all --target exe/logo
bin/progress
bin/splat split config/splat/exe/logo.yaml
bin/rebuild exe/logo --allow-nonmatching
bin/verify exe/logo --allow-nonmatching
```

`rebuild` writes under `out/rebuilt/` and intentionally zero-fills unmatched
regions until full source-only linkage is available. `verify` requires
an exact byte, length, and SHA1 match; the transitional rebuild is expected to
fail that check until every target region is reconstructed.

Dependency setup is split by responsibility:

```sh
bin/setup-toolchain
bin/setup-psyq
bin/setup-rust
bin/setup-wibo --download
```

Disc discovery and asset operations are also available:

```sh
bin/harness doctor [--strict]
bin/harness assets list|str validate|str convert
bin/harness psyq import
bin/harness emi unpack
```

STR validation preserves the extracted 2336-byte-sector source and writes a
2352-byte-sector FFmpeg wrapper plus JSON evidence under `out/assets/str/`.
Without `--expected-fps`, validation only reports observed structure and timing.
With an expected rate, the observational timing comparison allows two video
frames or two primary XA audio sectors. Conversion requires an explicit
frame rate and writes a widely supported lossless H.264/FLAC Matroska file. It
keeps the decoded dimensions and chroma layout, performs no scaling, and uses
frame-index timestamps so conversion neither drops nor duplicates frames. The
extracted 2336-byte STR remains the canonical source asset.
When the primary XA audio ends before the requested video duration, conversion
appends the calculated number of silent samples to that decoded audio stream;
it reports the padding in `conversion.json` and leaves other XA channels
separate.
