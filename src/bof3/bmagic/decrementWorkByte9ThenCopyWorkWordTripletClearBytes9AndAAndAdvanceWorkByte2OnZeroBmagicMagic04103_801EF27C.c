#include "bof3/bof3.h"

/* Shared battle record pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484
 * copies its words 0x34/0x38/0x3C into the same work words). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row. The original re-materialises the base with its own lui $vN,0x1F80 /
 * lw $vN,0x44($vN) pair for the countdown, for the byte 0xA store and for the
 * byte 2 advance, because each store through the previously loaded pointer may
 * alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF27C
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044,
 *           decrements the work object's unsigned byte at offset 0x9 at byte
 *           width (lbu $v0,0x9($v1) / addiu $v0,$v0,-1 / sb / andi $v0,$v0,0xFF)
 *           and returns without further work while the stored value stays
 *           non-zero. Only when the countdown reaches zero does it copy the
 *           three words at offsets 0x34, 0x38 and 0x3C of the shared battle
 *           record read through 0x801EB4E0 into the work object, clear the work
 *           object's byte 9 and byte 0xA, and advance its work byte 2 at byte
 *           width (lbu / addiu $v0,$v0,1 / sb). The word triplet plus the byte
 *           9 clear share one work-object pointer load; the byte 0xA clear and
 *           the byte 2 advance each reload the cell, and the 0x3C word store
 *           lands between the byte 0xA reload and its byte store, which is
 *           scheduler placement and not statement order. Leaf with no frame;
 *           the function takes no arguments, returns nothing and reads or
 *           writes no other state. The overlay's leading pointer run at
 *           0x801EEC04 reaches this address through its word 6, so no jal in
 *           the image calls it.
 *
 *           Shape: the countdown guard copies the statement order and operand
 *           forms of the sibling
 *           decrementWorkByte9ThenSubmitRecordStoreEffectByte9AndAdvance-
 *           WorkByte1OnZeroBmagicMagic04103_801EEE14, and the triplet copy,
 *           byte clear and byte 2 advance copy the sibling
 *           copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2-
 *           BmagicMagic04103_801EF484, both of which measured exact. This body
 *           adds the byte 0xA clear between them and drops the submit and
 *           effect calls those two carry. A scan of every extracted payload
 *           under out/binaries found no copy of these 0x8C bytes, so no clone
 *           applied.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyWorkWordTripletClearBytes9AndAAndAdvanceWorkByte2OnZeroBmagicMagic04103_801EF27C(
    void) {
  s32 *src;
  s32 *dst;

  if (--g_battle_work[9] == 0) {
    src = (s32 *)D_801EB4E0;
    dst = (s32 *)g_battle_work;
    dst[13] = src[13];
    dst[14] = src[14];
    dst[15] = src[15];
    ((u8 *)dst)[9] = 0;
    g_battle_work[0xA] = 0;
    g_battle_work[2]++;
  }
}
