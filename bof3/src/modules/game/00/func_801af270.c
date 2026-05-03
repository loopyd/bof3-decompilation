#include "internal.h"

/* does: returns a pointer into one of two sprite-rect tables, indexed by
 * `sprite_id * 4`, with the table chosen by the low bit of `flags`.
 * @source: 0x801af270 FUN_801af270
 */
u8* func_801af270(u8 sprite_id, u8 flags) {
  u8* table;

  flags &= 0xff;
  if (flags) {
    table = (u8*)0x801cdcccu;
  } else {
    table = (u8*)0x801ccbe4u;
  }
  return table + sprite_id * 4;
}
