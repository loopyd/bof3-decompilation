#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* @source 0x801F3F68
 * @behavior Overlay per-frame work-record step: the record published at the
 * scratchpad pointer cell 0x1F800044 has its countdown byte at 0x0A decremented
 * and its step word at 0x0C added into the accumulator word at 0x34; when the
 * countdown byte reaches zero the record is re-armed with 8 in that byte and
 * handler index 3 in the state byte at 0x04, which is the byte the overlay's
 * state-byte-4 dispatcher tests. Both paths then consume two units of the
 * halfword at 0x122 of the record published through the global pointer cell
 * 0x80146250. The record is reached through one word-typed view with its
 * countdown byte taken through a byte view derived from that pointer, and the
 * pointer cell is re-read for the countdown test and again for the state byte
 * because the intervening stores through the old record pointer may alias it.
 * Takes no arguments and returns nothing; leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkAccumulatorAndRearmCountdownThenConsumeSharedCountdownWorld01Area04113_801F3F68(void) {
  s32* words;
  u8* shared;

  words = (s32*)D_1F800044;
  ((u8*)words)[0x0a] = ((u8*)words)[0x0a] - 1;
  words[13] += words[3];
  if (D_1F800044[0x0a] == 0) {
    D_1F800044[0x0a] = 8;
    D_1F800044[4] = 3;
  }
  shared = D_80146250;
  *(u16*)(shared + 0x122) -= 2;
}
