#include "bof3/bof3.h"

/* Shared main-RAM flag bank of the main executable, outside this overlay's
 * payload, used only as the base argument of the shared flag tester. This span
 * probes one bit and keeps no other view of the bank. */
extern u8 D_80144F28[]; /* @source 0x80144F28 @kind data */

/* Front-end record pointer cell of the main executable, outside this overlay's
 * payload. This span advances its published halfword +0x0A, the same halfword
 * the already exact sibling 0x801F43B8 advanced by the same amount. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* Shared flag tester of the main executable, outside this overlay's payload:
 * returns nonzero when the bit its second argument selects is set in the bank
 * its first argument addresses. */
extern s32 func_8015B5D4(u32 arg0, s32 arg1); /* @source 0x8015B5D4 */

/* @source 0x801F40F4
 * @behavior Entry 9 of the 25-entry overlay handler table at 0x801F6830 of
 *           this payload, published by the word at 0x801F6854. Probes bit
 *           0x4B of the shared main-RAM flag bank 0x80144F28 through the
 *           shared flag tester func_8015B5D4 and, only while that bit reads
 *           clear, adds 3 to the published halfword +0x0A of the front-end
 *           record reached through the pointer cell 0x80148210. A set bit
 *           leaves the record untouched and returns at once. Takes no
 *           arguments and returns nothing; the 0x18-byte frame holds only the
 *           return address, and no jal in this payload reaches the span, so
 *           its roles stay structural and the frame is the callee-save the one
 *           call needs. The meaning of bit 0x4B and of the record halfword
 *           stays anchored to the offsets and constants the original bytes
 *           carry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateBy3SharedClear(void) {
  if (func_8015B5D4((u32)D_80144F28, 0x4B) == 0) {
    D_80148210[5] += 3;
  }
}
