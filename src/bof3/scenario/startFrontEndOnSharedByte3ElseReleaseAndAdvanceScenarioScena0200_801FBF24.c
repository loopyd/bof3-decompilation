#include "bof3/bof3.h"

extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146875;

/* Shared front-end startup helper at 0x8015C088. */
void func_8015C088(void);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);

/* @source 0x801FBF24
 * @behavior Takes no arguments and returns nothing. It branches on the shared
 * main-RAM byte D_80146866 (0x80146866), which it reads through one address
 * taken into a local pointer and therefore keeps in $s0: when that byte reads 3
 * it runs the shared front-end startup helper func_8015C088 and leaves every
 * byte alone; any other value runs the shared overlay release helper
 * func_8015C058, clears the next shared byte D_80146867, seeds the shared
 * sub-state byte D_80146875 with 8 and advances D_80146866 by one. Its
 * 0x18-byte frame only keeps $ra and $s0. The one in-image jal caller is
 * func_801F8740 (0x801F8740) at 0x801F8C5C, which polls D_80146867 for 2, 6 or
 * 0xB and ignores the result.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The u8 `next` local is the source shape: it pins the D_80146866 load at the
 * statement before the D_80146875 seed, which is where the original has it,
 * instead of leaving the read attached to the increment store.
 */
void startFrontEndOnSharedByte3ElseReleaseAndAdvanceScenarioScena0200_801FBF24(void) {
  u8 *state = &D_80146866;

  if (*state == 3u) {
    func_8015C088();
  } else {
    u8 next;

    func_8015C058();
    D_80146867 = 0;
    next = *state;
    D_80146875 = 8;
    *state = next + 1;
  }
}
