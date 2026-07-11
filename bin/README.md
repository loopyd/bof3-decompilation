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

`rebof3` is the BOF3 command surface:

```sh
rebof3 scan
rebof3 status [target]
rebof3 candidates [family]
rebof3 promote <archive#slot> --confirm-code
rebof3 next [target]
rebof3 lift <target@address>
rebof3 diff <source>
rebof3 flags <source>
rebof3 ghidra sync
rebof3 assets list
rebof3 disk verify|rebuild
```
