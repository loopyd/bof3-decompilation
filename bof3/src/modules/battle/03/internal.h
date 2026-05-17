#ifndef BOF3_SRC_MODULES_BATTLE_03_INTERNAL_H
#define BOF3_SRC_MODULES_BATTLE_03_INTERNAL_H

#include "bof3/modules/battle/03.h"
#include "bof3/context.h"

typedef void (*Battle03Handler)(void);
typedef void (*Battle03EnemyModeHandler)(s32 arg0);
typedef void (*Battle03ForwardingHandler)(s32 arg0, s32 arg1, s32 arg2,
                                          s32 arg3, s32 arg4, s32 arg5,
                                          s32 arg6, u8* selector);

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

#define BATTLE_LOCAL_WORK_ARRAY ((volatile Battle03LocalWork*)0x80145e90u)
#define BATTLE_ENEMY_WORK_ARRAY ((volatile Battle03EnemyWork*)0x801eb630u)
#define BATTLE_QUEUED_SLOT_ARRAY \
  ((volatile Battle03QueuedSlot*)0x801ec330u)
#define BATTLE_LOCAL_WORK_PTR  (*(volatile Battle03LocalWork**)0x80146250u)
#define BATTLE_LOCAL_BYTE_62EC (*(volatile u8*)0x801462ecu)
#define BATTLE_GLOBAL_BYTE_62E0 (*(volatile u8*)0x801462e0u)
#define BATTLE_GLOBAL_BYTE_62E1 (*(volatile u8*)0x801462e1u)
#define BATTLE_GLOBAL_BYTE_62E2 (*(volatile u8*)0x801462e2u)
#define BATTLE_GLOBAL_HALF_62E8 (*(volatile u16*)0x801462e8u)
#define BATTLE_GLOBAL_BYTE_62EA (*(volatile u8*)0x801462eau)
#define BATTLE_GLOBAL_BYTE_62EE (*(volatile u8*)0x801462eeu)
#define BATTLE_GLOBAL_BYTE_62F0 (*(volatile u8*)0x801462f0u)
#define BATTLE_GLOBAL_BYTE_62F4 (*(volatile u8*)0x801462f4u)
#define BATTLE_GLOBAL_BYTE_62F3 (*(volatile u8*)0x801462f3u)
#define BATTLE_GLOBAL_BYTE_62FC(index) \
  (*(volatile u8*)(0x801462fcu + (u32)(index)))
#define BATTLE_GLOBAL_BYTE_62F6(index) \
  (*(volatile u8*)(0x801462f6u + (u32)(index)))
#define BATTLE_GLOBAL_BYTE_6301 (*(volatile u8*)0x80146301u)
#define BATTLE_GLOBAL_BYTE_6302 (*(volatile u8*)0x80146302u)
#define BATTLE_GLOBAL_BYTE_6303 (*(volatile u8*)0x80146303u)
#define BATTLE_GLOBAL_BYTE_630C(index) \
  (*(volatile u8*)(0x8014630cu + (u32)(index)))
#define BATTLE_GLOBAL_WORD_59F0 (*(volatile u32*)0x801459f0u)
#define BATTLE_GLOBAL_WORD_598C (*(volatile u32*)0x8014598cu)
#define BATTLE_UI_CHAR_BUFFER   ((volatile u8*)0x80145ad4u)
#define BATTLE_SCRATCH_HALF_000 (*(volatile u16*)0x1f800000u)
#define BATTLE_SCRATCH_BYTE_000 (*(volatile u8*)0x1f800000u)
#define BATTLE_SCRATCH_BYTE_001 (*(volatile u8*)0x1f800001u)
#define BATTLE_SCRATCH_BYTE_002 (*(volatile u8*)0x1f800002u)
#define BATTLE_LOCAL_FLAG_63CE  (*(volatile u8*)0x801463ceu)
#define BATTLE_GLOBAL_BYTE_6325 (*(volatile u8*)0x80146325u)
#define BATTLE_GLOBAL_BYTE_6327 (*(volatile u8*)0x80146327u)
#define BATTLE_GLOBAL_BYTE_6328 (*(volatile u8*)0x80146328u)
#define BATTLE_GLOBAL_WORD_632C (*(volatile u32*)0x8014632cu)
#define BATTLE_GLOBAL_WORD_6330 (*(volatile u32*)0x80146330u)
#define BATTLE_GLOBAL_HALF_6334(index) \
  (*(volatile u16*)(0x80146334u + ((u32)(index) * 2u)))
