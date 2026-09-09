# BOF3 specifications index

Runtime, format, and reconstruction-tool specifications live here. Read
[`../INDEX.md`](../INDEX.md) first, then select only the topic needed. Established
lowercase paths are retained; `MACROS.md` is the canonical macro-resolution spec.

| Spec | Read it when |
| --- | --- |
| [MACROS.md](MACROS.md) | macro opportunity discovery, human-value ranking requirements, consumer scope, and reviewed resolution tooling |
| [HARNESS.md](HARNESS.md) | macro, naming and type owners, explicit cleanup routes, shared mechanisms, CLI ownership and refactor evidence boundaries |
| [CODEX.md](CODEX.md) | global MCP credentials/configuration, Pi extension equivalents and explicit exclusions |
| [DOCS.md](DOCS.md) | unified Markdown search/context, aggregation, repair/edit preparation and explicit compaction |
| [targets.md](targets.md) | identifying executable and overlay load addresses |
| [methods.md](methods.md) | pointer maps, table extraction, and evidence methods |
| [pseudocode.md](pseudocode.md) | source-backed runtime and extraction algorithms |
| [archives.md](archives.md) | EMI archive roles and entry lists |
| [runtime/runtime-layout.md](runtime/runtime-layout.md) | executable, overlay, and load-region boundaries |
| [runtime/frontend.md](runtime/frontend.md) | title, menu, and attract-path transitions |
| [runtime/emi-loader.md](runtime/emi-loader.md) | SLUS EMI entry dispatch and loading |
| [runtime/memory-layouts.md](runtime/memory-layouts.md) | byte layouts used by lifted code |
| [runtime/psyq-constants.md](runtime/psyq-constants.md) | SDK constants, ABI declarations, and pinned PsyQ contracts |
| [runtime/compiler-provenance.md](runtime/compiler-provenance.md) | historical compiler and delay-slot evidence |
| [runtime/compiler-quirks.md](runtime/compiler-quirks.md) | register allocation and delay-slot residuals |
| [runtime/compiler-variants.md](runtime/compiler-variants.md) | GCC variant probes and negative evidence |
| [runtime/battle-range-predicates.md](runtime/battle-range-predicates.md) | battle/15 predicate evidence |
| [runtime/naming-table-consumers.md](runtime/naming-table-consumers.md) | naming-table consumer evidence |
| [data/index.md](data/index.md) | data-spec map and recorded table families |
| [formats/emi.md](formats/emi.md) | EMI container and entry format |
| [formats/audio.md](formats/audio.md) | XA, VAB, SEP, PSF1, and SPU formats/runtime |
| [formats/str-xa.md](formats/str-xa.md) | extracted STR/XA sectors and playback |
| [formats/graphics.md](formats/graphics.md) | VRAM uploads, textures, palettes, and graphics formats |
| [formats/conversion.md](formats/conversion.md) | lossless interchange and provenance |

New durable findings or tool specifications belong in the narrowest existing spec;
create a new spec only when no existing topic owns them, and register it here.
Plans and transient execution state do not belong in specifications.
