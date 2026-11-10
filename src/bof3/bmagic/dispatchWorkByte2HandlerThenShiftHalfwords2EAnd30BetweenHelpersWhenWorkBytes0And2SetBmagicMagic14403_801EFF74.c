#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F04FC[])(void); /* @source 0x801F04FC @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801F01D0(void); /* @source 0x801F01D0 @kind function */

/* @source 0x801EFF74
 * @behavior Entry at table word 0x801F04E8. Calls the handler indexed by
 *   unsigned work byte 2 in the table at 0x801F04FC without bounding the
 *   index, then re-reads the work pointer cell at 0x1F800044. When work bytes
 *   0 and 2 are both nonzero, calls 0x800AF764, re-reads the work pointer,
 *   subtracts 0x10 from unsigned halfword 0x2E when byte 0xB equals 1 or
 *   adds 0x10 otherwise, subtracts 0xC from unsigned halfword 0x30, stores
 *   both halfwords, and calls 0x801F01D0. All calls take no arguments and
 *   their results are unused. This entry takes no arguments and returns
 *   nothing. Helper roles and work fields remain address- or offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenShiftHalfwords2EAnd30BetweenHelpersWhenWorkBytes0And2SetBmagicMagic14403_801EFF74(void) {
  u8 *stepWork;
  u8 *gateWork;
  u8 *shiftWork;
  u16 x;
  u16 y;

  stepWork = g_battle_work;
  D_801F04FC[stepWork[2]]();
  gateWork = g_battle_work;
  if (gateWork[0] != 0 && gateWork[2] != 0) {
    func_800AF764();
    shiftWork = g_battle_work;
    if (shiftWork[0x0B] == 1) {
      x = *(u16 *)(shiftWork + 0x2E) - 0x10;
      y = *(u16 *)(shiftWork + 0x30);
    } else {
      x = *(u16 *)(shiftWork + 0x2E) + 0x10;
      y = *(u16 *)(shiftWork + 0x30);
    }
    y -= 0xC;
    *(u16 *)(shiftWork + 0x2E) = x;
    *(u16 *)(shiftWork + 0x30) = y;
    func_801F01D0();
  }
}
