#include "bof3/bof3.h"

/* Shared function at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h declares it
 * `void func_80196718(void* arg0)`, and the exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9SetByteATo2ThenAdvanceWorkByte1BmagicMagic11700_801EEC90
 * calls the same address with the same one-argument shape. */
void func_80196718(void* arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the record
 * it is handed and returns the submitted effect byte.
 * include/bof3/battle/battle03_internal.h declares
 * `s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)`, and
 * the exact sibling submitRecordStoreEffectByteBClearWorkByte9SetByteATo2Then-
 * AdvanceWorkByte1BmagicMagic11700_801EEC90 passes the same 0/0/0/1 offsets. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Battle record published at 0x8014638C, outside this payload (0x801EEC00-
 * 0x801F0BE8) and shared with the other bmagic overlays: this body hands the
 * loaded pointer to both shared calls. Its role is not proven, so the name stays
 * address-anchored and Splat binds it through the generated
 * undefined_syms_auto.txt row. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row and read once per access: a store through the loaded pointer may alias the
 * cell, so the original re-materialises the base before each of the three
 * work-object accesses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F082C
 * @behavior Index 0 of the three-entry handler table at 0x801F0BDC that the
 * work-byte-2 dispatcher at 0x801F07E8 resolves. It loads the battle record
 * pointer published at 0x8014638C through one cell-address local whose base stays
 * in a saved register across the first call, submits that record through the
 * shared function at 0x80196718, then submits its positional effect through the
 * shared helper at 0x8019651C with the 0/0/0 offsets and mode 1 and stores the
 * low return byte - the submitted effect byte - in the scratchpad work object's
 * byte 0xB. It then clears the work object's countdown byte 9 and increments that
 * object's state byte 2 with a byte-width read-modify-write (lbu, addiu $v0,$v0,1,
 * sb), advancing the work-byte-2 dispatcher sequence. The work-object pointer
 * cell at 0x1F800044 is re-read for each of the three accesses, so the original
 * keeps three separate cell loads. The record pointer is loaded through the
 * cell-address local because the two loads of 0x8014638C share one lui/addiu base
 * kept in a saved register across the first call; writing the two accesses on the
 * named symbol instead folds the address into each load and loses that base. The
 * 0x20-byte frame holds that saved register, $ra and the stack slot of the fifth
 * func_8019651C argument. The function takes no arguments, returns nothing and
 * reads or writes no other state. Seeded from the exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9SetByteATo2ThenAdvanceWorkByte1BmagicMagic11700_801EEC90
 * (emi/bmagic/magic117/00@0x801EEC90), which is the same body plus one work-byte
 * 0xA store and increments byte 1 where this span increments byte 2; the whole
 * 0x8C-byte payload was also compared against every other overlay binary and no
 * byte-identical twin exists.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitRecordStoreEffectByteBClearWorkByte9AndIncrementWorkByte2BmagicMagic06000_801F082C(
    void) {
  u8** recordCell;

  recordCell = &D_8014638C;
  func_80196718(*recordCell);
  g_battle_work[0xB] = func_8019651C(*recordCell, 0, 0, 0, 1);
  g_battle_work[9] = 0;
  g_battle_work[2]++;
}
