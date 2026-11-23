#include "bof3/bof3.h"

/* @source 0x80143BB0 @kind data */
extern u8 D_80143BB0;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;
/* @source 0x80146875 @kind data */
extern u8 D_80146875;

void func_80150224(s32 arg0);
void func_8015C058(void);

/* @source 0x801FC778
 * @behavior Primary state 22 handler of this overlay, reached through word 22
 * (0x801FD640) of the thirty-two-word primary handler table at 0x801FD5E8 that
 * callPrimaryRunHandler (0x801F83EC) indexes
 * with the shared primary state byte D_80146874, so no in-image `jal` targets it
 * and that table word is its only image reference. It drives the shared
 * sub-state byte D_80146875 (0x80146875) through two steps and returns without
 * touching anything for any other sub-state value. Sub-state 0 starts action
 * 0x4C through the shared action helper func_80150224 (0x80150224), latches the
 * shared phase byte D_80143BB0 (0x80143BB0) to 2 and advances the sub-state byte
 * to 1. Sub-state 1 waits while that phase byte reads 2 and, once it changes, runs
 * the shared front-end release helper func_8015C058 (0x8015C058), clears the
 * sub-state byte and requests primary scene state 0 in the shared primary state
 * byte D_80146874 (0x80146874). It takes no arguments and returns nothing, and
 * its 0x18-byte frame keeps $ra across the two calls and $s0 for the sub-state
 * pointer, whose `lui`/`addiu` pair is scheduled between the two prologue saves.
 * The sub-state byte is read once into $v1 for the two-case test, the constants 1
 * and 2 are materialised into $v0 in the delay slots of that test so the 2 feeds
 * the phase-byte comparison in sub-state 1, the phase byte is stored through $at
 * while the sub-state byte is stored through $s0, and both zero stores of
 * sub-state 1 materialise the 0x8014 page in $at on their own, so the store order
 * D_80143BB0 then D_80146875 in sub-state 0 and D_80146875 then D_80146874 in
 * sub-state 1 is the source statement order. The seed is the exact in-target
 * sibling audioStartAction84
 * (0x801FC2F0): these 36 words differ from that body at exactly three of them,
 * the action id immediate passed to func_80150224 (0x4C here, 0x84 there) and
 * the two absolute `j` words, each shifted by the -0x488 delta between the two
 * spans, so this was a relocation-different twin and not a clone. The same three
 * differences hold against audioStartAction26
 * (0x801F8C1C, delta -0x3B5C) and
 * audioActionLatchRelease
 * (0x801F92EC, delta -0x348C).
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioStartAction4C(void) {
  u8 *substate = &D_80146875;

  switch (*substate) {
  case 0:
    func_80150224(0x4C);
    D_80143BB0 = 2;
    *substate = 1;
    break;
  case 1:
    if (D_80143BB0 != 2) {
      func_8015C058();
      *substate = 0;
      D_80146874 = 0;
    }
    break;
  }
}
