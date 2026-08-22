# Identity transactions

Apply only an evidence-approved `symbol`, `type`, `repair`, or `retained-lift` request. Naming evidence preparation belongs to `bof3-naming-evidence`; do not generate semantic evidence here or switch routes.

## Authority ceiling

- Symbol/type changes are target-local: sorted `symbols.txt` with one spelling/address, target `internal.h`/`symbols.c`, and same-target references. No aliases or generated `symbols/psyq.c` edits.
- Shared fixed-RAM is eligible only when already mapped shared, or recursive proof establishes identical address, content class, and runtime role in every composing consumer. Inventory every composing Splat and atomically use one spelling across declarations, bindings, annotations, and references. Otherwise keep it local; address/reference count alone is insufficient. This never shares function/source ownership.
- A rename never changes width, signedness, pointer depth, volatility, ABI, storage, extent, packing, code/CFG shape, matching aids, flags, or addresses.

## Identity invariants

Map plus `WEAK_SYMBOL_AT` own addresses. Never alter load addresses, Splat boundaries, SDK maps, public/shared headers, compiler flags, or target ownership. Lift metadata preflight and preservation are defined once by [Byte-safe cosmetics](BYTE_SAFE_COSMETICS.md#metadata-preflight-and-authority); relocation-specific identity is defined by [Source relocation](SOURCE_RELOCATION.md#ownership-and-invariants).

## Parent phase order and classification

The parent owns this immutable phase order: audit preflight → caller-approved safe `bin/naming-audit prepare TARGET --repair` when needed → naming audit/evidence graph → one isolated identity transaction → validation. Audit remains read-only and children never switch modes. Before editing, classify the candidate as a mechanical repair, scoped-plan work, or an ownership/evidence blocker; only the authorized mechanical identity transaction proceeds here.

## Transaction

1. Refuse overlap with a modified candidate unless the parent named that edit.
2. Record old/new spelling, unchanged address/layout, binding, local references, and approved evidence. Atomically update map, declaration, binding, and same-target references; remove compatibility/self aliases.
3. Retained partials preserve body, ABI, boundary, compiler settings, `@status partial`, `@match`, and `@residual` verbatim and report unchanged live non-baseline. Data declarations retain `@kind: bss|rodata|string|table`.
4. Shared fixed-RAM updates every consumer and live-validates representative consumers in each authored target family.
5. Verify no owned old spelling, unrelated target change, broken edited link, or omitted transaction-scope file.
6. Validate each touched selector per [Byte-safe cosmetics](BYTE_SAFE_COSMETICS.md#naming-and-validation), branching by transaction class: an exact transaction requires fresh `bin/asm-diff TARGET@0xADDRESS --detail normal` with no first difference and `bin/byte-match TARGET@0xADDRESS` exit 0; a retained-partial transaction requires post-edit `asm-diff`/`byte-match` reporting exactly unchanged metrics and first mismatch, so only drift fails the partial. Map/Splat changes also run `bin/splat TARGET` and fresh `bof3-review`. Failure reverts the transaction, never fixes forward.
7. Run `bin/symbols normalize TARGET --write`, `bin/symbols check TARGET`, and ensure sorted `name = 0xADDRESS;` entries.
8. After authoritative edits and the normal symbol/Splat/build/byte gates pass, inspect source/build graph readiness: regenerate stale graph metadata through `bin/build TARGET`, never edit `build/`, require each manifest source present and every superseded path absent. Then run `bin/rz-project status TARGET --json` and `bin/rev-query --json status`. If either is stale, run one batch `bin/index --recover`, require both fresh, rerun readiness, and never rebuild per file.
9. Before completion, require `git diff --check` and preserve the caller's staged index. A non-empty or mode-only pre-existing index is not transaction content: record it before edits and prove the same paths/modes/content afterward. Unexpected candidate/index overlap blocks; restore the transaction rather than staging, unstaging, or fixing forward.

## Repair and readiness

Before application run `bin/analysis-readiness [TARGET]`, close or explicitly retain its blockers, and refresh disposable indexes only after authoritative edits pass. After any recovery, rerun `bin/analysis-readiness [TARGET]` against the fresh index before declaring readiness. A safe metadata repair requires caller-approved `bin/naming-audit prepare TARGET --repair` with live asm-diff/byte-match exactness before progress metadata canonicalization; ownership/layout remain blocked. One transaction at a time; regeneration failure blocks.

## Rollback

Any scope, exactness, metadata, ownership, build, map, Splat, or review failure restores every file in the transaction. Report target, selector/symbol, failing command, observed result, and smallest evidence or approval needed.
