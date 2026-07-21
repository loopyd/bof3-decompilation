#ifndef DATA_ENCODING_H
#define DATA_ENCODING_H

#include "base/types.h"

#define EQUIP_RYU   (1 << 0)
#define EQUIP_NINA  (1 << 1)
#define EQUIP_GARR  (1 << 2)
#define EQUIP_TEEPO (1 << 3)
#define EQUIP_REI   (1 << 4)
#define EQUIP_MOMO  (1 << 5)
#define EQUIP_PECO  (1 << 6)
#define EQUIP_WHELP (1 << 7)

#define ELEM_FIRE      (1 << 0)
#define ELEM_ICE       (1 << 1)
#define ELEM_LIGHTNING (1 << 2)
#define ELEM_EARTH     (1 << 3)
#define ELEM_WIND      (1 << 4)
#define ELEM_HOLY      (1 << 5)
#define ELEM_PSIONIC   (1 << 6)
#define ELEM_STATUS    (1 << 7)

#define ITEM_USABLE_MENU          (1 << 7)
#define ITEM_SHOW_ANIMATION       (1 << 6)
#define ITEM_SHOW_NAME            (1 << 5)
#define ITEM_STORY_ITEM           (1 << 4)
#define ITEM_TARGET_ALL           (1 << 3)
#define ITEM_TARGET_ENEMY_DEFAULT (1 << 2)
#define ITEM_TARGET_SELECTABLE    (1 << 1)
#define ITEM_TARGET_BOTH          (1 << 0)

#define ABIL_AFFECTS_STATS       (1 << 4)
#define ABIL_UNKNOWN_3           (1 << 3)
#define ABIL_TARGET_ALLY_DEFAULT (1 << 2)
#define ABIL_EXAMINABLE          (1 << 1)
#define ABIL_UNKNOWN_0           (1 << 0)

enum {
  ARMOR_TYPE_NOTHING = 0,
  ARMOR_TYPE_SHIELD = 2,
  ARMOR_TYPE_HELMET = 3,
  ARMOR_TYPE_BODY = 4,
};

enum {
  SKILL_CLASS_HEALING = 0,
  SKILL_CLASS_ASSIST = 1,
  SKILL_CLASS_ATTACK = 2,
  SKILL_CLASS_EXAMINABLE = 3,
};

enum {
  ITEM_CAT_ITEM = 0,
  ITEM_CAT_WEAPON = 1,
  ITEM_CAT_ARMOR = 2,
  ITEM_CAT_ACCESSORY = 3,
  ITEM_CAT_KEY_ITEM = 4,
  ITEM_CAT_EMPTY = 0xFF,
};

#define NAME_END        0x00
#define NAME_SPACE      0xFF
#define NAME_APOSTROPHE 0x8E
#define NAME_HYPHEN     0x3D
#define NAME_PERIOD     0x3E
#define NAME_PLUS       0x8B
#define NAME_COLOR_TAG  0x05
#define NAME_COLOR_RED  0x02
#define NAME_COLOR_BLUE 0x03
#define NAME_NOCOLOR    0x06
#define NAME_NEWLINE    0x01

#define FORMATION_SLOT_EMPTY 0xFF
#define MASTER_SKILL_EMPTY   0xFF63
#define MASTER_ID_NONE       0xFF
#define MONSTER_COND_UNUSED  0x63
#define PTR_SLOT_EMPTY_LOW   0x00000
#define PTR_SLOT_EMPTY_HIGH  0xFFFFF

#define SHOP_ITEM_TYPE(ref)  ((ref) & 0xFF)
#define SHOP_ITEM_INDEX(ref) (((ref) >> 8) & 0xFF)

#define MASTER_SKILL_LEVEL(val)   ((val) & 0xFF)
#define MASTER_SKILL_ABILITY(val) (((val) >> 8) & 0xFF)

#endif
