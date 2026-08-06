#include "internal.h"

/* @behavior routes the primary SCENA16 area path into one local helper branch.
 * @source 0x801F6D90
 */
void func_801F6D90(void) {
  s8* state_base;
  u32 area_archive_id;

  state_base = PSX_PTR(s8, 0x80140000u);
  area_archive_id = *(volatile u16*)(state_base + 0x3f00);

  if (area_archive_id == 4u) {
    goto area_4;
  }

  if (area_archive_id >= 5u) {
    goto area_5_or_more;
  }

  if (area_archive_id == 2u) {
    func_801F6F30();
    goto store_one;
  }

  goto store_state;

area_5_or_more:
  if (area_archive_id == 0x1fu) {
    scena16_seed_route_enter_state3();
    *(volatile u8*)(state_base + 0x3c30) = 0u;
  }

  goto store_state;

area_4:
  scena16_seed_route_enter_state2();

store_one:
  *(volatile u8*)(state_base + 0x3c30) = 1u;

store_state:
  *(volatile s8*)(state_base + 0x6872) = 2;
}
