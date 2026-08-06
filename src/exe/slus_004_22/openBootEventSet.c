#include "bof3/context.h"
#include "internal.h"

extern s32 D_80145E14;
extern s32 D_80145E18;
extern s32 D_80145E1C;
extern s32 D_80145E20;
extern s32 D_80145E24;
extern s32 D_80145E28;
extern s32 D_80145E2C;

/* @behavior opens and enables the boot-side event set used after the logo path
 * hands back to the disc/runtime layer.
 * @source 0x8014B1A4
 */
void openBootEventSet(void) {
  EnterCriticalSection();
  D_80145E14 = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
  D_80145E18 = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
  D_80145E1C = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
  D_80145E20 = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
  D_80145E24 = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
  D_80145E28 = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
  D_80145E2C = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
  ExitCriticalSection();
  EnableEvent(D_80145E14);
  EnableEvent(D_80145E18);
  EnableEvent(D_80145E1C);
  EnableEvent(D_80145E20);
  EnableEvent(D_80145E24);
  EnableEvent(D_80145E28);
  EnableEvent(D_80145E2C);
  InitCARD(1);
  func_8017E07C();
  _bu_init();
  ChangeClearPAD(0);
}
