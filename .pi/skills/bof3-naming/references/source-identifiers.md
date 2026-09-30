# Source identifier naming

Arguments/locals are lexical C identifiers, not map symbols. Spelling only.
Widths, signedness, pointer depth and layout belong to `$bof3-types`; declaration
list changes are not renames.

## Discovery

`bin/harness naming source-identifiers TARGET` lists read-only leads over the
target's claimed sources in declaration order; `bin/harness naming
describe-source-identifier TARGET ID --expected-fingerprint PIN` requires exact
current membership. Discovery parses source text only: it never opens or
refreshes the reverse index, reports, maps or Splat, and a row is a lead, not
evidence or approval.

Retain caller `id`/`fingerprint` independently. Only code text binds identifiers;
comments/string literals prove neither use nor scope escape. Source edit invalidates
pin: reread row before proceeding.

## Evidence gate

Justify a proposed spelling from the identifier's role: parameter position,
declaration type head, assignment/use sites, callee or caller argument role,
guards, and result use. A single use or an analyzer guess is a lead. Keep the
current name until behavior, callers and consumers corroborate the semantic role,
and prefer an explicit gap to an invented name. This gate replaces map/address
corroboration, which does not exist for a function-local identifier.

## Transaction

1. Prepare one identifier at a time:
   `bin/harness naming prepare-source-transaction TARGET --selector
   TARGET@0xADDRESS --transaction argument:NAME|local:NAME --new-name NEW
   --output RECEIPT.json`. It binds the PRE source SHA-256, the exact
   implementation span and a `.pre` backup.
2. Prepare refuses a new name that already occurs in code anywhere in the file,
   an identifier used outside the function (scope escape), a keyword, a no-op
   rename, and any selector/target mismatch.
3. `apply-source-transaction TARGET RECEIPT.json` re-validates the PRE hash and
   rewrites only the implementation span. `verify-source-transaction` proves the
   rename is the only text change and requires `bin/harness lift asm-diff` and
   `bin/harness lift byte-match` to exit 0, so the recompiled object stays byte-identical.
4. `rollback-source-transaction TARGET RECEIPT.json` restores the PRE bytes.
5. Any scope, exactness, byte, map, ABI, boundary, metadata or review failure
   reverts the transaction; never fix forward. The native gate fails closed: a
   missing toolchain blocks instead of passing.
6. This route is exact-lift only. A retained partial has no byte-identity exit
   gate here; report it as a gap instead of weakening the gate. Map/Splat
   symbols keep [Identity transactions](identity-transactions.md), file moves
   keep [Source relocation](source-relocation.md), and naming style follows
   [Byte-safe cosmetics](byte-safe-cosmetics.md#naming-and-validation).

Accepted rename changes source text only. Map/Splat/ABI/storage/representation
stay unchanged. Required type change routes to `$bof3-types`, never bundled rename.
