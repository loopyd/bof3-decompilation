# BOF3 context

This repository recovers *Breath of Fire III* (US, PlayStation) as readable
C89 from independently loaded executable bytes. It is not one link target.

## Identity

| Object | Identity | Owner |
| --- | --- | --- |
| PS-X executable | Shipped image and verified header load address | `config/targets/exe/*.toml` |
| EMI archive | Shipped container path | User media / `out/` extraction |
| EMI entry | Archive path plus slot | Extracted payload and its target manifest if known |
| Target | Exact payload, load address, and source directory | `config/targets/<target>.toml` |
| Function | `TARGET@0xADDRESS` | Owning target source, map, and Splat split |

Analyze an executable image or extracted EMI entry, never an archive container.
Identical payloads or addresses at different load locations remain separate
targets until relocatability is proven.

## Facts and generated evidence

| Fact | Durable owner |
| --- | --- |
| Binary identity, image, base address | `config/targets/<target>.toml` |
| Segment boundaries | `config/splat/` |
| Target-local symbols | `config/symbols/<target>.txt` |
| Reviewed Rizin replay | `config/analysis/<target>/` |
| C89 source | `src/exe/` or `src/emi/` |
| Generated assembly | `out/splat/<target>/` |
| Rizin project/snapshot | `out/rizin/<target>/`, `out/reverse/<target>/` |
| Weak bindings | `out/bindings/<target>/symbols.c` |
| Cross-target index | `out/index/reverse.sqlite` |
| PsyQ object-signature matches | `out/psyq/index.json` |
| PsyQ matched callsites | `out/psyq/calls.json` |

Original bytes and PS-X headers outrank generated analyzer output. In
particular, read `t_addr` rather than assuming `0x80010000`.

## Source rules

- `src/exe/<name>/` owns standalone executable source.
- `src/emi/<family>/<archive>/<slot>/` owns one confirmed EMI entry.
- Each lift is `func_XXXXXXXX.c`, with an adjacent `internal.h` for target
  declarations. Shared declarations belong in `include/bof3/`.
- Raw names are `func_80143B40` and `D_80143B40`; map addresses use eight
  uppercase hexadecimal digits. Semantic/PsyQ names replace the raw map entry
  only after review. Lifted filenames remain address-based.
- PsyQ routines are external library code. Use official headers and
  target-local binding/map evidence; do not lift them.
- The pinned signature database identifies complete PsyQ objects/functions at
  target-local addresses. It is separate from PsyQ 4.7 headers (declarations)
  and Rizin snapshots (callsites/xrefs); a signature match is not a provenance
  claim for one SDK version.

## Operational path

```text
binary -> target map -> Splat/Rizin evidence -> C candidate
       -> compile -> generated weak bindings -> link at address
       -> asm diff + byte match
```

Use `bin/m2ctx`, `bin/m2c`, `bin/asm-diff`, `bin/byte-match`, and optionally
`bin/permute` for exactly one `TARGET@0xADDRESS`. `bin/promote` only validates
a candidate and prints required manual edits. `bin/decomp-status [TARGET...]`
recompiles every tracked lift and reports its exact, partial, or invalid state.
See [matching](docs/matching.md).

Rizin sessions are isolated by target. `bin/rz-project` regenerates their
projects and snapshots; `just index` accepts only fresh complete exports and
`bin/rev-query` queries the resulting cache. See
[Rizin and reverse index](docs/reverse-engineering.md).

To scan every target manifest against the pinned PsyQ signatures, initialize
submodules and run `bin/harness psyq scan --all`; then run
`bin/harness psyq calls --all` to join matches with target-local Rizin calls.
This limited adapter does not reintroduce the removed general harness workflow.

## Repository map

| Path | Role |
| --- | --- |
| `src/`, `include/` | Authored C89 source and declarations. |
| `config/targets/`, `config/splat/`, `config/symbols/`, `config/analysis/` | Tracked target facts and reviewed evidence. |
| `docs/specs/`, `LESSONS.md` | Durable reviewed findings and reusable gotchas. |
| `bin/`, `Makefile`, `justfile` | Canonical command entry points, compiler build, and repository recipes. |
| `third_party/` | Pinned upstream tools. |
| `inputs/` | Ignored user-owned media. |
| `out/`, `build/`, `toolchains/` | Disposable local/generated state. |

For setup and all command contracts, read [setup and tools](docs/setup.md).
