#include "bof3/battle/battle15_internal.h"

/* @behavior Initialises the active battle selection state at 0x80148570 from
 * the active message-slot work record: the state head byte and its byte +0x6C
 * are written through the 0x80148570 base, every other state byte through its
 * own symbol, and bit 1 of the active selection slot's flag word +0x10 is
 * stored into 0x80148584.
 * @source 0x80097FE8
 * @status partial
 * @match 64.58
 * @residual 31/48 instructions at the original 192-byte size. The original
 *   holds the 0x80148570 base in $a0 for the head and +0x6C stores. This build
 *   folds both into symbol-relative $at stores, leaving $a0 free and shifting
 *   the work, slot and value register assignments from the first instruction.
 */
void func_80097FE8(void) {
  BattleSelectionState *sel;
  u8 *work;
  u8 *active;
  u8 value;
  u8 eight;
  u32 flags;

  sel = &D_80148570;
  eight = 8;
  sel->first = 1;
  work = D_801EBF08;
  active = D_801EB4D8;
  D_80148571 = eight;
  D_80148572 = 2;
  D_80148573 = 2;
  D_8014857B = 2;
  value = work[5];
  D_8014857D = 0xFF;
  D_80148578 = 2;
  D_80148579 = 0;
  D_8014857A = value;
  flags = *(u32 *)(active + 0x10);
  D_80148574 = 0x140;
  D_80148576 = 0x3F;
  D_80148584 = (s16)(flags & 2);
  D_80148656 = work[5];
  sel->field_6C = 0;
  D_801485DD = eight;
  D_801485DE = 0;
}
