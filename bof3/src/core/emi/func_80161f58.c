#include "internal.h"
#include "bof3/original_symbols.h"

extern vu32 DAT_80146464;
extern vu8  DAT_8014648a;
extern vu32 DAT_8014648c;
extern vu8  DAT_801464a0[];
extern vu8  DAT_80146840;
extern s16  DAT_8014681a;

/* does: initializes the EMI/CD bootstrap state before the first active entry
 * is installed.
 * @source: 0x80161f58 FUN_80161f58
 */
void func_80161f58(void) {
  s32 i;
  u8* slot_state;

  while (func_801753ec() == 0) {
  }

  func_80174700(3);

  DAT_80146464 = 0x800E4800;
  DAT_80146840 = 0;
  DAT_8014648a = 0;
  DAT_8014648c = 0;
  DAT_8014681a = -1;

  i = 0x17;
  slot_state = (u8*)&DAT_801464a0[0x17];

  do {
    *slot_state = 0xff;
    i -= 1;
    slot_state -= 1;
  } while (i >= 0);
}
