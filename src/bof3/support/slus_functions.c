#include "bof3/context.h"

/* SLUS startup, callback scheduler, and executable-file loading. */
WEAK_SYMBOL_AT(bootNoop, 0x8014aa04);
WEAK_SYMBOL_AT(initBootDisplayEnvs, 0x8014ae08);
WEAK_SYMBOL_AT(clearBootOtEntry, 0x8014ae9c);
WEAK_SYMBOL_AT(runLogoExe, 0x8014aee0);
WEAK_SYMBOL_AT(func_8014AFC0, 0x8014afc0);
WEAK_SYMBOL_AT(rebuildBootRenderTables, 0x8014b020);
WEAK_SYMBOL_AT(linkRenderOtPackets, 0x8014b0f0);
WEAK_SYMBOL_AT(captureBootVsync, 0x8014b17c);
WEAK_SYMBOL_AT(dispatchCallbackSlots, 0x8014b33c);
WEAK_SYMBOL_AT(func_8014B6B4, 0x8014b6b4);
WEAK_SYMBOL_AT(tickCallbackSlotScheduler, 0x8014b73c);
WEAK_SYMBOL_AT(installCallbackSlot, 0x8014b854);
WEAK_SYMBOL_AT(func_8014E22C, 0x8014e22c);
WEAK_SYMBOL_AT(func_8014E6D0, 0x8014e6d0);
WEAK_SYMBOL_AT(func_8014EA80, 0x8014ea80);
WEAK_SYMBOL_AT(frontLocalModeCallbackLoop, 0x8014ed6c);
WEAK_SYMBOL_AT(func_8015CEBC, 0x8015cebc);
WEAK_SYMBOL_AT(func_8015D044, 0x8015d044);

/* EMI loader and CD callback path. */
WEAK_SYMBOL_AT(emiCdSyncCallback, 0x801621e8);
WEAK_SYMBOL_AT(emiCdReadyCallback, 0x80162230);
WEAK_SYMBOL_AT(isEmiLoaderReady, 0x80162d00);
WEAK_SYMBOL_AT(func_80163010, 0x80163010);

/* Newly discovered SLUS services — pending decompilation. */
WEAK_SYMBOL_AT(yieldCallbackSlotScheduler, 0x8014b87c);
WEAK_SYMBOL_AT(appendRenderPrim, 0x8014e5a0);
WEAK_SYMBOL_AT(fadeLoop, 0x8014f514);
WEAK_SYMBOL_AT(drawFadeTile, 0x8014f704);
WEAK_SYMBOL_AT(func_80150098, 0x80150098);
WEAK_SYMBOL_AT(dispatchSoundCue, 0x8015df18);
WEAK_SYMBOL_AT(adjustBoundedCounter, 0x801655f4);

/* Remaining reviewed SLUS services, kept address-traceable pending promotion. */