#define BATTLE_GLOBAL_BYTE_6354(index) \
  (*(volatile u8*)(0x80146354u + (u32)(index)))
#define BATTLE_GLOBAL_BYTE_6374  (*(volatile u8*)0x80146374u)
#define BATTLE_GLOBAL_BYTE_6375  (*(volatile u8*)0x80146375u)
#define BATTLE_GLOBAL_PTR_6380   (*(volatile u8**)0x80146380u)
#define BATTLE_GLOBAL_BYTE_6384  (*(volatile u8*)0x80146384u)
#define BATTLE_GLOBAL_BYTE_6324  (*(volatile u8*)0x80146324u)
#define BATTLE_GLOBAL_BYTE_6304  (*(volatile u8*)0x80146304u)
#define BATTLE_GLOBAL_BYTE_6308  (*(volatile u8*)0x80146308u)
#define BATTLE_GLOBAL_HALF_63B8  (*(volatile u16*)0x801463b8u)
#define BATTLE_GLOBAL_BYTE_63BA  (*(volatile u8*)0x801463bau)
#define BATTLE_GLOBAL_HALF_63C0  (*(volatile u16*)0x801463c0u)
#define BATTLE_GLOBAL_HALF_63C2  (*(volatile u16*)0x801463c2u)
#define BATTLE_GLOBAL_BYTE_6322  (*(volatile u8*)0x80146322u)
#define BATTLE_GLOBAL_BYTE_6323  (*(volatile u8*)0x80146323u)
#define BATTLE_GLOBAL_BYTE_63CE  (*(volatile u8*)0x801463ceu)
#define BATTLE_GLOBAL_HALF_63DA  (*(volatile u16*)0x801463dau)
#define BATTLE_GLOBAL_BYTE_63CA  (*(volatile u8*)0x801463cau)
#define BATTLE_GLOBAL_HALF_63D0  (*(volatile u16*)0x801463d0u)
#define BATTLE_GLOBAL_BYTE_C303  (*(volatile u8*)0x801ec303u)
#define BATTLE_GLOBAL_BYTE_EC324 (*(volatile u8*)0x801ec324u)
#define BATTLE_GLOBAL_HALF_EC30C (*(volatile u16*)0x801ec30cu)
#define BATTLE_GLOBAL_HALF_EC2EE (*(volatile u16*)0x801ec2eeu)
#define BATTLE_UI_BYTE_8356      (*(volatile u8*)0x80148356u)
#define BATTLE_UI_BYTE_8357      (*(volatile u8*)0x80148357u)
#define BATTLE_UI_HALF_8358      (*(volatile u16*)0x80148358u)
#define BATTLE_UI_HALF_835A      (*(volatile u16*)0x8014835au)
#define BATTLE_UI_BYTE_835C      (*(volatile u8*)0x8014835cu)
#define BATTLE_UI_BYTE_835D      (*(volatile u8*)0x8014835du)
#define BATTLE_UI_BYTE_835E      (*(volatile u8*)0x8014835eu)
#define BATTLE_UI_BYTE_837A      (*(volatile u8*)0x8014837au)
#define BATTLE_UI_BYTE_837B      (*(volatile u8*)0x8014837bu)
#define BATTLE_UI_HALF_837C      (*(volatile u16*)0x8014837cu)
#define BATTLE_UI_HALF_837E      (*(volatile u16*)0x8014837eu)
#define BATTLE_UI_BYTE_839E      (*(volatile u8*)0x8014839eu)
#define BATTLE_UI_BYTE_839F      (*(volatile u8*)0x8014839fu)
#define BATTLE_UI_BYTE_83C2      (*(volatile u8*)0x801483c2u)
#define BATTLE_UI_BYTE_83C3      (*(volatile u8*)0x801483c3u)
#define BATTLE_UI_HALF_83C4      (*(volatile u16*)0x801483c4u)
#define BATTLE_UI_HALF_83C6      (*(volatile u16*)0x801483c6u)
#define BATTLE_UI_BYTE_8332      (*(volatile u8*)0x80148332u)
#define BATTLE_UI_BYTE_8333      (*(volatile u8*)0x80148333u)
#define BATTLE_UI_BYTE_8333_INDEX(index) \
  (*(volatile u8*)(0x80148333u + ((u32)(index) * 0x24u)))
