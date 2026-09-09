# Identity transactions

Apply only an evidence-approved `symbol`, `type`, `repair`, or `retained-lift` request. Naming evidence preparation belongs to `bof3-naming-evidence`; never generate semantic evidence here or switch routes.

## Authority ceiling

- Symbol/type changes are target-local: sorted `symbols.txt` with one spelling/address, target `internal.h`/`symbols.c`, and same-target references. No aliases or generated `symbols/psyq.c` edits.
- Shared fixed-RAM requires either an existing shared map or recursive proof of identical address, content class, and runtime role in every composing consumer. Inventory every composing Splat; atomically unify spelling across declarations, bindings, annotations, and references. Otherwise keep it local: address/reference count is insufficient. Function/source ownership is never shared.
- Renames preserve width, signedness, pointer depth, volatility, ABI, storage, extent, packing, code/CFG shape, matching aids, flags, and addresses.

## Identity invariants

Map plus `WEAK_SYMBOL_AT` own addresses. Never alter load addresses, Splat boundaries, SDK maps, public/shared headers, compiler flags, or target ownership. [Byte-safe cosmetics](BYTE_SAFE_COSMETICS.md#metadata-preflight-and-authority) owns lift metadata preflight/preservation; [Source relocation](SOURCE_RELOCATION.md#ownership-and-invariants) owns relocation identity.

## Parent phase order and classification

Immutable parent order: audit preflight → caller-approved safe `bin/naming-audit prepare TARGET --repair --rows KIND:NAME[,KIND:NAME...]` when needed → naming audit/evidence graph → one isolated identity transaction → validation. Audit is read-only; children never switch modes. Before editing, classify the candidate as mechanical repair, scoped-plan work, or ownership/evidence blocker; only the authorized mechanical identity transaction proceeds here.

## Transaction

1. Refuse overlap with a modified candidate unless the parent named that edit.
2. Record old/new spelling, unchanged address/layout, binding, local references, and approved evidence. Atomically update map, declaration, binding, and same-target references; remove compatibility/self aliases.
3. Retained partials preserve body, ABI, boundary, compiler settings, `@status partial`, `@match`, and `@residual` verbatim and report unchanged live non-baseline. Data declarations retain `@kind: bss|rodata|string|table`.
4. Shared fixed-RAM updates every consumer and live-validates representative consumers in each authored target family.
5. Verify no owned old spelling, unrelated target change, broken edited link, or omitted transaction-scope file.
6. Validate every touched selector by transaction class per [Byte-safe cosmetics](BYTE_SAFE_COSMETICS.md#naming-and-validation). Map/Splat changes also require `bin/splat TARGET` and fresh `bof3-reviewer`. Any failure reverts the transaction, never fixes forward.
7. Run `bin/symbols normalize TARGET --write` then `bin/symbols check TARGET`; require sorted `name = 0xADDRESS;` entries.
8. After authoritative edits and normal symbol/Splat/build/byte gates pass, inspect source/build graph readiness. Regenerate stale graph metadata through `bin/build TARGET`, never edit `build/`; require every manifest source present and superseded path absent. Run `bin/rz-project status TARGET --json` then `bin/rev-query --json status`. If either is stale, run one batch `bin/index --recover`, require both fresh, rerun readiness, and never rebuild per file.
9. Require `git diff --check`; preserve the caller's staged index. Record any pre-existing non-empty or mode-only index and prove identical paths/modes/content afterward. Unexpected candidate/index overlap blocks: restore the transaction rather than staging, unstaging, or fixing forward.

## Repair and readiness

Run `bin/analysis-readiness [TARGET]` before application; close or explicitly retain blockers. Refresh disposable indexes only after authoritative edits pass. After recovery, rerun readiness against the fresh index before declaring readiness. Safe metadata repair requires caller-approved `bin/naming-audit prepare TARGET --repair` and live asm-diff/byte-match exactness before progress metadata canonicalization; ownership/layout remain blocked. One transaction at a time; regeneration failure blocks.

## Rollback

Any scope, exactness, metadata, ownership, build, map, Splat, or review failure restores every transaction file. Report target, selector/symbol, failing command, observed result, and smallest evidence or approval needed.
