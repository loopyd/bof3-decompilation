#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so the load folds to lui plus an offset load. The original loads
 * the cell once and reaches every seeded field through that one pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Battle selection byte in main RAM, outside this overlay's image. The battle
 * targets own the address; this lift only reads it as the enemy action record
 * index, the same selector the exact battle lift func_800A36F0 and the sibling
 * boss handler at 0x800C24DC read through D_80146374 - 3. Declared volatile
 * because the original re-reads the byte and recomputes the whole scaled index
 * for each of the three record fields instead of keeping the fetched byte live;
 * a plain load let the index computation fold to one copy. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is added here. */
extern volatile u8 D_80146374; /* @source 0x80146374 @kind unknown */


/* One 0x118-byte enemy action record. The shared battle header
 * include/bof3/battle/battle15_internal.h declares the same record array as
 * BattleEnemyActionRecord with these three words at 0, 4 and 8; this target
 * keeps its own declaration because it does not include the battle header. The
 * three words are signed here: the original divides their difference from the
 * work object by 16 with the bgez/addiu 0xF/sra sequence, which only a signed
 * division emits. The stride is carried by the record type rather than by an
 * explicit scale in the subscript because the original keeps the subtraction
 * as its own instruction (addiu of -3 before the scale). */
typedef struct Boss033EnemyActionRecord {
  s32 unk_00;
  s32 unk_04;
  s32 unk_08;
  u8 pad_0C[0x10C];
} Boss033EnemyActionRecord;

/* Shared enemy action record array in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. */
extern Boss033EnemyActionRecord D_801EB664[]; /* @source 0x801EB664 @kind unknown */

/* Readiness poll helper of the main executable. The battle and bmagic lifts
 * declare the same prototype; the original masks the returned byte with 0xFF
 * before testing it, so the declared return type is the byte it reports. */
extern u8 func_8014DAEC(void);

/* @source 0x800C2310
 * @behavior Work-step handler at index 4 of the overlay code-pointer run
 *           based at 0x800C27B4, the base word the step dispatcher
 *           func_800C1E70 indexes with work byte 2 scaled by four; run word 4 at
 *           0x800C27C4 holds this address and no jal in this payload names it.
 *           It first calls the readiness helper func_8014DAEC and keeps only
 *           the returned byte of that helper as a ready test, writing nothing
 *           while the helper reports not ready. Once it reports ready it loads the
 *           scratch work object published at the pointer cell 0x1F800044 once
 *           and seeds that object from the enemy action record selected by the
 *           battle selection byte 0x80146374 minus three: word 0x0C takes
 *           one sixteenth of the record word 0 minus the object word 0x34, word
 *           0x10 the same quotient of record word 4 minus object word 0x38, and
 *           word 0x18 that of record word 8 minus object word 0x3C. Between the
 *           second and third quotient it stores the constants 0x40 into word
 *           0x14 and -8 into word 0x20. It then advances object byte 2, the step
 *           index, by one, so the next dispatch selects the handler at index 5
 *           of the same run (0x800C244C), and finishes with the readiness helper
 *           func_8014DAEC again, whose returned byte it discards; that step
 *           index store fills the delay slot of the second call. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps the
 *           return address at 0x10 across its two calls. The roles of the words
 *           at 0x0C/0x10/0x14/0x18/0x20/0x34/0x38/0x3C and of byte 2 beyond the
 *           displacement to the next run word are not established, so the name
 *           describes the measured effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkThreeAxisStepDeltasAndCounterFromEnemyRecordAdvanceWorkByte2BossBoss03316_800C2310(
    void) {
  /* The original materialises the selector address once into a register and
   * reaches all three volatile reads through it, so the address is kept in a
   * local pointer instead of being folded into each load. */
  volatile u8 *sel;
  u8 *work;

  if (func_8014DAEC()) {
    sel = &D_80146374;
    work = D_1F800044;
    *(s32 *)(work + 0xC) =
        (D_801EB664[sel[0] - 3u].unk_00 - *(s32 *)(work + 0x34)) / 16;
    *(s32 *)(work + 0x10) =
        (D_801EB664[sel[0] - 3u].unk_04 - *(s32 *)(work + 0x38)) / 16;
    *(s32 *)(work + 0x14) = 0x40;
    *(s32 *)(work + 0x20) = -8;
    *(s32 *)(work + 0x18) =
        (D_801EB664[sel[0] - 3u].unk_08 - *(s32 *)(work + 0x3C)) / 16;
    work[2]++;
    func_8014DAEC();
  }
}
