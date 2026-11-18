#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The original re-reads the cell once per later access group
 * (the two byte clears, the saved pointer the publish block keeps in $s0) and
 * re-materialises the cell address with its own lui for the publish store and
 * again for the restoring store, so the accesses go through the named pointer
 * rather than a folded constant base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The original loads the pointer it holds and publishes it as the work object
 * for the duration of the update helper call. The generated undefined-syms map
 * already binds the address because the overlay's remaining assembly spans load
 * it, so the name needs no target-map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-RAM byte at 0x80145988, stored as 1 through its own lui/sb pair.
 * Other lifted bodies in this tree describe it as a stage serial, but nothing in
 * this target establishes that role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind unknown */

/* Shared main-executable helper at 0x8014D978, outside this overlay's image and
 * bound by the generated undefined_funcs_auto.txt row of the same name. The
 * original calls it with no argument set up and a nop in the delay slot, and
 * this target's exact sibling
 * publishBattleWorkRecordPointerInvokeUpdateHelperThenForwardBattleByteRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic13803_801EEED8
 * declares it the same way. */
extern void func_8014D978(void); /* @source 0x8014D978 @kind function */

/* @source 0x801EEC88
 * @behavior Entry stage 0 of the MAGIC138 slot 3 overlay: word 0 of the
 *           nine-word handler pointer run at 0x801EEC04 and one of the three
 *           entries the dispatcher at 0x801EEC28 copies into its own frame, so
 *           no jal in the payload names it; work byte 0x1 selects it. It seeds
 *           the scratchpad work object's halfwords 0x2E and 0x30 with 0xF0 and
 *           0xFA from one load of pointer cell 0x1F800044, then copies the
 *           sixteen unsigned halfwords of the table at 0x80036C00 into the
 *           table at 0x8003AC00 unchanged, testing a byte counter against 0x10
 *           at the loop bottom. It stores 1 into the shared main-RAM byte
 *           0x80145988 and clears work byte 0xB, re-reading the pointer cell for
 *           that clear; it then clears work byte 0x9 from a further cell read,
 *           publishes the shared battle work record pointer held at 0x801EB4E0
 *           into the cell while keeping the cell's previous value in $s0,
 *           invokes the shared update helper func_8014D978 for its effect only,
 *           restores the cell from $s0 and increments work byte 0x1 through that
 *           same saved pointer, which is the handler index the dispatcher tests.
 *           Takes no arguments and returns nothing; the 0x18-byte frame keeps
 *           $ra at 0x14 and the saved cell value at 0x10 across the one call.
 *           The roles of work halfwords 0x2E and 0x30, of work bytes 0xB and
 *           0x9, of the two halfword tables and of byte 0x80145988 are not
 *           established here, so they stay offset-anchored. The 0xD8-byte span
 *           was compared against every payload under out/binaries when this body
 *           was written and occurs only here, so no clone seed existed; the
 *           statement order, the pose seeds, the record-clear and reload pattern
 *           and the publish-and-restore block were taken from this target's
 *           exact sibling 0x801EEED8 and from the exact bodies
 *           forwardBattleByteAndRaiseFlag4OrCopyPoseAnd16HalfwordsThenAdvanceWorkByte1BmagicMagic11303_801EECE0
 *           and
 *           clearRecordBytes0To2SeedWorkMotionPairSetCountdownClearBytesABAdvanceWorkByte1BmagicMagic00303_801EED14,
 *           which carry the same sixteen-halfword copy and the same pose seeds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkPoseCopy16HalfwordsSetByte80145988ToOneClearWorkBytesBAnd9PublishWorkPointerInvokeUpdateHelperThenAdvanceWorkByte1BmagicMagic13803_801EEC88(
    void) {
  u8 index;
  u8 *work;
  u8 *clearB;
  u8 *clear9;
  u8 *saved;

  index = 0;
  work = g_battle_work;
  *(u16 *)(work + 0x2E) = 0xF0;
  *(u16 *)(work + 0x30) = 0xFA;
  for (; index < 0x10; index++) {
    PSX_PTR(u16, 0x8003AC00u)[index] = PSX_PTR(u16, 0x80036C00u)[index];
  }
  clearB = g_battle_work;
  D_80145988 = 1;
  clearB[0xB] = 0;
  clear9 = g_battle_work;
  clear9[9] = 0;
  saved = g_battle_work;
  g_battle_work = D_801EB4E0;
  func_8014D978();
  g_battle_work = saved;
  saved[1]++;
}
