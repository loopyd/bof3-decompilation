#include "bof3/bof3.h"
#include <rand.h>

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Active effect-record count of this payload's data blob: it sits inside the
 * Splat `textbin` run at 0x801D3918, so the generated undefined_syms_auto.txt
 * row already names it and no map row is owed. The draw lift at 0x801D2934 and
 * 0x801D34F0 walk the same records up to this halfword. */
extern u16 D_801D40A8; /* @source 0x801D40A8 @kind data */

/* Resident sound-cue dispatcher of the shared main executable at 0x8015DF18: it
 * consumes one cue id and returns nothing. The exact siblings of the same
 * dispatcher run declare the same word argument. */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801D26BC
 * @behavior Seeding step handler of this overlay: entry 3 of both the ten-word
 *           code-pointer run at 0x801D3954 and the eleven-word run at 0x801D397C
 *           that the two dispatchers index with work byte +0x1. It first fills a
 *           sixteen-byte stack array with the alternating byte pattern 0, 1 and
 *           keeps it as the blink-flag source of the record walk. Through the
 *           literal base 0x800EA200 it then walks the ten-byte effect records
 *           of the shared array up to this payload's own count halfword
 *           0x801D40A8, advancing the base by the 0xA stride.
 *           Every sixteenth pass, and therefore on the first one, it runs a
 *           sixteen-pass random redraw of that stack array - two `rand() & 0xF`
 *           draws give the two ends of the swap - and then restarts the flag
 *           cursor at zero. Each record takes its blink flag byte 0 from the
 *           cursor and advances it, then draws its velocity pair bytes 8 and 9
 *           by the phase that work byte 0xB selects: while that byte reads zero
 *           both come from `(rand() & 0x1F) - 0x10`, and while it reads non-zero
 *           from two `rand() & 0xF` draws ordered so byte 8 holds the larger one
 *           and byte 9 the negated smaller one. Work byte 0x6 then selects the
 *           life: while it reads zero the life byte 1 takes `(rand() & 0x1E) +
 *           0xA` and the velocity scaled by that life is pre-added to the
 *           position halfwords 4 and 6, which the record draw at 0x801D2934
 *           subtracts again; while it reads non-zero the life byte takes
 *           `0x28 - (rand() & 0x1E)` and the position is left alone. When the
 *           walk is done it re-reads the scratchpad cell for the countdown byte
 *           at +0x9 and arms it with 0x50, re-reads the cell once more and
 *           advances work byte +0x1 by one behind the cue dispatch of 0x20C. The
 *           cell is loaded twice because the first store may alias it. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedBlinkingEffectRecordsRandomizeVelocityByWorkByteBAndLifeByWorkByte6ArmCountdown80DispatchCue524AndAdvanceWorkByte1ScenarioSce15ef100_801D26BC(void) {
  u8 pattern[16];
  u8* record;
  u16 i;
  u16 j;
  u8 swap;
  u8 k;
  u8 l;

  for (i = 0; i < 16; i++) {
    pattern[i] = i & 1;
  }
  record = PSX_PTR(u8, 0x800EA200);
  j = 0;
  for (i = 0; i < D_801D40A8; record += 0xA, i++) {
    if ((i & 0xF) == 0) {
      for (j = 0; j < 16; j++) {
        k = rand() & 0xF;
        l = rand() & 0xF;
        swap = pattern[l];
        pattern[l] = pattern[k];
        pattern[k] = swap;
      }
      j = 0;
    }
    record[0] = pattern[j++];
    if (D_1F800044[0xB] == 0) {
      record[8] = (rand() & 0x1F) - 0x10;
      record[9] = (rand() & 0x1F) - 0x10;
    } else {
      record[8] = rand() & 0xF;
      record[9] = rand() & 0xF;
      if ((s8)record[8] < (s8)record[9]) {
        record[8] ^= record[9];
        record[9] ^= record[8];
        record[8] ^= record[9];
      }
      record[9] = -record[9];
    }
    if (D_1F800044[6] == 0) {
      record[1] = (rand() & 0x1E) + 0xA;
      *(u16*)(record + 4) -= (s8)record[8] * record[1];
      *(u16*)(record + 6) -= (s8)record[9] * record[1];
    } else {
      record[1] = 0x28 - (rand() & 0x1E);
    }
  }
  D_1F800044[9] = 0x50;
  D_1F800044[1] = D_1F800044[1] + 1;
  dispatchSoundCue(0x20C);
}
