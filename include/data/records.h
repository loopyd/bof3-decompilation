#ifndef DATA_RECORDS_H
#define DATA_RECORDS_H

#include "base/types.h"

typedef struct BaseStatsObject {
  u8  name[5];
  u8  character_index;
  u8  level;
  u8  unk_07;
  u32 exp;
  u16 status;
  u8  weapon;
  u8  shield;
  u8  helmet;
  u8  armor;
  u8  accessories[2];
  u16 current_hp;
  u16 current_ap;
  u8  current_willpower;
  u8  innoculation;
  u8  fatigue;
  u8  master_id;
  u16 max_hp;
  u16 max_ap;
  u16 power;
  u16 defense;
  u16 agility;
  u16 intellect;
  u16 unk_28;
  u8  willpower;
  u8  resistances[9];
  u8  surprise_chance;
  u8  reprisal_chance;
  u8  critical_chance;
  u8  evasion;
  u8  accuracy;
  u8  unk_39[3];
  u16 base_hp;
  u16 base_ap;
  u16 base_power;
  u16 base_defense;
  u16 base_agility;
  u16 base_intellect;
  u16 unk_48;
  u8  base_willpower;
  u8  base_resistances[9];
  u8  base_surprise_chance;
  u8  base_reprisal_chance;
  u8  base_critical_chance;
  u8  base_evasion;
  u8  base_accuracy;
  u8  unk_59[3];
  u8  healing_abilities[10];
  u8  assist_abilities[10];
  u8  attack_abilities[10];
  u8  skill_abilities[10];
  u8  unk_84;
  u8  level_up_modifiers[6];
  u8  unk_8b[0x19];
} BaseStatsObject;

ASSERT_SIZE(BaseStatsObject, 0xa4);

typedef struct MasterSkillsObject {
  u8 skill_levels[6][2];
} MasterSkillsObject;

ASSERT_SIZE(MasterSkillsObject, 0x0c);

typedef struct MasterStatsObject {
  s8 hp_bonus;
  s8 ap_bonus;
  s8 power_bonus;
  s8 defense_bonus;
  s8 agility_bonus;
  s8 intellect_bonus;
} MasterStatsObject;

ASSERT_SIZE(MasterStatsObject, 0x06);

/* unresolved: pointer ownership and consumer unknown */
typedef struct DragonPointers {
  u32 pointer[10];
} DragonPointers;

ASSERT_SIZE(DragonPointers, 0x28);

typedef struct EnemyAiBlock {
  u8 condition;
  u8 parameters[7];
  u8 skills[8];
} EnemyAiBlock;

ASSERT_SIZE(EnemyAiBlock, 0x10);

typedef struct EnemyObject {
  u8  name[8];
  u16 enemy_id;
  u8  choice_ai;
  u8  unk_0b[3];
  u8  target_preference;
  u8  unk_0f;
  u16 zenny;
  u16 exp;
  u8  level;
  u8  unk_15[3];
  u8  initial_skills[8];
  u16 hp;
  u16 ap;
  u16 power;
  u16 defense;
  u16 agility;
  u16 intellect;
  u8  steal_item_index;
  u8  steal_item_type;
  u16 steal_rate;
  u8  drop_item_index;
  u8  drop_item_type;
  u16 drop_rate;
  EnemyAiBlock ai[4];
  u8  unk_74[4];
  u8  resistances[9];
  u8  unk_81[7];
} EnemyObject;

ASSERT_SIZE(EnemyObject, 0x88);

typedef struct FormationObject {
  u8 enemy_indexes[8];
  u8 appearance_rate;
} FormationObject;

ASSERT_SIZE(FormationObject, 0x09);

typedef struct ChestObject {
  u8 memory;
  u8 item_index;
  u8 item_type;
} ChestObject;

ASSERT_SIZE(ChestObject, 0x03);

typedef struct GeneObject {
  u8 gene_index;
} GeneObject;

ASSERT_SIZE(GeneObject, 0x01);

typedef struct ChrysmObject {
  u8 gene_index;
} ChrysmObject;

ASSERT_SIZE(ChrysmObject, 0x01);

typedef struct FairyObject {
  u8 name[5];
  u8 stats[4];
} FairyObject;

ASSERT_SIZE(FairyObject, 0x09);

typedef struct ManilloItemObject {
  u8 item_index;
  u8 item_type;
  u8 fish_indexes[3];
  u8 fish_quantities[3];
} ManilloItemObject;

ASSERT_SIZE(ManilloItemObject, 0x08);

typedef struct ManilloStockObject {
  u8 trade_indexes[10];
} ManilloStockObject;

ASSERT_SIZE(ManilloStockObject, 0x0a);

typedef struct FairyGiftObject {
  u16 num_battles;
  u8  item_index;
  u8  item_type;
} FairyGiftObject;

ASSERT_SIZE(FairyGiftObject, 0x04);

typedef struct FairyExploreObject {
  u8 item_index;
  u8 item_type;
} FairyExploreObject;

ASSERT_SIZE(FairyExploreObject, 0x02);

/* unresolved: runtime load mapping for COMMU02 entry */
typedef struct FairyPrizeObject {
  u8 item_index;
  u8 item_type;
} FairyPrizeObject;

ASSERT_SIZE(FairyPrizeObject, 0x02);

enum {
  BASE_STATS_COUNT = 8,
  MASTER_COUNT = 17,
  DRAGON_POINTER_COUNT = 10,

  ENEMY_COUNT = 1400,
  FORMATION_COUNT = 1600,
  CHEST_COUNT = 224,
  GENE_COUNT = 17,
  CHRYSM_COUNT = 13,
  FAIRY_COUNT = 720,
  MANILLO_ITEM_COUNT = 165,
  MANILLO_STOCK_COUNT = 16,

  FAIRY_GIFT_COUNT = 20,
  FAIRY_EXPLORE_COUNT = 48,
  FAIRY_PRIZE_COUNT = 48,
};

enum {
  BASE_STATS_STRIDE = 0xa4,
  MASTER_SKILLS_STRIDE = 0x0c,
  MASTER_STATS_STRIDE = 0x06,
  ENEMY_STRIDE = 0x88,
  FORMATION_STRIDE = 0x09,
  CHEST_STRIDE = 0x03,
  FAIRY_STRIDE = 0x09,
  MANILLO_ITEM_STRIDE = 0x08,
  MANILLO_STOCK_STRIDE = 0x0a,
  FAIRY_GIFT_STRIDE = 0x04,
  FAIRY_EXPLORE_STRIDE = 0x02,
  FAIRY_PRIZE_STRIDE = 0x02,
};

#endif
