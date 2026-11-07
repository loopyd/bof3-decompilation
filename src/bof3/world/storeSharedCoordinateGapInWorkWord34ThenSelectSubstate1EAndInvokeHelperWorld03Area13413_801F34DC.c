#include "bof3/bof3.h"

/* Shared published work-pointer cell: retain each original handoff and reload. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80146888[];
extern s32 D_801468BC;
extern s32 D_80145EC4;
extern u8 D_80145E90[];
extern u8* D_80146250;
extern u8 D_80146875;

void func_80196070(void);

/* @source 0x801F34DC
 * @behavior Work-state handler: saves the work pointer published at scratchpad
 * pointer slot 0x1F800044, reads the current work-record coordinate word
 * 0x801468BC, publishes the work record at 0x80146888 into the slot, publishes
 * the shared position object 0x80145E90 at 0x80146250 and stores the signed gap
 * D_80145EC4 - 0x801468BC into the saved record's coordinate word at offset
 * 0x34, so the record is re-aimed from its stored coordinate to the shared
 * coordinate published at 0x80145EC4. It then restores the saved pointer,
 * briefly republishes 0x80145E90 and restores the saved pointer again; all
 * three stores go through the volatile slot so each one survives, and the
 * pointer stays restored on exit. Finally it selects the shared sub-state byte
 * 0x80146875 = 0x1E and invokes the shared reset helper func_80196070 through a
 * 24-byte frame that homes $ra at +0x10. Word 23 of the state-byte-1 handler
 * table D_801F5744, which the dispatcher at 0x801F2C04 indexes, holds this
 * address, so it is that table's work-state 0x17 handler; the neighbouring
 * same-frame handler at 0x801F3440 selects the same 0x1E sub-state from the
 * same table run. Takes no arguments and returns nothing.
 * @status partial
 * @match 87.88
 * @residual Size 132->132 and the same 24-byte frame. The saved pointer is in
 * $a0 and the position object in $v1, as in the original. Scheduling differs:
 * the $ra save and position-address setup occur early, and the 0x1E constant
 * is loaded after the first saved-pointer restore rather than before it.
 */
void storeSharedCoordinateGapInWorkWord34ThenSelectSubstate1EAndInvokeHelperWorld03Area13413_801F34DC(void) {
  u8* saved;
  s32 current;

  saved = D_1F800044;
  current = D_801468BC;
  D_1F800044 = D_80146888;
  do {
    D_80146250 = D_80145E90;
    FIELD_REF(s32, saved, 0x34) = D_80145EC4 - current;
  } while (0);
  D_1F800044 = saved;
  D_1F800044 = D_80145E90;
  D_1F800044 = saved;
  D_80146875 = 0x1E;
  func_80196070();
}
