#ifndef EXE_LOGO_SYMBOLS_VARIABLES_H
#define EXE_LOGO_SYMBOLS_VARIABLES_H

#include "bof3/bof3.h"

/* CAPCOM30.STR base LBA used by the LOGO.EXE scheduler. */
extern u_long capcomStrLba; /* @source 0x801D8BB0 @kind data */

extern u8 D_801EB440; /* @source 0x801EB440 @kind unknown */
extern s32 D_801EB444; /* @source 0x801EB444 @kind unknown */
extern s32 D_801EB448; /* @source 0x801EB448 @kind unknown */
extern s32 D_801EB44C; /* @source 0x801EB44C @kind unknown */
extern s32 D_801EB450; /* @source 0x801EB450 @kind unknown */
extern s32 D_801EB454; /* @source 0x801EB454 @kind unknown */
extern CdlATV D_801EB470; /* @source 0x801EB470 @kind unknown */

/* CAPCOM30.STR stream state for the frame pump func_801CEA98 and the stream
 * setup func_801CE930. D_801EB458 selects one of the two CD/MDEC work buffers
 * stored at 0x801EB44C/0x801EB450; D_801EB44C_BUFFERS is the array view of
 * that pair, a macro over D_801EB44C because the map owns the address under the
 * scalar element-0 name func_801CEBFC reads. D_801EB45C is the 24x240 frame
 * column RECT advanced by func_801CEC88, D_801EB464/D_801EB468 hold the
 * previous and current stream frame numbers, and D_801EB46C is the stream
 * transfer flag that func_801CEC88 spins on and func_801CECA4 completes. The
 * two D_801EB474/D_801EB475 stream flags and the D_801EB4AC status byte are
 * polled through func_801CFEF4 during the func_801CE930 stream setup.
 *
 * D_801EB46C is volatile because func_801CECA4, the MDEC output callback
 * installed for the stream, publishes it while func_801CEC88 busy-waits; the
 * original reloads the flag on every spin iteration. */
#define D_801EB44C_BUFFERS ((u_long**)&D_801EB44C)
extern u_long D_801EB458; /* @source 0x801EB458 @kind unknown */
extern RECT D_801EB45C; /* @source 0x801EB45C @kind unknown */
extern u_long D_801EB464; /* @source 0x801EB464 @kind unknown */
extern u_long D_801EB468; /* @source 0x801EB468 @kind unknown */
extern volatile s32 D_801EB46C; /* @source 0x801EB46C @kind unknown */
extern u8  D_801EB474; /* @source 0x801EB474 @kind unknown */
extern u8  D_801EB475; /* @source 0x801EB475 @kind unknown */
extern u8  D_801EB4AC; /* @source 0x801EB4AC @kind unknown */

/* LOGO.EXE double-buffered display environments produced by func_801CE7F4:
 * two 0x14-byte DISPENVs at 0x801EB480/0x801EB494 and the 0/1 buffer index
 * selected by func_801CE8DC at 0x801EB4A8. */
extern DISPENV D_801EB480[2]; /* @source 0x801EB480 @kind unknown */
extern s32     D_801EB4A8;    /* @source 0x801EB4A8 @kind unknown */

/* Word published by func_801D263C for the stream's MDEC output callback
 * func_801CECA4, which consumes and clears it once the pending state is
 * handled. */
extern s32 D_801EB678; /* @source 0x801EB678 @kind unknown */

#endif
