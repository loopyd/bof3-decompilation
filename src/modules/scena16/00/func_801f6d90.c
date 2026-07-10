#include "internal.h"

/* does: routes the primary SCENA16 area path into one local helper branch.
 * @source: 0x801f6d90 FUN_801f6d90
 */
void func_801f6d90(void) {
  s8* state_base;
  u32 area_archive_id;

  state_base = (s8*)0x80140000u;
  area_archive_id = *(vu16*)(state_base + 0x3f00);

  if (area_archive_id == 4u) {
    goto area_4;
  }

  if (area_archive_id >= 5u) {
    goto area_5_or_more;
  }

  if (area_archive_id == 2u) {
    func_801f6f30();
    goto store_one;
  }

  goto store_state;

area_5_or_more:
  if (area_archive_id == 0x1fu) {
    func_801f6e30();
    *(vu8*)(state_base + 0x3c30) = 0u;
  }

  goto store_state;

area_4:
  func_801f6eb0();

store_one:
  *(vu8*)(state_base + 0x3c30) = 1u;

store_state:
  *(vs8*)(state_base + 0x6872) = 2;
}
