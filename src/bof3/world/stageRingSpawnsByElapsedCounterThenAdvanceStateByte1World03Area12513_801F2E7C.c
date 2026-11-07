#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original reloads it after the frame
 * ladder, once more inside the spawn branch and once before the counter
 * advance, so the source keeps the named symbol form and evaluates it again at
 * every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

void func_801F3268(s32 count);
void func_801F33B0(void);

/* The overlay spawn schedule indexed by the stage byte this entry keeps at
 * 0x801F3834: the first five bytes are how many ring records the stage spawns
 * and the five bytes 8 higher are how many frames the stage waits after that
 * batch before it spawns the next one. Both sit inside the single data blob
 * claimed by the textbin row at D_801F364C, so they stay bound by address. */
extern u8 D_801F36F0[];
extern u8 D_801F36F8[];
extern u8 D_801F3834;

/* @source 0x801F2E7C
 * @behavior Spawn-schedule step of this overlay's handler run: entry 7 of the
 *           nine-word run at 0x801F36CC that
 *           dispatchStateByte1Table36ccWorld03Area12513_801F2C04 selects with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044 and calls with no
 *           arguments. It takes that work object once and reads the signed
 *           halfword frame counter at +0x2E. Below 0x1A4 frames it stores the
 *           spawn stage into the overlay byte at 0x801F3834, zero below 0x3C
 *           frames and otherwise 1, 2, 3 or 4 at the thresholds 0xB4, 0x12C and
 *           0x168, then takes the work object again and reads the signed
 *           countdown halfword at +0x30: when that countdown is zero it calls
 *           the ring spawner at 0x801F3268 with the stage count from the byte
 *           table at 0x801F36F0, takes the work object a third time and arms
 *           +0x30 with the stage wait from the byte table at 0x801F36F8, while
 *           a live countdown just drops by one through the object it already
 *           holds. It then takes the work object a fourth time and advances the
 *           frame counter at +0x2E by one. From 0x1A4 frames up it advances the
 *           work byte at +0x1 instead, which moves the next frame dispatch to
 *           entry 8 at 0x801F2F90. Both paths then call the overlay helper at
 *           0x801F33B0 and return through a 0x18-byte frame that holds only the
 *           return address, so no register survives the spawner call.
 *
 * Name evidence: entry 6 at 0x801F2E58 clears the halfwords at +0x2E and +0x30
 *           and advances the work byte at +0x1 into this entry, so +0x2E and
 *           +0x30 arrive cleared. The tables make the roles concrete: the count
 *           table holds 2, 4, 4, 2 and 1, the wait table holds 30, 30, 15, 30
 *           and 15 frames, and 0x801F3268 fills a stack array with the bytes 0
 *           to 15, shuffles it with rand and calls
 *           claimFreeWorkRecordSeedRingPositionAndDirectionWorld03Area12513_801F31B0
 *           once per entry of the shuffled list up to its argument, so the
 *           argument is a count of ring records. The frame counter, the stage
 *           byte and the wait halfword are read by nothing else in this payload,
 *           so their roles come from this body alone and stay spelled by offset.
 *
 * Shape evidence: the pointer-cell form follows the exact lift
 *           refreshLayersThenSelectWorkSequenceFromByte8HalfArmCountdownAndAdvanceWorkByte3PlcharPlp01600_801D0060,
 *           whose original also reloads 0x1F800044 between accesses and which
 *           records the named symbol form as the spelling that produces a
 *           separate load per access. No byte-identical or near-identical
 *           sibling body exists in any other payload, so the statement order,
 *           the operand forms and the offset-local spelling come from the
 *           original bytes: the four work-object reads, the byte loads of the
 *           stage and tables, the signed halfword reads, and the constant
 *           ladder as a conditional expression whose branches fill one shared
 *           register.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stageRingSpawnsByElapsedCounterThenAdvanceStateByte1World03Area12513_801F2E7C(void) {
  u8* work;
  s16 elapsed;

  work = D_1F800044;
  elapsed = *(s16*)(work + 0x2E);
  if (elapsed < 0x1A4) {
    if (elapsed < 0x3C) {
      D_801F3834 = 0;
    } else {
      D_801F3834 =
          elapsed < 0xB4 ? 1 : elapsed < 0x12C ? 2 : elapsed < 0x168 ? 3 : 4;
    }
    work = D_1F800044;
    if (*(s16*)(work + 0x30) == 0) {
      func_801F3268(D_801F36F0[D_801F3834]);
      *(u16*)(D_1F800044 + 0x30) = D_801F36F8[D_801F3834];
    } else {
      *(s16*)(work + 0x30) -= 1;
    }
    *(u16*)(D_1F800044 + 0x2E) += 1;
  } else {
    work[1] += 1;
  }
  func_801F33B0();
}
