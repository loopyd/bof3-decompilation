#include "bof3/bof3.h"

extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C @kind function */
extern void func_801E257C(u8 index, u8 scriptByte); /* @source 0x801E257C @kind function */
extern u8 D_80146374; /* @source 0x80146374 @kind data */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF468
 * @behavior Handler registered by the pointer at 0x801EEC2C. Calls helper
 *           0x8014DAEC three times; when the third returned byte is nonzero,
 *           invokes 0x8015D71C with 0x203 and 0x801E257C with the unsigned
 *           byte at 0x80146374 and 0xC. Decrements byte 9 of the work object
 *           read through scratchpad cell 0x1F800044, then reloads the cell
 *           and subtracts 3 from work byte 2 if the decremented byte is
 *           nonzero, or adds 1 otherwise. Byte updates wrap modulo 256.
 *           Takes no arguments and returns nothing; unknown roles remain
 *           address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelpersThenDecrementWorkByte9AndRetreatOrAdvanceWorkByte2WhenReadyBmagicMagic00100_801EF468(void) {
  u8 *work;

  func_8014DAEC();
  func_8014DAEC();
  if (func_8014DAEC() != 0) {
    func_8015D71C(0x203);
    func_801E257C(D_80146374, 0xC);
    work = D_1F800044;
    if (--work[9] != 0) {
      work = D_1F800044;
      work[2] -= 3;
    } else {
      work = D_1F800044;
      work[2]++;
    }
  }
}
