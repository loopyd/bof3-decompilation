#ifndef BOF3_SRC_EMI_BATTLE_03_INTERNAL_H
#define BOF3_SRC_EMI_BATTLE_03_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*Battle03Handler)(void);
typedef void (*Battle03EnemyModeHandler)(s32 arg0);
typedef void (*Battle03ForwardingHandler)(s32 arg0, s32 arg1, s32 arg2,
                                          s32 arg3, s32 arg4, s32 arg5,
                                          s32 arg6, u8* selector);

/*
 * The battle selector overlays the last eight bytes of each GAME ability
 * record.  The same bytes are consumed as element/ability flags by menu and
 * effect code, or as a 16-bit selection mask by battle code.  Keep the
 * overlay explicit until a caller proves a single semantic type.
 */
typedef union AbilityObjectTail {
  struct {
    u8 element;
    u8 ability_flags;
  } ability;
  u16 selection_mask;
} AbilityObjectTail;

typedef struct AbilityObject {
  u8                name[0x0c];
  u8                targeting_flags;
  u8                skill_type;
  u8                cost;
  u8                power;
  AbilityObjectTail tail_10;
  u8                control_12[2];
} AbilityObject;

extern volatile AbilityObject ABILITY_OBJECTS[];

typedef struct Battle03LocalWork {
  u8  flags_00;
  u8  unk_01;
  u8  unk_02;
  u8  unk_03;
  u8  unk_04;
  u8  pad_05[3];
  u8  unk_08;
  u8  pad_09[3];
  u32 unk_0c;
  u32 unk_10;
  u32 unk_14;
  u32 unk_18;
  u32 unk_1c;
  u32 unk_20;
  u8  pad_24[5];
  u8  unk_29;
  u8  pad_2a;
  u8  unk_2b;
  u8  pad_2c[8];
  s32 unk_34;
  s32 unk_38;
  u8  pad_3c[2];
  s16 unk_3e;
  u8  pad_40[8];
  u8  unk_48;
  u8  pad_49[0xf7];
} Battle03LocalWork;

typedef struct Battle03EnemyWork {
  u8                       unk_00;
  u8                       unk_01;
  u8                       pad_02[0xe2];
  Battle03EnemyModeHandler unk_e4;
  u8                       pad_e8[8];
  u8                       unk_f0;
  u8                       pad_f1[0x27];
} Battle03EnemyWork;

typedef struct Battle03QueuedSlot {
  u8  unk_00;
  u8  pad_01[4];
  u8  unk_05;
  u8  unk_06;
  u8  pad_07[0x6d];
  u32 unk_74;
} Battle03QueuedSlot;

#define BATTLE_LOCAL_WORK_ARRAY  VPTR(Battle03LocalWork, 0x80145e90u)
#define BATTLE_ENEMY_WORK_ARRAY  VPTR(Battle03EnemyWork, 0x801eb630u)
#define BATTLE_QUEUED_SLOT_ARRAY VPTR(Battle03QueuedSlot, 0x801ec330u)
#define BATTLE_LOCAL_WORK_PTR    VPPTR(Battle03LocalWork, 0x80146250u)
extern vu8  BATTLE_LOCAL_BYTE_62EC;
extern vu8  BATTLE_GLOBAL_BYTE_62E0;
extern vu8  BATTLE_GLOBAL_BYTE_62E1;
extern vu8  BATTLE_GLOBAL_BYTE_62E2;
extern vu16 BATTLE_GLOBAL_HALF_62E8;
extern vu8  BATTLE_GLOBAL_BYTE_62EA;
extern vu8  BATTLE_GLOBAL_BYTE_62EE;
extern vu8  BATTLE_GLOBAL_BYTE_62F0;
extern vu8  BATTLE_GLOBAL_BYTE_62F4;
extern vu8  BATTLE_GLOBAL_BYTE_62F3;
#define BATTLE_GLOBAL_BYTE_62FC(index) \
  (*(volatile volatile u8*)(0x801462fcu + (u32)(index)))
#define BATTLE_GLOBAL_BYTE_62F6(index) \
  (*(volatile volatile u8*)(0x801462f6u + (u32)(index)))
extern vu8 BATTLE_GLOBAL_BYTE_6301;
extern vu8 BATTLE_GLOBAL_BYTE_6302;
extern vu8 BATTLE_GLOBAL_BYTE_6303;
#define BATTLE_GLOBAL_BYTE_630C(index) \
  (*(volatile volatile u8*)(0x8014630cu + (u32)(index)))
