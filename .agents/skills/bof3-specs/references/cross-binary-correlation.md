# BOF3 cross-binary correlation

## Contents

- [Questions](#questions-this-workflow-answers)
- [Normalize inputs](#normalize-the-comparison-set)
- [Identity and relocation tiers](#function-identity-tiers)
- [Project follow-up](#project-follow-up)
- [Functions, structures, and values](#recover-a-shared-function-signature)
- [PsyQ](#psyq-correlation-across-all-blobs)
- [Promotion and automation](#promotion-rules)

Use this workflow for any independently mapped PSX blob: executable load image,
overlay, extracted archive entry, library member, embedded code range, or an
unknown raw region with reviewed bounds. Keep target identity and runtime load
address separate from semantic identity.

## Questions this workflow answers

- Does another target contain the exact same function bytes?
- Is a function the same implementation after address relocation?
- Do different implementations expose the same reviewed ABI/signature?
- Do several blobs use the same structure, enum, constant set, callback table,
  or PsyQ API?
- Which facts are truly shared and which remain target-local bindings?

## Normalize the comparison set

For every candidate record:

- target/blob ID and SHA-256;
- mapping base and bounded code/data range;
- function start/end and source of the boundary;
- engine/version and analysis settings;
- direct calls, global references, strings, and observed data widths.

Create a separate project/mapping per target. Equal bytes mapped at different
addresses are still separate runtime instances.

## Function identity tiers

Use increasingly weaker evidence tiers:

1. **Exact bytes and size**: strongest bounded implementation identity.
2. **Relocation-masked bytes**: candidate identity after masking reviewed MIPS
   jump/address relocation fields.
3. **Normalized instruction sequence**: candidate after normalizing absolute
   targets/register-independent presentation; review delay slots and constants.
4. **Control-flow/call/data fingerprint**: similar basic-block graph, callees,
   access widths, literal values, and referenced table shapes.
5. **Compatible ABI only**: distinct implementation with the same reviewed
   parameters, return contract, side effects, and callback/data ownership.

Only tier 1 proves exact implementation bytes. No tier by itself proves shared
runtime address, ownership, or relocatability.

### Safe PSX MIPS relocation masking

Never mask every immediate or address-looking operand. Build a relocation-site
list and retain it with the fingerprint:

- For `j`/`jal` (opcodes 2/3), only the low 26-bit target field is a relocation
  candidate; preserve the opcode and verify the reconstructed target within the
  caller's 256 MiB jump region. A matching masked call is still only a candidate
  until the callee relationship agrees.
- For `lui` plus `addiu`/load/store LO16, pair HI16/LO16 before masking. A signed
  low half may carry into HI16: reconstruct using the signed low value and check
  the corresponding `((value + 0x8000) >> 16)` high adjustment.
- For `lui` plus `ori`, the low half is zero-extended and has no signed carry.
- Prefer explicit archive/object relocation records (`R_MIPS_26`,
  `R_MIPS_HI16`, `R_MIPS_LO16`) when available. Raw linked blobs lack those
  records, so require a known symbol/reference and a consistent target-to-target
  delta before treating an instruction pair as relocated.
- Do not automatically mask PC-relative branches, arithmetic constants,
  structure offsets, GP-relative loads/stores, hardware addresses, or table
  indices. These often carry semantic identity.

A relocation-aware fingerprint must record original words, masks, paired sites,
reconstructed targets, and the proposed relocation delta. Existing graph
relocation groups are ranked candidates, not proof; review their sites before
promotion.

## Project follow-up

Use the project's existing correlation index before inventing another scan.
Follow up each ranked candidate in every independently mapped project:

```text
pdf @ 0xFUNCTION
afij @ 0xFUNCTION
axf 0xFUNCTION
axt 0xFUNCTION
agfj @ 0xFUNCTION
```

Export JSON and compare stable fields externally; do not compare UI rendering.

## Recover a shared function signature

For every implementation/caller set, record:

- values placed in `a0`-`a3` and stack arguments;
- sign/zero extension and access widths;
- whether `v0`/`v1` is consumed and how;
- callee-saved register behavior only as supporting evidence;
- direct/indirect call form and callback-table source;
- observable writes, globals, hardware registers, and blocking behavior.

Classify the result:

- **same implementation, same ABI**: share a semantic typedef/declaration when
  ownership supports it; keep target-local address bindings;
- **different implementation, same ABI**: share only the ABI type/contract;
- **same bytes, different surrounding state**: keep declarations target-local
  until state ownership and relocatability are proven;
- **conflicting callers**: retain address symbols and unresolved prototypes.

For a repeated callback contract, use a function-pointer typedef:

```c
typedef s32 (*PsxTransferCallback)(void* buffer, u32 byte_count);

extern void func_80123456(PsxTransferCallback callback);
#define transfer_set_callback func_80123456
```

Do not create a typedef merely to rename one direct function. Typedefs express
reused type contracts; semantic macros alias proven address symbols.

## Discover shared structures and values

Correlate layouts by observed behavior rather than field names:

- repeated stride and bounded element count;
- identical field offsets and access widths;
- consistent signedness and pointer arithmetic;
- compatible readers/writers across targets;
- enum comparisons, bit masks, sentinel values, and table indexing;
- callback fields with compatible recovered prototypes.

Start with fixed-width unknown fields and promote semantic names incrementally.
Two targets may share a struct layout while using different global addresses.
Share the type only; keep `DAT_XXXXXXXX` bindings target-local.

For constants and enums, distinguish source-level SDK definitions from emitted
runtime values. Compiler folding can erase the original macro/type provenance.

## PsyQ correlation across all blobs

Build a matrix keyed by SDK version and target, not one global address map:

| Target | Address | Official name | Header prototype | Archive/member | Evidence |
| --- | ---: | --- | --- | --- | --- |

Correlate official headers, archive/object signatures, caller ABI, body/call
shape, strings, and known side effects. The same PsyQ function may appear at
different addresses, be duplicated inside several binaries, or be absent from
an overlay that calls back into an executable.

Promote shared SDK function/type/constant names only when official provenance
and observed behavior agree. Preserve a separate target-local binding for every
runtime address. Never merge all address→name results globally; address
collisions across independently loaded targets are expected.

## Generic native workflow

Open each blob independently with verified settings:

```sh
rizin -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xBASE blob.bin
r2 -N -n -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xBASE blob.bin
```

Define reviewed ranges/functions before broad analysis, export `aflj`/xref/flag
JSON supported by the active version, and compare with a deterministic script.
Use `pdg` only after boundaries and call context are credible.

## Promotion rules

- Shared implementation evidence may justify a common semantic name or spec,
  but not a shared address binding.
- Shared ABI evidence may justify a typedef/prototype without implying identical
  code.
- Shared layout evidence may justify a compiled or analysis-only type without
  merging data symbols.
- PsyQ identities require official prototype plus call/body evidence and
  target-local provenance.
- Preserve `func_XXXXXXXX`/`DAT_XXXXXXXX` traceability in compiled source,
  analyzer replay, and cross-target reports.
- Re-run canonical pre/post diffs for every compiled declaration/type/alias
  promotion and rebuild analyzer state from tracked replay.

## Automation boundary

Automate byte hashes, relocation masking, instruction normalization, graph
features, ABI observations, and matrix generation. Keep semantic promotion a
reviewed decision. A useful correlation tool emits ranked candidates plus raw
evidence; it must not auto-rename functions or centralize types.
