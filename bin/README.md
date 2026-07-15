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
harness target list|show|doctor
harness index build
harness find <term>
harness show <id>
harness related <id>
harness graph <id>
harness profile list|show|resolve
harness context build|show <target> <function>
harness lift <target> <function>
harness diff <source-or-function-id>
harness permute <source-or-function-id> [--iterations 100] [--timeout 300] [--seed SEED] [-j JOBS] [--json]
harness adopt <candidate> --function <function-id> --apply
```

`harness permute` first compiles the generated `base.c`, then runs a bounded set
of candidates. The defaults are 100 iterations, a 300-second shared deadline, a
reported system-random seed, and one worker. Pass `--seed SEED` to reproduce a
run. Use `--json` for bounded run metadata, including the effective seed;
exit 0 means an improvement, 1 means a clean run with no improvement, and 2
means setup, timeout, or permuter failure. Candidate source and full logs remain
under the reported `out/matching/.../permuter` bundle.

Disc discovery and asset operations are also available:

```sh
harness scan
harness status [target]
harness candidates [family]
harness promote <archive#slot> --confirm-code
harness next [target]
harness flags <source>
harness analysis doctor
harness analysis init <target>
harness analysis generate [target]
harness analysis hotspots [--kind leaves] [--top 10] [--min-callers N] [--max-out N] [--min-size N] [--max-size N] [--status reviewed|lifted|unreviewed|unlifted] [--sort callers]
harness analysis graph
harness analysis query <target> <query>
harness ghidra sync
harness assets list
harness assets str validate <path> [--expected-fps FPS] [--json]
harness assets str convert <path> --fps FPS [--output PATH] [--json]
harness disk verify|rebuild
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
