#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, already bound at that address
 * by this target's map as g_battle_work, the name this target's exact lifts
 * reserveFirstFreeRecordBmagicMagic21903_801EF934 and
 * clearWorkBytes0Through4BmagicMagic21903_801EF998 use. This body reads the cell
 * three times: once per access group, because the byte stores in between may
 * alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * the name needs no map row, and every access here is byte-addressed, so the
 * local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared readiness helper of the shared main executable, outside this payload;
 * it answers an unsigned byte, which this body calls without a frame of its own
 * and never tests, so the answer is discarded here. This target's generated
 * undefined_funcs_auto.txt already binds the address. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */

/* This target's own helper that clears work bytes 0x00-0x04, declared with the
 * prototype its exact lift carries. */
extern void clearWorkBytes0Through4BmagicMagic21903_801EF998(void); /* @source 0x801EF998 */

/* @source 0x801EF5DC
 * @behavior Overlay step handler selected by table, not by call: the pointer
 *           word at 0x801EFA10 is entry 4 of the five-word handler run based at
 *           0x801EFA00, which dispatchStateByte1Tablefa00BmagicMagic21903_801EF024
 *           indexes with work byte 0x01, and no jal word in the payload names
 *           0x801EF5DC. The body first calls the shared readiness helper at
 *           0x8014DAEC with no arguments; the answer is discarded and neither a
 *           branch nor a store depends on it. It then reads the scratchpad work
 *           object through the pointer cell at 0x1F800044 and, only when that
 *           record's unsigned byte 0x0A is nonzero, decrements it by one at byte
 *           width (lbu / addiu -1 / sb through the single cell load). That store
 *           may alias the cell, so the countdown access reloads the cell: the
 *           body decrements work byte 0x09 the same way, keeps the truncated
 *           byte (andi 0xFF) and leaves the block only while it is nonzero. When
 *           byte 0x09 reaches zero the body loads the shared battle work-record
 *           pointer at 0x801EB4E0, decrements that record's byte 0x0B by one at
 *           byte width with the store filling the call delay slot, and invokes
 *           clearWorkBytes0Through4BmagicMagic21903_801EF998, which clears work
 *           bytes 0x00-0x04. The 0x18-byte frame only keeps $ra at 0x10 across
 *           the calls, the slots after the helper call and after the untaken
 *           branch stay nops, and the body returns through the jr $ra at
 *           0x801EF65C. Takes no arguments and returns nothing. The roles of work
 *           bytes 0x0A and 0x09 and of record byte 0x0B are not established, so
 *           they stay offset-anchored. A byte scan of the shipped payloads in
 *           this lane found this 0x88-byte span at this address only, so no twin
 *           supplied a seed and the body was transcribed from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndCountDownByte9ThenDecrementRecordByteBAndClearAtZeroBmagicMagic21903_801EF5DC(
    void) {
  func_8014DAEC();
  if (g_battle_work[0xA] != 0) {
    g_battle_work[0xA]--;
  }
  if (--g_battle_work[9] == 0) {
    D_801EB4E0[0xB]--;
    clearWorkBytes0Through4BmagicMagic21903_801EF998();
  }
}
