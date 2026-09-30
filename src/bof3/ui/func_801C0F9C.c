#include "bof3/ui/game00_internal.h"

/* @source 0x801C0F9C
 * @behavior commits the pending front-end local-work request held at cursor
 * D_80146262. The request is rejected (0) unless the world flags D_80146258
 * carry no bit for the cursor and either the previous cursor bit is set or the
 * previous 0x140-byte work record's gate byte (D_80145E91) reads 1; a cursor of
 * 2 additionally requires record 0's gate byte to read 1. An accepted request
 * clears the cursor bit of the world flags D_8014625A, and when the cursor's own
 * D_80146258 bit is clear it fills the cursor's work record (bytes 0..4 from the
 * four-byte mode record D_801CD4AC + mode * 4, byte 0 masked with 0xBF, and byte
 * 0x12C masked with 0xFD), then advances D_80146262. 1 is returned for an
 * ordinary advance; reaching the record count D_80146254 clears the active flag
 * D_80146260 and request bit 4 of D_8014625A and returns 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801C0F9C(s32 arg0) {
  s32 mode_index;
  s32 offset;
  s32 prev;
  u8  next;
  u16 flags;
  u8* cursor;

  if (D_80146262 != 0) {
    if (((D_80146258 >> D_80146262) & 1) == 0) {
      prev = D_80146262 - 1;
      if (((D_80146258 >> prev) & 1) != 0) {
        goto check_two;
      }
      if (D_80145E91[prev * 0x140] != 1) {
        return 0;
      }
    }
  }
  if ((D_80146258 >> (D_80146262 - 1)) & 1) {
check_two:
    if (D_80146262 == 2) {
      if (D_80145E91[0] != 1) {
        return 0;
      }
    }
  }
  if (D_80146262 != 0) {
    if (((D_80146258 >> D_80146262) & 1) == 0) {
      prev = D_80146262 - 1;
      if (((D_80146258 >> prev) & 1) == 0) {
        if (D_80145E91[prev * 0x140] != 1) {
          return 0;
        }
      }
    }
    D_8014625A &= ~(1 << D_80146262);
  }
  cursor = &D_80146262;
  if (((D_80146258 >> *cursor) & 1) == 0) {
    mode_index = (u8)arg0 * 4;
    offset = *cursor * 0x140;
    D_80145E90[offset] &= 0xBF;
    D_80145E91[*cursor * 0x140] = D_801CD4AC[mode_index];
    D_80145E92[*cursor * 0x140] = D_801CD4AC[mode_index + 1];
    D_80145E93[*cursor * 0x140] = D_801CD4AC[mode_index + 2];
    D_80145E94[*cursor * 0x140] = D_801CD4AC[mode_index + 3];
    offset = *cursor * 0x140;
    D_80145FBC[offset] &= 0xFD;
  }
  next = *cursor + 1;
  *cursor = next;
  if (next != D_80146254) {
    return 1;
  }
  flags = D_8014625A;
  D_80146260 = 0;
  D_8014625A = flags & 0xFFEF;
  return 2;
}
