---
name: bof3-identity-maintenance
description: Repairs evidence-approved BOF3 symbol, type, metadata, retained-lift, relocation-batch, and byte-safe cosmetic identities with rollback. Use for only an explicit identity-maintenance route; exclude semantic lift rewrites, target relocation, silent route switching, and ordinary reverse or review tasks.
---

# BOF3 identity maintenance

Perform only the caller-authorized transaction; never infer or switch modes. Preserve behavior, bytes, ABI, addresses, boundaries, compiler settings, and partial-lift status metadata.

| Route | Contract |
|---|---|
| symbol, type, repair, retained-lift, metadata integration | [Identity transactions](references/IDENTITY_TRANSACTIONS.md): authority, validation, rollback |
| relocation-batch | [Source relocation](references/SOURCE_RELOCATION.md): atomic ownership and build-graph updates |
| cosmetic | [Byte-safe cosmetics](references/BYTE_SAFE_COSMETICS.md): safe/guarded/never-safe ladder |
