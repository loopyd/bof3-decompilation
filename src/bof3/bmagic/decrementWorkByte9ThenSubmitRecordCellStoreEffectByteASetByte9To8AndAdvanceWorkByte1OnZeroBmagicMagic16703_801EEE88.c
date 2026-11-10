#include "bof3/bof3.h"

/* Shared main-executable helpers; signatures are also declared in
 * include/bof3/battle/battle03_internal.h. */
extern void func_80196718(void* record); /* @source 0x80196718 @kind function */
extern s32 func_8019651C(void* record, s32 x, s32 y, s32 z, s32 mode); /* @source 0x8019651C @kind function */

extern u8* D_8014638C; /* @source 0x8014638C @kind unknown */
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE88
 * @behavior Decrements work byte 9 with byte-width wraparound. When it reaches
 * zero, submits the record read from cell 0x8014638C through func_80196718,
 * re-reads that cell for func_8019651C with arguments 0, 0, 0, 1, stores the
 * low return byte in work byte A, sets byte 9 to 8 and increments byte 1.
 * The work pointer is re-read for each tail update. The record cell is not
 * read on the nonzero countdown path, and each call reads its current value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSubmitRecordCellStoreEffectByteASetByte9To8AndAdvanceWorkByte1OnZeroBmagicMagic16703_801EEE88(void) {
  u8** record_cell;

  if (--g_battle_work[9] == 0) {
    record_cell = &D_8014638C;
    func_80196718(*record_cell);
    g_battle_work[0xA] = func_8019651C(*record_cell, 0, 0, 0, 1);
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
