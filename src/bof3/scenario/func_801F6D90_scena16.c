#include "bof3/scenario/scena16_internal.h"

/* @behavior routes the primary SCENA16 area path into one local helper branch.
 * @source 0x801F6D90
 * @status partial
 * @match 73.17
 * @residual volatile halfword mask and load scheduling; 164 bytes versus 160 original.
 */
void func_801F6D90(void) {
  s8* state_base;
  s32 area_archive_id;

  state_base = PSX_PTR(s8, 0x80140000u);
  area_archive_id = *(volatile u16*)(state_base + 0x3f00);

  switch (area_archive_id) {
  case 2:
    func_801F6F30();
    *(volatile u8*)(state_base + 0x3c30) = 1u;
    break;
  case 4:
    seedRouteEnterState2();
    *(volatile u8*)(state_base + 0x3c30) = 1u;
    break;
  case 0x1f:
    seedRouteEnterState3();
    *(volatile u8*)(state_base + 0x3c30) = 0u;
    break;
  }
  *(volatile s8*)(state_base + 0x6872) = 2;
}
