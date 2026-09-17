# BOF3 game specifications index

Read [the documentation index](../INDEX.md) first. This directory stores BOF3
runtime, binary-format, data-layout and target-specific reconstruction evidence.
Agent policy, harness tooling and session workflows live in [agents](../agents/INDEX.md).

| Spec | Read it when |
| --- | --- |
| [bof3-eu/README.md](bof3-eu/README.md) | BOF3JS EU knowledgebase: engine, formats, maps, scripts, entities, battle, audio and address leads; not reviewed US facts |
| [targets.md](targets.md) | identifying executable and overlay load addresses |
| [pseudocode.md](pseudocode.md) | source-backed runtime and extraction algorithms |
| [archives.md](archives.md) | EMI archive roles and entry lists |
| [runtime/runtime-layout.md](runtime/runtime-layout.md) | executable, overlay, and load-region boundaries |
| [runtime/frontend.md](runtime/frontend.md) | title, menu, and attract-path transitions |
| [runtime/emi-loader.md](runtime/emi-loader.md) | SLUS EMI entry dispatch and loading |
| [runtime/memory-layouts.md](runtime/memory-layouts.md) | byte layouts used by lifted code |
| [runtime/psyq-constants.md](runtime/psyq-constants.md) | SDK constants, ABI declarations, and pinned PsyQ contracts |
| [runtime/compiler-provenance.md](runtime/compiler-provenance.md) | historical compiler and delay-slot evidence |
| [runtime/compiler-variants.md](runtime/compiler-variants.md) | GCC variant probes and negative evidence |
| [runtime/battle-range-predicates.md](runtime/battle-range-predicates.md) | battle/15 predicate evidence |
| [runtime/battle-dispatch-tables.md](runtime/battle-dispatch-tables.md) | reviewed Battle 15 dispatch-table ranges and consumers |
| [data/INDEX.md](data/INDEX.md) | data-spec map and recorded table families |
| [formats/emi.md](formats/emi.md) | EMI container and entry format |
| [formats/audio.md](formats/audio.md) | XA, VAB, SEP, PSF1, and SPU formats/runtime |
| [formats/str-xa.md](formats/str-xa.md) | extracted STR/XA sectors and playback |
| [formats/graphics.md](formats/graphics.md) | VRAM uploads, textures, palettes, and graphics formats |
| [formats/conversion.md](formats/conversion.md) | lossless interchange and provenance |

Add game findings to the narrowest existing spec; register new topics here.
Tool contracts belong in `docs/agents/`, active work in `docs/plans/`, and
other external reference material in `docs/reference/`. The imported BOF3JS EU
knowledgebase is preserved here with its provenance and regional limits;
placement does not make its claims reviewed US facts. No transient execution state.
