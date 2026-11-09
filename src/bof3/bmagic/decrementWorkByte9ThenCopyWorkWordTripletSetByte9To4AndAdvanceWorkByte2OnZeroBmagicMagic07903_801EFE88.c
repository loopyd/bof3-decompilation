#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm span func_801EF130 still references it, so this lift needs no
 * target-map row. The offsets read here (words 0x34, 0x38 and 0x3C) are the ones
 * the exact sibling
 * copyRecordTripletMinus4000Plus4000000SetBytes9BTo10AndAToB0ThenAdvanceWorkByte2BmagicMagic07903_801EEE64
 * adjusts from the adjacent record cell 0x8014638C; the layout is shared, the
 * address is this target's own. */
extern u8 *D_8014637C; /* @source 0x8014637C @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. Declared as the named cell the exact siblings on this target use: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), and
 * the original re-reads the cell before each later store group because a store
 * through the loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFE88
 * @behavior Entry 0 of this overlay's four-entry work-byte-2 handler table based
 *           at 0x801F036C, the table the per-frame dispatcher at 0x801EFE00
 *           indexes with the scratchpad work object's byte 0x2 and calls through
 *           jalr, so no in-image jal names this address. The body reads the
 *           work-object pointer cell at 0x1F800044 once, decrements the work
 *           object's unsigned countdown byte 0x9 by one and stores it back at
 *           byte width, masks the stored register with andi 0xFF and returns
 *           through the branch delay slot's nop while that byte is not zero.
 *           Only when the countdown reaches zero does it load the record pointer
 *           published at 0x8014637C, re-read the pointer cell, copy the record
 *           words 0x34 and 0x38 straight into the work object, hold record word
 *           0x3C in a local, store 4 into work byte 0x9, re-read the cell again
 *           after that store, store the held word 0x3C through the earlier
 *           pointer, and increment work byte 0x2 at byte width through the last
 *           reload, advancing the work-byte-2 dispatcher to the next handler
 *           step. Leaf with no frame; takes no arguments, returns nothing and
 *           reads or writes no other state. The bodies of the exact siblings
 *           decrementWorkByte9Subtract40FromWorkDistanceThenResetByte9To20AndAdvanceWorkByte2OnZeroBmagicMagic07903_801EF1DC
 *           (countdown prefix) and
 *           copyWorkWordTripletSetBytes9To4AndAToB0ThenAdvanceWorkByte2BmagicMagic07903_801EFC74
 *           (record-triplet copy arm) supply both halves of this shape; the
 *           uninstantiated asm span func_801EF130 carries the same prefix and the
 *           0x8014637C triplet with the subtract-0x4000 arm of 0x801EEE64, so
 *           this body is authored from those spellings rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyWorkWordTripletSetByte9To4AndAdvanceWorkByte2OnZeroBmagicMagic07903_801EFE88(void) {
  u8 *source;
  u8 *step;
  u8 *work;
  u8 *next;
  u8 countdown;
  u32 value;

  step = g_battle_work;
  countdown = step[9] - 1;
  step[9] = countdown;
  if (countdown == 0) {
    source = D_8014637C;
    work = g_battle_work;
    *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
    *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
    value = *(u32 *)(source + 0x3C);
    work[9] = 4;
    next = g_battle_work;
    *(u32 *)(work + 0x3C) = value;
    next[2]++;
  }
}
