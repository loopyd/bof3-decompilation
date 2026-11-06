#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is the named cell
 * symbol the exact sibling clearWorkBytesBmagicMagic12403_801F088C uses: the
 * symbolic address lets the assembler fold it into each load (lui $vN,0x1F80 /
 * lw $vN,68($vN)). The plain cell is kept rather than the volatile one several
 * siblings use: the original re-reads it before every store group because the
 * stores through the previously loaded pointer may alias the cell, and the
 * sibling seedMotionChannelTripletTintAndCountdownThenAdvanceWorkByte2
 * BmagicMagic15103_801EFE90 measured the volatile spelling identical on this
 * same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F028C
 * @behavior Per-frame seed of the overlay work object published at the
 *           scratchpad pointer cell 0x1F800044: it writes the tint triplet at
 *           work bytes 0x5D/0x5E/0x5F with 8/6/0xF, clears countdown work byte
 *           9, sets work byte 0xA to 2 and then advances work byte 2 - the
 *           handler index the frame driver 0x801F020C indexes its table
 *           D_801F08F4 with - through a byte-width read-modify-write
 *           (lbu/addiu/sb), which is the store in the return delay slot. The
 *           original re-reads the pointer cell before each store, so every
 *           store keeps its own cell load in the sibling's order; each store of
 *           a non-zero immediate materialises that constant into $v0 for the
 *           store's load delay slot, while the zero store leaves the slot
 *           unfilled. Leaf with no frame; nothing is returned and no other
 *           state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedTintTripletClearCountdownSetByteATo2AndAdvanceWorkByte2BmagicMagic12403_801F028C(void) {
  g_battle_work[0x5D] = 8;
  g_battle_work[0x5E] = 6;
  g_battle_work[0x5F] = 0xF;
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 2;
  g_battle_work[2]++;
}
