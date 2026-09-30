#include "bof3/ui/game00_internal.h"

/* @source 0x801C0E48
 * @behavior advances the pending front-end local-work request. With the cursor
 * D_80146262 clear the request advances unconditionally; otherwise byte 1 of the
 * previous 0x140-byte-stride work record (D_80145E91) must read 1, and the
 * per-request bit of the world flags D_8014625A is cleared. Once the cursor
 * equals the record count D_80146254 the request is finished instead: the
 * request bits of D_8014625A and the active flag D_80146260 are cleared and 0 is
 * returned. An advancing request publishes the address of D_80145E90[cursor] to
 * the scratchpad work pointer and D_80146250, stores cursor + 1 back into
 * D_80146262 and clears the request bits of D_8014625A again when the new cursor
 * reaches the record count.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801C0E48(void) {
  u8  advance;
  u8  gate;
  u8  limit;
  u8  index;
  u8  next;
  s32 offset;
  u8* cursor;
  u16 flags;
  u16 mask_flags;
  u8* record;

  advance = 0;
  if (D_80146262 == 0) {
    advance = 1;
    goto publish;
  }
  offset = D_80146262 - 1;
  gate = D_80145E91[offset * 0x140];
  if (gate != 1) {
    goto publish;
  }
  if (D_80146262 == gate) {
    flags = D_8014625A;
    limit = D_80146254;
    D_8014625A = flags & 0xFFF7;
    advance = 1;
    if (limit == D_80146262) {
      D_80146260 = 0;
      D_8014625A = flags & 0xFDF7;
      return 0;
    }
  } else {
    advance = 1;
  }
  D_8014625A &= ~(1 << D_80146262);
publish:
  if (advance != 0) {
    cursor = &D_80146262;
    index = *cursor;
    next = index + 1;
    record = &D_80145E90[index * 0x140];
    g_game_work = (struct GameWorkArea*)record;
    D_80146250 = record;
    *cursor = next;
    if (D_80146254 != 1 && next == D_80146254) {
      mask_flags = D_8014625A;
      D_8014625A = mask_flags & 0xFFE7;
      D_80146260 = 0;
      D_8014625A = mask_flags & 0xFDE7;
    }
  }
  return advance;
}
