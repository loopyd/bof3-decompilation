#include "bof3/bof3.h"
#include <rand.h>

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);
void projectWorkCursorStoreScreenPairAndDrawSpritePairAndRingScenarioScena0800_801F8534(void);

/* @source 0x801F7268
 * @behavior Overlay work-state handler, entry 6 of the 51-entry per-frame
 * handler run at 0x801FE894 and entry 3 of the twelve-entry sub-table at
 * 0x801FE8A0 that the work-byte-1 dispatcher at 0x801F707C selects with
 * unsigned byte 1 of the scratchpad work object: it projects and draws through
 * the overlay helper at 0x801F8534, adds the low byte of rand() to work
 * halfword 0x58 modulo 65536 and, when the low two bits of work halfword 0x5A
 * are clear, decrements work halfword 0x2C modulo 65536. It then reloads the
 * scratchpad work pointer at 0x1F800044 and queues the front-end sound cue
 * 0x20C when the 0x5A countdown reads exactly 2, reloads the work pointer once
 * more, decrements that countdown modulo 65536 and, when it reaches zero,
 * re-arms it with 0x40 and increments work byte 1 modulo 256. Takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra across its
 * three calls. The helper's wider role and the work fields' meanings remain
 * unknown; shape and helper identity follow the byte-adjacent sibling
 * 0x801F74E0, which drives the same 0x58/0x5A/0x2C fields.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addRandomByteToWorkHalfword58Decrement2CQueueCue20CAndTickCountdown5AAdvanceStateScenarioScena0800_801F7268(void) {
  s32 random;
  u8* work;
  u8* cue;
  u8* next;
  u16 countdown;

  projectWorkCursorStoreScreenPairAndDrawSpritePairAndRingScenarioScena0800_801F8534();
  random = rand();
  work = scenarioScena0800Work;
  *(u16*)(work + 0x58) += random & 0xFF;
  if ((*(u16*)(work + 0x5A) & 3) == 0) {
    *(u16*)(work + 0x2C) -= 1;
  }
  cue = scenarioScena0800Work;
  if (*(u16*)(cue + 0x5A) == 2) {
    dispatchSoundCue(0x20C);
  }
  next = scenarioScena0800Work;
  countdown = *(u16*)(next + 0x5A) - 1;
  *(u16*)(next + 0x5A) = countdown;
  if (countdown == 0) {
    *(u16*)(next + 0x5A) = 0x40;
    next[1]++;
  }
}
