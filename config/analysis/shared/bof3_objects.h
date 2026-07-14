/* Analysis-only packed storage layouts. Compiled declarations stay target-local. */

#pragma pack(push, 1)
typedef struct ItemObject { unsigned char name[12]; unsigned char flags; unsigned char unknown_0d[3]; unsigned short price; } ItemObject;
typedef struct KeyItemObject { unsigned char name[12]; unsigned char unknown_0c[4]; } KeyItemObject;
typedef struct WeaponObject { unsigned char name[12]; unsigned char equipability; unsigned char unknown_0d[2]; unsigned char element; unsigned char weight; unsigned char unknown_11; unsigned char power; unsigned char unknown_13[3]; unsigned short price; } WeaponObject;
typedef struct ArmorObject { unsigned char name[12]; unsigned char equipability; unsigned char unknown_0d; unsigned char equip_type; unsigned char weight; unsigned char power; unsigned char unknown_11[3]; unsigned short price; } ArmorObject;
typedef struct AccessoryObject { unsigned char name[12]; unsigned char equipability; unsigned char unknown_0d[2]; unsigned char weight; unsigned char unknown_10[2]; unsigned short price; } AccessoryObject;
typedef struct AbilityObject { unsigned char name[12]; unsigned char targeting_flags; unsigned char skill_type; unsigned char cost; unsigned char power; unsigned char element; unsigned char ability_flags; unsigned char reserved[2]; } AbilityObject;
typedef struct LevelObject { unsigned short exp; unsigned char hp; unsigned char ap; unsigned char power_defense; unsigned char agility_intellect; unsigned char ability; unsigned char unknown_07; } LevelObject;
typedef struct ShopItemRef { unsigned char item_type; unsigned char item_index; } ShopItemRef;
typedef struct ShopObject { unsigned char item_count; ShopItemRef slots[11]; } ShopObject;
typedef struct EnemyAiBlock { unsigned char condition; unsigned char parameters[7]; unsigned char skills[8]; } EnemyAiBlock;
typedef struct EnemyObject { unsigned char name[8]; unsigned short enemy_id; unsigned char choice_ai; unsigned char unknown_0b[3]; unsigned char target_preference; unsigned char unknown_0f; unsigned short zenny; unsigned short exp; unsigned char level; unsigned char unknown_15[3]; unsigned char initial_skills[8]; unsigned short hp; unsigned short ap; unsigned short power; unsigned short defense; unsigned short agility; unsigned short intellect; unsigned char steal_item_index; unsigned char steal_item_type; unsigned short steal_rate; unsigned char drop_item_index; unsigned char drop_item_type; unsigned short drop_rate; EnemyAiBlock ai[4]; unsigned char unknown_78[4]; unsigned char resistances[9]; unsigned char unknown_85[7]; } EnemyObject;
typedef struct BaseStatsObject {
  unsigned char name[5]; unsigned char character_index; unsigned char level; unsigned char unknown_07; unsigned int exp; unsigned short status;
  unsigned char weapon; unsigned char shield; unsigned char helmet; unsigned char armor; unsigned char accessories[2];
  unsigned short current_hp; unsigned short current_ap; unsigned char current_willpower; unsigned char innoculation;
  unsigned char fatigue; unsigned char master; unsigned short hp; unsigned short ap; unsigned short power; unsigned short defense;
  unsigned short agility; unsigned short intellect; unsigned short unknown_28; unsigned char willpower; unsigned char resistances[9];
  unsigned char surprise_chance; unsigned char reprisal_chance; unsigned char critical_chance; unsigned char evasion;
  unsigned char accuracy; unsigned char unknown_39[3]; unsigned short base_hp; unsigned short base_ap; unsigned short base_power;
  unsigned short base_defense; unsigned short base_agility; unsigned short base_intellect; unsigned short unknown_48;
  unsigned char base_willpower; unsigned char base_resistances[9]; unsigned char base_surprise_chance;
  unsigned char base_reprisal_chance; unsigned char base_critical_chance; unsigned char base_evasion;
  unsigned char base_accuracy; unsigned char unknown_59[3]; unsigned char healing_abilities[10];
  unsigned char assist_abilities[10]; unsigned char attack_abilities[10]; unsigned char skill_abilities[10];
  unsigned char unknown_84; unsigned char level_up_modifiers[6]; unsigned char unknown_8b[0x19];
} BaseStatsObject;
/* STATUS.EMI stores a byte-identical copy of BaseStatsObject. */
typedef BaseStatsObject BaseStats2Object;
typedef struct MasterSkillsObject { unsigned char skill_levels[6][2]; } MasterSkillsObject;
typedef struct MasterStatsObject { unsigned char stat_bonus[6]; } MasterStatsObject;
typedef struct DragonPointers { unsigned int pointer[10]; } DragonPointers;
typedef struct GeneObject { unsigned char gene_index; } GeneObject;
typedef struct ChrysmObject { unsigned char gene_index; } ChrysmObject;
typedef struct FormationObject { unsigned char enemy_indexes[8]; unsigned char appearance_rate; } FormationObject;
typedef struct ChestObject { unsigned char memory; unsigned char item_index; unsigned char item_type; } ChestObject;
typedef struct FairyObject { unsigned char name[5]; unsigned char stats[4]; } FairyObject;
typedef struct FairyGiftObject { unsigned short num_battles; unsigned char item_index; unsigned char item_type; } FairyGiftObject;
typedef struct FairyExploreObject { unsigned char item_index; unsigned char item_type; } FairyExploreObject;
typedef struct FairyPrizeObject { unsigned char item_index; unsigned char item_type; } FairyPrizeObject;
typedef struct ManilloItemObject { unsigned char item_index; unsigned char item_type; unsigned char fish_indexes[3]; unsigned char fish_quantities[3]; } ManilloItemObject;
typedef struct ManilloStockObject { unsigned char trade_indexes[10]; } ManilloStockObject;
#pragma pack(pop)
