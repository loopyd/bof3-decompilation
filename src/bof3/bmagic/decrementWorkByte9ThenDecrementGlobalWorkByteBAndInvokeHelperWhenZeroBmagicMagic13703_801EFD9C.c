#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044 as g_battle_work, the name
 * its exact lifts clearWorkByte9SetWorkByteATo88AndIncrementWorkByte2Bmagic
 * Magic13703_801EFCC0 and dispatchWorkByte2HandlerThenInvokeFDF8WhenWorkByte0
 * NonzeroBmagicMagic13703_801EFC58 already use. This body reads the cell once,
 * so the single access stays one plain pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * no target-map row is added, and every access here is byte-addressed, so the
 * local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFD9C
 * @behavior Entry index 3, the last word, of the four-entry work-byte-2 step
 *           table at 0x801F0114: the payload word at 0x801F0120 holds this
 *           address and is the only word in the image that does, so the entry is
 *           reached through that table and has no jal caller. Reads the
 *           scratchpad work object through the pointer cell at 0x1F800044 and
 *           decrements its unsigned byte 9 in place (lbu / addiu -1 / sb /
 *           andi 0xFF / bnez), testing the stored byte truncated to eight bits
 *           rather than a reload, so the rest of the body runs only when that
 *           countdown reaches zero. On zero it reads the shared battle work
 *           record pointer at 0x801EB4E0, decrements that record's byte 0xB by
 *           one at byte width and invokes the shared helper at 0x801E5988, the
 *           store filling the call delay slot. The pointer-cell load precedes
 *           frame creation because the countdown needs no stack; the 0x18-byte
 *           frame only keeps $ra at 0x10 across the call. Takes no arguments,
 *           returns nothing and reads or writes no other state.
 *
 *           The 92-byte span is byte-for-byte identical to the same-named
 *           bodies at emi/bmagic/magic006/03 0x801EF41C, magic041/03
 *           0x801EF564, magic062/03 0x801EF61C, magic067/03 0x801F0068,
 *           magic076/03 0x801EF63C, magic082/03 0x801EF6C4, magic088/03
 *           0x801EF630, magic110/03 0x801EF93C, magic111/03 0x801EF2D4,
 *           magic126/03 0x801EF0BC, magic131/03 0x801F02B0, magic167/03
 *           0x801EF454, magic222/03 0x801EF770 and magic223/03 0x801EF18C,
 *           compared on all fifteen payloads, so this body came from the clone
 *           lever: the seed was
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZ
 *           eroBmagicMagic00603_801EF41C, whose metadata recorded a 100.00 gate
 *           match when the seed was taken, with this target's own pointer-cell
 *           and record-pointer bindings written from its bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic13703_801EFD9C(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B]--;
    func_801E5988();
  }
}
