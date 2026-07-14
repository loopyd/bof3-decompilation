---
type: Runtime
title: Frontend flow
description: Reviewed title, menu, and attract-path transitions.
tags: [runtime, frontend, title]
---

# Frontend flow

`GAME.EMI#1` owns the reviewed title and New Game/Load presentation loop.
`GAME.EMI#0` and `SCENA16.EMI#0` participate in later frontend transitions.
The addresses below are target-local.

## Reviewed title transitions

| Function | Observed transition |
| --- | --- |
| `GAME.EMI#1 @ 0x801d0c90` | Selects layout bank 2, requests `DEMO.EMI`, waits for the loader, initializes the title fade/window state, and advances the local state. |
| `GAME.EMI#1 @ 0x801d0d5c` | Waits for fade phase 2, arms a 360-tick timer, and advances the local state. |
| `GAME.EMI#1 @ 0x801d0d94` | At timer expiry, selects fade phase 3, opens the window phase, arms 900 ticks, and advances the local state. |
| `GAME.EMI#1 @ 0x801d0df0` | At timer expiry, enters local mode 0, opens the selection effect, and advances the local state. |
| `GAME.EMI#1 @ 0x801d104c` | Intercepts Start: an early transition is redirected to state 2; a later open-popup transition closes the mode, queues cue `0x105`, and redirects to state 7. |
| `GAME.EMI#1 @ 0x801d0e54` | After the selection effect closes, resets frontend phases, initializes `GAME.EMI#0`, requests scenario index 16 through `0x801a7704`, and installs the `GAME.EMI#0 @ 0x80197068` callback loop. |

`GAME.EMI#1 @ 0x801d12cc` draws the reviewed New Game/Load prompt panels,
labels, and selection marker. The 900-tick expiry is not yet proven to launch
the inactivity demo; the reviewed function only advances the local title state.

## Scenario 16 bridge

`GAME.EMI#0 @ 0x801a7704` stores scenario index 16, waits for its SCENA payload,
then enters the scenario-local dispatch path. In `SCENA16.EMI#0`,
`0x801f6d90` branches on the current area identifier and has distinct paths for
area IDs 2, 4, and 31. A tracked reference table independently describes area
31 as an intro area used after waiting at the title screen. These facts do not
prove that scenario index 16 and area ID 31 are the same identifier, nor do they
yet identify the transition that selects area 31.

## Unresolved transitions

- The callback or state that launches the inactivity demo after title input
  remains unknown.
- The New Game path from the prompt into character-name entry remains unknown.
- The name editor's code owner remains unknown. The established destination is
  the mutable character record at `0x80144968`: its five-byte name occupies
  offsets `0x00` through `0x04`.
- Recover the `GAME.EMI#0` callback tables at `0x801c7b08`, `0x801c7b14`,
  `0x801c7b44`, `0x801c7b54`, `0x801c7b7c`, `0x801c7b88`, `0x801c7b98`,
  `0x801c7ba4`, and `0x801c7bb0`, then follow callbacks that write the mutable
  character-name bytes. Do not promote semantic names before those xrefs are
  verified from the original payload.

## GAME.EMI#0 lift queue

The first callback-table recovery pass established these additional
boundaries:

- `0x80197378` is lifted and structurally near-exact; instruction scheduling
  remains to be resolved.
- `0x80198170`, `0x801981b4`, and `0x801981d4` split the former
  `0x80198170..0x80198234` span and match exactly.
- `0x80198bc4` is an exact function ending at `0x80198c38`; the following
  dispatch/update region begins with distinct functions at `0x80198c38`,
  `0x80198cac`, `0x80198f1c`, `0x801990d0`, `0x801991b8`, `0x80199230`,
  `0x801992b8`, `0x80199308`, `0x80199368`, `0x80199398`, `0x801993f0`,
  `0x80199418`, and `0x80199440`.
- `0x801c7b44[1]` points to `0x80199368`; it calls the scenario-transition
  helper at `0x801a782c`, then the frontend callback at `0x801d1740`, before
  finalizing the shared frontend frame. `0x801c7b7c[1]`,
  `0x801c7b98[1]`, and `0x801c7ba4[1]` likewise point to `0x80199398`,
  `0x801993f0`, and `0x80199418` respectively.

The remaining `asm` entries are unresolved spans, not proven single functions.
Recover internal code/data boundaries from the original payload before changing
any span to C.

| Priority | Tracked span | Size | Reason |
| --- | --- | ---: | --- |
| 1 | `0x80199440..0x801A1AE4` | `0x86A4` | Large unresolved code/data span after the recovered callback/update entries; split code, tables, strings, and padding before lifting. |
| 2 | `0x8019615C..0x80196F78` | `0xE1C` | Early entry-state implementation; split before lifting. |
| 3 | `0x80197378` | — | Lifted callback is structurally close; resolve the remaining instruction scheduling difference. |

For the name-entry path, prioritize xrefs from the callback tables above and
writes to `0x80144968..0x8014496C`. For the inactivity path, begin at the title
timer transition and trace the callback installed after the 900-tick expiry.