extern vu32 BATTLE_GLOBAL_WORD_59F0;
extern vu32 BATTLE_GLOBAL_WORD_598C;
#define BATTLE_UI_CHAR_BUFFER VPTR(u8, 0x80145ad4u)
extern vu16 BATTLE_SCRATCH_HALF_000;
extern vu8  BATTLE_SCRATCH_BYTE_000;
extern vu8  BATTLE_SCRATCH_BYTE_001;
extern vu8  BATTLE_SCRATCH_BYTE_002;
extern vu8  BATTLE_LOCAL_FLAG_63CE;
extern vu8  BATTLE_GLOBAL_BYTE_6325;
extern vu8  BATTLE_GLOBAL_BYTE_6327;
extern vu8  BATTLE_GLOBAL_BYTE_6328;
extern vu32 BATTLE_GLOBAL_WORD_632C;
extern vu32 BATTLE_GLOBAL_WORD_6330;
#define BATTLE_GLOBAL_HALF_6334(index) \
  (*(volatile volatile u16*)(0x80146334u + ((u32)(index) * 2u)))
#define BATTLE_GLOBAL_BYTE_6354(index) \
  (*(volatile volatile u8*)(0x80146354u + (u32)(index)))
extern vu8 BATTLE_GLOBAL_BYTE_6374;
extern vu8 BATTLE_GLOBAL_BYTE_6375;
#define BATTLE_GLOBAL_PTR_6380 VPPTR(u8, 0x80146380u)
extern vu8  BATTLE_GLOBAL_BYTE_6384;
extern vu8  BATTLE_GLOBAL_BYTE_6324;
extern vu8  BATTLE_GLOBAL_BYTE_6304;
extern vu8  BATTLE_GLOBAL_BYTE_6308;
extern vu16 BATTLE_GLOBAL_HALF_63B8;
extern vu8  BATTLE_GLOBAL_BYTE_63BA;
extern vu16 BATTLE_GLOBAL_HALF_63C0;
extern vu16 BATTLE_GLOBAL_HALF_63C2;
extern vu8  BATTLE_GLOBAL_BYTE_6322;
extern vu8  BATTLE_GLOBAL_BYTE_6323;
extern vu8  BATTLE_GLOBAL_BYTE_63CE;
extern vu16 BATTLE_GLOBAL_HALF_63DA;
extern vu8  BATTLE_GLOBAL_BYTE_63CA;
extern vu16 BATTLE_GLOBAL_HALF_63D0;
extern vu8  BATTLE_GLOBAL_BYTE_C303;
extern vu8  BATTLE_GLOBAL_BYTE_EC324;
extern vu16 BATTLE_GLOBAL_HALF_EC30C;
extern vu16 BATTLE_GLOBAL_HALF_EC2EE;
extern vu8  BATTLE_UI_BYTE_8356;
extern vu8  BATTLE_UI_BYTE_8357;
extern vu16 BATTLE_UI_HALF_8358;
extern vu16 BATTLE_UI_HALF_835A;
extern vu8  BATTLE_UI_BYTE_835C;
extern vu8  BATTLE_UI_BYTE_835D;
extern vu8  BATTLE_UI_BYTE_835E;
extern vu8  BATTLE_UI_BYTE_837A;
extern vu8  BATTLE_UI_BYTE_837B;
extern vu16 BATTLE_UI_HALF_837C;
extern vu16 BATTLE_UI_HALF_837E;
extern vu8  BATTLE_UI_BYTE_839E;
extern vu8  BATTLE_UI_BYTE_839F;
extern vu8  BATTLE_UI_BYTE_83C2;
extern vu8  BATTLE_UI_BYTE_83C3;
extern vu16 BATTLE_UI_HALF_83C4;
extern vu16 BATTLE_UI_HALF_83C6;
extern vu8  BATTLE_UI_BYTE_8332;
extern vu8  BATTLE_UI_BYTE_8333;
#define BATTLE_UI_BYTE_8333_INDEX(index) \
  (*(volatile volatile u8*)(0x80148333u + ((u32)(index) * 0x24u)))
#define BATTLE_UI_BYTE_833A(index) \
  (*(volatile volatile u8*)(0x8014833au + ((u32)(index) * 0x24u)))
