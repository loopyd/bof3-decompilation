#include "bof3/bof3.h"

/* @source 0x801F9E00
 * @behavior Reads the work pointer from scratchpad slot 0x44, seeds its words
 * at 0x34/0x38/0x3C with 0x198000/0x198000/0x4000000, copies those three
 * words to 0x0C/0x10/0x14 with 0x40000 added to the first, and increments
 * its unsigned state byte at offset 1 modulo 256. Makes no calls.
 * @status partial
 * @match 40.00
 * @residual State-byte and word-copy scheduling/allocation differ; 8/20 instructions match.
 */
void seedWorkWords34AndCopyTo0CThenAdvanceStateScenarioScena0800_801F9E00(void) {
  u8* work;
  u32 first;
  u32 second;
  u32 third;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  *(u32*)(work + 0x34) = 0x198000;
  *(u32*)(work + 0x38) = 0x198000;
  *(u32*)(work + 0x3C) = 0x4000000;
  work[1]++;
  first = *(u32*)(work + 0x34);
  second = *(u32*)(work + 0x38);
  third = *(u32*)(work + 0x3C);
  *(u32*)(work + 0x0C) = first + 0x40000;
  *(u32*)(work + 0x10) = second;
  *(u32*)(work + 0x14) = third;
}
