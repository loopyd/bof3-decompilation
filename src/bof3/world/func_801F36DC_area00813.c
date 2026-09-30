#include "bof3/world/area00813_internal.h"

extern void func_8014D6B8(u32 flag);

/* @source 0x801F36DC
 * @behavior selects area mode 14 when the shared high-bit flag is set;
 *           otherwise sets the activity flag of the work-area entity selected
 *           by the scratch state entity index, records the shared secondary
 *           state byte as 3, installs that entity as the scratch work pointer,
 *           requests the shared area resource selector with argument 1,
 *           selects mode 1 on the previously installed state and finally
 *           restores it as the scratch work pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F36DC(void) {
  World00Area008State* state;
  World00Area008State* previous;
  World00Area008State* work;
  u8 entityIndex;

  if (D_80146867 & 0x80) {
    g_areaWork->mode = 14;
  } else {
    state = g_areaWork;
    entityIndex = state->entityIndex;
    previous = (World00Area008State*)&D_80146888[entityIndex];
    /* The selected work-area entity is the record the mode handlers install
     * as the scratch state, so its activity flag is byte 0 of that record. */
    ((World00Area008Entity*)previous)->flags |= 0x40;
    previous = g_areaWork;
    work = (World00Area008State*)&D_80146888[previous->entityIndex];
    D_80146866 = 3;
    g_areaWork = work;
    func_8014D6B8(1);
    previous->mode = 1;
    g_areaWork = previous;
  }
}
