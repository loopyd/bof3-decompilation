#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original re-materialises the cell with its own lui / lw pair before
 * each of the four reads this body makes (0xCC, 0x108, 0x134 and 0x144), and
 * every reload sits behind stores issued through the pointer or through the
 * pool bytes loaded before it, so each read stays written at its own site on
 * the named cell. The two slot writes keep one pointer local each, $v1 for the
 * first entry and $a0 for the second, so neither load shares a live range with
 * the other block's slot index. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches each field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one shared
 * record base, so every addressed field keeps its own flat symbol: byte +0x01
 * and the word at +0x74 of one entry, the same two fields the exact sibling of
 * this family fills. Splat already binds both addresses through this target's
 * generated undefined_syms_auto.txt. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared action-slot allocator in the main executable: claims a slot for the
 * given kind/mode pair and returns the index of the entry it took, or 0xFF when
 * the pool holds no free entry. This body masks the answer to a byte and never
 * tests it, so the 0xFF arm is not part of this span. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Target-kind roll and record flag raiser this step runs after it has queued
 * the two slots: it picks the current battle target's kind, rolls the record
 * delta and stores work byte 4. It takes no argument and returns nothing. */
void raiseBattleTargetKindFlagFromRecordDeltaRollThenStoreWorkByte4BmagicMagic06000_801EEF50(void); /* @source 0x801EEF50 */

/* @source 0x801EECA4
 * @behavior Work-byte-1 handler of this overlay, index 0 of the five-entry code
 *           pointer run at 0x801EEC04 (0x801EECA4, 0x801EED68, 0x801EEE00,
 *           0x801EEE84 and 0x801EEF04, terminated by a zero word) that the
 *           dispatcher func_801EEC34 copies into its 0x30-byte frame and
 *           selects with the scratchpad work object's byte at offset 0x1
 *           scaled by four. This step runs unconditionally: it takes no
 *           argument, returns nothing, guards on nothing and leaves the work
 *           object's byte 0x1 to be advanced only at its very end. It claims an
 *           action slot through the shared allocator func_801E590C(1, 0x12)
 *           twice, masks each returned index to a byte and scales it by the
 *           0x78-byte slot stride, and never tests the allocator's 0xFF answer.
 *           The first entry receives 0 in its byte at pool offset +0x01 and the
 *           scratchpad work-object pointer in its word at +0x74, with the
 *           pointer cell read once between the two shifts of the stride product
 *           so the load fills that multiply instead of following it. The second
 *           entry receives 1 in byte +0x01 and the re-read pointer in word
 *           +0x74, and there the stride product completes before the cell is
 *           read into the call-argument register. It then calls
 *           raiseBattleTargetKindFlagFromRecordDeltaRollThenStoreWorkByte4BmagicMagic06000_801EEF50
 *           with no argument and a nop in the delay slot, clears the work
 *           object's byte 0xB through a fresh read of the pointer cell, and
 *           closes by re-reading that cell again to increment the work object's
 *           byte 0x1. The 0x18-byte frame keeps only $ra at 0x10. The roles of
 *           work bytes 1 and 0xB and of the slot fields stay unestablished, so
 *           the byte offsets anchor the name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueTwoSlotsSetByte1ToZeroAndOnePublishWorkPointerRaiseBattleTargetKindFlagClearWorkByteBAndAdvanceWorkByte1BmagicMagic06000_801EECA4(void) {
  u8 *work;
  u8 *work2;
  u32 slot;
  u32 offset;

  slot = (u8)func_801E590C(1u, 0x12u);
  offset = slot * 0x78;
  work = g_battle_work;
  D_801EC331[offset] = 0;
  *(u32 *)&D_801EC3A4[offset] = (u32)work;
  slot = (u8)func_801E590C(1u, 0x12u);
  offset = slot * 0x78;
  work2 = g_battle_work;
  D_801EC331[offset] = 1;
  *(u32 *)&D_801EC3A4[offset] = (u32)work2;
  raiseBattleTargetKindFlagFromRecordDeltaRollThenStoreWorkByte4BmagicMagic06000_801EEF50();
  g_battle_work[0xB] = 0;
  g_battle_work[1]++;
}
