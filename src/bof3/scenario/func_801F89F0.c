#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F89F0
 * @behavior Seeds the shared scratchpad work object (published at 0x1F800044)
 * from the 152-byte work-area record at 0x80147A58: copies the record's cursor
 * words at 0x34/0x38/0x3C and its origin words at 0x64/0x68/0x6C into the work
 * object at the same offsets, resolves the work object's fixed-point origin
 * through func_801AC1DC(origin, work byte 0x1C), adds the resolved x and y
 * words to the work object's cursor words at 0x34/0x38, adds the resolved z
 * word to the depth halfword at 0x3E and advances the dispatch byte at 0x01 by
 * one. Takes no arguments and returns nothing; the address is entry 24 of the
 * per-frame handler table at 0x801FC980 indexed by byte 0x01 of that work
 * object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F89F0(void) {
  u8* work;
  s32 origin[3];

  work = D_1F800044;
  *(s32 *)(work + 0x34) = *(s32 *)(D_80147A58 + 0x34);
  *(s32 *)(work + 0x38) = *(s32 *)(D_80147A58 + 0x38);
  *(s32 *)(work + 0x3C) = *(s32 *)(D_80147A58 + 0x3C);
  *(s32 *)(work + 0x64) = *(s32 *)(D_80147A58 + 0x64);
  *(s32 *)(work + 0x68) = *(s32 *)(D_80147A58 + 0x68);
  *(s32 *)(work + 0x6C) = *(s32 *)(D_80147A58 + 0x6C);
  func_801AC1DC(origin, work[0x1C]);
  *(s32 *)(D_1F800044 + 0x34) += origin[0];
  *(s32 *)(D_1F800044 + 0x38) += origin[1];
  *(u16 *)(D_1F800044 + 0x3E) += (u16)origin[2];
  *(u8 *)(D_1F800044 + 0x01) += 1;
}
