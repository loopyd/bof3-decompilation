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
authored lifts) is [the documentation index](project-context.md#ownership); generated `out/` evidence is
disposable, never durable facts. The exception is the generated PsyQ binding
source (`psyq_source`): tracked because the build compiles it. Original bytes,
PS-X headers, and reviewed configuration outrank analyzer output.

## Source model

Ownership is explicit: lifts, bindings, headers, and PsyQ source are claimed
in `config/targets/<target>/target.toml`
(`sources`/`support_sources`/`headers`/`psyq_source`) with `@source`/
`@behavior`; identity (binary, layout, symbols) stays centralized; `source_dir`
is only the historical Splat root. Until the [combiner rollout](combiner.md) clears
its supporting gates, each production lift is one C source plus its claimed
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
| `docs/specs/` | Reviewed BOF3 runtime, format and data knowledge | Yes |
| `docs/agents/` | Agent/tooling contracts and matching workflows | Yes |
| `docs/plans/`, `docs/reference/` | Active scoped work; external research leads | Yes |
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

## Hard repository contract


- Work only in this repository. Never commit user media from `inputs/`.
- `out/` is disposable working state, never reviewed truth. Do not hand-edit
  `build/`, `toolchains/`, or generated PsyQ bindings.
- BOF3 binaries load independently. Qualify one function as
  `TARGET@0xADDRESS`; shipped EMI entries use
  `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`.
- Original bytes and PS-X headers outrank analyzer output; verify `t_addr`.
- No stage, commit, push, release, or external mutation without approval.
- Complete assigned scope. Stop only on an evidence-backed blocker and report
  what was tried plus the smallest unblocking action.

## Ownership


| Fact | Owner |
| --- | --- |
| Binary identity and load address | `config/targets/<target>/target.toml` |
| Reviewed layout | `config/targets/<target>/splat.yaml` |
| Shared Splat execution options | `config/splat.yaml` |
| Target-local symbols | `config/targets/<target>/symbols.txt` |
| Shared SDK symbol maps | `config/sdk/psyq-{slus,logo}.txt` |
| Reviewed Rizin annotations | `config/targets/<target>/reviewed.rz` |
| Authored lifts | Metadata-resolved source under `src/bof3/` |
| Macro opportunity policy and tool reference | `docs/agents/macros.md` |

Until the [multi-function rollout](combiner.md) clears registry and native
gates, keep new production lifts one function per C source; inspection and read-only
source/index ownership support attached per-function records. Existing grouped
sources are not grandfathered into native or transaction acceptance. This is a
staged migration, not a permanent ban on
cohesive multi-function files. Lift identity comes from manifest claims,
maps, Splat, and function-level `@source`/`@behavior` metadata—not filenames or
directory ancestry. Maps use sorted `name = 0xADDRESS;` rows with uppercase
addresses. Bind target symbols through plain declarations plus sanctioned
`WEAK_SYMBOL_AT` entries; keep PsyQ external. See the matching and project
context docs for the complete source/symbol contract.

## Header layout


| Directory | Domain |
| --- | --- |
| `include/base/`, `include/memory/` | common types, compiler attributes, fixed RAM and hardware addresses |
| `include/gpu/`, `include/frontend/`, `include/callback/` | rendering, frontend, callbacks |
| `include/loader/`, `include/panel/`, `include/battle/` | loader, panel, battle runtime |
| `include/data/`, `include/media/`, `include/ui/`, `include/game/` | records, media, UI and game templates |

Completed implementation history is in `git log`; current scoped work belongs
under `docs/plans/` only while active.
