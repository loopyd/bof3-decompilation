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
harness permute <source-or-function-id> [--iterations 100] [--timeout 300] [--seed 0] [-j JOBS] [--json]
harness adopt <candidate> --function <function-id> --apply
```

`harness permute` first compiles the generated `base.c`, then runs a bounded,
deterministic set of candidates. The defaults are 100 iterations, a 300-second
shared deadline, seed 0, and one worker. Use `--json` for bounded run metadata;
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
harness ghidra sync
harness assets list
harness disk verify|rebuild
```
