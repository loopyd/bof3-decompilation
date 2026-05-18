# Project Status

Current frontier only.

## Proven Chain

```text
SLUS post-logo bootstrap
-> FIRST.EMI
-> GAME.EMI#1
-> DEMO.EMI
-> title/front loop
-> SCENA16 request boundary
-> SCENA16 secondary finalize seam (0x801f7188)
-> EXE slot/thread exit seam (0x8014b8b0)
```

## Active Source Modules

- `bof3/src/core/callback_scheduler/`: EXE callback/thread seam
- `bof3/src/modules/logo/`: `LOGO.EXE`
- `bof3/src/modules/game/00/`: `GAME.EMI#0`
- `bof3/src/modules/game/01/`: `GAME.EMI#1`
- `bof3/src/modules/commu00/00/`: `COMMU00.EMI#0`
- `bof3/src/modules/battle/03/`: `BATTLE.EMI#3`
- `bof3/src/modules/battle/15/`: `BATTLE.EMI#15`

Use shipped archive + slot naming for lifted code:

- `game/00`, `game/01`
- `commu00/00`
- `battle/03`, `battle/15`

## Current Gaps

- exact NEW/LOAD mapping into later `SCENA16` routed values
- first true gameplay module after `0x801f7188 -> 0x8014b8b0`
- card/load frontend branch
- timeout/demo branch after `GAME.EMI#1`
- `LOGO.EXE -> CAPCOM30.STR` execution path

## Current Reports

- `output/inventory/`
- `output/harness/report.json`
- `output/harness/dashboard/index.html`

## Next Targets

1. Close the authored NEW/LOAD -> `SCENA16` route mapping.
2. Recover the first true gameplay module after `0x801f7188 -> 0x8014b8b0`.
3. Recover the card/load branch.
4. Lift contiguous `BATTLE.EMI#3` functions to whole-module parity.
