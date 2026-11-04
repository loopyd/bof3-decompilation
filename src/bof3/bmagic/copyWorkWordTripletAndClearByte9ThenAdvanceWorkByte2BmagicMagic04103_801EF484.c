#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4 tests its
 * byte 0xB, incrementWorkByte2WhenGlobalWorkByte1Is2BmagicMagic15103_801EFD80
 * tests its byte 0x1). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is a named cell
 * symbol, the form the exact sibling
 * subtractWorkColorTripletBy3ThenAdvanceWorkByte2WhenChannelIs80BmagicMagic00800_801F11C4
 * uses: the symbolic address lets the assembler fold it into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)). An integer-constant cell address is instead
 * folded once by CSE into a full-address register (lui/ori) and reused, which
 * does not match. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF484
 * @behavior Copies the three words at offsets 0x34, 0x38 and 0x3C of the shared
 *           battle work record read through 0x801EB4E0 into the scratchpad work
 *           object published at the pointer cell 0x1F800044, clears that
 *           object's byte 9, then reads the pointer cell a second time and
 *           advances its byte 2 with a byte-width add (lbu/addiu/sb), which is
 *           the instruction in the return delay slot. The second cell read is in
 *           the original: the word stores through the first pointer may alias
 *           the cell, so the byte 2 bump goes through a freshly loaded pointer.
 *           The record is addressed through a word-typed view, the form that
 *           reproduces the original's word copies and its second cell load
 *           placement; a byte-typed view with offset casts schedules that load
 *           after the last word store. Leaf with no frame; nothing is returned
 *           and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484(void) {
  s32 *src;
  s32 *dst;

  src = (s32 *)D_801EB4E0;
  dst = (s32 *)g_battle_work;
  dst[13] = src[13];
  dst[14] = src[14];
  dst[15] = src[15];
  ((u8 *)dst)[9] = 0;
  g_battle_work[2]++;
}
