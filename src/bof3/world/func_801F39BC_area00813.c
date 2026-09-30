#include "bof3/world/area00813_internal.h"

extern void func_8014D6B8(u32 flag);
extern void func_80196070(void);

/* @source 0x801F39BC
 * @behavior raises bit 0x40 in the activity flags of the work-area entity
 *           selected by the scratch state entity index, installs that entity
 *           as the scratch work pointer, requests the shared area resource
 *           selector with argument 1, restores the previous scratch work
 *           pointer and finally calls the shared 0x80196070 work-area reset
 *           helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F39BC(void) {
  World00Area008State* state;
  World00Area008State* previous;
  World00Area008State* work;
  u8 entityIndex;

  state = g_areaWork;
  entityIndex = state->entityIndex;
  previous = (World00Area008State*)&D_80146888[entityIndex];
  /* The selected work-area entity is the record the mode handlers install
   * as the scratch state, so its activity flag is byte 0 of that record. */
  ((World00Area008Entity*)previous)->flags |= 0x40;
  previous = g_areaWork;
  work = (World00Area008State*)&D_80146888[previous->entityIndex];
  g_areaWork = work;
  func_8014D6B8(1);
  g_areaWork = previous;
  func_80196070();
}
