#include "bof3/bof3.h"

/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80145F09 @kind unknown - party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;
/* @source 0x8014933B @kind unknown */
extern u8 D_8014933B;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FD7F4
 * @behavior Overlay handler reached only indirectly: the address is word 5
 * (the word stored at 0x801FE4D0) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that dispatchCoordinateLookupHandlerScenarioScena0900_801FD280
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 0x0E) and calls with jalr, and that dispatcher returns this
 * handler's signed low byte, so no in-image jal or image data word of this
 * payload addresses 0x801FD7F4. It first reads the party byte of local work
 * record 0 (D_80145F09, 0x80145F09, read with lui/lbu). While that byte holds
 * 5 it walks a three-step chain of the two-stage wait func_8015B5D4 on the
 * shared flag word D_8014686C (0x8014686C, reloaded with lui/lw into $a0 before
 * each probe) for wait ids 0x0D, then 0x10, then 0x0F. The first wait that
 * reports zero means the wait is done, so it runs the shared front-end startup
 * helper func_8015C088, arms the shared sub-state byte D_80146875 with 2, 3 or
 * 6 for wait 0x0D, 0x10 or 0x0F respectively, answers 1 and requests primary
 * scene state 6 through the shared state byte D_80146874. When all three waits
 * are still running it instead raises the shared byte D_8014933B to 3 and
 * answers 1 without touching D_80146874, so that arm leaves the primary scene
 * state unchanged. When the party byte holds anything other than 5 no wait is
 * taken at all: it runs func_8015C088, arms sub-state 0 and requests primary
 * scene state 6. It takes no arguments, reads no state beyond that one party
 * byte and the wait word, and its 0x18-byte frame only keeps $ra across the
 * calls; the $ra save fills the bne delay slot, the 0x8014933B arm holds its
 * constant 3 in $v0 while the byte store materialises the 0x8014 page in $at,
 * and every other arm answers 1 through the constant in a j delay slot. This
 * 0xFC-byte span sits at table word 5, one address after the table word 4 at
 * 0x801FD6F8: that neighbour carries the same statement order and differed only
 * in its three wait-id immediates and its shared-byte constant, so it seeded
 * this body as shape evidence rather than clone proof. No shipped EMI payload
 * under out/binaries held a byte-identical copy of this span when checked, so no
 * clone candidate exists.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitFlags0D100FForParty5ThenRequestPrimaryState6Substate2Or3Or6ElseSetSharedByte3ScenarioScena0900_801FD7F4(void) {
  if (D_80145F09 == 5) {
    if (func_8015B5D4(D_8014686C, 0x0D) == 0) {
      func_8015C088();
      D_80146875 = 2;
      D_80146874 = 6;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x10) == 0) {
      func_8015C088();
      D_80146875 = 3;
      D_80146874 = 6;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x0F) == 0) {
      func_8015C088();
      D_80146875 = 6;
      D_80146874 = 6;
      return 1;
    }
    D_8014933B = 3;
    return 1;
  }
  func_8015C088();
  D_80146875 = 0;
  D_80146874 = 6;
  return 1;
}
