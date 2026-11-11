#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, already bound at that address
 * by this target's map as g_battle_work, the name this target's exact lifts
 * clearWorkBytesBmagicMagic21303_801EF850 and
 * incrementWorkByteARecordCountersThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic21303_801EF23C
 * use. This body reads the cell once, so the access stays one plain pointer
 * load that precedes frame creation. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * the name needs no map row, and every access here is byte-addressed, so the
 * local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared helper in the main executable, outside this overlay's image. Its
 * generated undefined_funcs_auto.txt row already binds the address, so the
 * prototype adds no map row. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF490
 * @behavior Overlay step handler selected by table, not by call: the pointer
 *           word at 0x801EF8B0 is an entry of the handler run based at
 *           0x801EF8A0, which dispatchStateByte1Tablef8a0BmagicMagic21303_801EEFC0
 *           indexes with work byte 0x01 (index 4) and which
 *           dispatchStateByte2Tablef8a4BmagicMagic21303_801EF004 indexes with
 *           work byte 0x02 from the run's second word 0x801EF8A4 (index 3); no
 *           jal word in the payload names 0x801EF490. It reads the scratchpad
 *           work object once through the pointer cell at 0x1F800044 and tests
 *           its unsigned byte 0x0B with no store in between. Only when that
 *           byte is zero does it read the shared battle work-record pointer at
 *           0x801EB4E0, decrement that record's byte 0x0B by one at byte width
 *           (lbu / addiu -1 / sb through the single pointer load) and invoke the
 *           shared helper at 0x801E5988, the store filling the call delay slot.
 *           The cell load precedes frame creation because the test needs no
 *           stack; the 0x18-byte frame only keeps $ra at 0x10 across the call.
 *           Takes no arguments, returns nothing through the jr $ra at
 *           0x801EF4D8 and reads or writes no other state. The roles of work
 *           object byte 0x0B and record byte 0x0B are not established, so both
 *           stay offset-anchored. The 80-byte span is byte-identical to the
 *           payload spans of the exact lifts
 *           decrementGlobalWorkByteBAndInvokeHelperWhenWorkByteBZeroBmagicMagic08303_801EF744
 *           and
 *           decrementGlobalWorkByteBAndInvokeHelperWhenWorkByteBZeroBmagicMagic06703_801F033C,
 *           proven by one payload comparison per span in this lane; the first of
 *           them supplies the body transcribed here, unchanged, and only the
 *           own-target facts above differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperWhenWorkByteBZeroBmagicMagic21303_801EF490(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0) {
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
