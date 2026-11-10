#include "bof3/bof3.h"

extern void func_8015D71C(s32 selector); /* @source 0x8015D71C @kind function */
extern void func_801E257C(u8 index, u8 scriptByte); /* @source 0x801E257C @kind function */
extern void func_800A9610(u8 index, s32 flags); /* @source 0x800A9610 @kind function */
extern u8 D_80146374; /* @source 0x80146374 @kind data */
extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EF280
 * @behavior Handler referenced by the overlay pointer at 0x801EEC1C. Adds work
 *           word 0x20 to word 0x14 and accumulates the new word into halfword
 *           0x3E, while decrementing work byte 9 modulo 256. When that byte
 *           reaches zero, invokes 0x8015D71C with 0x203, 0x801E257C with the
 *           byte at 0x80146374 and 0xC, then 0x800A9610 with the byte at
 *           0x80146384 and 0x10. Re-reads the scratchpad work pointer to set
 *           byte 9 to 3 and again to advance byte 2. No arguments or return
 *           value; unestablished helper and field roles remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void integrateWorkWord14IntoHalfword3EAndDecrementByte9ThenInvokeHelpersAndAdvanceAtZeroBmagicMagic00100_801EF280(void) {
  u8 *work;
  u8 countdown;
  u32 value;
  u16 accumulator;

  work = D_1F800044;
  countdown = work[9] - 1;
  value = *(u32 *)(work + 0x14) + *(u32 *)(work + 0x20);
  accumulator = *(u16 *)(work + 0x3E) + value;
  work[9] = countdown;
  *(u32 *)(work + 0x14) = value;
  *(u16 *)(work + 0x3E) = accumulator;
  if (countdown == 0) {
    func_8015D71C(0x203);
    func_801E257C(D_80146374, 0xC);
    func_800A9610(D_80146384, 0x10);
    D_1F800044[9] = 3;
    D_1F800044[2]++;
  }
}
