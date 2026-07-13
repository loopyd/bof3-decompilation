#include "bof3/context.h"
#include "internal.h"

extern void func_8017e028(s32 arg0);

extern s32 DAT_80145e14;
extern s32 DAT_80145e18;
extern s32 DAT_80145e1c;
extern s32 DAT_80145e20;
extern s32 DAT_80145e24;
extern s32 DAT_80145e28;
extern s32 DAT_80145e2c;

/* @behavior opens and enables the boot-side event set used after the logo path
 * hands back to the disc/runtime layer.
 * @source 0x8014b1a4 FUN_8014b1a4
 */
void func_8014b1a4(void) {
  func_8017ee0c();
  DAT_80145e14 = OpenEvent(0xF4000001u, 4, 0x2000, NULL);
  DAT_80145e18 = OpenEvent(0xF4000001u, 0x100, 0x2000, NULL);
  DAT_80145e1c = OpenEvent(0xF4000001u, 0x2000, 0x2000, NULL);
  DAT_80145e20 = OpenEvent(0xF4000001u, 0x8000, 0x2000, NULL);
  DAT_80145e24 = OpenEvent(0xF0000011u, 4, 0x2000, NULL);
  DAT_80145e28 = OpenEvent(0xF0000011u, 0x100, 0x2000, NULL);
  DAT_80145e2c = OpenEvent(0xF0000011u, 0x8000, 0x2000, NULL);
  func_8017ee1c();
  EnableEvent(DAT_80145e14);
  EnableEvent(DAT_80145e18);
  EnableEvent(DAT_80145e1c);
  EnableEvent(DAT_80145e20);
  EnableEvent(DAT_80145e24);
  EnableEvent(DAT_80145e28);
  EnableEvent(DAT_80145e2c);
  func_8017e028(1);
  func_8017e07c();
  _bu_init();
  ChangeClearPAD(0);
}
