#include "bof3/bof3.h"

extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern void func_800AFC98(s32 arg0, s32 arg1); /* @source 0x800AFC98 @kind function */
extern void func_801E257C(u8 index, u8 scriptByte); /* @source 0x801E257C @kind function */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* @source 0x801EF164
 * @behavior Handler referenced by the pointer at 0x801EEC14. Calls the shared
 *           readiness helper at 0x8014DAEC; when its returned byte is nonzero,
 *           invokes 0x800AFC98 with (2, 0), then 0x801E257C with the unsigned
 *           battle byte at 0x80146374 and 0xB. Reads the scratchpad work pointer
 *           once, seeds word 0x14 to 0x100 and word 0x20 to -8, then increments
 *           work byte 2 modulo 256. Takes no arguments and returns nothing.
 *           Unestablished helper and field roles remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeBattleHelpersSeedWorkWordsAndAdvanceWorkByte2WhenHelperReadyBmagicMagic00100_801EF164(void) {
  u8 *work;

  if (func_8014DAEC() != 0) {
    func_800AFC98(2, 0);
    func_801E257C(D_80146374, 0xB);
    work = SPAD_PTR_SLOT(u8, 0x44u);
    *(s32 *)(work + 0x14) = 0x100;
    *(s32 *)(work + 0x20) = -8;
    work[2]++;
  }
}
