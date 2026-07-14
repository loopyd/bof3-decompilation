# r2pipe automation

## Contents

- [Bindings and Python API](#official-binding-ecosystem)
- [Transport choices](#transport-choices)
- [PSX automation](#high-value-psx-automations)
- [Safe batch shape](#safe-batch-shape)
- [JSON and token discipline](#json-validation-and-token-discipline)
- [Errors and Rizin compatibility](#error-and-lifecycle-rules)

Use the native CLI for focused interactive analysis and one-shot commands. Use
r2pipe when the job needs typed JSON parsing, iteration across many independently
mapped blobs, correlation, validation, deterministic export, or regression tests.

## Official binding ecosystem

The official `radare2-r2pipe` repository contains bindings or examples for many
languages, including Python, Rust, Go, TypeScript/Node.js, C/C++, Java, Ruby,
Swift, Zig, and others. Presence in the repository is not a promise of equal
maturity; verify the selected binding, release, and installed radare2 version.

Choose by repository pressure:

| Binding | Prefer when |
| --- | --- |
| Python `r2pipe` | Existing Python harness, fast scripting, JSON correlation, tests |
| Rust | Long-running typed tooling where async/concurrency and strong models pay off |
| Go | Standalone concurrent scanner with simple distribution |
| Haskell/OCaml | Existing codebase or type-heavy analysis pipeline already uses it |
| Ruby | Existing Ruby automation; otherwise Python has stronger local fit here |

For this repository, Python is the default because the harness and its tests are
already Python. Do not add another language or a new package merely to wrap one
or two commands.

If `r2pipe` is absent, prefer the existing subprocess adapter over an
implicit install. Invoke an argument list (never a shell string) with explicit
raw mapping flags and one or more `-c` JSON commands, capture stdout/stderr and
timeout, validate JSON, then close the process. Use a persistent pipe only when
measured startup cost or stateful interaction justifies the extra lifecycle
surface.

## Python API

The official package is `r2pipe`:

```sh
python -m pip install r2pipe
```

Dependency installation is an explicit project decision; first check whether
the repository already declares it. Core use:

```python
import r2pipe

flags = [
    "-2",  # close inherited stderr after capability/preflight checks
    "-a", "mips",
    "-b", "32",
    "-e", "cfg.bigendian=false",
    "-e", "scr.color=0",  # stable JSON/text without ANSI escapes
    "-m", f"0x{load_address:08x}",
]
with r2pipe.open(binary_path, flags=flags) as r2:
    r2.cmd("aa")
    functions = r2.cmdj("aflj")
    info = r2.cmdj("ij")
```

The official Python implementation accepts `flags`, inserts them into the
spawned radare2 command before its own `-q0`, accepts `radare2home` for an
alternate executable directory, and implements a context manager that quits the
process. The installed synchronous API does not expose a command-timeout
parameter; enforce wall-clock bounds in the harness/process supervisor.
Its spawned process also inherits stderr. For stable routine automation, `-2`
prevents terminal-control/warning noise after a separate capability preflight;
for diagnosis, omit `-2` and use the subprocess adapter so stderr is captured to
an artifact instead of leaking into LLM/stdout output.

Disabling color is an automation boundary, not an interactive default. Native
terminal sessions should inherit the user's color configuration. Set
`scr.color=0` only for JSON, persisted evidence, tests, and LLM-bounded output,
where ANSI bytes add noise and can break parsing. Probe `e scr.color=?` before
depending on nonzero numeric modes; related color keys differ by engine and
version.

`cmd()` returns text. `cmdj()` parses JSON into Python values. The Python package
also documents `cmdJ()` named-tuple conversion, but plain `cmdj()` is preferable
for stable serialization and explicit schema validation.

Raw PSX architecture/base flags must be applied before trusting analysis. Verify
the installed binding API/version before relying on newer options, or
spawn/connect to an engine started with the verified CLI:

```sh
r2 -N -n -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xBASE RAW.bin
```

Never open a raw blob with default settings and repair addresses after analysis.

## Transport choices

The official r2pipe implementation supports these connection styles:

- spawned radare2 pipe: best local default and process isolation;
- HTTP: useful for remote/cloud-style queries, with explicit trust boundaries;
- TCP: supported by some bindings;
- RAP/native remote protocol: binding-dependent;
- asynchronous operation: explicitly documented in the Rust binding matrix.

Prefer one spawned engine per target/blob for deterministic local analysis.
Avoid sharing mutable analyzer sessions between parallel workers. Remote modes
expand security, lifecycle, and reproducibility concerns and are unnecessary for
ordinary repository analysis.

## High-value PSX automations

Python r2pipe can improve this workflow in these areas:

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
do not leave repeated one-off r2pipe scripts scattered around the tree.

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

## Rizin compatibility

`r2pipe` drives radare2, not Rizin. When the harness selects Rizin, use its
verified subprocess adapter or the separate `rzpipe` package behind the same
internal analyzer interface. Do not pass one engine's project/type commands to
the other. Rizin in-process Python plugins and Cutter plugins are separate
extension choices, not replacements for deterministic headless exports.

## Official sources

- Official r2pipe repository and binding inventory:
  https://github.com/radareorg/radare2-r2pipe
- Official Python implementation:
  https://github.com/radareorg/radare2-r2pipe/tree/master/python
- Official Python examples:
  https://github.com/radareorg/radare2-r2pipe/tree/master/python/examples
- Official radare2 r2pipe reference:
  https://book.rada.re/scripting/r2pipe.html
