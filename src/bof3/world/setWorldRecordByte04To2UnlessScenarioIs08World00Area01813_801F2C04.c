#include "bof3/bof3.h"

extern s8 D_80146870;    /* @source 0x80146870 @kind data */
extern u16 D_80143F00;   /* @source 0x80143F00 @kind data */
extern u8 *D_80181164[]; /* @source 0x80181164 @kind table */

/* @source 0x801F2C04
 * @behavior Reads the shared scenario-state byte D_80146870 and selects the
 *           world record reached through the pointer table D_80181164 at the
 *           pending world id D_80143F00. When the scenario byte is 8 it stores
 *           0 into byte 4 of that record, and otherwise it stores 2 there.
 *           Each arm spells the whole lookup expression, so the pending world
 *           id is read from the halfword cell and the record pointer loaded
 *           again per store; the clear arm stores the zero register directly
 *           and the set arm materialises 2 once. The signed byte load follows
 *           the original's lb of D_80146870. Takes no arguments, returns
 *           nothing, calls nothing and reserves no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 * Written directly from the original bytes: no payload under out/binaries/
 * carries this 100-byte span (one find over the payload bytes returned only
 * this overlay), so the clone lever had no representative and the neighbouring
 * world00/area015/13 body at the same address supplied statement shape only.
 */
void setWorldRecordByte04To2UnlessScenarioIs08World00Area01813_801F2C04(void) {
  if (D_80146870 == 8) {
    D_80181164[D_80143F00][4] = 0;
  } else {
    D_80181164[D_80143F00][4] = 2;
  }
}
