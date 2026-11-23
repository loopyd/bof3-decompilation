#include "bof3/bof3.h"

/* Local work-record table in main RAM, outside this overlay's image. The
 * 0x140-byte-stride record array that the front end and the battle overlays
 * select by a global byte. Splat binds the address through this target's
 * generated undefined_syms_auto.txt, so no target-map row is added here. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */

/* Battle selection byte in main RAM, outside this overlay's image. It is read
 * unsigned, once, and scaled into the record table above. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Scratchpad work-object pointer cell, already bound by this target's map row
 * g_battle_work = 0x1F800044. Declared as a named symbol rather than through
 * SPAD_PTR_SLOT so each access re-materialises the cell address with its own
 * lui, which is what the original does: it loads the cell into a saved
 * register, stores the selected record back through the cell, and stores the
 * saved value again on both arms. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable readiness poll helper, outside this payload. The
 * sibling bmagic and plchar lifts declare the same byte-returning prototype
 * and test only that byte. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */

/* @source 0x801EEE1C
 * @behavior Publishes the selected local work record as the scratchpad work
 *           object, polls the shared readiness helper against it, then restores
 *           the previous work object and steps the overlay handler state. The
 *           body reads the battle selection byte at 0x80146374 unsigned, keeps
 *           the scratchpad work-object pointer cell 0x1F800044 in a saved
 *           register, and stores the address of the 0x140-byte-stride record at
 *           that index of the D_80145E90 table into the cell. While that record
 *           is the published work object it calls the shared readiness helper
 *           func_8014DAEC and keeps only the helper's returned byte: a non-zero
 *           result restores the previous work object and advances that object's
 *           unsigned byte 2 by one, the step index the byte-2 dispatcher reads
 *           next, while a zero result restores the previous work object and
 *           writes 0x10 into that object's byte 9. The restore store is
 *           duplicated on both arms, so the original stores the cell once per
 *           path, and the not-ready constant 0x10 is materialised into the
 *           branch delay slot. The same target's func_801EF570 shows this body
 *           with the not-ready arm empty, which places the restore store and
 *           the byte-2 advance on the ready path at the same offsets. Takes no
 *           arguments and returns nothing; the 0x18-byte frame keeps the return
 *           address at 0x14 and the saved pointer at 0x10. No other state is
 *           read or written, and the role of work byte 9 is not established
 *           here, so the name reports the literal it receives.
 *
 *           Representation notes: the ready arm written first as
 *           if (ready) { advance } else { set 0x10 } measured 30/35 with the
 *           polarity inverted (beqz on the helper result, so the ready arm
 *           fell through) while the frame and both stores already agreed; the
 *           inverted test with the not-ready arm first put the branch on the
 *           ready arm, which is the layout the original shows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSelectedHelperReady(
    void) {
  u8 *saved;
  u8 *record;
  u8 index;

  index = D_80146374;
  saved = g_battle_work;
  record = &D_80145E90[index * 0x140];
  g_battle_work = record;
  if (!func_8014DAEC()) {
    g_battle_work = saved;
    saved[9] = 0x10;
  } else {
    g_battle_work = saved;
    saved[2]++;
  }
}
