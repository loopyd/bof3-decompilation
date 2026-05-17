# AGENTS.md - rebof3-simple

## Files & Organization

```
bof3/
├── include/bof3/
│   ├── defines.h          → u8, s16, bool, ARRAY_COUNT, volatile typedefs
│   ├── symbols.h          → SYMBOL_AT, DATA_AT, DEFINE_FUNC_AT macros
│   ├── compiler.h         → BOF3_NO_SIBLING_CALLS, compiler pragmas
│   ├── context.h          → shared context: includes symbols.h, psyq_compat.h,
│   │                         DEFINE_FUNC_AT entries for not-yet-lifted BOF3 funcs
│   └── psyq_compat.h      → PsyQ header includes + #define aliases
│                             (e.g. #define ChangeTh func_8017edbc)
├── src/
│   ├── boot/
│   │   ├── startup.s      → PS1 entry point
│   │   └── symbols.c      → monolithic weak symbols (SYMBOL_AT for all
│   │                         unresolved data/funcs/PsyQ addresses)
│   ├── core/
│   │   ├── disc/          → SLUS_004.22: slot table, disc exec, draw
│   │   ├── callback_scheduler/
│   │   ├── emi/           → EMI streaming engine
│   │   └── game_front/
│   └── modules/
│       ├── logo/          → LOGO.EXE
│       ├── battle/03/     → BATTLE.EMI#3 overlay
│       ├── game/00/       → GAME.EMI#0 overlay
│       └── world00/areaXXX/NN/  → WORLD00.AREAXXX.EMI#NN overlays
│
│   Each leaf directory has:
│   ├── internal.h         → structs, enums, volatile macros, extern funcs
│   └── func_XXXXXXXX.c    → one lifted function per file
└── cmake/
    ├── sources.cmake       → per-module source file lists
    └── modules/*.cmake     → artifact definitions (raw .bin targets)
```

**Where things live:**

- Weak symbols → `src/boot/symbols.c` (uses `SYMBOL_AT` from `symbols.h`)
- Shared macros → `include/bof3/symbols.h`, `compiler.h`, `defines.h`
- PsyQ aliases → `include/bof3/psyq_compat.h`
- Per-module structs/defines/externs → `internal.h` in each source directory
- Not-yet-lifted BOF3 funcs → `include/bof3/context.h` (remove as lifted)
- Each function → its own `func_XXXXXXXX.c` file, self-contained via `#include "internal.h"`

## Scope

- Work inside `rebof3-simple/`.
- Do not edit the sibling `rebof3/` tree unless the user explicitly asks.
- Use `bin/` as the maintained command surface.
- Keep `make` limited to setup, extraction, inventory, Ghidra bootstrap, build,
  test, and formatting.

## Setup Checks

Use these commands to confirm the repo is ready:

```bash
make doctor
make extract
make inventory
make ghidra
make configure
make build
bin/doctor --strict
```

## Files

They are gitignored under inputs/and out.

Extract the disk (and all files)
Make sure we have the slus/logo and emi files.
To work with emi files, we need to unpack them.
out/ should have a folder for the full disk extracting with the same matching folder structure (all bin files, emi files, and unpacked emi files)
For code bearing in EMI files, use the matching \*.bin in that module folder.

## Decomp Loop

Work one function at a time:

```bash
bin/asm-diff-one bof3/src/core/emi/func_80162178.c
```

Read the generated outputs under `out/asm-diff/<function>/`, edit the matching
source/header, then rerun the same command.

Prefer visible source or header definitions over linker-script symbol patches.
Use `bof3/include/bof3/context.h` only for not-yet-lifted original calls
and shared decomp context; remove original-call entries as functions are lifted.
Do not reverse/decomp functions from psyqheaders, they already exist in psyq. Rather have include and use macros to match the offsets of the functions.
Do not lift/reverse functions that are from psyq/psyq library.
Investigate code, offsets, variables, functions and verify with ghidra/rizin. (Ghidra has more context)

## Guidelines

