# BOF3 context

This repository models BOF3 as independently loaded binaries, not one link
target.

## Identity

| Object | Identity |
| --- | --- |
| PS-X executable | Shipped image plus verified header load address |
| EMI archive | Shipped container path |
| EMI entry | Archive path plus slot |
| Target | Exact payload, load address, and source directory |
| Function | `TARGET@0xADDRESS` |

Analyze executable images or extracted entries. Keep identical payloads or
addresses as separate targets until relocatability is proven.

## Durable facts

| Fact | Owner |
| --- | --- |
| Binary identity and load address | `config/targets/<target>.toml` |
| Segment boundaries | `config/splat/` |
| Target-local symbols | `config/symbols/<target>.txt` |
| Reviewed Rizin annotations | `config/analysis/<target>/` |
| C89 source and local declarations | `src/exe/`, `src/emi/` |
| Reviewed findings | `docs/specs/`, `LESSONS.md` |

Generated evidence and working candidates live under `out/` and are
disposable. They may be edited while iterating but are never durable facts.
Original bytes, PS-X headers, and reviewed configuration outrank analyzer
output.

## Source model

- `src/exe/<name>/` owns executable lifts.
- `src/emi/<family>/<archive>/<slot>/` owns one EMI entry.
- Each lift is `func_XXXXXXXX.c` with adjacent target-local `internal.h`.
- Function C, local headers, maps, and Splat layouts are intentionally
  hand-edited as reviewed evidence improves.
- Shared declarations belong in `include/bof3/` only when multiple targets or
  an external contract require them.
- Cross-target embedded implementations may live as non-standalone templates
  under `src/shared/`; address-owned wrappers remain in each target directory.
- PsyQ signatures identify objects and addresses; official headers provide C
  declarations; Rizin snapshots provide callsites and xrefs. None substitutes
  for another.

See [setup](docs/setup.md), [matching](docs/matching.md), and
[Rizin evidence](docs/reverse-engineering.md) for procedures.

## Repository map

| Path | Contents | Tracked? |
| --- | --- | --- |
| `config/targets/` | Target identity, image path, load address | Yes |
| `config/splat/` | Reviewed binary layout | Yes |
| `config/symbols/` | Target-local symbol maps | Yes |
| `config/analysis/` | Reviewed Rizin annotations | Yes |
| `src/`, `include/` | Authored C89 and declarations | Yes |
| `docs/specs/`, `LESSONS.md` | Reviewed findings and gotchas | Yes |
| `bin/`, `tools/` | Command entrypoints and implementations | Yes |
| `third_party/` | Pinned upstream source | Yes |
| `inputs/disc/`, `inputs/external/` | User-owned media and private inputs | No |
| `out/binaries/`, `out/extracted/` | Normalized images and extracted entries | No |
| `out/splat/`, `out/reverse/`, `out/index/` | Generated assembly, snapshots, index | No |
| `out/m2ctx/`, `out/matching/`, `out/permuter/` | Disposable matching workspaces | No |
| `build/` | Build products | No |
| `toolchains/` | Tracked metadata plus mostly ignored staged tools | Mixed |

Ignored paths remain available to local tools and agents. Inspect them when
needed; do not cite them as durable facts or commit their generated contents.
