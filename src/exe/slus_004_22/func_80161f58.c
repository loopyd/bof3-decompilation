#include "bof3/context.h"
#include "internal.h"

extern vu32 D_80146464;
extern vu8  D_8014648A;
extern vu32 D_8014648C;
extern vu8  D_801464A0[];
extern vu8  D_80146840;
extern u16  D_8014681A;

/* @behavior initializes the EMI/CD bootstrap state before the first active entry
 * is installed.
 * @source 0x80161F58
 */
void func_80161F58(void) {
  s32   i;
  u8*   slot_state;
  u32   bootstrap_address;
  u8    empty_state;
  vu32* loader_state;

  while (func_801753EC() == 0) {
  }

  VSync(3);

  bootstrap_address = 0x800E4800;
  empty_state = 0xff;
  i = 0x17;
  loader_state = &D_80146464;
  slot_state = (u8*)loader_state + 0x53;

  *loader_state = bootstrap_address;
  D_80146840 = 0;
  D_8014648A = 0;
  D_8014648C = 0;
  D_8014681A = 0xffff;

  do {
    *slot_state = empty_state;
    i -= 1;
    slot_state -= 1;
  } while (i >= 0);
}
