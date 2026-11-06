#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. It holds the record being stepped: this overlay's work-byte-1
 * dispatcher at 0x801EF534 reads the record's byte 1 through it, and
 * func_801EF7A0 decrements the record's byte 0xB through it before calling the
 * clear below. The cell is read once per access, so the original
 * re-materializes its address (lui $v0,0x1F80 / lw $v0,68($v0)) before each
 * byte store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFDB0
 * @behavior Clears the first five bytes (offsets 0x00-0x04) of the scratchpad
 * work record published through the pointer cell at 0x1F800044, storing $zero
 * with sb for each byte. The cell is read once per access, so the original
 * re-materializes the cell address and reloads the work pointer before every
 * store; the last sb sits in the return delay slot. Its only in-image caller is
 * func_801EF7A0, which calls it when the record's byte 0xB countdown reaches
 * zero. Takes no arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytesBmagicMagic02103_801EFDB0(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
