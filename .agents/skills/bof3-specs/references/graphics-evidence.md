# Graphics and CLUT evidence

Use this guide to classify BOF3 graphics payloads and palette relationships.
The exact EMI and graphics layouts remain owned by
`docs/specs/formats/emi.md` and `docs/specs/formats/graphics.md`.

## Evidence chain

Treat palette-shaped bytes as a candidate until the runtime chain agrees:

```text
entry type/size/load argument
-> reviewed CPU or VRAM destination
-> palette-bank or CLUT selector
-> indexed-texture primitive/consumer
-> reviewed render or equivalent runtime evidence
```

Archive adjacency, compatible sizes, type `0`, repeated color words, or a
colorful preview does not prove a texture-to-palette association. Inventory,
preview, and target-specific reconstruction may legitimately use different
candidate filters.

## Promote only facts

- Preserve archive entry identity, payload hash, load argument, and target.
- Decode PSX color words only after confirming the expected word layout.
- Verify width/height/stride and palette row or bank from consumers or loader
  behavior, not appearance alone.
- Keep entry, runtime data symbol, and CLUT consumer target-local even when the
  shared PSX representation is known.
- Store previews and candidate tables under `out/`; put stable reviewed findings
  in the owning `docs/specs/` concept.
