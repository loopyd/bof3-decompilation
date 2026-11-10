#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u8 *D_1F800044;

/* @source 0x801F04D8 @kind data */
extern void (*func_801F04D8[])(void);

/* @source 0x801F00C4 @kind function */
extern void func_801F00C4(void);

/* @source 0x801EFF24
 * @behavior Calls the handler selected by unsigned work byte 2 from the
 * pointer table at 0x801F04D8. Reloads the work-object pointer from scratchpad
 * cell 0x1F800044 after that call and invokes 0x801F00C4 when work byte 0 is
 * nonzero. Both calls take no arguments; no result is consumed or returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvoke00C4WhenWorkByte0NonzeroBmagicMagic06703_801EFF24(void) {
  u8 *work = D_1F800044;

  func_801F04D8[work[2]]();
  work = D_1F800044;
  if (work[0] != 0) {
    func_801F00C4();
  }
}
