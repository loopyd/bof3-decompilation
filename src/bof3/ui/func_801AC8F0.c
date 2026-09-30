#include "bof3/ui/game00_internal.h"

/*
 * @behavior Resolves a work-script jump operand: when the state cursor at
 * +0x0A has bit 0x4000 clear that 16-bit absolute target is returned unchanged,
 * otherwise the script is scanned from offset 0 for the 0x0A label definition
 * whose id byte equals label and the offset of that definition is returned. The
 * scan offset advances by the per-opcode width table at 0x801C87FC, with the
 * 0xF8 and 0x0E/0x0F widths special-cased.
 * @source 0x801AC8F0
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801AC8F0(GameWorkScriptState* state, u8 label, const u8* script) {
  u16       cursor = state->cursor_0A;
  u16       index = 0;
  const u8* entry;

  if (!(cursor & 0x4000)) {
    return cursor;
  }
  while (script[index] != 0x0A || script[index + 1] != label) {
    entry = script + index;
    if (entry[0] == 0xF8) {
      if (entry[1] == 7) {
        index += 5;
      } else {
        index += 2;
      }
    } else if ((u8)(entry[0] - 14) < 2) {
      if (entry[3] & 2) {
        index += 5;
      } else {
        index += 4;
      }
    } else {
      index += D_801C87FC[entry[0]];
    }
  }
  return index;
}
