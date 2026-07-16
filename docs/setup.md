# Setup and tools

Set up the supported local BOF3 toolchain. Game media, staged SDK files, and
all generated output remain untracked.

## Quick path

```sh
just setup
just doctor
just binaries
```

`just setup` initializes retained submodules, stages the supported PSX/PsyQ
compatibility toolchain, and builds the first-party `bof3-disk` and `emi-ex`
tools. It does not create a disc image, unpack every archive, or link a
reconstructed executable image.

The Psy-Q object-signature scan additionally needs its pinned database:

```sh
git submodule update --init
bin/harness psyq scan --all
bin/harness psyq calls --all
```

This is the only supported `bin/harness` adapter. It does not replace the
focused command surface or restore the former general harness workflow.

Place user-owned US BIN/CUE media under `inputs/disc/` when using the read-only
disc tools. Never commit media, `build/`, `out/`, or `toolchains/`.

## Command contracts

Repository-wide recipes are:

```text
just setup doctor binaries build check format index clean
```

Focused tools are:

```text
bin/splat        bin/bof3-disk     bin/emi-ex        bin/psyq-import
bin/psyq-find    bin/symbols       bin/rz-project    bin/rev-query
bin/m2ctx        bin/m2c           bin/asm-diff      bin/byte-match
bin/permute      bin/flag-search   bin/promote       bin/decomp-status
bin/str-media
```

Run `--help` or `--example` for exact operands. Focused tools use stdout for
results, stderr for diagnostics, no pager or color when non-interactive, and
exit 0 for success, 1 for a valid negative result, and 2 for usage/config/tool
errors. Mutating commands require `--write`.

## Tool roles

| Tool | Role |
| --- | --- |
| Splat | Split the mapped binary into generated assembly. |
| `bin/cc` and PSX binutils adapters | Compile C90 source with the supported compatibility profile. |
| `bin/bof3-disk`, `bin/emi-ex` | Read-only disc extraction and EMI inspection/extraction. |
| `bin/str-media` | Inspect, validate, and convert STR/XA media. |
| `bin/symbols` | Validate, normalize, import, and generate disposable weak bindings. |
| `bin/psyq-find` | Produce read-only PsyQ archive provenance evidence across available SDK archives and targets. |
| `bin/harness psyq` | Match pinned complete Psy-Q objects and join matches to Rizin call evidence. |
| Rizin and `bin/rz-project` | Build isolated, reproducible target analysis evidence. |
| `bin/asm-diff`, `bin/byte-match` | Compare compiler output by instruction and raw bytes. |
| m2c and `bin/permute` | Produce and refine a one-function C candidate. |
| `bin/decomp-status` | Recompute exact/partial/invalid lift status and report supplementary index coverage. |

PsyQ 4.7 is the build-facing header baseline, not proof of the game’s SDK
provenance. `bin/psyq-find` scans staged archive members; `bin/harness psyq`
scans the pinned JSON object-signature database. Review either result before
using `bin/symbols import-psyq --write` to change a target-local map; never
infer an SDK address in another target.
The signature adapter scans every pinned release from 2.60 through 4.70 and
retains all matching versions plus per-target best-compatible-version evidence;
it writes disposable `out/psyq/index.json` and `out/psyq/calls.json`. See
[Psy-Q signatures](reverse-engineering.md#psy-q-signatures).

## Checks

`just check` runs the small Python quality suite, `bin/symbols check`, and the
target-qualified link/diff audit for every retained C file.
Use a function’s `bin/asm-diff` and `bin/byte-match` while iterating; no
full-image rebuild or verification command is supported. Use
`bin/decomp-status [TARGET...]` when the complete current lift report is needed;
valid partial lifts are reported without failing the command.
