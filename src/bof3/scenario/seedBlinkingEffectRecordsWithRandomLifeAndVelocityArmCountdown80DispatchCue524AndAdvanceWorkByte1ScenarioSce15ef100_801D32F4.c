#include "bof3/bof3.h"
#include <rand.h>

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Active effect-record count of this payload's data blob: it sits inside the
 * Splat `textbin` run at 0x801D3918, so the generated undefined_syms_auto.txt
 * row already names it and no map row is owed. The exact lift at 0x801D34F0
 * walks the same records up to this halfword and the lift at 0x801D30AC raises
 * it once per emitted record. */
extern u16 D_801D40A8; /* @source 0x801D40A8 @kind data */

/* Resident sound-cue dispatcher of the shared main executable at 0x8015DF18: it
 * consumes one cue id and returns nothing. The exact siblings of the same
 * dispatcher run declare the same word argument. */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801D32F4
 * @behavior Seeding step handler of this overlay: entry 8 of the eleven-word
 *           code-pointer run at 0x801D397C that the dispatcher 0x801D2DEC
 *           indexes with work byte +0x1. It first fills a sixteen-byte stack
 *           array with the alternating byte pattern 0, 1 and keeps it as the
 *           blink-flag source of the record walk. Through the two literal bases
 *           0x800EA200 and 0x800EA206 it then walks the ten-byte effect records
 *           of the shared array up to this payload's own count halfword
 *           0x801D40A8, advancing both bases by the 0xA stride. Every sixteenth
 *           pass, and therefore on the first one, it runs a sixteen-pass random
 *           redraw of that stack array - one `rand() & 0xF` draw supplies both
 *           ends of the swap, so the pass reads and rewrites a single byte - and
 *           then restarts the flag cursor at zero. Each record takes its blink
 *           flag byte 0 from the cursor and advances it, draws its velocity pair
 *           bytes 8 and 9 from two more `rand() & 0xF` draws and orders the pair
 *           so byte 8 holds the larger draw and byte 9 the negated smaller one,
 *           arms its life byte 1 with `(rand() & 0x1E) + 0xA`, and pre-adds the
 *           velocity scaled by that life to the position halfwords 4 and 6, the
 *           offsets the record draw at 0x801D34F0 subtracts from again. When the
 *           walk is done it re-reads the scratchpad cell for the countdown byte
 *           at +0x9, arms it with 0x50, re-reads the cell once more and advances
 *           work byte +0x1 by one behind the cue dispatch of 0x20C. The cell is
 *           loaded twice because the first store may alias it. Takes no
 *           arguments and returns nothing.
 * @status partial
 * @match 55.91
 * @residual Two blocks. The random redraw measures one load and one store where
 *           the original keeps two of each through a single pattern base and
 *           re-materialises the drawn index for the second address; every clean
 *           spelling measured so far either folds the self-swap to one store or
 *           splits the pattern base into a second register. The register
 *           permutation that follows from it: the candidate holds the record
 *           base in $s4 and the pattern array in $s5 while the original holds
 *           the pattern array in $s4, the record base in $s5 and the drawn index
 *           in $s0, so the prologue, the sixteen-byte fill loop and the frame
 *           size match and the first difference lands on the walk setup. The
 *           count reload, the double cell load and the cue delay slot match as
 *           measured.
 */
void seedBlinkingEffectRecordsWithRandomLifeAndVelocityArmCountdown80DispatchCue524AndAdvanceWorkByte1ScenarioSce15ef100_801D32F4(void) {
  u8 pattern[16];
  u8* record;
  u8* motion;
  u16 i;
  u16 j;
  u16 k;
  u8 swap;

  for (i = 0; i < 16; i++) {
    pattern[i] = i & 1;
  }
  record = PSX_PTR(u8, 0x800EA200);
  motion = PSX_PTR(u8, 0x800EA206);
  j = 0;
  for (i = 0; i < D_801D40A8; record += 0xA, motion += 0xA, i++) {
    if ((i & 0xF) == 0) {
      for (j = 0; j < 16; j++) {
        k = rand() & 0xF;
        swap = pattern[k];
        pattern[k] = pattern[k];
        pattern[k] = swap;
      }
      j = 0;
    }
    record[0] = pattern[j++];
    motion[2] = rand() & 0xF;
    motion[3] = rand() & 0xF;
    if ((s8)motion[2] < (s8)motion[3]) {
      swap = motion[2];
      motion[2] = motion[3];
      motion[3] = swap;
    }
    motion[3] = -motion[3];
    record[1] = (rand() & 0x1E) + 0xA;
    *(u16*)(record + 4) += (s8)motion[2] * record[1];
    *(u16*)(record + 6) += (s8)motion[3] * record[1];
  }
  D_1F800044[9] = 0x50;
  D_1F800044[1] = D_1F800044[1] + 1;
  dispatchSoundCue(0x20C);
}
