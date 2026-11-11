#include "bof3/context.h"
#include "data/equipment.h"

/* @source 0x801C90DC @kind table */
extern const WeaponObject WEAPON_OBJECTS[];
/* @source 0x801C98A4 @kind table */
extern const ArmorObject ARMOR_OBJECTS[];

/* @source 0x801D1AB8
 * @behavior builds two four-halfword equipment stat blocks from a base stat
 *           block and writes a per-stat comparison code. The first block takes
 *           the power of the equipped weapon WEAPON_OBJECTS at the first byte of
 *           the index pair added to the first stat and the power of the equipped
 *           armor ARMOR_OBJECTS at the second byte added to the second stat,
 *           copies the third stat unchanged and stores the fourth stat minus
 *           both records' weights, floored at zero. The second block repeats the
 *           build with the candidate record whose index the last argument holds
 *           in the weapon slot when the mode argument is one and in the armor
 *           slot otherwise, the other slot keeping the equipped index. For each
 *           of the four stats it then writes two when the first block is below
 *           the second, one when it is above and zero when the two are equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void buildEquipStatComparison(u16* base, u16* current, u16* candidate,
                              u8* comparison, u8* pair, u8 mode, u8 other) {
  u8* weapon;
  u8* armor;
  s16 height;
  u8 index;

  weapon = (u8*)&WEAPON_OBJECTS[pair[0]];
  armor = (u8*)&ARMOR_OBJECTS[pair[1]];
  current[0] = base[0] + weapon[0x12];
  current[1] = base[1] + armor[0x10];
  current[2] = base[2];
  height = base[3] - (weapon[0x10] + armor[0x0F]);
  current[3] = height;
  if (height < 0) {
    current[3] = 0;
  }

  weapon = (u8*)&WEAPON_OBJECTS[mode == 1 ? other : pair[0]];
  armor = (u8*)&ARMOR_OBJECTS[mode == 1 ? pair[1] : other];
  candidate[0] = base[0] + weapon[0x12];
  candidate[1] = base[1] + armor[0x10];
  candidate[2] = base[2];
  height = base[3] - (weapon[0x10] + armor[0x0F]);
  candidate[3] = height;
  if (height < 0) {
    candidate[3] = 0;
  }

  for (index = 0; index < 4; index++) {
    if (current[index] < candidate[index]) {
      comparison[index] = 2;
    } else if (candidate[index] < current[index]) {
      comparison[index] = 1;
    } else {
      comparison[index] = 0;
    }
  }
}
