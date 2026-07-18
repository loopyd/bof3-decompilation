# Command surface

`bin/cc` and the adjacent binutils wrappers are the only compiler adapters.
`bin/build [all|TARGET|TARGET@0xADDRESS|clean]` provides the CMake build frontend and
uses Ninja when available. The Makefile remains a compatibility frontend and
produces the same `build/src/` objects. A TARGET selection compiles its authored
objects; it does not relink a complete target image.
All repository workflow commands are target-qualified focused tools documented
in the root README. Run each command with `--help` or `--example`.
Context-heavy commands use `--detail minimal|normal|full`; see the ordered
[tool usage](../docs/usage.md) guide. Full evidence remains under `out/`.
