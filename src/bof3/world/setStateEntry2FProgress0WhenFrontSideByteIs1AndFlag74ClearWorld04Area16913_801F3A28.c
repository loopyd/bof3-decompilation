#include "bof3/bof3.h"

/* Shared main-executable cells of the front-end state block. All of them lie
 * outside this overlay's payload; this target's generated undefined-syms map
 * binds them, so no target-map row is needed. */
/* @source 0x80143F03 @kind data */
extern u8 D_80143F03;
/* @source 0x80144F28 @kind data */
extern u8 D_80144F28[];
/* @source 0x801448EB @kind data */
extern u8 D_801448EB;
/* @source 0x801448EC @kind data */
extern u8 D_801448EC;

/* Shared flag tester and UI refresh of the main executable. */
/* @source 0x8015B5D4 @kind function */
extern s32 func_8015B5D4(u32 arg0, s32 arg1);
/* @source 0x8015C088 @kind function */
extern void func_8015C088(void);

/* @source 0x801F3A28
 * @behavior Requires the shared front-side byte D_80143F03 to read 1 and flag
 *           0x74 of the main-RAM flag bank D_80144F28 to be clear, which it
 *           reads through the shared flag tester func_8015B5D4. Only on that
 *           path it runs the shared UI refresh func_8015C088 once, writes 0x2F
 *           to the shared state-entry byte D_801448EB and clears the shared
 *           progress byte D_801448EC, then returns 1; a front-side byte other
 *           than 1 or a set flag returns 0 and touches nothing else. Takes no
 *           arguments; the 0x18-byte frame keeps only the return address and
 *           both reject paths return through the single shared epilogue.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setStateEntry2FProgress0WhenFrontSideByteIs1AndFlag74ClearWorld04Area16913_801F3A28(
    void) {
  if (D_80143F03 != 1) {
    return 0;
  }
  if (func_8015B5D4((u32)&D_80144F28, 0x74) != 0) {
    return 0;
  }
  func_8015C088();
  D_801448EB = 0x2F;
  D_801448EC = 0;
  return 1;
}
