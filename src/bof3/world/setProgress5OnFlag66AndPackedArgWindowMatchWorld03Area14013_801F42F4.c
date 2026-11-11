#include "bof3/bof3.h"

/* Front-side byte of the shared 0x80143F00 game-mode tuple. The sibling
 * overlay request helpers of this family gate on it, and this body reads it
 * with `lbu` before the frame is created.
 * @source 0x80143F03 @kind data */
extern u8 D_80143F03;

/* Shared main-RAM flag bank of the main executable, used only as the base
 * argument of the shared flag tester. Splat binds the address through this
 * target's generated undefined-syms map, so no target-map row is needed.
 * @source 0x80144F28 @kind data */
extern u8 D_80144F28;

/* Progress byte of the shared state-entry tuple 0x801448EB..0x801448ED, which
 * the sibling world handlers of this family write with one `sb` per arm.
 * @source 0x801448EC @kind data */
extern u8 D_801448EC;

/* Shared flag tester of the main executable, outside this payload: it answers
 * whether the flag bit selected by its second argument is set in the supplied
 * flag bank.
 * @source 0x8015B5D4 */
extern s32 func_8015B5D4(u32 bank, s32 flag);

/* Shared front-end startup helper of the main executable, outside this
 * payload. Takes no arguments and returns nothing.
 * @source 0x8015C088 */
extern void func_8015C088(void);

/* @source 0x801F42F4
 * @behavior Overlay request that starts one front-end step from a packed map
 *           word pair. While the front-side byte D_80143F03 reads zero it
 *           tests flag 0x66 of the main-RAM flag bank 0x80144F28 through the
 *           shared flag tester func_8015B5D4, and returns 0 as soon as that
 *           flag is clear. With the flag set it requires the first packed word
 *           to equal 0x558000 exactly and the second word's high half to fall
 *           in 0xA..0xB (the `sra 16`, `addiu -0xA`, `sltiu 2` triple), and
 *           returns 0 when either test fails. On a full match it runs the
 *           shared front-end startup helper func_8015C088 and stores progress
 *           5 into D_801448EC; every path returns 0. The 0x20-byte frame keeps
 *           $s0 and $s1 across the call, the whole-word copy of each argument
 *           into its saved register precedes the `$ra` store, and the 0x558000
 *           constant materialises in the delay slot of the flag test's result
 *           branch.
 *
 * Name evidence: D_80143F03 is the front-side byte the sibling overlay
 * requests return on, D_80144F28 the main-RAM flag bank and func_8015B5D4 its
 * tester, func_8015C088 the shared front-end startup helper of the world
 * family, and D_801448EC the progress byte of the shared state-entry tuple, so
 * the body is one of that family's progress requests. The two incoming words
 * carry no evidenced role beyond the fixed edge constant and the two-value
 * high-half window, and no image word of this target holds 0x801F42F4, so the
 * body is reached by the overlay loader and both argument roles stay
 * structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setProgress5OnFlag66AndPackedArgWindowMatchWorld03Area14013_801F42F4(s32 arg0, s32 arg1) {
  if (D_80143F03 != 0) {
    return 0;
  }
  if (func_8015B5D4((u32)&D_80144F28, 0x66) == 0) {
    return 0;
  }
  if (arg0 != 0x558000) {
    return 0;
  }
  if ((u32)((arg1 >> 16) - 0xA) >= 2u) {
    return 0;
  }
  func_8015C088();
  D_801448EC = 5;
  return 0;
}
