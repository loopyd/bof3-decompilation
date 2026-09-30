#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7A5C
 * @behavior Seeds the scratchpad work object published at 0x1F800044 for the
 * trail phase: stores the fixed cursor 0x638000 (x) and 0xD0000 (y) in the
 * words at offsets 0x34 and 0x38, stores 0x300 in the halfword at offset 0x3E,
 * arms the countdown byte at offset 0x09 with 0x78, zeroes the words at
 * offsets 0x64, 0x68 and 0x6C, sets the three signed tint channels at offsets
 * 0x5F, 0x5E and 0x5D to -1, stores 1 in the word at offset 0x0C, zeroes the
 * words at offsets 0x10, 0x20, 0x1C and 0x18 and the bytes at offsets 0x0A,
 * 0x0B, 0x08, 0x07 and 0x06, queues frontend cue 0x20E and advances the
 * object's dispatch byte at offset 0x01 to 1. Takes no arguments and returns
 * nothing; the address is entry 11 of the per-frame handler table at
 * 0x801FC980 indexed by byte 0x01 of that work object, and the first entry of
 * the second handler table at 0x801FC9AC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F7A5C(void) {
  u8 *work;
  u8 *tint;
  u8 *fields;
  u8 *tail;

  work = D_1F800044;
  *(s32 *)(work + 0x34) = 0x638000;
  *(s32 *)(work + 0x38) = 0xD0000;
  *(u16 *)(work + 0x3E) = 0x300;
  *(s8 *)(work + 0x09) = 0x78;
  tint = D_1F800044;
  *(s32 *)(work + 0x64) = 0;
  *(s32 *)(work + 0x68) = 0;
  *(s32 *)(work + 0x6C) = 0;
  *(s8 *)(tint + 0x5F) = -1;
  *(s8 *)(tint + 0x5E) = -1;
  *(s8 *)(tint + 0x5D) = -1;
  fields = D_1F800044;
  *(s32 *)(fields + 0x0C) = 1;
  *(s32 *)(fields + 0x10) = 0;
  *(s32 *)(fields + 0x20) = 0;
  *(s32 *)(fields + 0x1C) = 0;
  *(s32 *)(fields + 0x18) = 0;
  *(u8 *)(fields + 0x0A) = 0;
  *(u8 *)(fields + 0x0B) = 0;
  *(u8 *)(fields + 0x08) = 0;
  *(u8 *)(fields + 0x07) = 0;
  *(u8 *)(fields + 0x06) = 0;
  game_queue_frontend_cue(0x20E);
  tail = D_1F800044;
  *(s8 *)(tail + 0x01) = 1;
}
