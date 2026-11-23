#include "bof3/bof3.h"

void func_80196718(void *arg0);
void func_800AFC54(void);
void func_800AEF98(s32 event, s32 value);
void func_800A9820(u8 arg0);

/* Shared battle record pointer cell in main executable RAM at 0x801EB4E0,
 * outside this overlay's image. The sibling lifts of this same target read the
 * record through the same cell, and this target's generated
 * undefined_syms_auto.txt already binds the address, so no target-map row is
 * added here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. Declared as a named cell symbol, the form the same
 * target's lifts copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2
 * BmagicMagic05603_801EF27C and copyRecordByte27ThenAdvanceWorkByte2
 * BmagicMagic05603_801EFFE8 use: the symbolic address folds into each load
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)), so the cell is materialised once for the
 * opening guard and again after the calls, because the stores in between may
 * alias it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle selection byte at 0x80146374 in main executable RAM, outside
 * this overlay's image; it holds the acting battler and is re-read before every
 * seeded field because the stores may alias it. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * the readiness helper func_800A9820 consumes. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Local party work-record array at 0x80145E90, whose entries stride by 0x140
 * bytes and are selected by selection bytes below three. The original reaches
 * the record base and each seeded field as its own flat symbol scaled by the
 * party stride (lui $at,%hi(ADDR) / addu $at,$at,index / sb|sh %lo(ADDR)($at)),
 * so the fields keep their own bases instead of one record pointer: byte +1 is
 * 0x80145E91, byte +2 is 0x80145E92, halfword +0x80 is 0x80145F10 and halfword
 * +0x88 is 0x80145F18, which the shared battle lifts read as this record's flag
 * halfword and leading halfword. Splat binds all five addresses through this
 * target's generated undefined_syms_auto.txt. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_80145E91[]; /* @source 0x80145E91 @kind unknown */
extern u8 D_80145E92[]; /* @source 0x80145E92 @kind unknown */
extern u8 D_80145F10[]; /* @source 0x80145F10 @kind data */
extern u8 D_80145F18[]; /* @source 0x80145F18 @kind data */

/* Local enemy work-record arrays for selection bytes of three and above. The
 * record this body submits is 0x801EB2E8 plus the unresized selection byte
 * times the 0x118-byte enemy stride; the four seeded fields are the byte pair
 * at 0x801EB631 and 0x801EB632 and the halfwords at 0x801EB6B2 and 0x801EB6C4,
 * each scaled by the selection byte less three, which lands them on bytes +1,
 * +2, +0x82 and +0x94 of the entry the submitted pointer names. They are flat
 * symbols for the same reason as the party fields. */
extern u8 D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */
extern u8 D_801EB631[]; /* @source 0x801EB631 @kind data */
extern u8 D_801EB632[]; /* @source 0x801EB632 @kind data */
extern u8 D_801EB6B2[]; /* @source 0x801EB6B2 @kind data */
extern u8 D_801EB6C4[]; /* @source 0x801EB6C4 @kind data */

/* @source 0x801EEF3C
 * @behavior Word 1 of the three-entry code pointer run at 0x801EEC04, whose
 *           words are 0x801EEC94, 0x801EEF3C and 0x801EF17C: the dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic05603_801EEC34 copies them into
 *           its 0x28-byte frame and selects one with the scratchpad work
 *           object's byte 1 scaled by four. It reads the work-object pointer
 *           cell at 0x1F800044 before its frame exists and returns at once when
 *           work byte 0xB is non-zero. Otherwise it clears bit 0x40 of byte 0 of
 *           the shared battle record read through the pointer at 0x801EB4E0,
 *           that store filling the delay slot of the shared buffer-copy helper
 *           at 0x800AFC54, dispatches event selector 0x1C with the value 0
 *           through the shared helper at 0x800AEF98, and forwards the battle
 *           target byte at 0x80146384 to the readiness helper at 0x800A9820. It
 *           then selects the battler named by the battle selection byte
 *           0x80146374: indices below three seed the party record that
 *           stride-0x140 entry 0x80145E90 holds, whose gate byte +1 becomes 6,
 *           byte +2 becomes 4, halfword +0x80 becomes 0x4000 and halfword +0x88
 *           becomes 0, while higher indices seed the same four fields of the
 *           stride-0x118 enemy entry through the selection byte less three at
 *           0x801EB631, 0x801EB632, 0x801EB6B2 and 0x801EB6C4. Each arm submits
 *           its record through func_80196718 - the party base plus the index
 *           times 0x140, or 0x801EB2E8 plus the index times 0x118 - and both
 *           arms jump to one shared call, whose argument add fills the call's
 *           delay slot. The body ends by re-reading the cell and incrementing
 *           work byte 1, which selects run entry 2 (0x801EF17C). It takes no
 *           arguments, returns nothing and writes no other state; the 0x18-byte
 *           frame keeps only the return address at 0x10 across the calls, and
 *           every store group re-reads the selection byte because a store may
 *           alias it.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation notes from the shape probes measured before this body
 * matched: spelling the enemy fields as one expression over the selection byte
 * with the inline reduction, `D_801EB631[(D_80146374 - 3) * 0x118]`, made the
 * compiler fold the reduction into the symbol displacement and drop the four
 * in-place decrements (134/144 instructions, 560 bytes); taking the selector as
 * an `u8[]` whose element is read per field also made the compiler keep the
 * selector address in one register instead of materialising it per read (same
 * score); and `enemy = D_80146374 - 3` in one statement reached the right length
 * but loaded the byte and the reduced value into different registers (135/144,
 * 576 bytes). Reading the selector into the local and then decrementing it, as
 * the body below does, keeps both the per-field reload and the in-place
 * decrement.
 */
void stateForwardSelectedBattler(
    void) {
  s32 enemy;

  if (g_battle_work[0xB] != 0) {
    return;
  }
  D_801EB4E0[0] &= 0xBF;
  func_800AFC54();
  func_800AEF98(0x1C, 0);
  func_800A9820(D_80146384);
  if (D_80146374 < 3u) {
    D_80145E91[D_80146374 * 0x140] = 6;
    D_80145E92[D_80146374 * 0x140] = 4;
    *(u16 *)(D_80145F18 + D_80146374 * 0x140) = 0;
    *(u16 *)(D_80145F10 + D_80146374 * 0x140) = 0x4000;
    func_80196718(D_80145E90 + D_80146374 * 0x140);
  } else {
    /* The selection byte is read and reduced again before each field because a
     * store may alias it, and the reduction is kept in the local so the symbol
     * offsets stay unscaled by it. */
    enemy = D_80146374;
    enemy -= 3;
    D_801EB631[enemy * 0x118] = 6;
    enemy = D_80146374;
    enemy -= 3;
    D_801EB632[enemy * 0x118] = 4;
    enemy = D_80146374;
    enemy -= 3;
    *(u16 *)(D_801EB6C4 + enemy * 0x118) = 0;
    enemy = D_80146374;
    enemy -= 3;
    *(u16 *)(D_801EB6B2 + enemy * 0x118) = 0x4000;
    func_80196718(D_801EB2E8 + D_80146374 * 0x118);
  }
  g_battle_work[1]++;
}
