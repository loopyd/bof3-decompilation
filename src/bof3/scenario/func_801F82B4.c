#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F82B4
 * @behavior Runs one frame of the scratchpad work object's entry-effect fade
 * for dispatch byte 0x14 (entry 20, the word at 0x801FC9D0, of the per-frame
 * handler table at 0x801FC980 indexed by byte 0x01 of that object published at
 * 0x1F800044): when the object's byte at offset 0x09 is non-zero it runs
 * func_801F8360, advances the object's word at offset 0x0C by 4 while that word
 * is below 0x80, raises its word at offset 0x10 by 2 while that word is below
 * 0xB6 and otherwise pins the word at 0xB6, then decrements the byte at offset
 * 0x09 by one; when the byte at offset 0x09 is already zero the frame is just
 * the shared func_80196070 work-flag reset. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F82B4(void) {
  if (D_1F800044[0x09] != 0) {
    func_801F8360();

    if (*(s32 *)(D_1F800044 + 0x0C) < 0x80) {
      *(s32 *)(D_1F800044 + 0x0C) += 4;
    }

    if (*(s32 *)(D_1F800044 + 0x10) < 0xB6) {
      *(s32 *)(D_1F800044 + 0x10) += 2;
    } else {
      *(s32 *)(D_1F800044 + 0x10) = 0xB6;
    }

    D_1F800044[0x09] -= 1;
  } else {
    func_80196070();
  }
}
