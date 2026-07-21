#ifndef DATA_EQUIPMENT_H
#define DATA_EQUIPMENT_H

#include "base/types.h"

typedef struct KeyItemObject {
  u8 name[0x0c];
  u8 unknown_0c[4];
} KeyItemObject;

typedef struct ItemObject {
  u8  name[0x0c];
  u8  flags;
  u8  unknown_0d[3];
  u16 price;
} ItemObject;

typedef struct WeaponObject {
  u8  name[0x0c];
  u8  equipability;
  u8  unknown_0d[2];
  u8  element;
  u8  weight;
  u8  unknown_11;
  u8  power;
  u8  unknown_13[3];
  u16 price;
} WeaponObject;

typedef struct ArmorObject {
  u8  name[0x0c];
  u8  equipability;
  u8  unknown_0d;
  u8  equip_type;
  u8  weight;
  u8  power;
  u8  unknown_11[3];
  u16 price;
} ArmorObject;

typedef struct AccessoryObject {
  u8  name[0x0c];
  u8  equipability;
  u8  unknown_0d[2];
  u8  weight;
  u8  unknown_10[2];
  u16 price;
} AccessoryObject;

typedef struct ShopItemRef {
  u8 item_type;
  u8 item_index;
} ShopItemRef;

typedef struct ShopObject {
  u8          item_count;
  ShopItemRef slots[11];
} ShopObject;

typedef struct LevelObject {
  u16 exp;
  u8  hp;
  u8  ap;
  u8  power_defense;
  u8  agility_intellect;
  u8  ability;
  u8  unk_07;
} LevelObject;

enum {
  ITEM_COUNT = 92,
  WEAPON_COUNT = 83,
  ARMOR_COUNT = 68,
  ACCESSORY_COUNT = 52,
  KEY_ITEM_COUNT = 16,
  SHOP_COUNT = 40,
  ABILITY_COUNT = 228,
  LEVEL_COUNT = 693,
};

enum {
  ITEM_STRIDE = 0x12,
  KEY_ITEM_STRIDE = 0x10,
  WEAPON_STRIDE = 0x18,
  ARMOR_STRIDE = 0x16,
  ACCESSORY_STRIDE = 0x14,
  SHOP_STRIDE = 0x17,
  ABILITY_STRIDE = 0x14,
  LEVEL_STRIDE = 0x08,
};

ASSERT_SIZE(ShopItemRef, 0x02);
ASSERT_SIZE(ShopObject, 0x17);
ASSERT_SIZE(LevelObject, 0x08);

#endif
