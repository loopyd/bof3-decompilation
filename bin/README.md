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

CMake supplies the verified BOF3 compiler and assembler flags. Both
`just build` and the default build preset use the native build tool's full
parallelism. Configure a separate compiler experiment with:

```sh
cmake --preset default -DPSX_C_COMPILER=/path/to/cc
cmake --build --preset default
```

`harness` is the BOF3 command surface:

```sh
harness setup [options]
harness discover
harness targets [target] [--json]
harness promote <archive#slot> --confirm-code
harness reverse <target[@address]> [--run]
harness diff <source-or-function-id>
```

Disc discovery and asset operations are also available:

```sh
harness doctor [--strict]
harness assets list|str validate|str convert
harness psyq import
harness emi unpack
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
