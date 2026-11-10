#include "bof3/bof3.h"

extern u32 D_8014686C; /* @source 0x8014686C @kind data */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */
extern s32 func_8015B5D4(u32 bank, s32 flag); /* @source 0x8015B5D4 */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* @source 0x801F56EC
 * @behavior Tests flag 4 through the shared tester at 0x8015B5D4 using the
 *           bank word loaded from 0x8014686C. Requests action 8 when the test
 *           is nonzero or action 3 when zero through 0x80150224, then sets
 *           the shared frontend mode byte at 0x80143BB0 to 2. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startAction8Or3BySharedFlag4SetFrontendMode2World02Area08213_801F56EC(void) {
  s32 action;

  action = func_8015B5D4(D_8014686C, 4) != 0 ? 8 : 3;
  func_80150224(action);
  D_80143BB0 = 2;
}
