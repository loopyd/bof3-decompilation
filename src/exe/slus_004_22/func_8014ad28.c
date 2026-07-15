#include "bof3/context.h"
#include "internal.h"

extern void func_8014b1a4(void);
extern void func_8014b3c4(void);

/* @behavior initializes the post-logo disc/event path and hands control to the
 * first boot-side callback chain.
 * @source 0x8014ad28 FUN_8014ad28
 */
void func_8014ad28(void) {
  func_801748e4();
  emi_loader_initialize();

  if (D_8018B300 == 0) {
    VSync(0);
    func_8017af0c(0);
    func_801753c4(0);
    D_8018B300 = 1;
    func_8017ee0c();
    func_8017ed7c(func_8017ed3c(0xF0000010, 0x1000, 0x1000, func_8014b3c4));
    func_8017ee1c();
  } else {
    VSync(0);
    func_8017af0c(3);
  }

  func_80178660();
  func_801790c8(0x3e8);
  func_801790a8(0xa0, 0x78);
  func_80174668(0);
  func_8014b1a4();
  func_8017eebc(0);
}
