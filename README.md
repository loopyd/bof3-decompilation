# BOF3 reverse-engineering workspace

One reproducible path for independently loaded BOF3 binaries:

```text
binary -> target map -> Splat/Rizin evidence -> C candidate
       -> compile -> generated weak bindings -> link at address
       -> asm diff + byte match
```

Original bytes and target manifests are authoritative. An EMI archive is a
container, not an analysis target.

## Quick path

```sh
just setup
just doctor
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS > candidate.c
# edit src/<target>/func_address.c
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
bin/permute TARGET@0xADDRESS --time-limit 300
bin/promote TARGET@0xADDRESS src/<target>/func_address.c
bin/decomp-status TARGET
```

`bin/promote` validates only; it never copies a candidate or changes reviewed
source, Splat layouts, or maps.
`bin/decomp-status` recompiles all tracked lifts in scope, reports each as
exact, partial, or invalid, and adds Rizin-index coverage when it is fresh.

## Ownership

| Fact | Owner |
| --- | --- |
| Binary identity, image, base address | `config/targets/<target>.toml` |
| Canonical target-local symbols | `config/symbols/<target>.txt` |
| Generated assembly | `out/splat/<target>/` |
| Generated weak bindings | `out/bindings/<target>/symbols.c` |
| Rizin replay/project/snapshot | `config/analysis/<target>/`, `out/rizin/<target>/`, `out/reverse/<target>/` |
| Cross-target query cache | `out/index/reverse.sqlite` |

Raw names are `func_80143B40` and `D_80143B40`: eight uppercase hexadecimal
digits. Proven semantic/PsyQ names replace raw map names directly.

## Commands

Repository-wide recipes are deliberately limited to:

```text
just setup doctor binaries build check format index clean
```

Focused tools are `bin/splat`, `bin/bof3-disk`, `bin/emi-ex`, `bin/psyq-import`,
`bin/psyq-find`, `bin/symbols`, `bin/rz-project`, `bin/rev-query`, `bin/m2ctx`,
`bin/m2c`, `bin/asm-diff`, `bin/byte-match`, `bin/permute`, `bin/flag-search`,
`bin/promote`, `bin/decomp-status`, and `bin/str-media`.

Run `--help` or `--example` on a focused tool for its accepted operands.

`bin/harness psyq` is the one retained harness adapter: it matches pinned Psy-Q
object signatures and joins those matches to Rizin call evidence. It is not a
general workflow command; see [Psy-Q signatures](docs/reverse-engineering.md#psy-q-signatures).

## Rizin and index

Each binary has its own generated Rizin recipe and snapshot. Never combine
overlapping load addresses in one session.

```sh
bin/rz-project rebuild TARGET
bin/rz-project analyze TARGET
just index
bin/rev-query hotspots
```

`just index` accepts only fresh, complete Rizin exports and atomically replaces
the SQLite cache. The previous complete index survives a failed rebuild.

See [matching](docs/matching.md), [Rizin/index workflow](docs/reverse-engineering.md),
and [setup/tools](docs/setup.md) for details.
