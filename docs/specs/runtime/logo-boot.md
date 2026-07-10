---
type: Runtime flow
title: Logo boot path
description: Secondary LOGO.EXE handoff from the main BOF3 executable.
tags: [runtime, boot, logo, slus]
---

# Logo Boot Path

This document records the currently proven boot-logo branch that is separate from the `GAME.EMI` front-controller path.

## Proven `SLUS` Handoff

`SLUS_004.22` contains an explicit secondary executable handoff to `LOGO/LOGO.EXE`.

@source: 0x8014aee0 FUN_8014aee0
@source: 0x8014e0fc FUN_8014e0fc

Observed pseudocode:

```c
void boot_logo_exe(void) {
  load_secondary_psx_exe("\\LOGO\\LOGO.EXE;1");
  DAT_80143ea0 = &DAT_801ff000;
  DAT_80143ea4 = 0;
  StopCallback();
  PadStop();
  frontend_shutdown_a();
  frontend_shutdown_b();
  Exec(&DAT_80143e80, 0, 0);
  frontend_resume();
  setup_display();
}
```

Current interpretation:

- `SLUS` does not play the splash/video content directly
- it loads a secondary PS-X EXE and then transfers execution with `Exec`
- the splash/logo path is therefore a sibling boot branch, not part of the `GAME.EMI` main-menu runtime

## Secondary EXE Loader Helper

`FUN_8014e0fc` is the current best local name for the PS-X EXE load helper used by the boot path.

@source: 0x8014e0fc FUN_8014e0fc

Observed behavior:

- retries `CdSearchFile` for the requested path up to 10 times
- issues `CdControl` / `CdControlB` reads
- copies the first loaded sector into `DAT_80143e70`
- uses header fields from that sector to drive a second read into the target address range
- returns `0` on success or `0xffffffff` on failure

Current interpretation:

- the first read is parsing the PS-X EXE header
- the second read loads the executable body into the destination described by that header

## `CAPCOM30.STR`

This part is now locally proven.

@source: 0x801cedfc FUN_801cedfc
@source: 0x801ce760 FUN_801ce760
@source: 0x801ce930 FUN_801ce930
@source: 0x801cea98 FUN_801cea98
@source: 0x801cebfc FUN_801cebfc

Observed pseudocode:

```c
void logo_exe_main(void) {
  CdInit();
  logo_stream_boot(&DAT_8003b800, 0x2d78c);
  while ((PadRead(0) & 0x800) == 0) {
    if (logo_stream_tick()) {
      break;
    }
  }
  logo_stream_shutdown();
}
```

`0x2d78c` is decimal `186252`, which matches the disc LBA of
`LOGO/CAPCOM30.STR`.

Current interpretation:

- `LOGO.EXE` streams `CAPCOM30.STR` directly by absolute sector/LBA
- this path does not go back through the generic top-level slot table helper
- `LOGO.EXE` initializes CD, MDEC, display state, and audio fade locally
- the splash exits either when the STR stream completes or when pad bit `0x800`
  is pressed

Proven chain:

- `SLUS -> LOGO.EXE` is proven
- `LOGO.EXE -> CAPCOM30.STR` is now proven
- the next EXE-side continuation after `LOGO.EXE` returns is `0x8014ea80`,
  documented in `first-overlay.md`
