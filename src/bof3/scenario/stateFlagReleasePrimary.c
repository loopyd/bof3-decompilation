#include "bof3/bof3.h"

/* @source 0x80146864 @kind data */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind data */
extern u8 D_80146865;
/* @source 0x80146866 @kind data */
extern u8 D_80146866;
/* @source 0x80146867 @kind data */
extern u8 D_80146867;
/* @source 0x8014686C @kind data */
extern u32 D_8014686C;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;
/* @source 0x80146875 @kind data */
extern u8 D_80146875;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);

/* @source 0x801F8B94
 * @behavior Primary state 6 handler of this overlay: it acts only while the
 * shared sub-state byte D_80146875 (0x80146875) reads zero and the shared
 * scenario progress byte g_ScenarioProgress (0x80146864) reads 0x18, and
 * otherwise returns without touching anything. On that entry it sets flag bit
 * 0xA of the shared scenario flag word D_8014686C through the shared bit-set
 * helper func_8015B580 (0x8015B580), which receives that word loaded with
 * `lui`/`lw` and the bit index 0xA, then runs the shared front-end release
 * helper func_8015C058 (0x8015C058), then clears the three shared main-RAM
 * state bytes 0x80146865, 0x80146866 and 0x80146867, clears the shared
 * sub-state byte D_80146875 and requests primary scene state 0 in the shared
 * primary state byte D_80146874. It leaves the progress byte alone, takes no
 * arguments and returns nothing, and its 0x18-byte frame keeps $ra across the
 * two calls and $s0 for the sub-state pointer, which the body keeps live from
 * the guard read to the store after both calls. The three state bytes and the
 * primary state byte each materialise the 0x8014 page in $at on their own
 * while D_80146875 is stored through that saved pointer, so the store order
 * 0x80146865, 0x80146866, 0x80146867, D_80146875, D_80146874 is the source
 * statement order, and the constant 0x18 is materialised into $v0 in the delay
 * slot of the D_80146875 test to feed the second comparison. Its only image
 * word is 0x801FD600, word 6 of the thirty-two-word primary handler table at
 * 0x801FD5E8, so the primary state dispatcher
 * dispatchPrimaryStateHandlerScenarioScena0500_801F83EC (0x801F83EC) reaches
 * the address only through that table word indexed by D_80146874 and no
 * in-image `jal` targets it. No span of any payload under `out/binaries`
 * compares equal to its 136 bytes, so it has no byte-identical twin; its
 * helper-and-clear tail repeats the shape of the exact in-target sibling
 * clearProgressAndSharedStateBytesThenRequestPrimaryState0DScenarioScena0500_801FCBD4
 * (0x801FCBD4), which runs the same clears and stores 0xD where this body
 * stores 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateFlagReleasePrimary(void) {
  u8 *substate = &D_80146875;

  if (*substate == 0 && g_ScenarioProgress == 0x18) {
    func_8015B580(D_8014686C, 0xA);
    func_8015C058();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    *substate = 0;
    D_80146874 = 0;
  }
}
