#include "bof3/context.h"
#include "internal.h"

extern vu32 DAT_80146464;
extern vu8  DAT_8014648a;
extern vu32 DAT_8014648c;
extern vu8  DAT_801464a0[];
extern vu8  DAT_80146840;
extern u16  DAT_8014681a;

/* @behavior initializes the EMI/CD bootstrap state before the first active entry
 * is installed.
 * @source 0x80161f58 FUN_80161f58
 */
void func_80161f58(void) {
  s32 i;
  u8* slot_state;
  u32 bootstrap_address;
  u8  empty_state;
  vu32* loader_state;

  while (func_801753ec() == 0) {
  }

  func_80174700(3);

  bootstrap_address = 0x800E4800;
  empty_state = 0xff;
  i = 0x17;
  loader_state = &DAT_80146464;
  slot_state = (u8*)loader_state + 0x53;

  *loader_state = bootstrap_address;
  DAT_80146840 = 0;
  DAT_8014648a = 0;
  DAT_8014648c = 0;
  DAT_8014681a = 0xffff;

  do {
    *slot_state = empty_state;
    i -= 1;
    slot_state -= 1;
  } while (i >= 0);
}
