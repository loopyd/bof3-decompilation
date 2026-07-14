# rzpipe automation

Use the native CLI for focused interactive analysis and one-shot commands. Use
rzpipe when the job needs typed JSON parsing, iteration across many independently
mapped blobs, correlation, validation, deterministic export, or regression tests.

## Available bindings

The official Rizin project advertises rzpipe bindings for six languages:
Python, Haskell, OCaml, Ruby, Rust, and Go. The current handbook feature matrix
explicitly details Python, Haskell, OCaml, and Rust and also provides a Ruby
example; verify the maintenance/version state of a chosen binding before adding
it to durable tooling.

Choose by repository pressure:

| Binding | Prefer when |
| --- | --- |
| Python `rzpipe` | Existing Python harness, fast scripting, JSON correlation, tests |
| Rust | Long-running typed tooling where async/concurrency and strong models pay off |
| Go | Standalone concurrent scanner with simple distribution |
| Haskell/OCaml | Existing codebase or type-heavy analysis pipeline already uses it |
| Ruby | Existing Ruby automation; otherwise Python has stronger local fit here |

For this repository, Python is the default because the harness and its tests are
already Python. Do not add another language or a new package merely to wrap one
or two commands.

## Python API

The official package is `rzpipe`:

```sh
python -m pip install rzpipe
```

Dependency installation is an explicit project decision; first check whether
the repository already declares it. Core use:

```python
import rzpipe

flags = [
    "-a", "mips",
    "-b", "32",
    "-e", "cfg.bigendian=false",
    "-m", f"0x{load_address:08x}",
]
with rzpipe.open(binary_path, flags=flags, cmd_timeout_secs=30) as rz:
    rz.cmd("aa")
    functions = rz.cmdj("aflj")
    info = rz.cmdj("ij")
```

The official Python implementation accepts `flags`, inserts them into the
spawned `rizin` command before its own `-q0`, accepts `rizin_home` for an
alternate executable directory, supports `cmd_timeout_secs`/`set_timeout()`, and
implements a context manager that quits the process. It launches `rizin`, not
radare2; use the separate `r2pipe` binding for a radare2 fallback.

`cmd()` returns text. `cmdj()` parses JSON into Python values. The Python package
also documents `cmdJ()` named-tuple conversion, but plain `cmdj()` is preferable
for stable serialization and explicit schema validation.

Raw PSX architecture/base flags must be applied before trusting analysis. Verify
the installed binding API/version before relying on newer options, or
spawn/connect to an engine started with the verified CLI:

```sh
rizin -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xBASE RAW.bin
```

Never open a raw blob with default settings and repair addresses after analysis.

## Transport choices

The official handbook documents these connection styles:

- spawned pipe (`rizin -0`): best local default and process isolation;
- HTTP: useful for remote/cloud-style queries, with explicit trust boundaries;
- TCP: supported by some bindings;
- RAP/native remote protocol: binding-dependent;
- asynchronous operation: explicitly documented in the Rust binding matrix.

Prefer one spawned engine per target/blob for deterministic local analysis.
Avoid sharing mutable analyzer sessions between parallel workers. Remote modes
expand security, lifecycle, and reproducibility concerns and are unnecessary for
ordinary repository analysis.

## High-value PSX automations

Python rzpipe can improve this workflow in these areas:

1. **Capability probe**: capture engine version, MIPS/32/LE settings, `pdg?`,
   project commands, and required JSON command schemas.
2. **Mapping verifier**: compare `pxj`/raw bytes at known addresses, maps, entry
   instructions, and target manifest bounds before analysis.
3. **Reviewed replay verifier**: import replay/types, reopen a project, and assert
   sentinel functions/flags/types/comments exist.
4. **Deterministic export**: collect `ij`, `aflj`, xrefs, flags, types, comments,
   and selected decompilation JSON; normalize and sort externally.
5. **Cross-binary correlation**: iterate separately mapped targets and compute
   exact, relocation-aware, instruction, CFG/call/data, and ABI evidence.
6. **PsyQ correlation**: join target-local calls/functions with SDK prototypes,
   archive/member fingerprints, constants, structs, and version provenance.
7. **Boundary auditing**: compare analyzer functions with reviewed Splat ranges
   and reject code inferred in headers/data/padding.
8. **Regression fixtures**: run a tiny PSX MIPS blob through the adapter and
   verify base, endianness, delay-slot decoding, replay, project reopen, and JSON.
9. **LLM-bounded views**: persist complete JSON while rendering only counts,
   ranked rows, omitted counts, and artifact paths to stdout.

These belong behind existing harness seams when they are repository workflows;
do not leave repeated one-off rzpipe scripts scattered around the tree.

## Safe batch shape

Model each target as immutable input and each analyzer process as isolated:

```text
load manifest -> verify input hash/base/bounds -> spawn engine
-> probe capabilities -> apply bounded reviewed analysis/replay
-> collect JSON -> validate schemas -> canonicalize/sort
-> write full artifact -> render bounded summary -> close engine
```

Use explicit timeouts, per-target artifact directories, stable ordering, and a
bounded worker count. Record failures per target rather than discarding a whole
repository scan. Never let one target's address→name map overwrite another's.

## JSON validation and token discipline

- Treat every `cmdj()` response as external/versioned data.
- Treat `None` from `cmdj()` as a parse/empty-result failure unless the command's
  contract explicitly allows no result; the official implementation prints its
  JSON parse diagnostic to stderr and returns `None`.
- Validate top-level type and required fields before use.
- Keep raw engine output and stderr in artifacts when diagnosing failures.
- Sort functions/flags by address and name; normalize integer formats.
- Persist full JSON; truncate only terminal/LLM rendering.
- Emit count, limit, omitted count, and artifact path for bounded views.
- Reject non-JSON output from a command expected to return JSON with the command,
  engine version, and a short diagnostic—not the full uncontrolled response.

## Error and lifecycle rules

- Always close/quit the engine in `finally` or a context wrapper.
- Capture startup and command errors; some engines return success while writing
  warnings/errors to stderr.
- Verify side effects such as project save, replay import, or type placement by
  querying a sentinel afterward.
- Hash the binary and replay/type inputs in metadata so stale projects fail fast.
- Do not auto-promote analyzer names or decompiler output from batch scripts.
- Use one engine process per target and avoid concurrent writes to one project.
- Avoid `syscmd()`/`syscmdj()` with constructed or untrusted strings: the
  official Python implementation invokes them through `shell=True`. Prefer
  Python subprocess argument lists at the harness boundary.

## Rizin plugins versus rzpipe

Rizin also supports in-process Python plugins via `rzlang`. Use a plugin only
when new engine commands or architecture/analysis behavior genuinely require an
in-process extension. For repository orchestration, rzpipe keeps failure and
version boundaries clearer and is easier to test. Cutter Python plugins are a
GUI integration choice, not a replacement for deterministic headless exports.

## Official sources

- Rizin rzpipe handbook and feature matrix:
  https://book.rizin.re/src/scripting/rz-pipe.html
- Rizin scripting overview:
  https://book.rizin.re/src/scripting/intro.html
- Official Rizin repository (advertised language bindings):
  https://github.com/rizinorg/rizin
- Official Python package:
  https://pypi.org/project/rzpipe/
- Official Python source and examples:
  https://github.com/rizinorg/rz-pipe/tree/master/python
  https://github.com/rizinorg/rz-pipe/tree/master/python/examples
- Rizin Python plugins (`rzlang`):
  https://book.rizin.re/src/plugins/python.html
- radare2 r2pipe reference for fallback automation:
  https://book.rada.re/scripting/r2pipe.html
