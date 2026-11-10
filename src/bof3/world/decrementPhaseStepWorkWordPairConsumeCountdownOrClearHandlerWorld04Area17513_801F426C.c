#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */
extern s8 D_801F5378[]; /* @source 0x801F5378 @kind data */

/* @source 0x801F426C
 * @behavior Decrements byte 0x0A of the scratchpad work record at 0x1F800044.
 * While the result is nonzero, adds the signed table entry at 0x801F5378
 * selected by its low nibble, scaled by 0x800, to word 0x34 and subtracts
 * the independently selected entry from word 0x38. Consumes two units of
 * halfword 0x7E in the record published through 0x80146884. On expiry,
 * clears work handler byte 0x04 instead. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementPhaseStepWorkWordPairConsumeCountdownOrClearHandlerWorld04Area17513_801F426C(void) {
  u8* record;
  volatile u8* work;
  u8* shared;
  s32 first;
  s32 second;

  record = D_1F800044;
  if (--record[0x0a] != 0) {
    work = D_1F800044;
    first = work[0x0a] & 0x0f;
    second = work[0x0a] & 0x0f;
    *(s32*)(work + 0x34) += D_801F5378[first] << 11;
    shared = D_80146884;
    *(s32*)(work + 0x38) -= D_801F5378[second] << 11;
    *(u16*)(shared + 0x7e) -= 2;
  } else {
    D_1F800044[4] = 0;
  }
}
