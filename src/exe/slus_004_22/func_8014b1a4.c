#include "bof3/context.h"
#include "internal.h"

extern void func_8017E028(s32 arg0);

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
void func_8014B1A4(void) {
  EnterCriticalSection();
  D_80145E14 = OpenEvent(0xF4000001u, 4, 0x2000, NULL);
  D_80145E18 = OpenEvent(0xF4000001u, 0x100, 0x2000, NULL);
  D_80145E1C = OpenEvent(0xF4000001u, 0x2000, 0x2000, NULL);
  D_80145E20 = OpenEvent(0xF4000001u, 0x8000, 0x2000, NULL);
  D_80145E24 = OpenEvent(0xF0000011u, 4, 0x2000, NULL);
  D_80145E28 = OpenEvent(0xF0000011u, 0x100, 0x2000, NULL);
  D_80145E2C = OpenEvent(0xF0000011u, 0x8000, 0x2000, NULL);
  ExitCriticalSection();
  EnableEvent(D_80145E14);
  EnableEvent(D_80145E18);
  EnableEvent(D_80145E1C);
  EnableEvent(D_80145E20);
  EnableEvent(D_80145E24);
  EnableEvent(D_80145E28);
  EnableEvent(D_80145E2C);
  func_8017E028(1);
  func_8017E07C();
  _bu_init();
  ChangeClearPAD(0);
}
