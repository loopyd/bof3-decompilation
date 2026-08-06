#include "bof3/context.h"
#include "internal.h"

extern void openBootEventSet(void);
extern void func_8014B3C4(void);

/* @behavior initializes the post-logo disc/event path and hands control to the
 * first boot-side callback chain.
 * @source 0x8014AD28
 */
void initBootDiscEvents(void) {
  ResetCallback();
  initEmiLoader();

  if (D_8018B300 == 0) {
    VSync(0);
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    D_8018B300 = 1;
    EnterCriticalSection();
    func_8017ED7C(func_8017ED3C(HwCPU, EvSpTRAP, EvMdINTR, func_8014B3C4));
    ExitCriticalSection();
  } else {
    VSync(0);
    ResetGraph(3);
  }

  InitGeom();
  SetGeomScreen(0x3e8);
  SetGeomOffset(0xa0, 0x78);
  PadInit(0);
  openBootEventSet();
  func_8017EEBC(0);
}