- C89
- Use `REG32()`, `REG16()`, `REG8()` macros from `defines.h` for hardware register access
- No new headers in `bof3/include/bof3/modules/` — put ALL function declarations in `internal.h`
- Each module source directory MUST be listed in `BOF3_ALL_MODULE_SOURCES` in `sources.cmake` so every `.c` gets an individual `.obj` target for fast asm-diff
- For new overlay modules, add a `_OVERLAY_BINARIES` entry in `tools/python/rebof3/match/asm_diff.py` mapping source path prefix → (binary path, load address)
- New PLACEHOLDER modules (zero functions yet) don't need DECLARED_SOURCES in `bof3_define_module_artifact`
- Readable > Complicated Code
- Be good enough to match almost 100%, if gets complex, a readable, clean code, with comments is better than forcing a 100% match that is not understable.
- Use patterns for the equivalent time of development PS1 C, and good practices.
- Code should be clean, beautiful, readable.
- Prefer promoting with m2c the reverse assembly or file, and try to permut before forcing micro optimizations.
- Prefer having a self contained C file, with externs to avoid redefining external functions, and defines for magic addresses. once we have a good enough matching C code, you can start promoting things slowly.

## Verification

Before handing work back, run the smallest relevant checks plus:

```bash
bin/build
bin/doctor --strict
PYTHONDONTWRITEBYTECODE=1 .venv/bin/python -m pytest -q -p no:cacheprovider tools/python/tests
```

## Guidelines

Do not use hardcoded values or addresses.
Do not use assembly to fill in the functions. I rather have defines for offsets or extern.
Prefer using defines, or figuring out structs. Verify all values with ghidra or assembly xrefs.
Prefer readable C code that tries to have the highest matching percentage.
Use m2c, build as much context as possible (should be in harness or script)
C files should be self contained (per function), and once structs,xrefs,variables are figured out, slowly promote across internal.h or other symbol/headers to be reusable.

Use simple names (semantic, relevant, logic, organic), that match canonical C development and style for PSX development or Reverse Engineering.

Prefer Readable pleasant code > complicated. Use defines,macros, or other ways to try to get uncomplex and clean code.
It's ok for now using the same jump tables, offsets, but in the end the goal is to have 100% matching C code to the assembly, that will compile to the same thing (without magic values (?))
Consider each module with code bearing, and try to match the code for each module. (Promote 100% duplicated functions to a common module)
If needed (check xrefs) and mark variables or structs as extern.

## Durable Patterns (lessons that outlive specific build systems)

### Function declarations
- **One declaration source per module**: ALL function declarations live in `internal.h` in the module's source directory. Never create parallel headers in `bof3/include/bof3/modules/` — they will get out of sync and cause conflicting-types errors.
- **`DEFINE_FUNC_AT` for cross-module calls**: Generates direct `jal` (matching). The old `#define func_XXXX FUNC_AT(...)` pattern generates `jalr` (non-matching). Remove `DEFINE_FUNC_AT` entry from `context.h` once the function is lifted.

### PsyQ / Compiler
- **No `#define` PsyQ aliases in `psyq_compat.h`**: GCC 2.7.2 for PSX does not apply preprocessor `#define` after including PsyQ headers — relocations show `R_MIPS_26 ChangeTh` not `R_MIPS_26 func_8017edbc`. Use bare PsyQ declarations + `SYMBOL_AT` weak symbols in `symbols.c`.

### Decomp loop
- **One function = one .c file = one .obj**: enables independent `bin/asm-diff-one` per function. Parallel subagents can work on different functions simultaneously without build conflicts.
- **Size inference**: when only one function exists in a module directory (no sibling source to infer size from), the size is looked up from the Ghidra function index `body_min`/`body_max`.
- **Readable C over 100% match**: when matching gets too complex or magical, clean readable code is preferred. Micro-optimizations can come later via permuter.

### Parallel agents
- **No shared-file conflicts**: each agent works on its own `func_XXXXXXXX.c` files. The only shared files are `sources.cmake` (source list additions) and `internal.h` (type declarations). Merge these carefully.
- **Each agent validates independently** with `bin/asm-diff-one` before running `make build`.
