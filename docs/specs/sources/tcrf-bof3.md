# Source Summary: TCRF BOF3

Source page:

- `https://tcrf.net/Breath_of_Fire_III`

## What It Is

The Cutting Room Floor page for BOF3 is a community-maintained catalog of unused, cut, debug, and odd leftover game content.

In this repo it should be treated as:

- a useful lead source for reachability questions
- a clue source for cut/debug assets
- not a primary proof source for runtime behavior

## Current Relevant Leads

- `LOGO/CAPCOM30.STR` is reported externally as the animated Capcom boot-logo asset.
- BOF3 likely contains shipped-but-unwired or cut/debug content.
- text/audio hacking notes around BOF3 exist in the broader romhacking community and may help decode remaining text or voice assets once imported into repo-native docs.

## Current Local Corroboration

- `processed/inventory/inventory.sqlite` now resolves the full 887-entry top-level slot table, including:
  - `build/extracted/BIN/WORLD04/AREA197.EMI`
  - `build/extracted/BIN/WORLD04/AREA198.EMI`
  - `build/extracted/BIN/WORLD04/AREA199.EMI`
  - `build/extracted/LOGO/CAPCOM30.STR`
  - `build/extracted/LOGO/LOGO.EXE`
  - `build/extracted/SYSTEM.CNF`
  - `build/extracted/SLUS_004.22`
- `build/Breath of Fire III (v1.1).xml` confirms that these files are present in the shipped US v1.1 image layout.
- `third_party/references/vast_violence/tables/pointers_formations_1.1.txt` and `third_party/references/vast_violence/tables/pointers_monsters_1.1.txt` both index data inside `AREA197.EMI`, `AREA198.EMI`, and `AREA199.EMI`.

Current repo interpretation:

- `CAPCOM30.STR` should remain `unproven reachability` until the boot/logo code path is locally traced, but it is now proven to be part of the top-level slot table.
- `AREA197.EMI`, `AREA198.EMI`, and `AREA199.EMI` are also proven to be part of the top-level slot table.
- the three `AREA19x` archives are clearly contentful; the unresolved part is whether normal game flow reaches their slots or whether they are cut/test-only content.

## Limits

- The TCRF page is community-curated, not a primary vendor or binary-derived source.
- In this environment, direct automated retrieval of the page is blocked by an anti-bot response, so any TCRF-specific claim must still be cross-checked locally before being promoted to a runtime fact.
- The page is best used to guide follow-up work, not to replace loader tracing in `SLUS_004.22`, `LOGO.EXE`, or EMI overlays.
