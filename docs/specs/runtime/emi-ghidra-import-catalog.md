# EMI Ghidra Import Catalog

This document records the current practical Ghidra import set for BOF3 `.EMI`
archives and the archive-side inputs that still matter for full reverse
engineering.

It is broader than the conservative documented subset in
[module-map.md](runtime/module-map.md).
The goal here is import triage: which archives should be considered code-bearing
or overlay-like first when building or refreshing the Ghidra project.

## Scan Rule

Local scan target:

- `build/extracted/BIN/**/*.EMI`

Scanner:

- `build/tools/emi-ex-v2/cli/emi-ex extract -q --dry-run --print-manifest --manifest-type-status`

Archive inclusion rule:

- at least one entry with:
  - `type == 0`
  - `ram_ptr >= 0x80000000`
  - `0 < first4 <= 0x400`

Explicit local exclusions:

- `BIN/ETC/FIRST.EMI`
- `BIN/ETC/AFLDKWA.EMI`

Reason for the exclusions:

- current local runtime docs classify `FIRST.EMI`, `DEMO.EMI`, and the duplicate
  `AFLDKWA` CPU-RAM payload as title/menu resource packs or table/text payloads,
  not trustworthy overlay roots

Current scan result:

- `524` candidate archives out of `880` total `.EMI` archives under
  `build/extracted`

This catalog is for Ghidra import planning. It is not a claim that every
matching type-`0` entry is a direct execution root.

## Full RE Split

For BOF3 lift-and-match work, the Ghidra target should stay split into:

- code-side imports
  - `SLUS_004.22`
  - `LOGO.EXE`
  - code-like EMI payloads imported as programs at their runtime addresses
- archive-side inputs
  - the original `.EMI` archives under `build/extracted/BIN/`
  - especially mixed title/menu packs such as `FIRST.EMI`, `DEMO.EMI`, and
    `AFLDKWA.EMI`

Reason for the split:

- imported programs are what make decompilation, function promotion, and
  match-oriented lifting practical
- raw archives remain necessary for graphics/audio/table recovery because many
  important runtime inputs are not execution roots:
  - type-`3` VRAM uploads
  - small palette or CLUT rows
  - text/table payloads
  - presentation-side control blobs

## Folder Coverage

### Explicit archive list

`BATTLE`:

- `BATL_END.EMI`
- `BATL_OVR.EMI`
- `BATTLE.EMI`
- `BATTLE2.EMI`

`ETC`:

- `BATE.EMI`
- `COMMU00.EMI`
- `COMMU01.EMI`
- `COMMU02.EMI`
- `COMMU02B.EMI`
- `COMMU03.EMI`
- `COMMU04.EMI`
- `COMMU05.EMI`
- `GAME.EMI`
- `MTEST.EMI`
- `RTEST.EMI`
- `SHISU.EMI`
- `SHOP.EMI`
- `SISYOU.EMI`
- `START.EMI`
- `STATUS.EMI`

### Full-folder coverage

Every archive in these folders matched the current import rule:

- `BIN/BMAGIC/*.EMI` (`144` archives)
- `BIN/BOSS/*.EMI` (`40` archives)
- `BIN/SCENARIO/*.EMI` (`25` archives)
- `BIN/WORLD00/*.EMI` (`38` archives)
- `BIN/WORLD01/*.EMI` (`38` archives)
- `BIN/WORLD02/*.EMI` (`38` archives)
- `BIN/WORLD03/*.EMI` (`38` archives)
- `BIN/WORLD04/*.EMI` (`48` archives)

### Pattern coverage

`PLCHAR`:

- every `PLP*.EMI` archive under `BIN/PLCHAR/` (`19` archives)

`BPLCHAR`:

- `BPLU349.EMI`
- every `CRYUD*.EMI`
- every `CRYUU*.EMI`
- every `DRG*.EMI`
- every `PAPYD*.EMI`
- every `PAPYU*.EMI`
- every `REID*.EMI`
- every `REIU*.EMI`
- every `RTD0*.EMI`
- every `RTU0*.EMI`
- every `RYUD*.EMI`
- every `RYUU*.EMI`
- combined total: `76` archives

## Practical Import Order

For a conservative Ghidra bootstrap, import in this order:

1. `SLUS_004.22`
2. documented menu/controller overlays such as `GAME`, `COMMU00`, `STATUS`,
   `START`, `SHOP`
3. representative battle overlays such as `BATTLE` and `BATTLE2`
4. scenario and world folders
5. battle-magic, boss, and character-side overlay families

## Known Non-Code Packs

Do not treat these as default code imports:

- `FIRST.EMI`
- `DEMO.EMI`
- `AFLDKWA.EMI`

They remain important runtime resource packs, and for full extraction they
should stay adjacent to the Ghidra project as raw archive inputs, but current
local docs do not defend them as trustworthy overlay roots.
