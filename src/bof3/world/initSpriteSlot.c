#include "bof3/world/area02813_internal.h"

/* @behavior initialises one AREA028 sprite slot: marks it active (offset 0 = 1),
 * assigns two signed random offsets in [-128, 127] at offsets 4 and 6, and
 * sets a fixed 0x280 halfword bound at offset 8.
 * @source 0x801F2FB0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initSpriteSlot(void* arg0) {
  World00Area028Work* work;

  work = (World00Area028Work*)arg0;
  work->unk_00[0] = 1u;
  work->field_04 = (s16)((rand() & 0xFF) - 0x80);
  work->field_06 = (s16)((rand() & 0xFF) - 0x80);
  work->scale = 0x280;
}
