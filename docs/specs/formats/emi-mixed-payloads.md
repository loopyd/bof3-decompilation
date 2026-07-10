---
type: Payload format reference
title: EMI mixed and unresolved payload semantics
description: Classification constraints for mixed-purpose and unresolved EMI payloads.
tags: [format, emi, evidence]
---

# EMI Mixed And Unresolved Payload Semantics

This document tracks EMI payload classes that are mixed-purpose or still unresolved.

Use `emi.md` for container structure and base type map. Use module runtime docs for behavior once the controlling module is proven.

## Status

- Confidence: low to medium
- Scope:
  - type `0` generic payloads (code + non-code)
  - type `1` large blobs with unresolved full semantics
  - unresolved script/table/data-only candidates in code-bearing families

## Type-0 Reality

Type `0` is not one format. It includes:

- executable overlays and helper code blobs
- palette-like data companions
- runtime tables and other non-code binary data

Implication:

- do not classify type `0` as code-only or data-only by type id alone
- require per-entry evidence (bytes, load region, callers, xrefs, and runtime behavior)

## Type-1 Working Model

Type `1` appears in several families as large content blobs and may involve additional runtime handling, but complete semantics remain unresolved.

Use conservative language until end-to-end module assignment and decode behavior are proven.

## Suggested Classification Workflow

1. Gather manifest facts: entry index, type, load arg, size.
2. Check load region and family context from inventory docs.
3. Probe for executable boundaries only when code evidence exists.
4. Keep unresolved entries explicitly labeled and list the next proof step.

Related references:

- `docs/specs/formats/emi.md`
- `docs/specs/content/asset-families.md`
- `docs/specs/runtime/emi-loader.md`

## Open Points

- stable discriminators for type-`0` code-vs-data triage across all families
- exact type-`1` runtime path and post-load semantics
- script/table payload signatures that can be promoted from tentative to proven
