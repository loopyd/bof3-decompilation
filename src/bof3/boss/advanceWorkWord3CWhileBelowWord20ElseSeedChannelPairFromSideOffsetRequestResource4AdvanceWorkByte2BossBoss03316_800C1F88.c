#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each load folds to lui plus an offset load, and because a store
 * through the loaded pointer may alias the cell the original re-reads the cell
 * after the record byte store instead of carrying the entry value into the
 * displacement store. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Battle state byte in main RAM, outside this overlay's image. The sibling boss
 * handlers of this module raise it to 3, 4 and 5 and gate their own writes on
 * it; this handler only reads it, as the below-three test that selects the
 * doubled side index. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Battle side byte in main RAM, outside this overlay's image. The exact battle
 * lift func_8009B794 doubles it to index the same signed offset table, and the
 * bmagic channel-pair lifts select their own motion pairs with the same byte.
 * Declared volatile because the original re-reads it after every record store
 * (two separate lbu fetches inside each arm) and never keeps the fetched byte
 * live across one. */
extern volatile u8 D_801462EC; /* @source 0x801462EC @kind unknown */

/* Per-side signed coordinate offset pairs in main RAM, outside this overlay's
 * image. Row i holds the pair at byte 2i and 2i + 1: the exact battle lift
 * func_8009B794 adds entry 2i to the shared horizontal word and entry 2i + 1 to
 * the shared vertical word. Splat binds the address through this target's
 * generated undefined_syms_auto.txt. */
extern s8 D_800B44C0[]; /* @source 0x800B44C0 @kind table */

/* Shared fixed-point channel pair in main RAM, outside this overlay's image.
 * The bmagic channel-pair lifts add their motion pair into the same two words
 * and store the sums into work words 0x34 and 0x38. Splat binds both addresses
 * through this target's generated undefined_syms_auto.txt. */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Shared main-executable resource request helper; the sibling boss and bmagic
 * lifts declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* Readiness poll helper of the main executable. The battle and bmagic lifts
 * declare the same prototype; the original masks the returned byte with 0xFF
 * before testing it, so the declared return type is the byte it reports. */
extern u8 func_8014DAEC(void);

/* @source 0x800C1F88
 * @behavior Work-step handler at index 2 of this overlay's code-pointer run
 *           based at 0x800C27B4, the base word the step dispatcher
 *           func_800C1E70 indexes with work byte 2 scaled by four; run word 2 at
 *           0x800C27BC holds this address and no jal in this payload names it.
 *           It loads the scratch work object published at the pointer cell
 *           0x1F800044 once and compares its word 0x3C with its word 0x20. While
 *           0x3C is below 0x20 it advances 0x3C by 0x1000000 and writes nothing
 *           else. Otherwise it seeds the object's fixed-point channel pair: word
 *           0x34 takes the shared word at 0x80149308 plus the first component of
 *           the per-side signed offset pair, and word 0x38 takes the shared word
 *           at 0x8014930C plus that pair's second component. The pair is row
 *           D_801462EC of the table at 0x800B44C0; while the battle state byte
 *           0x80146384 is below three the handler also flips object byte 8 by 2,
 *           selects the side index through that same 2 flip, and re-reads the
 *           scratchpad cell before the word store. It then requests resource 4
 *           through func_8014D6B8, re-reads the cell to advance object byte 2,
 *           the step index, by one, and finishes with the readiness helper
 *           func_8014DAEC, whose returned byte it discards. The 0x18-byte frame
 *           only keeps the return address at 0x10. The roles of resource 4, of
 *           the words at 0x20/0x34/0x38/0x3C, of byte 8 and of byte 2 beyond the
 *           displacement to the next run word are not established, so the name
 *           describes the measured effect. The join of both arms reuses the one
 *           side index across the 0x38 store, and the cell re-read inside the
 *           flip arm is what keeps the 0x38 store's base in the entry register.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord3CWhileBelowWord20ElseSeedChannelPairFromSideOffsetRequestResource4AdvanceWorkByte2BossBoss03316_800C1F88(
    void) {
  u8 *work;
  u8 side;

  work = D_1F800044;
  if (*(s32 *)(work + 0x3C) < *(s32 *)(work + 0x20)) {
    *(s32 *)(work + 0x3C) += 0x1000000;
  } else {
    if (D_80146384 >= 3) {
      *(s32 *)(work + 0x34) = D_80149308[0] + D_800B44C0[D_801462EC * 2];
      side = D_801462EC;
    } else {
      work[8] ^= 2;
      work = D_1F800044;
      *(s32 *)(work + 0x34) =
          D_80149308[0] + D_800B44C0[(D_801462EC ^ 2) * 2];
      side = D_801462EC ^ 2;
    }
    *(s32 *)(work + 0x38) = D_8014930C[0] + D_800B44C0[side * 2 + 1];
    func_8014D6B8(4);
    D_1F800044[2]++;
  }
  func_8014DAEC();
}
