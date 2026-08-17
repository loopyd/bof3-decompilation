# BOF3 context

BOF3 is modeled as independently loaded binaries, not one link target.

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

Fact ownership (binary identity, layout, symbols, SDK maps, reviewed Rizin,
authored lifts) is `AGENTS.md` §Ownership; generated `out/` evidence is
disposable, never durable facts. The exception is the generated PsyQ binding
source (`psyq_source`): tracked because the build compiles it. Original bytes,
PS-X headers, and reviewed configuration outrank analyzer output.

## Source model

Ownership is explicit: lifts, bindings, headers, and PsyQ source are claimed
in `config/targets/<target>/target.toml`
(`sources`/`support_sources`/`headers`/`psyq_source`) with `@source`/
`@behavior`; identity (binary, layout, symbols) stays centralized; `source_dir`
is only the historical Splat root. Each lift is one C source plus its claimed
target-private header (`include/bof3/<subsystem>/`); filenames never supply
address fallback. Function C, local headers, maps, and Splat layouts are
hand-edited as evidence improves. Shared declarations belong in
`include/<subsystem>/` only when multiple targets or an external contract
require them. Follow [exact duplicate groups](matching.md#exact-duplicate-groups)
before extracting a template; colocated wrappers stay independently
manifest-owned. PsyQ signatures identify objects/addresses; official headers
give C declarations; Rizin snapshots give callsites/xrefs; none substitutes
another.

## Repository map

| Path | Contents | Tracked? |
| --- | --- | --- |
| `config/targets/` | Target identity, layout, symbols, analysis | Yes |
| `config/sdk/` | Shared PsyQ SDK symbol maps (slus/logo) | Yes |
| `src/`, `include/` | Authored C89 (`src/bof3/` semantic root; metadata and manifest claims own target identity) | Yes |
| `../specs/`, `lessons.md` | Reviewed findings and gotchas | Yes |
| `bin/`, `tools/` | Command entrypoints and implementations | Yes |
| `third_party/` | Pinned upstream source | Yes |
| `inputs/external/` | User-owned CUE/BIN media or `BreathOfFireIIIv1.1.7z`, plus private inputs | No |
| `out/binaries/`, `out/extracted/` | Normalized images and extracted entries | No |
| `out/splat/`, `out/reverse/`, `out/index/` | Generated assembly, snapshots, index | No |
| `out/m2ctx/`, `out/matching/`, `out/permuter/` | Disposable matching workspaces | No |
| `build/` | Build products | No |
| `toolchains/` | Tracked metadata plus mostly ignored staged tools | Mixed |

Ignored paths stay available to local tools/agents; inspect when needed;
never cite them as durable facts or commit their generated contents.
