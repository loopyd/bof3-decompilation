#include "bof3/battle/battle15_internal.h"

/* @behavior Initialises the secondary battle selection state at 0x80148570 from
 * the saved group/scroll/cursor bytes of table row 3: the state head byte and
 * its byte +0x6C are written through the 0x80148570 base, the remaining state
 * bytes through their own symbols, and the saved bytes are copied into the
 * state row 0x8014857A..0x8014857C.
 * @source 0x8009AF84
 * @status partial
 * @match 56.52
 * @residual live byte-match DIFFER from +0x0000: instruction count and range
 *   size now agree (46 instructions, 184 bytes), but the original materialises
 *   the 0x80148570 base into a register (lui/addiu) and reaches the head byte as
 *   `sb v0,0x0(a0)`, with `sb zero,0x6C(a0)` at the tail; every clean-C spelling
 *   measured here lets the assembler fold those two accesses to symbol-relative
 *   `lui $at` stores, so the state byte stores are re-scheduled and the constant
 *   registers shift. Measured shapes (instructions matched of 46): `Battle
 *   SelectionState *` base, retained here, 26/46 with 184 bytes; plain `u8 *`
 *   base 26/46 with 184 bytes; `volatile u8 *` pointee 26/47 with 188 bytes -
 *   the base then materialises, but into $v0, and the +0x6C store rematerialises;
 *   head byte through the base with the tail byte absolute 26/47.
 */
void func_8009AF84(void) {
  BattleSelectionState *sel;
  u8 group;
  u8 scroll;
  u8 cursor;
  u8 eight;

  sel = &D_80148570;
  eight = 8;
  sel->first = 1;
  D_80148573 = 2;
  D_80148578 = 2;
  group = D_801454FD;
  scroll = D_801454FE;
  cursor = D_801454FF;
  D_80148572 = 1;
  D_8014857D = 0xFF;
  D_80148574 = -0xAA;
  D_80148571 = eight;
  D_80148579 = 0;
  D_80148580 = 0;
  D_80148576 = 0x3F;
  D_8014857A = group;
  D_8014857B = scroll;
  D_8014857C = cursor;
  sel->field_6C = 0;
  D_801485DD = eight;
  D_801485DE = 0;
}
