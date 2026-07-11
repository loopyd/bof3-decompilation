#include "internal.h"

/* @behavior seeds one local sprite work entry from scratchpad positions and three
 * random vector components, then normalizes the vector and installs fixed
 * mode bytes for the later draw path.
 * @source 0x801f2fd4 FUN_801f2fd4
 */
void func_801f2fd4(void* arg0) {
  World00Area024SpriteWork* work;

  work = (World00Area024SpriteWork*)arg0;

  work->field_04 = WORLD00_AREA024_SCRATCH_PTR->field_34;
  work->field_08 = WORLD00_AREA024_SCRATCH_PTR->field_38;
  work->field_0c = WORLD00_AREA024_SCRATCH_PTR->field_3c;
  work->field_14.vx = (rand() & 0xff) - 0x80;
  work->field_14.vy = (rand() & 0xff) - 0x80;
  work->field_14.vz = rand() & 0x7f;
  VectorNormal(&work->field_14, &work->field_14);
  work->field_00 = 1u;
  work->field_03 = 0x40u;
  work->field_01 = 0u;
  work->field_24 = 0u;
  work->field_02 = 4u;
  work->field_14.vz <<= 8;
}
