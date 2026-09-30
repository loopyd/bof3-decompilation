#include "bof3/world/area02414_internal.h"

/* @behavior per-entry initializer of the local eight-entry work array (called by
 * func_801F3080 for each 0x28-byte slot): copies the three scratch words at
 * 0x34/0x38/0x3c into the entry's word fields, seeds the entry's vector
 * components at 0x14/0x18/0x1c from `rand()`, hands that vector to the shared
 * helper func_80178818 in place, then sets the entry's leading bytes to 1/0/4/
 * 0x40, clears its size halfword at 0x24 and scales the vector z component by
 * 256.
 * @source 0x801F2FD4
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 43/43 instructions and 172 -> 172 bytes.
 */
void func_801F2FD4(void* arg0) {
  World00Area024SpriteWork*       work;
  volatile World00Area024Scratch* scratch;

  work = (World00Area024SpriteWork*)arg0;
  scratch = WORLD00_AREA024_SCRATCH_PTR;
  work->field_04 = scratch->field_34;
  work->field_08 = scratch->field_38;
  work->field_0c = scratch->field_3c;
  work->field_14.vx = (rand() & 0xFF) - 0x80;
  work->field_14.vy = (rand() & 0xFF) - 0x80;
  work->field_14.vz = rand() & 0x7F;
  func_80178818(&work->field_14, &work->field_14);
  work->field_00 = 1;
  work->field_03 = 0x40;
  work->field_01 = 0;
  work->field_24 = 0;
  work->field_02 = 4;
  work->field_14.vz = work->field_14.vz << 8;
}
