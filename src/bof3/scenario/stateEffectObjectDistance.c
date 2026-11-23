#include "bof3/bof3.h"

/* Fixed-address state cell named by this target's own map: the scratchpad
 * work-pointer cell at 0x1F800044. Each source-level read keeps its own cell
 * load, because the handler-advance arm's pointer read follows stores through
 * the pointer an earlier read returned. */
extern u8* scenarioScena0700Work; /* @source 0x1F800044 @kind unknown */

/* Overlay-local pointer cell holding the shared main-RAM effect object at
 * 0x800E5148, which this handler publishes into it. The cell has no target map
 * row; the generated undefined_syms_auto.txt row binds it. */
/* @source 0x801FDFBC @kind data */
extern u8* D_801FDFBC;

/* Distance helper of the shared main executable: it takes the signed 16.16 word
 * pair as its two coordinates and answers their distance, which this overlay
 * sees as a word return.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F7638
 * @behavior Overlay handler and word 0 of the four-word in-image code-pointer
 * run labelled D_801FDD8C at 0x801FDD8C, which
 * dispatchWorkByte2HandlerScenarioScena0700_801F75F4 indexes with the scratchpad
 * work object's unsigned byte at offset 0x02, so it runs while that byte reads
 * zero; no in-image `jal` reaches the address and that run word is the only
 * image word naming it. It loads the work-object pointer cell at 0x1F800044
 * before it creates its frame and passes that object's signed words at 0x34 and
 * 0x38 to the shared main-executable distance helper func_8015477C, which reads
 * them as a 16.16 coordinate pair. Through a second read of the pointer cell it
 * stores the helper's answer shifted left by 16 into the object's word at 0x3C,
 * so the object carries the pair's distance in 16.16 form, and it copies that
 * position pair and distance into the shared effect object at 0x800E5148: word
 * 0x800E5148 takes the object's 0x34 word, 0x800E514C its 0x38 word and
 * 0x800E5150 its 0x3C distance, re-read from the object, while the halfword at
 * 0x800E5158 - the ramp counter the quad helper func_801F97E0 walks - is
 * zeroed. The address 0x800E5148 is materialised once and both stored into the
 * effect-object cell at 0x801FDFBC and used as the base of the first copy, so
 * that cell becomes the pointer the sibling run member at 0x801F77AC ticks
 * afterwards. It then clears the object's byte at offset 0x09 and, through a
 * third read of the pointer cell, arms the object's countdown word at 0x0C with
 * 0x5F and increments the object's byte at 0x02, the handler index that advances
 * this run. Takes no arguments and returns nothing; its 0x28-byte frame keeps
 * only $ra at 0x20 and its single `jr $ra` epilogue has the usual nop delay
 * slot.
 *
 * Representation notes. Seeded from this target's already-exact distance-pair
 * handlers statePairDistanceArm
 * (0x801F7D6C) and stateDistanceTransitionArm
 * (0x801F7A30), which supplied the statement order, operand forms, offset
 * spelling and declaration style, and from the dispatcher
 * dispatchWorkByte2HandlerScenarioScena0700_801F75F4 for the run membership. The
 * clone lever was declined: a scan of the 398 payloads under out/binaries for
 * this 0x98-byte span matched the span itself only, so no byte-identical twin
 * exists anywhere. Probes before this gate, every one of them 152 bytes unless
 * noted: the SPAD_PTR_SLOT(u8,0x44u) spelling measured 24/38 (164 bytes) because
 * it kept 0x1F800044 in $s0 and emitted a `lui`/`ori` base; the map's named cell
 * scenarioScena0700Work with three separate reads measured 30/38; adding the
 * 0x10-byte frame reservation measured 34/38, where 0x20 bytes of reservation
 * gave 30/38; the copy order 0x800E5148 / 0x801FDFBC / 0x800E514C / 0x800E5158 /
 * 0x800E5150 measured 34/38 and 36/38 in its permutations, and the order
 * 0x800E5148 / 0x800E514C / 0x801FDFBC / 0x800E5150 / 0x800E5158 measured
 * 38/38.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateEffectObjectDistance(
    void) {
  u8* work;
  u8* saved;
  u8* advance;
  s32 distance;
  /* Frame reservation only: the original owns 0x10 bytes of stack that no
   * instruction touches, so no use is made of this region. */
  u32 pad[4];

  work = scenarioScena0700Work;
  distance = func_8015477C(*(s32*)(work + 0x34), *(s32*)(work + 0x38));
  saved = scenarioScena0700Work;
  *(s32*)(saved + 0x3C) = distance << 16;
  PSX_REF(s32, 0x800E5148u) = *(s32*)(saved + 0x34);
  PSX_REF(s32, 0x800E514Cu) = *(s32*)(saved + 0x38);
  D_801FDFBC = PSX_PTR(u8, 0x800E5148u);
  PSX_REF(s32, 0x800E5150u) = *(s32*)(saved + 0x3C);
  PSX_REF(u16, 0x800E5158u) = 0;
  saved[9] = 0;
  advance = scenarioScena0700Work;
  *(s32*)(advance + 0x0C) = 0x5F;
  advance[2]++;
}
