#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_8014832E;  /* @source 0x8014832E @kind unknown */

void func_801F4060(s16 arg0);

/* @source 0x801F342C
 * @behavior Overlay step handler for state byte 1: advances word 0x0C of the
 * work record published through the scratchpad pointer cell 0x1F800044 by 8
 * and passes that new value to the tile routine at 0x801F4060 as a signed
 * halfword colour. It then decrements the record's countdown byte at 0x09 and,
 * while the stored byte stays non-zero, returns without further work. On the
 * frame that byte reaches zero it reloads the cell to write 0x140 into the
 * halfword at 0x32, 0x10 into the countdown byte at 0x09 and 0x100 into the
 * step words at 0x0C and 0x10, then loads the cell once more to advance state
 * byte 0x01 for the state-1 dispatcher at 0x801F33AC and arms the shared
 * effect bank byte at 0x8014832E with 0x1F. Takes no arguments and returns
 * nothing; its 0x18-byte frame only keeps $ra at 0x10 across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceStepDrawTileUntilCountdownThenSeedStepsAndArmEffectWorld04Area17213_801F342C(
    void) {
  u32* words;
  u8* count;
  u8* seed;
  u8* next;

  words = (u32*)D_1F800044;
  words[3] += 8;
  func_801F4060((s16)words[3]);
  count = D_1F800044;
  if (--count[9] == 0) {
    seed = D_1F800044;
    *(u16*)(seed + 0x32) = 0x140;
    seed[9] = 0x10;
    next = D_1F800044;
    *(u32*)(seed + 0xc) = 0x100;
    *(u32*)(seed + 0x10) = 0x100;
    next[1]++;
    D_8014832E = 0x1f;
  }
}