#define BATTLE_UI_BYTE_833A(index) \
  (*(volatile u8*)(0x8014833au + ((u32)(index) * 0x24u)))
#define BATTLE_UI_HALF_8334 (*(volatile u16*)0x80148334u)
#define BATTLE_UI_HALF_8336 (*(volatile u16*)0x80148336u)
#define BATTLE_LOCAL_SCRATCH_PTR \
  (*(volatile Battle03LocalWork**)0x1f800044u)
#define BATTLE_ENEMY_SCRATCH_PTR \
  (*(volatile Battle03EnemyWork**)0x1f800044u)
#define BATTLE_CURRENT_ENEMY_PTR \
  (*(volatile Battle03EnemyWork**)0x801eb4e8u)
#define BATTLE_CURRENT_QUEUED_SLOT_PTR \
  (*(volatile Battle03QueuedSlot**)0x801ec2e0u)
#define BATTLE_CURRENT_QUEUED_WORD_4B20 (*(volatile u32*)0x801eb4e0u)
#define BATTLE_CURRENT_QUEUED_PTR_4B20  (*(volatile u8**)0x801eb4e0u)
#define BATTLE_SLOT_STORE_FLAG(index) \
  (*(volatile u8*)(0x801ec339u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_PTR(index) \
  (*(volatile u32*)(0x801ec3a4u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_WORD_34(index) \
  (*(volatile u32*)(0x801ec364u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_WORD_38(index) \
  (*(volatile u32*)(0x801ec368u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_WORD_3C(index) \
  (*(volatile u32*)(0x801ec36cu + ((u32)(index) * 0x78u)))
#define BATTLE_EVENT_SLOT_FLAG(index) \
  (*(volatile u8*)(0x801eb4f0u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_A(index) \
  (*(volatile u8*)(0x801eb4f1u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_B(index) \
  (*(volatile u8*)(0x801eb4f2u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_C(index) \
  (*(volatile u8*)(0x801eb4f3u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_KIND(index) \
  (*(volatile u8*)(0x801eb4f5u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_MODE(index) \
  (*(volatile u8*)(0x801eb4f6u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_WORD(index) \
  (*(volatile u32*)(0x801eb4f4u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_HALF(index) \
  (*(volatile u16*)(0x801eb4f8u + ((u32)(index) * 0x0cu)))
#define BATTLE_EVENT_SLOT_BYTE(index) \
  (*(volatile u8*)(0x801eb4fau + ((u32)(index) * 0x0cu)))
#define BATTLE_SLOT_STORE_BYTE_01(index) \
  (*(volatile u8*)(0x801ec331u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_02(index) \
  (*(volatile u8*)(0x801ec332u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_05(index) \
  (*(volatile u8*)(0x801ec335u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_06(index) \
  (*(volatile u8*)(0x801ec336u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_29(index) \
  (*(volatile u8*)(0x801ec359u + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5C(index) \
  (*(volatile u8*)(0x801ec38cu + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5D(index) \
  (*(volatile u8*)(0x801ec38du + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5E(index) \
  (*(volatile u8*)(0x801ec38eu + ((u32)(index) * 0x78u)))
#define BATTLE_SLOT_STORE_BYTE_5F(index) \
  (*(volatile u8*)(0x801ec38fu + ((u32)(index) * 0x78u)))
#define BATTLE_LOCAL_STATE_TABLE \
  ((Battle03Handler const volatile*)0x801eb120u)
#define BATTLE_LOCAL_FLAGS_80(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x80u))
#define BATTLE_LOCAL_BYTE_79(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x79u))
#define BATTLE_LOCAL_BYTE_7A(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x7au))
#define BATTLE_LOCAL_BYTE_82(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x82u))
#define BATTLE_LOCAL_BYTE_85(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x85u))
#define BATTLE_LOCAL_BYTE_86(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x86u))
#define BATTLE_LOCAL_BYTE_87(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x87u))
#define BATTLE_LOCAL_BYTE_4B(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x4bu))
#define BATTLE_LOCAL_BYTE_09(work) \
  (*(volatile u8*)((volatile u8*)(work) + 9u))
#define BATTLE_LOCAL_BYTE_0A(work) \
  (*(volatile u8*)((volatile u8*)(work) + 10u))
#define BATTLE_LOCAL_BYTE_119(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x119u))
#define BATTLE_LOCAL_BYTE_118(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x118u))
#define BATTLE_LOCAL_HALF_11A(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x11au))
#define BATTLE_LOCAL_WORD_124(work) \
  (*(volatile u32*)((volatile u8*)(work) + 0x124u))
#define BATTLE_LOCAL_WORD_128(work) \
  (*(volatile u32*)((volatile u8*)(work) + 0x128u))
#define BATTLE_LOCAL_KIND_MASK(kind) \
  (*(volatile u16*)(0x801ca71cu + ((u32)(kind) * 0x14u)))
#define BATTLE_PANEL_SLOT_MASK(kind) \
  (*(volatile u8*)(0x801d90ebu + ((u32)(kind) * 0x18u)))
#define BATTLE_LOCAL_HALF_88(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x88u))
#define BATTLE_LOCAL_HALF_8A(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x8au))
#define BATTLE_LOCAL_HALF_90(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x90u))
#define BATTLE_LOCAL_HALF_92(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x92u))
#define BATTLE_LOCAL_HALF_96(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x96u))
#define BATTLE_LOCAL_HALF_98(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x98u))
#define BATTLE_LOCAL_BYTE_8C(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x8cu))
#define BATTLE_LOCAL_BYTE_2A(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x2au))
#define BATTLE_LOCAL_BYTE_9E(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x9eu))
#define BATTLE_LOCAL_BYTE_A6(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0xa6u))
#define BATTLE_LOCAL_BYTE_A9(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0xa9u))
#define BATTLE_LOCAL_BYTE_21(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x21u))
#define BATTLE_LOCAL_HALF_1C(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x1cu))
#define BATTLE_LOCAL_HALF_1E(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x1eu))
#define BATTLE_LOCAL_BYTE_136(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x136u))
#define BATTLE_LOCAL_BYTE_137(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x137u))
#define BATTLE_LOCAL_BYTE_138(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x138u))
#define BATTLE_LOCAL_BYTE_139(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x139u))
#define BATTLE_LOCAL_WORD_134(work) \
  (*(volatile u32*)((volatile u8*)(work) + 0x134u))
#define BATTLE_LOCAL_BYTE_134(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x134u))
#define BATTLE_LOCAL_BYTE_120(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x120u))
#define BATTLE_LOCAL_BYTE_121(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x121u))
#define BATTLE_LOCAL_BYTE_122(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x122u))
#define BATTLE_LOCAL_BYTE_13C(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x13cu))
#define BATTLE_LOCAL_HALF_2C(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x2cu))
#define BATTLE_GLOBAL_BYTE_63C9  (*(volatile u8*)0x801463c9u)
#define BATTLE_GLOBAL_BYTE_44F58 (*(volatile u8*)0x80144f58u)
#define BATTLE_GLOBAL_BYTE_4952  (*(volatile s8*)0x80144952u)
#define BATTLE_LOCAL_STATE_TABLE_015C \
  ((Battle03Handler const volatile*)0x801eb15cu)
#define BATTLE_LOCAL_STATE_TABLE_0188 \
  (*(Battle03Handler const volatile*)0x801eb188u)
#define BATTLE_LOCAL_BYTE_TABLE_018C ((const volatile u8*)0x801eb18cu)
#define BATTLE_LOCAL_BYTE_TABLE_0198 ((const volatile u8*)0x801eb198u)
#define BATTLE_LOCAL_SUBSTATE3_TABLE \
  ((Battle03Handler const volatile*)0x801eb1e0u)
#define BATTLE_LOCAL_STATE4_TABLE \
  ((Battle03Handler const volatile*)0x801eb210u)
#define BATTLE_LOCAL_ALT_STATE3_TABLE \
  ((Battle03Handler const volatile*)0x801eb218u)
#define BATTLE_LOCAL_STATE2_CLASS_TABLE \
  ((Battle03Handler const volatile*)0x801eb224u)
#define BATTLE_LOCAL_STATE2_EVENT_TABLE \
  ((Battle03Handler const volatile*)0x801eb26cu)
#define BATTLE_LOCAL_STATE2_FOLLOWUP_TABLE \
  ((Battle03Handler const volatile*)0x801eb274u)
#define BATTLE_LOCAL_DEFAULT_CLASS_TABLE \
  ((Battle03Handler const volatile*)0x801eb27cu)
#define BATTLE_ENEMY_DISPATCH_TABLE_A \
  ((Battle03Handler const volatile*)0x801eb294u)
#define BATTLE_ENEMY_DISPATCH_TABLE_B \
  ((Battle03Handler const volatile*)0x801eb298u)
#define BATTLE_ENEMY_FLAGS_82(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x82u))
#define BATTLE_ENEMY_FLAGS_80(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x80u))
#define BATTLE_ENEMY_BYTE_7E(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x7eu))
#define BATTLE_ENEMY_BYTE_7D(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x7du))
#define BATTLE_ENEMY_BYTE_7C(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x7cu))
#define BATTLE_ENEMY_BYTE_7F(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x7fu))
#define BATTLE_ENEMY_BYTE_02(work) \
  (*(volatile u8*)((volatile u8*)(work) + 2u))
#define BATTLE_ENEMY_BYTE_03(work) \
  (*(volatile u8*)((volatile u8*)(work) + 3u))
#define BATTLE_ENEMY_BYTE_04(work) \
  (*(volatile u8*)((volatile u8*)(work) + 4u))
#define BATTLE_ENEMY_BYTE_05(work) \
  (*(volatile u8*)((volatile u8*)(work) + 5u))
#define BATTLE_ENEMY_HALF_AA(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0xaau))
#define BATTLE_ENEMY_HALF_94(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0x94u))
#define BATTLE_ENEMY_BYTE_88(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x88u))
#define BATTLE_ENEMY_HALF_A8(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0xa8u))
#define BATTLE_ENEMY_HALF_A0(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0xa0u))
#define BATTLE_ENEMY_HALF_F8(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0xf8u))
#define BATTLE_ENEMY_HALF_FA(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0xfau))
#define BATTLE_ENEMY_BYTE_F5(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0xf5u))
#define BATTLE_ENEMY_BYTE_FC(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0xfcu))
#define BATTLE_ENEMY_BYTE_FD(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0xfdu))
#define BATTLE_ENEMY_HALF_F6(work) \
  (*(volatile u16*)((volatile u8*)(work) + 0xf6u))
#define BATTLE_ENEMY_WORD_100(work) \
  (*(volatile u32*)((volatile u8*)(work) + 0x100u))
#define BATTLE_ENEMY_BYTE_100(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x100u))
#define BATTLE_ENEMY_WORD_104(work) \
  (*(volatile u32*)((volatile u8*)(work) + 0x104u))
#define BATTLE_ENEMY_BYTE_112(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x112u))
#define BATTLE_ENEMY_BYTE_114(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x114u))
#define BATTLE_ENEMY_BYTE_115(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0x115u))
#define BATTLE_ENEMY_BYTE_E6(work) \
  (*(volatile u8*)((volatile u8*)(work) + 0xe6u))
#define BATTLE_ENEMY_PTR_EC(work) \
  (*(volatile u8**)((volatile u8*)(work) + 0xecu))
#define BATTLE_WEIGHT_TABLE_0394       ((const volatile u8*)0x801eb394u)
#define BATTLE_WEIGHT_TABLE_039C       ((const volatile u8*)0x801eb39cu)
#define BATTLE_RANDOM_TABLE_AC58       ((const volatile u8*)0x801eac58u)
#define BATTLE_RANDOM_TABLE_AC78       ((const volatile u8*)0x801eac78u)
#define BATTLE_RETRY_TABLE_AFF4        ((const volatile u8*)0x801eaff4u)
#define BATTLE_COUNTER_TABLE_AFFC      ((const volatile u8*)0x801eaffcu)
#define BATTLE_PERCENT_TABLE_AF3C      ((const volatile u16*)0x801eaf3cu)
#define BATTLE_RANDOM_BONUS_TABLE_AF48 ((const volatile s8*)0x801eaf48u)
#define BATTLE_RANK_TABLE_AF88         ((const volatile u8*)0x801eaf88u)
#define BATTLE_VARIANCE_TABLE_AFA0     ((const volatile s32*)0x801eafa0u)
#define BATTLE_SCALE_TABLE_AFC0        ((const volatile s16*)0x801eafc0u)
#define BATTLE_DAMAGE_SCALE_TABLE_0C7C ((const volatile u8*)0x801d0c7cu)
#define BATTLE_EFFECT_TABLE_AFD0       ((const volatile u16*)0x801eafd0u)
#define BATTLE_EVENT_PICK_TABLE_0C98   ((const volatile u8*)0x801d0c98u)
#define BATTLE_EVENT_PICK_TABLE_0CB8   ((const volatile u8*)0x801d0cb8u)
#define BATTLE_EVENT_SCRIPT_TABLE_B09C ((const volatile u16*)0x801eb09cu)
#define BATTLE_COUNTER_PTR_TABLE_893C  ((volatile u32**)0x801c893cu)
#define BATTLE_COUNTER_BYTE_TABLE_8950 ((volatile u8**)0x801c8950u)
#define BATTLE_TRIGGER_TABLE_6178      ((volatile u32**)0x800b6178u)
#define BATTLE_VARIANCE_TABLE_AF94     ((const volatile u8*)0x801eaf94u)
#define BATTLE_TARGET_MODE_PACK(index) \
  (*(volatile u8*)(0x800b51f8u + (u32)(index)))
#define BATTLE_ENEMY_SLOT_KIND(index) \
  (*(volatile u8*)(0x801eb6acu + ((u32)(index) * 0x118u)))
#define BATTLE_KIND_BYTE_00(kind) \
  (*(volatile u8*)(0x801ca718u + ((u32)(kind) * 0x14u)))
#define BATTLE_LOCAL_PRESENTATION_STATE1_TABLE \
  ((Battle03Handler const volatile*)0x801eb3b0u)
#define BATTLE_LOCAL_PRESENTATION_BYTE3_TABLE \
  ((Battle03Handler const volatile*)0x801eb430u)
#define BATTLE_QUEUED_RESULT_SUBSTATE_TABLE \
  ((Battle03Handler const volatile*)0x801eb454u)
#define BATTLE_ACTIVE_SLOT_TABLE_0 \
  ((Battle03Handler const volatile*)0x801d0cd0u)
#define BATTLE_QUEUED_SLOT_TABLE \
  ((Battle03Handler const volatile*)0x801d0cc0u)
#define BATTLE_PANEL_TASK_ROOT_TABLE \
  ((Battle03Handler const volatile*)0x801d0f80u)
#define BATTLE_PANEL_TASK_ARG_DISPATCH_TABLE \
  ((Battle03ForwardingHandler const volatile*)0x801d0fecu)
#define BATTLE_PANEL_TASK_PTR (*(volatile u8**)0x80148648u)
#define BATTLE_PANEL_TASK_HALF_04 \
  (*(volatile u16*)(BATTLE_PANEL_TASK_PTR + 4))
#define BATTLE_PANEL_TASK_HALF_06 \
  (*(volatile u16*)(BATTLE_PANEL_TASK_PTR + 6))
#define BATTLE_PANEL_TASK_BYTE_03 \
  (*(volatile u8*)(BATTLE_PANEL_TASK_PTR + 3))
#define BATTLE_PANEL_TASK_BYTE_0F \
  (*(volatile u8*)(BATTLE_PANEL_TASK_PTR + 0xf))
#define BATTLE_PANEL_TASK_BYTE_0A \
  (*(volatile u8*)(BATTLE_PANEL_TASK_PTR + 0x0au))
#define BATTLE_PANEL_TASK_BYTE_0B \
  (*(volatile u8*)(BATTLE_PANEL_TASK_PTR + 0x0bu))
#define BATTLE_PANEL_TASK_BYTE_0D \
  (*(volatile u8*)(BATTLE_PANEL_TASK_PTR + 0x0du))
#define BATTLE_PANEL_TASK_HALF_10 \
  (*(volatile u16*)(BATTLE_PANEL_TASK_PTR + 0x10))
#define BATTLE_PANEL_TASK_HALF_12 \
  (*(volatile u16*)(BATTLE_PANEL_TASK_PTR + 0x12))
#define BATTLE_UI_RING_INDEX  (*(volatile u8*)0x801ebf04u)
#define BATTLE_UI_RING_TARGET (*(volatile u8*)0x801ec328u)
#define BATTLE_UI_RING_BYTE0(index) \
  (*(volatile u8*)(0x801eb5b0u + ((u32)(index) * 8u)))
#define BATTLE_UI_RING_BYTE1(index) \
  (*(volatile u8*)(0x801eb5b1u + ((u32)(index) * 8u)))
#define BATTLE_UI_RING_WORD2(index) \
  (*(volatile u32*)(0x801eb5b4u + ((u32)(index) * 8u)))
#define BATTLE_UI_RING_BYTE(index) \
  (*(volatile u8*)(0x801eb4fau + ((u32)(index) * 0x0cu)))
#define BATTLE_UI_RING_WORD(index) \
  (*(volatile u32*)(0x801eb4f4u + ((u32)(index) * 0x0cu)))
#define BATTLE_UI_MODE_TABLE_AF27       ((const volatile u8*)0x801eaf27u)
#define BATTLE_QUAD_OFFSET_TABLE_AD30   ((const volatile s16*)0x801ead30u)
#define BATTLE_SPRITE_OFFSET_TABLE_AE50 ((const volatile s16*)0x801eae50u)
#define BATTLE_ICON_OFFSET_TABLE_AE94   ((const volatile u8*)0x801eae94u)
#define BATTLE_PANEL_FRAME_TABLE_AEE8   ((const volatile s16*)0x801eaee8u)
#define BATTLE_PANEL_ICON_TABLE_AEB0    ((const volatile u32*)0x801eaeb0u)
#define BATTLE_ICON_CLUT_TABLE_0C64     ((const volatile u8*)0x801d0c64u)
#define BATTLE_GLOBAL_PTR_BF08          (*(volatile u8**)0x801ebf08u)
#define BATTLE_LOCAL_ALT_WORK_ARRAY     ((volatile u8*)0x801ebf20u)
#define BATTLE_LOCAL_STATUS_ARRAY       ((volatile u8*)0x801ec048u)
#define BATTLE_PANEL_TASK_ICON_TABLE \
  ((Battle03Handler const volatile*)0x801d0ff8u)
#define BATTLE_RESULT_UI_AUX_HANDLER_0 ((Battle03Handler)0x801e8684u)
#define BATTLE_RESULT_UI_AUX_HANDLER_1 ((Battle03Handler)0x801e8d04u)
#define BATTLE_PREVIEW_SEQUENCE_TABLE \
  ((Battle03Handler const volatile*)0x801d0f44u)
#define BATTLE_SAVED_PREVIEW_RESULT_TABLE \
  ((Battle03Handler const volatile*)0x801d0f6cu)

void func_8014d290(void);
void func_8014d5f0(u8 arg0, u32 arg1, s32 arg2);
void func_8014f800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
u16  func_8017a620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
u8   func_8017e3d4(void);
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

#endif
