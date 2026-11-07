#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed, and the only
 * access here is byte-addressed, so the local declaration keeps the byte view
 * only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original re-materialises the cell for each of the three accesses (lui
 * $vN,0x1F80 / lw $vN,68($vN)) because the store into byte 0xB separates the two
 * reads of the loaded pointer, so a plain cell is enough and no volatile
 * qualifier is needed. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF628
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once (lui / lw)
 *           and bounds its byte at offset 0xB against two unsigned (lbu /
 *           sltiu), so the guard is taken for the values 0x00 and 0x01. Only
 *           on that path it reads the scratchpad work pointer cell at
 *           0x1F800044 and stores 1 into the work object's byte at offset 0xB;
 *           that constant is materialised in the branch delay slot (addiu
 *           $v0,$zero,1) and stored from the same register. It then re-reads
 *           the cell with its own lui/lw pair and increments the work object's
 *           byte at offset 0x2 modulo 256 (lbu / addiu 1 / sb), advancing the
 *           overlay's work byte 2 handler sequence. Leaf with no frame; takes no
 *           arguments, returns nothing and reads or writes no other state.
 *
 *           0x801EF628 is word index 7 of the pointer run at 0x801F0388 (that
 *           run holds 0x801EF31C, 0x06000206, 0x04000006, 0x00060000,
 *           0x801EF404, 0x801EF578, 0x801EF5D8, 0x801EF628, 0x801EF67C in
 *           order), and this target's dispatcher at 0x801EF2D8 indexes that run
 *           with the work record's byte at offset 0x1 and calls the word it
 *           finds, so the entry is reached through that table and has no jal
 *           caller in the payload.
 *
 *           Bytes 0x801EF648..0x801EF673 are byte-identical to the tail of the
 *           exact sibling add4ToWorkByteAThenSetWorkByteBToOneAndAdvanceWorkByte2
 *           WhenAtLeast20BmagicMagic15103_801F0C50 (0x801F0C7C..0x801F0CA7), so
 *           the "store byte 0xB from the delay-slot constant, reload the cell,
 *           advance byte 0x2" statement forms, reload count and register choice
 *           are taken from that lift unchanged. The guard follows
 *           incrementWorkByte2WhenGlobalWorkByteBBelow2BmagicMagic08803_801EF5EC,
 *           which bounds the same record byte below two unsigned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByteBToOneAndAdvanceWorkByte2WhenGlobalWorkByteBBelow2BmagicMagic22503_801EF628(void) {
  if (D_801EB4E0[0xB] < 2u) {
    g_battle_work[0xB] = 1;
    g_battle_work[2]++;
  }
}
