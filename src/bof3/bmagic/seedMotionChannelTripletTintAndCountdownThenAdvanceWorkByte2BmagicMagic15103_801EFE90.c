#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484
 * copies its words 0x34/0x38/0x3C into the same work words). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is a named cell
 * symbol, the form the exact sibling
 * copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484
 * uses: the symbolic address lets the assembler fold it into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)), and every later evaluation is separated by
 * a store through the previously loaded pointer, so each one reloads the cell.
 * The plain cell is kept rather than the volatile one several siblings use: the
 * reloads follow from the intervening stores and no asynchronous mutation is
 * evidenced, and the volatile spelling measured the same bytes here. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFE90
 * @behavior Seeds one work step from the shared battle work record read through
 *           0x801EB4E0: its words 0x34, 0x38 and 0x3C are copied into the
 *           scratchpad work object published at the pointer cell 0x1F800044,
 *           the tint triplet at work bytes 0x5D/0x5E/0x5F is set to
 *           0x10/0x10/0x24, work bytes 4, 0xB and 9 are cleared, work byte 0xA
 *           is set to 0x1E and work byte 2 is advanced with a byte-width
 *           read-modify-write (lbu/addiu/sb), which is the store in the return
 *           delay slot. The original materialises 0x10 once and re-uses that
 *           register for bytes 0x5D and 0x5E, and it re-reads the pointer cell
 *           before each later store because the stores through the loaded
 *           pointer may alias the cell; the leading word triplet and the 0x5D
 *           byte share one cell load, which is the shape of the exact sibling
 *           0x801EF484. The word store to 0x3C sits between the second cell load
 *           and that load's byte store in the original, which is scheduler
 *           placement and not statement order: writing the 0x3C copy after the
 *           0x5D byte store instead measured 30/45 instructions at 176 bytes.
 *           Leaf with no frame; nothing is returned and no other state is read
 *           or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedMotionChannelTripletTintAndCountdownThenAdvanceWorkByte2BmagicMagic15103_801EFE90(void) {
  s32 *src;
  s32 *dst;

  src = (s32 *)D_801EB4E0;
  dst = (s32 *)g_battle_work;
  dst[13] = src[13];
  dst[14] = src[14];
  dst[15] = src[15];
  ((u8 *)dst)[0x5D] = 0x10;
  g_battle_work[0x5E] = 0x10;
  g_battle_work[0x5F] = 0x24;
  g_battle_work[4] = 0;
  g_battle_work[0xB] = 0;
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0x1E;
  g_battle_work[2]++;
}
