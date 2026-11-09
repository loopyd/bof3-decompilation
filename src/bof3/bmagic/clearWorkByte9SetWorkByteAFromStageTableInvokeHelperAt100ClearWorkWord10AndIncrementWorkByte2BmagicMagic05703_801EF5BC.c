#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, published as g_battle_work
 * by the sibling lifts of this module (for example
 * clearWorkByte9SetWorkByteATo88AndIncrementWorkByte2BmagicMagic06703_801EFF8C
 * and decrementWorkByte9ThenCopyRecordTripletAdd800000ToWord3CSetDirectionAnd
 * AngleToward8014638CRearmByte9AndAdvanceWorkByte2BmagicMagic00603_801EEF28).
 * The original re-materialises the cell with its own lui 0x1F80 / lw 0x44 pair
 * at every access, because a store through the loaded pointer may alias the
 * cell, so this lift reaches the cell through the map symbol and lets each
 * statement that stores re-read it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Stage reload-count byte table at 0x801EFADC, the three-byte tail segment of
 * this overlay image. This body and the per-frame stage dispatcher
 * func_801EF620 read it the same way (lbu of work byte 0x9, addu into the table
 * base, lbu), the dispatcher doing so right after it advances byte 0x9
 * (lbu / addiu 1 / sb / andi 0xFF) and writes the selected byte to work byte
 * 0x0A, which it then counts down one per frame. Byte 0x9 is therefore the
 * stage index of a three-entry sequence and byte 0x0A the per-stage frame
 * countdown; what the three entries 0x0A, 0x0E, 0x0E mean beyond that indexing
 * is not established, so the table keeps its address-anchored name. */
extern u8 D_801EFADC[]; /* @source 0x801EFADC @kind data */

/* One-argument overlay helper whose argument every caller in this tree sets to
 * an encoded constant, here 0x100. Bound by this target's generated
 * undefined_funcs_auto.txt row. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EF5BC
 * @behavior Stage reset of the scratchpad work object published through the
 *           pointer cell at 0x1F800044. It clears work byte 0x09, the stage
 *           index the per-frame dispatcher func_801EF620 advances through the
 *           three-entry byte table at 0x801EFADC, so this body restarts the
 *           sequence at stage 0. It then re-reads the cell, because the byte
 *           store may alias it, and loads the byte at the new stage index into
 *           work byte 0x0A through that table base; the store of the table byte
 *           fills the delay slot of the call to the shared helper
 *           func_8015D71C, which is given the selector 0x100. Work byte 0x0A is
 *           the per-stage frame countdown the same dispatcher decrements, so
 *           this body arms one stage's worth of frames.
 *           Last it reads the cell once more and, through that one pointer,
 *           loads work byte 0x02, clears work word 0x10 and stores back byte
 *           0x02 plus one. Byte 0x02 is the handler index with which the state
 *           handler func_801EF55C indexes the three-word table at 0x801EEC38,
 *           and the three stores keep the original's order: the byte 0x02 load
 *           first, then the word 0x10 store, then the byte 0x02 store after
 *           $ra is reloaded.
 *           The cell load for that group happens once and serves all three
 *           accesses, so the pointer is held in a local; the two earlier groups
 *           each re-read the cell after their own store.
 *           It reads or writes no other state, takes no arguments and returns
 *           nothing through the jr $ra at 0x801EF618 inside a 0x18-byte frame
 *           that keeps $ra at 0x10. Work word 0x10 stays offset-anchored: this
 *           body only clears it, while the phase handlers at 0x801EF7A0 and
 *           0x801EF7E0 ramp it between -3 and zero.
 *           This 0x64-byte span is entry 0 of the three-word code pointer table
 *           at 0x801EEC38 (0x801EF5BC, 0x801EF620, 0x801EF81C), which the state
 *           handler func_801EF55C copies into its own 0x28-byte frame and
 *           indexes with work byte 0x02 before jalr, and no jal in the image
 *           names it. Because this body stores byte 0x02 plus one, the next
 *           frame of that state runs entry 1, the phase dispatcher at
 *           0x801EF620.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9SetWorkByteAFromStageTableInvokeHelperAt100ClearWorkWord10AndIncrementWorkByte2BmagicMagic05703_801EF5BC(void) {
  u8 *work;
  u8 count;

  g_battle_work[9] = 0;
  g_battle_work[0xA] = D_801EFADC[g_battle_work[9]];
  func_8015D71C(0x100);
  work = g_battle_work;
  count = work[2];
  *(s32 *)(work + 0x10) = 0;
  work[2] = count + 1;
}
