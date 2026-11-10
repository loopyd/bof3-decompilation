#include "bof3/bof3.h"

/* @source 0x801F2EF0 @kind function */
extern s32 func_801F2EF0(void);
void func_80196070(void);

/* @source 0x801F2D08
 * @behavior Calls the local work-table update at 0x801F2EF0 and clears the
 * scratchpad work flags through 0x80196070 when the result's low byte is zero.
 * The update scans 256 records of stride 0x18 at 0x800E4800 and returns one
 * when any active record is processed, otherwise zero. Takes no arguments
 * and returns nothing; the handler table at 0x801F4308 contains its address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenWorkTableIdleWorld03Area14313_801F2D08(void) {
  if ((func_801F2EF0() & 0xFF) == 0) {
    func_80196070();
  }
}
