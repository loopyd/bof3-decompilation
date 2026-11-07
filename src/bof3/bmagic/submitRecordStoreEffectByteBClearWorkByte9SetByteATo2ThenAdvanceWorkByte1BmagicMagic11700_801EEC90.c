#include "bof3/bof3.h"

/* Shared function at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h declares it
 * `void func_80196718(void* arg0)`, and the exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic01000_801EEC80
 * calls the same address with the same one-argument shape. */
void func_80196718(void* arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the
 * record it is handed and returns the submitted effect byte.
 * include/bof3/battle/battle03_internal.h declares
 * `s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)`, and
 * the exact sibling submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWork-
 * Byte1BmagicMagic01000_801EEC80 passes the same 0/0/0/1 offsets. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Battle record published at 0x8014638C: this body hands the loaded pointer to
 * both shared calls. Its role is not proven, so the name stays address-anchored;
 * Splat keeps binding it through the generated undefined_syms_auto.txt row,
 * which the still-asm spans func_801EF06C, func_801EF20C and func_801EEE40 of
 * this overlay keep alive. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row and read once per access: a store through the loaded pointer may alias the
 * cell, so the original re-materialises the base before each of the four
 * work-object stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEC90
 * @behavior Loads the battle record pointer published at 0x8014638C once per
 * call: it first submits that record through func_80196718, then submits its
 * positional effect through func_8019651C with the 0/0/0 offsets and mode 1 and
 * stores the low return byte - the submitted effect byte - in the scratchpad
 * work object's byte 0xB. It then clears the work object's countdown byte 9,
 * sets its mode byte 0xA to 2 and advances its state byte 1 with a byte-width
 * read-modify-write. The work-object pointer cell at 0x1F800044 is re-read for
 * the byte 0xB store, the byte 9 clear, the byte 0xA store and the byte 1
 * increment, so the original keeps four separate cell loads. The record pointer
 * is loaded through a cell-address local - the two loads of 0x8014638C share one
 * `lui`/`addiu` base kept in a saved register across the first call; writing the
 * two accesses on the named symbol instead folds the address into each load and
 * loses that base. The 0x20-byte frame holds the saved register, $ra and the
 * stack slot of the fifth func_8019651C argument. The function takes no
 * arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitRecordStoreEffectByteBClearWorkByte9SetByteATo2ThenAdvanceWorkByte1BmagicMagic11700_801EEC90(
    void) {
  u8** recordCell;

  recordCell = &D_8014638C;
  func_80196718(*recordCell);
  g_battle_work[0xB] = func_8019651C(*recordCell, 0, 0, 0, 1);
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 2;
  g_battle_work[1]++;
}
