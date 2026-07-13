/* Analysis-only packed storage layouts. Compiled declarations stay target-local. */
typedef unsigned char u8;
typedef unsigned short u16;

#pragma pack(push, 1)
typedef struct ItemObject { u8 name[12]; u8 flags; u8 unknown_0d[3]; u16 price; } ItemObject;
typedef struct KeyItemObject { u8 name[12]; u8 unknown_0c[4]; } KeyItemObject;
typedef struct WeaponObject { u8 name[12]; u8 equipability; u8 unknown_0d[2]; u8 element; u8 weight; u8 unknown_11; u8 power; u8 unknown_13[3]; u16 price; } WeaponObject;
typedef struct ArmorObject { u8 name[12]; u8 equipability; u8 unknown_0d; u8 equip_type; u8 weight; u8 power; u8 unknown_11[3]; u16 price; } ArmorObject;
typedef struct AccessoryObject { u8 name[12]; u8 equipability; u8 unknown_0d[2]; u8 weight; u8 unknown_10[2]; u16 price; } AccessoryObject;
typedef struct AbilityObject { u8 name[12]; u8 targeting_flags; u8 skill_type; u8 cost; u8 power; u8 element; u8 ability_flags; u8 reserved[2]; } AbilityObject;
typedef struct LevelObject { u16 exp; u8 hp; u8 ap; u8 power_defense; u8 agility_intellect; u8 ability; u8 unknown_07; } LevelObject;
typedef struct ShopItemRef { u8 item_type; u8 item_index; } ShopItemRef;
typedef struct ShopObject { u8 item_count; ShopItemRef slots[11]; } ShopObject;
typedef struct EnemyAiBlock { u8 condition; u8 parameters[7]; u8 skills[8]; } EnemyAiBlock;
typedef struct EnemyObject { u8 name[8]; u16 enemy_id; u8 choice_ai; u8 unknown_0b[3]; u8 target_preference; u8 unknown_0f; u16 zenny; u16 exp; u8 level; u8 unknown_15[3]; u8 initial_skills[8]; u16 hp; u16 ap; u16 power; u16 defense; u16 agility; u16 intellect; u8 steal_item_index; u8 steal_item_type; u16 steal_rate; u8 drop_item_index; u8 drop_item_type; u16 drop_rate; EnemyAiBlock ai[4]; u8 unknown_78[4]; u8 resistances[9]; u8 unknown_85[7]; } EnemyObject;
typedef struct FormationObject { u8 enemy_indexes[8]; u8 appearance_rate; } FormationObject;
typedef struct ChestObject { u8 memory; u8 item_index; u8 item_type; } ChestObject;
typedef struct FairyObject { u8 name[5]; u8 stats[4]; } FairyObject;
typedef struct FairyGiftObject { u16 num_battles; u8 item_index; u8 item_type; } FairyGiftObject;
typedef struct FairyItemObject { u8 item_index; u8 item_type; } FairyItemObject;
typedef struct ManilloItemObject { u8 item_index; u8 item_type; u8 fish_indexes[3]; u8 fish_quantities[3]; } ManilloItemObject;
typedef struct ManilloStockObject { u8 trade_indexes[10]; } ManilloStockObject;
#pragma pack(pop)
