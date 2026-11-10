#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801469EA @kind unknown */
extern u16 workSeedHalfword801469EA;
/* @source 0x80196070 @kind function */
extern void func_80196070(void);

/* @source 0x801F33CC
 * @behavior Overlay step handler: entry 6 of the seven-word handler run based
 * at 0x801F3ED8. Copies the published work record's unsigned halfword at +0x32
 * into the shared seed halfword at 0x801469EA, then calls the reset helper at
 * 0x80196070, which clears the work object's bytes at +0x00 to +0x04. Takes no
 * arguments and returns nothing. The roles of the record's halfword at +0x32
 * and of the shared seed halfword stay described by offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkHalfword32ToSeedThenClearWorkFlagsWorld03Area13013_801F33CC(
    void) {
  u8* work;

  work = workRecordCursor1F800044;
  workSeedHalfword801469EA = *(u16*)(work + 0x32);
  func_80196070();
}
