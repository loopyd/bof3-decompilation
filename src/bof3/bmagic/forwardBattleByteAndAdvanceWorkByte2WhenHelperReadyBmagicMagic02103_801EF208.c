#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern void func_800A9820(u8 arg0); /* @source 0x800A9820 @kind function */

/* @source 0x801EF208
 * @behavior Calls the shared readiness helper at 0x8014DAEC. When its
 *           returned byte is nonzero, forwards the unsigned battle byte at
 *           0x80146384 to 0x800A9820, then reads the scratchpad work pointer
 *           and increments work byte 2 modulo 256. Otherwise changes no work
 *           bytes. Takes no arguments and returns nothing. The entry appears
 *           at payload word 0x801EEC18; helper and field roles beyond these
 *           accesses remain address- or offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteAndAdvanceWorkByte2WhenHelperReadyBmagicMagic02103_801EF208(void) {
  if (func_8014DAEC() != 0) {
    func_800A9820(D_80146384);
    g_battle_work[2]++;
  }
}
