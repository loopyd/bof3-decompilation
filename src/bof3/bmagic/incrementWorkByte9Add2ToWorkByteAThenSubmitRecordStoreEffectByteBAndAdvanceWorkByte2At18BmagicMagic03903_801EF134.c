#include "bof3/bof3.h"

/* Shared function at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h declares it
 * `void func_80196718(void* arg0)`, and the exact bmagic sibling
 * submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWorkByte1BmagicMagic
 * 01000_801EEC80 calls the same address with the same one-argument shape. */
void func_80196718(void* arg0);

/* Shared effect helper at 0x8019651C: submits a positional effect for the
 * record it is handed and returns the submitted effect byte.
 * include/bof3/battle/battle03_internal.h declares
 * `s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)`, and
 * the exact sibling submitRecordStoreEffectByteBClearWorkByte9ThenAdvanceWork
 * Byte1BmagicMagic01000_801EEC80 passes the same 0/0/0/1 arguments and stores
 * the low return byte in the work object's effect byte 0xB. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Battle record published at 0x8014638C: handed to both shared calls by the
 * exact bmagic sibling submitRecordStoreEffectByteBClearWorkByte9ThenAdvance
 * WorkByte1BmagicMagic01000_801EEC80, which also keeps the cell address in a
 * saved register so its two loads share one lui/addiu base. Its role is not
 * proven, so the name stays address-anchored. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row and re-read for every access group: a store through the loaded pointer
 * may alias the cell, so the original materialises the lui/lw pair four times. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF134
 * @behavior One stage of this overlay's per-frame work byte 2 handler sequence:
 *           adds 1 to the scratchpad work-object byte 0x9 and adds 2 to the
 *           work-object byte 0xA, both as byte-width read-modify-writes (lbu /
 *           addiu / sb) that re-read the pointer cell at 0x1F800044 first. The
 *           value just stored into byte 0xA is zero-extended with andi 0xFF -
 *           no second load - and compared against 0x18, materialised with
 *           addiu $v1,$zero,0x18. Only when byte 0xA has reached 0x18 does the
 *           body take the battle record published at 0x8014638C, submit it
 *           through the shared func_80196718, then submit its positional effect
 *           through the shared func_8019651C with the 0/0/0 offsets and mode 1
 *           and store the low return byte - the submitted effect byte - in the
 *           work object's byte 0xB, and finally increment the work object's
 *           dispatch state byte 0x2 to advance this sequence. The record cell
 *           address is built once in a saved register so both loads of
 *           0x8014638C share one base, and the work-object pointer cell is
 *           re-read for the byte 0xB store and again for the byte 0x2
 *           increment. The 0x20-byte frame holds that saved register, $ra and
 *           the stack slot of the fifth func_8019651C argument; the pointer
 *           load for byte 0x9 is issued before the frame is created. The
 *           function takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9Add2ToWorkByteAThenSubmitRecordStoreEffectByteBAndAdvanceWorkByte2At18BmagicMagic03903_801EF134(
    void) {
  u8** recordCell;

  g_battle_work[9] += 1;
  if ((g_battle_work[0xA] += 2) == 0x18) {
    recordCell = &D_8014638C;
    func_80196718(*recordCell);
    g_battle_work[0xB] = func_8019651C(*recordCell, 0, 0, 0, 1);
    g_battle_work[2]++;
  }
}