extern vu16 BATTLE_UI_HALF_8334;
extern vu16 BATTLE_UI_HALF_8336;
#define BATTLE_LOCAL_SCRATCH_PTR       VPPTR(Battle03LocalWork, 0x1f800044u)
#define BATTLE_ENEMY_SCRATCH_PTR       VPPTR(Battle03EnemyWork, 0x1f800044u)
#define BATTLE_CURRENT_ENEMY_PTR       VPPTR(Battle03EnemyWork, 0x801eb4e8u)
#define BATTLE_CURRENT_QUEUED_SLOT_PTR VPPTR(Battle03QueuedSlot, 0x801ec2e0u)
extern vu32 BATTLE_CURRENT_QUEUED_WORD_4B20;
#define BATTLE_CURRENT_QUEUED_PTR_4B20 VPPTR(u8, 0x801eb4e0u)
#define BATTLE_SLOT_STORE_FLAG(index) \
  (*(volatile volatile u8*)(0x801ec339u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_PTR(index) \
  (*(volatile volatile u32*)(0x801ec3a4u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_WORD_34(index) \
  (*(volatile volatile u32*)(0x801ec364u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_WORD_38(index) \
  (*(volatile volatile u32*)(0x801ec368u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_WORD_3C(index) \
  (*(volatile volatile u32*)(0x801ec36cu + ((u32)(index) * 0x78u)))
#define BATTLE_EVENT_SLOT_FLAG(index) \
  (*(volatile volatile u8*)(0x801eb4f0u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_A(index) \
  (*(volatile volatile u8*)(0x801eb4f1u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_B(index) \
  (*(volatile volatile u8*)(0x801eb4f2u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_C(index) \
  (*(volatile volatile u8*)(0x801eb4f3u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_KIND(index) \
  (*(volatile volatile u8*)(0x801eb4f5u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_MODE(index) \
  (*(volatile volatile u8*)(0x801eb4f6u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_WORD(index) \
  (*(volatile volatile u32*)(0x801eb4f4u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_HALF(index) \
  (*(volatile volatile u16*)(0x801eb4f8u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_BYTE(index) \
  (*(volatile volatile u8*)(0x801eb4fau + ((u32)(index) * 0x0cu)))
#define BATTLE_SLOT_STORE_BYTE_01(index) \
  (*(volatile volatile u8*)(0x801ec331u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_02(index) \
  (*(volatile volatile u8*)(0x801ec332u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_05(index) \
  (*(volatile volatile u8*)(0x801ec335u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_06(index) \
  (*(volatile volatile u8*)(0x801ec336u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_29(index) \
  (*(volatile volatile u8*)(0x801ec359u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5C(index) \
  (*(volatile volatile u8*)(0x801ec38cu + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5D(index) \
  (*(volatile volatile u8*)(0x801ec38du + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5E(index) \
  (*(volatile volatile u8*)(0x801ec38eu + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5F(index) \
  (*(volatile volatile u8*)(0x801ec38fu + ((u32)(index) * 0x78u)))
#define BATTLE_LOCAL_STATE_TABLE CVPTR(Battle03Handler, 0x801eb120u)
#define BATTLE_LOCAL_FLAGS_80(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x80u))
#define BATTLE_LOCAL_BYTE_79(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x79u))
#define BATTLE_LOCAL_BYTE_7A(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x7au))
#define BATTLE_LOCAL_BYTE_82(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x82u))
#define BATTLE_LOCAL_BYTE_85(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x85u))
#define BATTLE_LOCAL_BYTE_86(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x86u))
#define BATTLE_LOCAL_BYTE_87(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x87u))
#define BATTLE_LOCAL_BYTE_4B(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x4bu))
#define BATTLE_LOCAL_BYTE_09(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 9u))
#define BATTLE_LOCAL_BYTE_0A(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 10u))
#define BATTLE_LOCAL_BYTE_119(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x119u))
#define BATTLE_LOCAL_BYTE_118(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x118u))
#define BATTLE_LOCAL_HALF_11A(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x11au))
#define BATTLE_LOCAL_WORD_124(work) \
  (*(volatile volatile u32*)((volatile u8*)(work) + 0x124u))
#define BATTLE_LOCAL_WORD_128(work) \
  (*(volatile volatile u32*)((volatile u8*)(work) + 0x128u))
#define BATTLE_ABILITY_RECORD_TABLE ABILITY_OBJECTS
#define BATTLE_LOCAL_KIND_MASK(kind) \
  (BATTLE_ABILITY_RECORD_TABLE[(kind)].tail_10.selection_mask)
#define BATTLE_PANEL_SLOT_MASK(kind) \
  (*(volatile volatile u8*)(0x801d90ebu + ((u32)(kind) * 0x18u)))
#define BATTLE_LOCAL_HALF_88(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x88u))
#define BATTLE_LOCAL_HALF_8A(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x8au))
#define BATTLE_LOCAL_HALF_90(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x90u))
#define BATTLE_LOCAL_HALF_92(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x92u))
#define BATTLE_LOCAL_HALF_96(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x96u))
#define BATTLE_LOCAL_HALF_98(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x98u))
#define BATTLE_LOCAL_BYTE_8C(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x8cu))
#define BATTLE_LOCAL_BYTE_2A(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x2au))
#define BATTLE_LOCAL_BYTE_9E(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x9eu))
#define BATTLE_LOCAL_BYTE_A6(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0xa6u))
#define BATTLE_LOCAL_BYTE_A9(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0xa9u))
#define BATTLE_LOCAL_BYTE_21(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x21u))
#define BATTLE_LOCAL_HALF_1C(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x1cu))
#define BATTLE_LOCAL_HALF_1E(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x1eu))
#define BATTLE_LOCAL_BYTE_136(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x136u))
#define BATTLE_LOCAL_BYTE_137(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x137u))
#define BATTLE_LOCAL_BYTE_138(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x138u))
#define BATTLE_LOCAL_BYTE_139(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x139u))
#define BATTLE_LOCAL_WORD_134(work) \
  (*(volatile volatile u32*)((volatile u8*)(work) + 0x134u))
#define BATTLE_LOCAL_BYTE_134(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x134u))
#define BATTLE_LOCAL_BYTE_120(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x120u))
#define BATTLE_LOCAL_BYTE_121(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x121u))
#define BATTLE_LOCAL_BYTE_122(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x122u))
#define BATTLE_LOCAL_BYTE_13C(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x13cu))
#define BATTLE_LOCAL_HALF_2C(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x2cu))
extern vu8 BATTLE_GLOBAL_BYTE_63C9;
extern vu8 BATTLE_GLOBAL_BYTE_44F58;
extern s8  BATTLE_GLOBAL_BYTE_4952;
#define BATTLE_LOCAL_STATE_TABLE_015C CVPTR(Battle03Handler, 0x801eb15cu)
#define BATTLE_LOCAL_STATE_TABLE_0188 \
  (*(Battle03Handler const volatile*)0x801eb188u)
#define BATTLE_LOCAL_BYTE_TABLE_018C       CVPTR(u8, 0x801eb18cu)
#define BATTLE_LOCAL_BYTE_TABLE_0198       CVPTR(u8, 0x801eb198u)
#define BATTLE_LOCAL_SUBSTATE3_TABLE       CVPTR(Battle03Handler, 0x801eb1e0u)
#define BATTLE_LOCAL_STATE4_TABLE          CVPTR(Battle03Handler, 0x801eb210u)
#define BATTLE_LOCAL_ALT_STATE3_TABLE      CVPTR(Battle03Handler, 0x801eb218u)
#define BATTLE_LOCAL_STATE2_CLASS_TABLE    CVPTR(Battle03Handler, 0x801eb224u)
#define BATTLE_LOCAL_STATE2_EVENT_TABLE    CVPTR(Battle03Handler, 0x801eb26cu)
#define BATTLE_LOCAL_STATE2_FOLLOWUP_TABLE CVPTR(Battle03Handler, 0x801eb274u)
#define BATTLE_LOCAL_DEFAULT_CLASS_TABLE   CVPTR(Battle03Handler, 0x801eb27cu)
#define BATTLE_ENEMY_DISPATCH_TABLE_A      CVPTR(Battle03Handler, 0x801eb294u)
#define BATTLE_ENEMY_DISPATCH_TABLE_B      CVPTR(Battle03Handler, 0x801eb298u)
#define BATTLE_ENEMY_FLAGS_82(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x82u))
#define BATTLE_ENEMY_FLAGS_80(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x80u))
#define BATTLE_ENEMY_BYTE_7E(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x7eu))
#define BATTLE_ENEMY_BYTE_7D(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x7du))
#define BATTLE_ENEMY_BYTE_7C(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x7cu))
#define BATTLE_ENEMY_BYTE_7F(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x7fu))
#define BATTLE_ENEMY_BYTE_02(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 2u))
#define BATTLE_ENEMY_BYTE_03(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 3u))
#define BATTLE_ENEMY_BYTE_04(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 4u))
#define BATTLE_ENEMY_BYTE_05(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 5u))
#define BATTLE_ENEMY_HALF_AA(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0xaau))
#define BATTLE_ENEMY_HALF_94(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0x94u))
#define BATTLE_ENEMY_BYTE_88(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x88u))
#define BATTLE_ENEMY_HALF_A8(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0xa8u))
#define BATTLE_ENEMY_HALF_A0(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0xa0u))
#define BATTLE_ENEMY_HALF_F8(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0xf8u))
#define BATTLE_ENEMY_HALF_FA(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0xfau))
#define BATTLE_ENEMY_BYTE_F5(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0xf5u))
#define BATTLE_ENEMY_BYTE_FC(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0xfcu))
#define BATTLE_ENEMY_BYTE_FD(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0xfdu))
#define BATTLE_ENEMY_HALF_F6(work) \
  (*(volatile volatile u16*)((volatile u8*)(work) + 0xf6u))
#define BATTLE_ENEMY_WORD_100(work) \
  (*(volatile volatile u32*)((volatile u8*)(work) + 0x100u))
#define BATTLE_ENEMY_BYTE_100(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x100u))
#define BATTLE_ENEMY_WORD_104(work) \
  (*(volatile volatile u32*)((volatile u8*)(work) + 0x104u))
#define BATTLE_ENEMY_BYTE_112(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x112u))
#define BATTLE_ENEMY_BYTE_114(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x114u))
#define BATTLE_ENEMY_BYTE_115(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0x115u))
#define BATTLE_ENEMY_BYTE_E6(work) \
  (*(volatile volatile u8*)((volatile u8*)(work) + 0xe6u))
#define BATTLE_ENEMY_PTR_EC(work) VPPTR(u8, (volatile u8*)(work) + 0xecu)
#define BATTLE_WEIGHT_TABLE_0394  CVPTR(u8, 0x801eb394u)
#define BATTLE_WEIGHT_TABLE_039C  CVPTR(u8, 0x801eb39cu)
#define BATTLE_RANDOM_TABLE_AC58  CVPTR(u8, 0x801eac58u)
#define BATTLE_RANDOM_TABLE_AC78  CVPTR(u8, 0x801eac78u)
extern volatile u8 BATTLE_RANDOM_TABLE_AC58_DATA[];
extern volatile u8 BATTLE_RANDOM_TABLE_AC78_DATA[];
#define BATTLE_RETRY_TABLE_AFF4        CVPTR(u8, 0x801eaff4u)
#define BATTLE_COUNTER_TABLE_AFFC      CVPTR(u8, 0x801eaffcu)
#define BATTLE_PERCENT_TABLE_AF3C      CVPTR(u16, 0x801eaf3cu)
#define BATTLE_RANDOM_BONUS_TABLE_AF48 CVPTR(s8, 0x801eaf48u)
#define BATTLE_RANK_TABLE_AF88         CVPTR(u8, 0x801eaf88u)
#define BATTLE_VARIANCE_TABLE_AFA0     CVPTR(s32, 0x801eafa0u)
#define BATTLE_SCALE_TABLE_AFC0        CVPTR(s16, 0x801eafc0u)
#define BATTLE_DAMAGE_SCALE_TABLE_0C7C CVPTR(u8, 0x801d0c7cu)
#define BATTLE_EFFECT_TABLE_AFD0       CVPTR(u16, 0x801eafd0u)
#define BATTLE_EVENT_PICK_TABLE_0C98   CVPTR(u8, 0x801d0c98u)
#define BATTLE_EVENT_PICK_TABLE_0CB8   CVPTR(u8, 0x801d0cb8u)
#define BATTLE_EVENT_SCRIPT_TABLE_B09C CVPTR(u16, 0x801eb09cu)
#define BATTLE_COUNTER_PTR_TABLE_893C  ((volatile u32**)0x801c893cu)
#define BATTLE_COUNTER_BYTE_TABLE_8950 ((volatile u8**)0x801c8950u)
#define BATTLE_TRIGGER_TABLE_6178      ((volatile u32**)0x800b6178u)
#define BATTLE_VARIANCE_TABLE_AF94     CVPTR(u8, 0x801eaf94u)
#define BATTLE_TARGET_MODE_PACK(index) \
  (*(volatile volatile u8*)(0x800b51f8u + (u32)(index)))
#define BATTLE_ENEMY_SLOT_KIND(index) \
  (*(volatile volatile u8*)(0x801eb6acu + ((u32)(index) * 0x118u)))
#define BATTLE_KIND_BYTE_00(kind) \
  (*(volatile volatile u8*)(0x801ca718u + ((u32)(kind) * 0x14u)))
#define BATTLE_LOCAL_PRESENTATION_STATE1_TABLE \
  CVPTR(Battle03Handler, 0x801eb3b0u)
#define BATTLE_LOCAL_PRESENTATION_BYTE3_TABLE \
  CVPTR(Battle03Handler, 0x801eb430u)
#define BATTLE_QUEUED_RESULT_SUBSTATE_TABLE CVPTR(Battle03Handler, 0x801eb454u)
#define BATTLE_ACTIVE_SLOT_TABLE_0          CVPTR(Battle03Handler, 0x801d0cd0u)
#define BATTLE_QUEUED_SLOT_TABLE            CVPTR(Battle03Handler, 0x801d0cc0u)
#define BATTLE_PANEL_TASK_ROOT_TABLE        CVPTR(Battle03Handler, 0x801d0f80u)
#define BATTLE_PANEL_TASK_ARG_DISPATCH_TABLE \
  CVPTR(Battle03ForwardingHandler, 0x801d0fecu)
#define BATTLE_PANEL_TASK_PTR VPPTR(u8, 0x80148648u)
#define BATTLE_PANEL_TASK_HALF_04 \
  (*(volatile volatile u16*)(BATTLE_PANEL_TASK_PTR + 4))
#define BATTLE_PANEL_TASK_HALF_06 \
  (*(volatile volatile u16*)(BATTLE_PANEL_TASK_PTR + 6))
#define BATTLE_PANEL_TASK_BYTE_03 \
  (*(volatile volatile u8*)(BATTLE_PANEL_TASK_PTR + 3))
#define BATTLE_PANEL_TASK_BYTE_0F \
  (*(volatile volatile u8*)(BATTLE_PANEL_TASK_PTR + 0xf))
#define BATTLE_PANEL_TASK_BYTE_0A \
  (*(volatile volatile u8*)(BATTLE_PANEL_TASK_PTR + 0x0au))
#define BATTLE_PANEL_TASK_BYTE_0B \
  (*(volatile volatile u8*)(BATTLE_PANEL_TASK_PTR + 0x0bu))
#define BATTLE_PANEL_TASK_BYTE_0D \
  (*(volatile volatile u8*)(BATTLE_PANEL_TASK_PTR + 0x0du))
#define BATTLE_PANEL_TASK_HALF_10 \
  (*(volatile volatile u16*)(BATTLE_PANEL_TASK_PTR + 0x10))
#define BATTLE_PANEL_TASK_HALF_12 \
  (*(volatile volatile u16*)(BATTLE_PANEL_TASK_PTR + 0x12))
extern vu8 BATTLE_UI_RING_INDEX;
extern vu8 BATTLE_UI_RING_TARGET;
#define BATTLE_UI_RING_BYTE0(index) \
  (*(volatile volatile u8*)(0x801eb5b0u + ((u32)(index) * 8u)))
#define BATTLE_UI_RING_BYTE1(index) \
  (*(volatile volatile u8*)(0x801eb5b1u + ((u32)(index) * 8u)))
#define BATTLE_UI_RING_WORD2(index) \
  (*(volatile volatile u32*)(0x801eb5b4u + ((u32)(index) * 8u)))
#define BATTLE_UI_RING_BYTE(index) \
  (*(volatile volatile u8*)(0x801eb4fau + ((u32)(index) * 0x0cu)))
#define BATTLE_UI_RING_WORD(index) \
  (*(volatile volatile u32*)(0x801eb4f4u + ((u32)(index) * 0x0cu)))
#define BATTLE_UI_MODE_TABLE_AF27         CVPTR(u8, 0x801eaf27u)
#define BATTLE_QUAD_OFFSET_TABLE_AD30     CVPTR(s16, 0x801ead30u)
#define BATTLE_SPRITE_OFFSET_TABLE_AE50   CVPTR(s16, 0x801eae50u)
#define BATTLE_ICON_OFFSET_TABLE_AE94     CVPTR(u8, 0x801eae94u)
#define BATTLE_PANEL_FRAME_TABLE_AEE8     CVPTR(s16, 0x801eaee8u)
#define BATTLE_PANEL_ICON_TABLE_AEB0      CVPTR(u32, 0x801eaeb0u)
#define BATTLE_ICON_CLUT_TABLE_0C64       CVPTR(u8, 0x801d0c64u)
#define BATTLE_GLOBAL_PTR_BF08            VPPTR(u8, 0x801ebf08u)
#define BATTLE_LOCAL_ALT_WORK_ARRAY       VPTR(u8, 0x801ebf20u)
#define BATTLE_LOCAL_STATUS_ARRAY         VPTR(u8, 0x801ec048u)
#define BATTLE_PANEL_TASK_ICON_TABLE      CVPTR(Battle03Handler, 0x801d0ff8u)
#define BATTLE_RESULT_UI_AUX_HANDLER_0    ((Battle03Handler)0x801e8684u)
#define BATTLE_RESULT_UI_AUX_HANDLER_1    ((Battle03Handler)0x801e8d04u)
#define BATTLE_PREVIEW_SEQUENCE_TABLE     CVPTR(Battle03Handler, 0x801d0f44u)
#define BATTLE_SAVED_PREVIEW_RESULT_TABLE CVPTR(Battle03Handler, 0x801d0f6cu)

extern u8 func_8017e3d4(void);
extern u8 func_800a94a8(void);
extern u8 func_800a955c(void);

void func_8014d290(void);
void func_8014d5f0(u8 arg0, u32 arg1, s32 arg2);
void func_8014f800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
u16  func_8017a620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017a904(u32 arg0, s32 arg1);
void func_8017a9a4(u32 arg0);
void func_8017a9b8(u32 arg0);
u16  func_8017a6f0(s32 arg0, s32 arg1);
void func_8017aa6c(u32 arg0);
void func_8017aa80(u32 arg0);
void func_8017aa1c(void);
void func_8017e3f4(void* arg0, const void* arg1, ...);
void func_8017e364(void* arg0, const void* arg1);
void func_8017c2d8(u32 arg0, s32 arg1, s32 arg2, u16 arg3, s32 arg4);
void func_80158db8(u8 arg0, u8 arg1);
u8   func_8014d978(void);
u8   func_8014daec(void);
void func_8014e5a0(u8 arg0, u8 arg1);
u32  func_8014d8d4(u8 arg0);
u8   func_801db524(u8 arg0);
u8   func_800a9304(u8 arg0);
u8   func_800a94a8(void);
u8   func_800a955c(void);
void func_800a36f0(u32 arg0, u32 arg1);
u16  func_800a2ae0(u8 arg0);
u8   func_800a3df8(u32 arg0, u32 arg1);
void func_800a31e0(u32 arg0, u32 arg1);
void func_800ad074(u8 arg0);
void func_800aaa74(void);
void func_800a4458(void);
void func_800a9bd8(u8 arg0);
void func_8015df18(u16 arg0);
void func_801636a0(u32 arg0, u32 arg1);
void func_80164a44(u32 arg0);
void func_8019651c(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_80196718(void* arg0);
void func_801d7eb0(s32 arg0, s32 arg1);
u8   func_801d64c4(u32 arg0);
u8   func_801ddcb4(u32 arg0);
void func_801644d8(u32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, u32 arg5);
void func_801deeb4(void);
void func_801de190(u32 arg0);
void func_801de560(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u32 arg4);
void func_801de9a8(u32 arg0);
void func_801dea18(u32 arg0);
void func_801dcef8(u32 arg0);
u32  func_801502d0(u32 arg0);
void func_801501e4(void* arg0, u32 arg1, u32 arg2);
void func_80150098(s16 arg0, s16 arg1, u32 arg2, void* arg3);
u32  func_801e590c(u32 arg0, u32 arg1);
u8   func_801e2e30(void);
s16  func_8015477c(s32 arg0, s32 arg1);

void func_801dece0(void);
void func_801ded54(void);
u8   func_801dede4(void);
u8   func_801dee4c(void);
void func_801def0c(void);
void func_801defe4(void);
void func_801df34c(void);
void func_801df8ac(void);
void func_801df914(void);
void func_801e019c(void);
void func_801e046c(void);
void func_801e1298(void);
void func_801e1450(void);
void func_801e1670(void);
void func_801e1b64(void);
void func_801e1cd8(void);
void func_801e1e7c(void);
void func_801e2170(void);
void func_801e25e0(u8 arg0);
void func_801e2948(s8 arg0);
void func_801d9304(u8 arg0);
u8   func_801d3844(void);
void func_801d4850(void);
void func_801d9388(u8 arg0);
void func_801d93e4(u8 arg0);
void func_801d9428(u8 arg0);
void func_801d9484(void);
u8   func_801d54f8(void);
u8   func_801d5658(void);
u8   func_801d57ac(void);
u8   func_801d590c(void);
u8   func_801d5a60(void);
u8   func_801d5bc0(void);
u8   func_801d5dcc(void);
void func_801d4d44(void);
void func_801d750c(s32 arg0, s32 arg1);
void func_801d7a40(s16 arg0, s16 arg1);
void func_801d7d10(u8 arg0, s16 arg1, s16 arg2, u16 arg3, u8 arg4, u8 arg5);
void func_801d8270(s32 arg0, s32 arg1);
void func_801d8450(u32 arg0);
void func_801d8690(s32 arg0, s32 arg1, s32 arg2);
void func_801d8ae4(s32 arg0, s32 arg1, s32 arg2);
void func_801d8df8(s32 arg0, s32 arg1, u32 arg2);
u8   func_801db058(void);
void func_801d94d4(s16 arg0, u16 arg1, s32 arg2, s16 arg3);
void func_801d9684(s16 arg0, u16 arg1, s32 arg2, u16 arg3);
void func_801d9ab4(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801d9c80(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801d9dbc(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4);
void func_801d9e9c(s16 arg0, s16 arg1, u16 arg2, u16 arg3, s8 arg4);
void func_801da078(s16 arg0, s16 arg1, s32 arg2);
void func_801da408(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
u8   func_801da69c(u32 arg0);
u32  func_801db434(u8 arg0, u32 arg1);
u8   func_801db844(u32 arg0);
u8   func_801db9e4(u32 arg0);
u8   func_801db2f8(u32 arg0);
u8   func_801db3a0(u32 arg0, u32 arg1, u32 arg2);
u8   func_801db3e4(u32 arg0, u32 arg1, u32 arg2);
void func_801db494(void);
void func_801da7d4(void);
void func_801daae4(void);
u32  func_801dc73c(s16 arg0, u32 arg1, u32 arg2);
u32  func_801dccb0(void);
u32  func_801dcd50(u32 arg0, u8 arg1, s32 arg2);
u32  func_801dc044(u8 arg0, u8 arg1, u16 arg2);
u32  func_801dcad8(u8 arg0, u8 arg1, s8 arg2);
u32  func_801dc894(s16 arg0, u8 arg1, u32 arg2);
u32  func_801dbb78(u8 arg0, u8 arg1);
void func_801dd08c(void);
void func_801dd14c(u8 arg0);
void func_801dd29c(void);
void func_801dd350(s32 arg0);
void func_801dd3cc(s32 arg0);
u8   func_801dd448(void);
void func_801dd858(u32 arg0);
void func_801dd8ac(u32 arg0);
void func_801ddab4(u32 arg0);
void func_801ddaf0(void);
void func_801dd800(void);
u8   func_801dde7c(u32 arg0, u32 arg1);
void func_801ddf28(void);
u8   func_801ddf50(u16 arg0, u32 arg1);
void func_801ddfec(u32 arg0);
void func_801de1b0(u32 arg0);
void func_801de1d4(void);
void func_801de60c(u32 arg0, u8 arg1, u8 arg2, u8 arg3, u8 arg4, u32 arg5);
void func_801de804(void);
u8   func_801de858(s8 arg0);
void func_801de8c0(s8 arg0, s8 arg1, u32 arg2);
u8   func_801de92c(void);
void func_801dea64(s32 arg0);
void func_801ddb7c(void);
u8   func_801e0e0c(void);
void func_801e0b64(void);
void func_801e1b2c(void);
void func_801e1dd4(void);
u8   func_801e3160(void);
u8   func_801e4368(void);
void func_801e2314(u32 arg0);
void func_801e4f34(void);
void func_801e531c(void);
void func_801e52f0(s16 arg0);
void func_801e54ec(void);
void func_801e567c(void);
void func_801e5704(void);
void func_801e5988(void);
void func_801e5a38(void);
void func_801e62bc(u8 arg0);
u8   func_801e7b34(void);
void func_801e8dd8(void);
u8   func_801e8fa8(void);
void func_801deae0(void);
void func_801debc4(u32 arg0, u32 arg1);
void func_801d9804(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801d9900(void);
void func_801d99ac(s16 arg0, s16 arg1, s32 arg2);
void func_801da4b4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
void func_801da5a8(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
u8   func_801e29b4(u8 arg0);
s8   func_801e2a88(u8 arg0);
u8   func_801e2ca4(void);
u8   func_801e2d4c(s8 arg0);
u8   func_801e2d90(void);
u8   func_801e30f8(void);
u8   func_801e30b8(s8 arg0);
void func_801e31c8(void);
void func_801e4490(void);
void func_801e4928(void);
void func_801e5af4(void);
void func_801e5824(void);
void func_801e6c84(void);
void func_801e7818(void);
void func_801e9074(void);
void func_801ea174(void);
void func_801ea1a4(void);
void func_801ea1e0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, u8* selector);
void func_801e862c(void);
void func_801ea650(void);
void func_801ea7dc(void);
void func_801eaab8(void);
u8   func_801eab38(void);
s8   func_801eab6c(u8* arg0);

#endif
