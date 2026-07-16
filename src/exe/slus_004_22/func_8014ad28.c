#include "bof3/context.h"
#include "internal.h"

extern void func_8014B1A4(void);
extern void func_8014B3C4(void);

/* @behavior initializes the post-logo disc/event path and hands control to the
 * first boot-side callback chain.
 * @source 0x8014AD28
 */
void func_8014AD28(void) {
  func_801748E4();
  emi_loader_initialize();

  if (D_8018B300 == 0) {
    VSync(0);
    func_8017AF0C(0);
    func_801753C4(0);
    D_8018B300 = 1;
    func_8017EE0C();
    func_8017ED7C(func_8017ED3C(0xF0000010, 0x1000, 0x1000, func_8014B3C4));
    func_8017EE1C();
  } else {
    VSync(0);
    func_8017AF0C(3);
  }

  func_80178660();
  func_801790C8(0x3e8);
  func_801790A8(0xa0, 0x78);
  func_80174668(0);
  func_8014B1A4();
  func_8017EEBC(0);
}
