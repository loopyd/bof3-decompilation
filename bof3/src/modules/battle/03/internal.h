#ifndef BOF3_SRC_MODULES_BATTLE_03_INTERNAL_H
#define BOF3_SRC_MODULES_BATTLE_03_INTERNAL_H

#include "bof3/bof3.h"


typedef void (*Battle03Handler)(void);
typedef void (*Battle03EnemyModeHandler)(s32 arg0);
typedef void (*Battle03ForwardingHandler)(s32 arg0, s32 arg1, s32 arg2,
 s32 arg3, s32 arg4, s32 arg5,
 s32 arg6, u8* selector);

typedef struct Battle03LocalWork {
 u8 flags_00;
 u8 unk_01;
 u8 unk_02;
 u8 unk_03;
 u8 unk_04;
 u8 pad_05[3];
 u8 unk_08;
 u8 pad_09[3];
 u32 unk_0c;
 u32 unk_10;
 u32 unk_14;
 u32 unk_18;
 u32 unk_1c;
 u32 unk_20;
 u8 pad_24[5];
 u8 unk_29;
 u8 pad_2a;
 u8 unk_2b;
 u8 pad_2c[8];
 s32 unk_34;
 s32 unk_38;
 u8 pad_3c[2];
 s16 unk_3e;
 u8 pad_40[8];
 u8 unk_48;
 u8 pad_49[0xf7];
} Battle03LocalWork;

typedef struct Battle03EnemyWork {
 u8 unk_00;
 u8 unk_01;
 u8 pad_02[0xe2];
 Battle03EnemyModeHandler unk_e4;
 u8 pad_e8[8];
 u8 unk_f0;
 u8 pad_f1[0x27];
} Battle03EnemyWork;

typedef struct Battle03QueuedSlot {
 u8 unk_00;
 u8 pad_01[4];
 u8 unk_05;
 u8 unk_06;
 u8 pad_07[0x6d];
 u32 unk_74;
} Battle03QueuedSlot;

#define BATTLE_LOCAL_WORK_ARRAY VPTR(Battle03LocalWork, 0x80145e90u)
#define BATTLE_ENEMY_WORK_ARRAY VPTR(Battle03EnemyWork, 0x801eb630u)
#define BATTLE_QUEUED_SLOT_ARRAY \
 VPTR(Battle03QueuedSlot, 0x801ec330u)
#define BATTLE_LOCAL_WORK_PTR VPPTR(Battle03LocalWork, 0x80146250u)
#define BATTLE_LOCAL_BYTE_62EC VU8(0x801462ecu)
#define BATTLE_GLOBAL_BYTE_62E0 VU8(0x801462e0u)
#define BATTLE_GLOBAL_BYTE_62E1 VU8(0x801462e1u)
#define BATTLE_GLOBAL_BYTE_62E2 VU8(0x801462e2u)
#define BATTLE_GLOBAL_HALF_62E8 VU16(0x801462e8u)
#define BATTLE_GLOBAL_BYTE_62EA VU8(0x801462eau)
#define BATTLE_GLOBAL_BYTE_62EE VU8(0x801462eeu)
#define BATTLE_GLOBAL_BYTE_62F0 VU8(0x801462f0u)
#define BATTLE_GLOBAL_BYTE_62F4 VU8(0x801462f4u)
#define BATTLE_GLOBAL_BYTE_62F3 VU8(0x801462f3u)
#define BATTLE_GLOBAL_BYTE_62FC(index) \
 VU8(0x801462fcu + (u32)(index))
#define BATTLE_GLOBAL_BYTE_62F6(index) \
 VU8(0x801462f6u + (u32)(index))
#define BATTLE_GLOBAL_BYTE_6301 VU8(0x80146301u)
#define BATTLE_GLOBAL_BYTE_6302 VU8(0x80146302u)
#define BATTLE_GLOBAL_BYTE_6303 VU8(0x80146303u)
#define BATTLE_GLOBAL_BYTE_630C(index) \
 VU8(0x8014630cu + (u32)(index))
#define BATTLE_GLOBAL_WORD_59F0 VU32(0x801459f0u)
#define BATTLE_GLOBAL_WORD_598C VU32(0x8014598cu)
#define BATTLE_UI_CHAR_BUFFER VPTR(u8, 0x80145ad4u)
#define BATTLE_SCRATCH_HALF_000 VU16(0x1f800000u)
#define BATTLE_SCRATCH_BYTE_000 VU8(0x1f800000u)
#define BATTLE_SCRATCH_BYTE_001 VU8(0x1f800001u)
#define BATTLE_SCRATCH_BYTE_002 VU8(0x1f800002u)
#define BATTLE_LOCAL_FLAG_63CE VU8(0x801463ceu)
#define BATTLE_GLOBAL_BYTE_6325 VU8(0x80146325u)
#define BATTLE_GLOBAL_BYTE_6327 VU8(0x80146327u)
#define BATTLE_GLOBAL_BYTE_6328 VU8(0x80146328u)
#define BATTLE_GLOBAL_WORD_632C VU32(0x8014632cu)
#define BATTLE_GLOBAL_WORD_6330 VU32(0x80146330u)
#define BATTLE_GLOBAL_HALF_6334(index) \
 VU16(0x80146334u + ((u32)(index) * 2u))
#define BATTLE_GLOBAL_BYTE_6354(index) \
 VU8(0x80146354u + (u32)(index))
#define BATTLE_GLOBAL_BYTE_6374 VU8(0x80146374u)
#define BATTLE_GLOBAL_BYTE_6375 VU8(0x80146375u)
#define BATTLE_GLOBAL_PTR_6380 VPPTR(u8, 0x80146380u)
#define BATTLE_GLOBAL_BYTE_6384 VU8(0x80146384u)
#define BATTLE_GLOBAL_BYTE_6324 VU8(0x80146324u)
#define BATTLE_GLOBAL_BYTE_6304 VU8(0x80146304u)
#define BATTLE_GLOBAL_BYTE_6308 VU8(0x80146308u)
#define BATTLE_GLOBAL_HALF_63B8 VU16(0x801463b8u)
#define BATTLE_GLOBAL_BYTE_63BA VU8(0x801463bau)
#define BATTLE_GLOBAL_HALF_63C0 VU16(0x801463c0u)
#define BATTLE_GLOBAL_HALF_63C2 VU16(0x801463c2u)
#define BATTLE_GLOBAL_BYTE_6322 VU8(0x80146322u)
#define BATTLE_GLOBAL_BYTE_6323 VU8(0x80146323u)
#define BATTLE_GLOBAL_BYTE_63CE VU8(0x801463ceu)
#define BATTLE_GLOBAL_HALF_63DA VU16(0x801463dau)
#define BATTLE_GLOBAL_BYTE_63CA VU8(0x801463cau)
#define BATTLE_GLOBAL_HALF_63D0 VU16(0x801463d0u)
#define BATTLE_GLOBAL_BYTE_C303 VU8(0x801ec303u)
#define BATTLE_GLOBAL_BYTE_EC324 VU8(0x801ec324u)
#define BATTLE_GLOBAL_HALF_EC30C VU16(0x801ec30cu)
#define BATTLE_GLOBAL_HALF_EC2EE VU16(0x801ec2eeu)
#define BATTLE_UI_BYTE_8356 VU8(0x80148356u)
#define BATTLE_UI_BYTE_8357 VU8(0x80148357u)
#define BATTLE_UI_HALF_8358 VU16(0x80148358u)
#define BATTLE_UI_HALF_835A VU16(0x8014835au)
#define BATTLE_UI_BYTE_835C VU8(0x8014835cu)
#define BATTLE_UI_BYTE_835D VU8(0x8014835du)
#define BATTLE_UI_BYTE_835E VU8(0x8014835eu)
#define BATTLE_UI_BYTE_837A VU8(0x8014837au)
#define BATTLE_UI_BYTE_837B VU8(0x8014837bu)
#define BATTLE_UI_HALF_837C VU16(0x8014837cu)
#define BATTLE_UI_HALF_837E VU16(0x8014837eu)
#define BATTLE_UI_BYTE_839E VU8(0x8014839eu)
#define BATTLE_UI_BYTE_839F VU8(0x8014839fu)
#define BATTLE_UI_BYTE_83C2 VU8(0x801483c2u)
#define BATTLE_UI_BYTE_83C3 VU8(0x801483c3u)
#define BATTLE_UI_HALF_83C4 VU16(0x801483c4u)
#define BATTLE_UI_HALF_83C6 VU16(0x801483c6u)
#define BATTLE_UI_BYTE_8332 VU8(0x80148332u)
#define BATTLE_UI_BYTE_8333 VU8(0x80148333u)
#define BATTLE_UI_BYTE_8333_INDEX(index) \
 VU8(0x80148333u + ((u32)(index) * 0x24u))
#define BATTLE_UI_BYTE_833A(index) \
 VU8(0x8014833au + ((u32)(index) * 0x24u))
#define BATTLE_UI_HALF_8334 VU16(0x80148334u)
#define BATTLE_UI_HALF_8336 VU16(0x80148336u)
#define BATTLE_LOCAL_SCRATCH_PTR \
 VPPTR(Battle03LocalWork, 0x1f800044u)
#define BATTLE_ENEMY_SCRATCH_PTR \
 VPPTR(Battle03EnemyWork, 0x1f800044u)
#define BATTLE_CURRENT_ENEMY_PTR \
 VPPTR(Battle03EnemyWork, 0x801eb4e8u)
#define BATTLE_CURRENT_QUEUED_SLOT_PTR \
 VPPTR(Battle03QueuedSlot, 0x801ec2e0u)
#define BATTLE_CURRENT_QUEUED_WORD_4B20 VU32(0x801eb4e0u)
#define BATTLE_CURRENT_QUEUED_PTR_4B20 VPPTR(u8, 0x801eb4e0u)
#define BATTLE_SLOT_STORE_FLAG(index) \
 VU8(0x801ec339u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_PTR(index) \
 VU32(0x801ec3a4u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_34(index) \
 VU32(0x801ec364u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_38(index) \
 VU32(0x801ec368u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_3C(index) \
 VU32(0x801ec36cu + ((u32)(index) * 0x78u))
#define BATTLE_EVENT_SLOT_FLAG(index) \
 VU8(0x801eb4f0u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_A(index) \
 VU8(0x801eb4f1u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_B(index) \
 VU8(0x801eb4f2u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_C(index) \
 VU8(0x801eb4f3u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_KIND(index) \
 VU8(0x801eb4f5u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_MODE(index) \
 VU8(0x801eb4f6u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_WORD(index) \
 VU32(0x801eb4f4u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_HALF(index) \
 VU16(0x801eb4f8u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_BYTE(index) \
 VU8(0x801eb4fau + ((u32)(index) * 0x0cu))
#define BATTLE_SLOT_STORE_BYTE_01(index) \
 VU8(0x801ec331u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_02(index) \
 VU8(0x801ec332u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_05(index) \
 VU8(0x801ec335u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_06(index) \
 VU8(0x801ec336u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_29(index) \
 VU8(0x801ec359u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5C(index) \
 VU8(0x801ec38cu + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5D(index) \
 VU8(0x801ec38du + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5E(index) \
 VU8(0x801ec38eu + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5F(index) \
 VU8(0x801ec38fu + ((u32)(index) * 0x78u))
#define BATTLE_LOCAL_STATE_TABLE \
 CVPTR(Battle03Handler, 0x801eb120u)
#define BATTLE_LOCAL_FLAGS_80(work) \
 VU16((volatile u8*)(work) + 0x80u)
#define BATTLE_LOCAL_BYTE_79(work) \
 VU8((volatile u8*)(work) + 0x79u)
#define BATTLE_LOCAL_BYTE_7A(work) \
 VU8((volatile u8*)(work) + 0x7au)
#define BATTLE_LOCAL_BYTE_82(work) \
 VU8((volatile u8*)(work) + 0x82u)
#define BATTLE_LOCAL_BYTE_85(work) \
 VU8((volatile u8*)(work) + 0x85u)
#define BATTLE_LOCAL_BYTE_86(work) \
 VU8((volatile u8*)(work) + 0x86u)
#define BATTLE_LOCAL_BYTE_87(work) \
 VU8((volatile u8*)(work) + 0x87u)
#define BATTLE_LOCAL_BYTE_4B(work) \
 VU8((volatile u8*)(work) + 0x4bu)
#define BATTLE_LOCAL_BYTE_09(work) \
 VU8((volatile u8*)(work) + 9u)
#define BATTLE_LOCAL_BYTE_0A(work) \
 VU8((volatile u8*)(work) + 10u)
#define BATTLE_LOCAL_BYTE_119(work) \
 VU8((volatile u8*)(work) + 0x119u)
#define BATTLE_LOCAL_BYTE_118(work) \
 VU8((volatile u8*)(work) + 0x118u)
#define BATTLE_LOCAL_HALF_11A(work) \
 VU16((volatile u8*)(work) + 0x11au)
#define BATTLE_LOCAL_WORD_124(work) \
 VU32((volatile u8*)(work) + 0x124u)
#define BATTLE_LOCAL_WORD_128(work) \
 VU32((volatile u8*)(work) + 0x128u)
#define BATTLE_LOCAL_KIND_MASK(kind) \
 VU16(0x801ca71cu + ((u32)(kind) * 0x14u))
#define BATTLE_PANEL_SLOT_MASK(kind) \
 VU8(0x801d90ebu + ((u32)(kind) * 0x18u))
#define BATTLE_LOCAL_HALF_88(work) \
 VU16((volatile u8*)(work) + 0x88u)
#define BATTLE_LOCAL_HALF_8A(work) \
 VU16((volatile u8*)(work) + 0x8au)
#define BATTLE_LOCAL_HALF_90(work) \
 VU16((volatile u8*)(work) + 0x90u)
#define BATTLE_LOCAL_HALF_92(work) \
 VU16((volatile u8*)(work) + 0x92u)
#define BATTLE_LOCAL_HALF_96(work) \
 VU16((volatile u8*)(work) + 0x96u)
#define BATTLE_LOCAL_HALF_98(work) \
 VU16((volatile u8*)(work) + 0x98u)
#define BATTLE_LOCAL_BYTE_8C(work) \
 VU8((volatile u8*)(work) + 0x8cu)
#define BATTLE_LOCAL_BYTE_2A(work) \
 VU8((volatile u8*)(work) + 0x2au)
#define BATTLE_LOCAL_BYTE_9E(work) \
 VU8((volatile u8*)(work) + 0x9eu)
#define BATTLE_LOCAL_BYTE_A6(work) \
 VU8((volatile u8*)(work) + 0xa6u)
#define BATTLE_LOCAL_BYTE_A9(work) \
 VU8((volatile u8*)(work) + 0xa9u)
#define BATTLE_LOCAL_BYTE_21(work) \
 VU8((volatile u8*)(work) + 0x21u)
#define BATTLE_LOCAL_HALF_1C(work) \
 VU16((volatile u8*)(work) + 0x1cu)
#define BATTLE_LOCAL_HALF_1E(work) \
 VU16((volatile u8*)(work) + 0x1eu)
#define BATTLE_LOCAL_BYTE_136(work) \
 VU8((volatile u8*)(work) + 0x136u)
#define BATTLE_LOCAL_BYTE_137(work) \
 VU8((volatile u8*)(work) + 0x137u)
#define BATTLE_LOCAL_BYTE_138(work) \
 VU8((volatile u8*)(work) + 0x138u)
#define BATTLE_LOCAL_BYTE_139(work) \
 VU8((volatile u8*)(work) + 0x139u)
#define BATTLE_LOCAL_WORD_134(work) \
 VU32((volatile u8*)(work) + 0x134u)
#define BATTLE_LOCAL_BYTE_134(work) \
 VU8((volatile u8*)(work) + 0x134u)
#define BATTLE_LOCAL_BYTE_120(work) \
 VU8((volatile u8*)(work) + 0x120u)
#define BATTLE_LOCAL_BYTE_121(work) \
 VU8((volatile u8*)(work) + 0x121u)
#define BATTLE_LOCAL_BYTE_122(work) \
 VU8((volatile u8*)(work) + 0x122u)
#define BATTLE_LOCAL_BYTE_13C(work) \
 VU8((volatile u8*)(work) + 0x13cu)
#define BATTLE_LOCAL_HALF_2C(work) \
 VU16((volatile u8*)(work) + 0x2cu)
#define BATTLE_GLOBAL_BYTE_63C9 VU8(0x801463c9u)
#define BATTLE_GLOBAL_BYTE_44F58 VU8(0x80144f58u)
#define BATTLE_GLOBAL_BYTE_4952 VS8(0x80144952u)
#define BATTLE_LOCAL_STATE_TABLE_015C \
 CVPTR(Battle03Handler, 0x801eb15cu)
#define BATTLE_LOCAL_STATE_TABLE_0188 \
 (*(Battle03Handler const volatile*)0x801eb188u)
#define BATTLE_LOCAL_BYTE_TABLE_018C CVPTR(u8, 0x801eb18cu)
#define BATTLE_LOCAL_BYTE_TABLE_0198 CVPTR(u8, 0x801eb198u)
#define BATTLE_LOCAL_SUBSTATE3_TABLE \
 CVPTR(Battle03Handler, 0x801eb1e0u)
#define BATTLE_LOCAL_STATE4_TABLE \
 CVPTR(Battle03Handler, 0x801eb210u)
#define BATTLE_LOCAL_ALT_STATE3_TABLE \
 CVPTR(Battle03Handler, 0x801eb218u)
#define BATTLE_LOCAL_STATE2_CLASS_TABLE \
 CVPTR(Battle03Handler, 0x801eb224u)
#define BATTLE_LOCAL_STATE2_EVENT_TABLE \
 CVPTR(Battle03Handler, 0x801eb26cu)
#define BATTLE_LOCAL_STATE2_FOLLOWUP_TABLE \
 CVPTR(Battle03Handler, 0x801eb274u)
#define BATTLE_LOCAL_DEFAULT_CLASS_TABLE \
 CVPTR(Battle03Handler, 0x801eb27cu)
#define BATTLE_ENEMY_DISPATCH_TABLE_A \
 CVPTR(Battle03Handler, 0x801eb294u)
#define BATTLE_ENEMY_DISPATCH_TABLE_B \
 CVPTR(Battle03Handler, 0x801eb298u)
#define BATTLE_ENEMY_FLAGS_82(work) \
 VU16((volatile u8*)(work) + 0x82u)
#define BATTLE_ENEMY_FLAGS_80(work) \
 VU16((volatile u8*)(work) + 0x80u)
#define BATTLE_ENEMY_BYTE_7E(work) \
 VU8((volatile u8*)(work) + 0x7eu)
#define BATTLE_ENEMY_BYTE_7D(work) \
 VU8((volatile u8*)(work) + 0x7du)
#define BATTLE_ENEMY_BYTE_7C(work) \
 VU8((volatile u8*)(work) + 0x7cu)
#define BATTLE_ENEMY_BYTE_7F(work) \
 VU8((volatile u8*)(work) + 0x7fu)
#define BATTLE_ENEMY_BYTE_02(work) \
 VU8((volatile u8*)(work) + 2u)
#define BATTLE_ENEMY_BYTE_03(work) \
 VU8((volatile u8*)(work) + 3u)
#define BATTLE_ENEMY_BYTE_04(work) \
 VU8((volatile u8*)(work) + 4u)
#define BATTLE_ENEMY_BYTE_05(work) \
 VU8((volatile u8*)(work) + 5u)
#define BATTLE_ENEMY_HALF_AA(work) \
 VU16((volatile u8*)(work) + 0xaau)
#define BATTLE_ENEMY_HALF_94(work) \
 VU16((volatile u8*)(work) + 0x94u)
#define BATTLE_ENEMY_BYTE_88(work) \
 VU8((volatile u8*)(work) + 0x88u)
#define BATTLE_ENEMY_HALF_A8(work) \
 VU16((volatile u8*)(work) + 0xa8u)
#define BATTLE_ENEMY_HALF_A0(work) \
 VU16((volatile u8*)(work) + 0xa0u)
#define BATTLE_ENEMY_HALF_F8(work) \
 VU16((volatile u8*)(work) + 0xf8u)
#define BATTLE_ENEMY_HALF_FA(work) \
 VU16((volatile u8*)(work) + 0xfau)
#define BATTLE_ENEMY_BYTE_F5(work) \
 VU8((volatile u8*)(work) + 0xf5u)
#define BATTLE_ENEMY_BYTE_FC(work) \
 VU8((volatile u8*)(work) + 0xfcu)
#define BATTLE_ENEMY_BYTE_FD(work) \
 VU8((volatile u8*)(work) + 0xfdu)
#define BATTLE_ENEMY_HALF_F6(work) \
 VU16((volatile u8*)(work) + 0xf6u)
#define BATTLE_ENEMY_WORD_100(work) \
 VU32((volatile u8*)(work) + 0x100u)
#define BATTLE_ENEMY_BYTE_100(work) \
 VU8((volatile u8*)(work) + 0x100u)
#define BATTLE_ENEMY_WORD_104(work) \
 VU32((volatile u8*)(work) + 0x104u)
#define BATTLE_ENEMY_BYTE_112(work) \
 VU8((volatile u8*)(work) + 0x112u)
#define BATTLE_ENEMY_BYTE_114(work) \
 VU8((volatile u8*)(work) + 0x114u)
#define BATTLE_ENEMY_BYTE_115(work) \
 VU8((volatile u8*)(work) + 0x115u)
#define BATTLE_ENEMY_BYTE_E6(work) \
 VU8((volatile u8*)(work) + 0xe6u)
#define BATTLE_ENEMY_PTR_EC(work) \
 VPPTR(u8, (volatile u8*)(work) + 0xecu)
#define BATTLE_WEIGHT_TABLE_0394 CVPTR(u8, 0x801eb394u)
#define BATTLE_WEIGHT_TABLE_039C CVPTR(u8, 0x801eb39cu)
#define BATTLE_RANDOM_TABLE_AC58 CVPTR(u8, 0x801eac58u)
#define BATTLE_RANDOM_TABLE_AC78 CVPTR(u8, 0x801eac78u)
#define BATTLE_RETRY_TABLE_AFF4 CVPTR(u8, 0x801eaff4u)
#define BATTLE_COUNTER_TABLE_AFFC CVPTR(u8, 0x801eaffcu)
#define BATTLE_PERCENT_TABLE_AF3C CVPTR(u16, 0x801eaf3cu)
#define BATTLE_RANDOM_BONUS_TABLE_AF48 CVPTR(s8, 0x801eaf48u)
#define BATTLE_RANK_TABLE_AF88 CVPTR(u8, 0x801eaf88u)
#define BATTLE_VARIANCE_TABLE_AFA0 CVPTR(s32, 0x801eafa0u)
#define BATTLE_SCALE_TABLE_AFC0 CVPTR(s16, 0x801eafc0u)
#define BATTLE_DAMAGE_SCALE_TABLE_0C7C CVPTR(u8, 0x801d0c7cu)
#define BATTLE_EFFECT_TABLE_AFD0 CVPTR(u16, 0x801eafd0u)
#define BATTLE_EVENT_PICK_TABLE_0C98 CVPTR(u8, 0x801d0c98u)
#define BATTLE_EVENT_PICK_TABLE_0CB8 CVPTR(u8, 0x801d0cb8u)
#define BATTLE_EVENT_SCRIPT_TABLE_B09C CVPTR(u16, 0x801eb09cu)
#define BATTLE_COUNTER_PTR_TABLE_893C ((volatile u32**)0x801c893cu)
#define BATTLE_COUNTER_BYTE_TABLE_8950 ((volatile u8**)0x801c8950u)
#define BATTLE_TRIGGER_TABLE_6178 ((volatile u32**)0x800b6178u)
#define BATTLE_VARIANCE_TABLE_AF94 CVPTR(u8, 0x801eaf94u)
#define BATTLE_TARGET_MODE_PACK(index) \
 VU8(0x800b51f8u + (u32)(index))
#define BATTLE_ENEMY_SLOT_KIND(index) \
 VU8(0x801eb6acu + ((u32)(index) * 0x118u))
#define BATTLE_KIND_BYTE_00(kind) \
 VU8(0x801ca718u + ((u32)(kind) * 0x14u))
#define BATTLE_LOCAL_PRESENTATION_STATE1_TABLE \
 CVPTR(Battle03Handler, 0x801eb3b0u)
#define BATTLE_LOCAL_PRESENTATION_BYTE3_TABLE \
 CVPTR(Battle03Handler, 0x801eb430u)
#define BATTLE_QUEUED_RESULT_SUBSTATE_TABLE \
 CVPTR(Battle03Handler, 0x801eb454u)
#define BATTLE_ACTIVE_SLOT_TABLE_0 \
 CVPTR(Battle03Handler, 0x801d0cd0u)
#define BATTLE_QUEUED_SLOT_TABLE \
 CVPTR(Battle03Handler, 0x801d0cc0u)
#define BATTLE_PANEL_TASK_ROOT_TABLE \
 CVPTR(Battle03Handler, 0x801d0f80u)
#define BATTLE_PANEL_TASK_ARG_DISPATCH_TABLE \
 CVPTR(Battle03ForwardingHandler, 0x801d0fecu)
#define BATTLE_PANEL_TASK_PTR VPPTR(u8, 0x80148648u)
#define BATTLE_PANEL_TASK_HALF_04 \
 VU16(BATTLE_PANEL_TASK_PTR + 4)
#define BATTLE_PANEL_TASK_HALF_06 \
 VU16(BATTLE_PANEL_TASK_PTR + 6)
#define BATTLE_PANEL_TASK_BYTE_03 \
 VU8(BATTLE_PANEL_TASK_PTR + 3)
#define BATTLE_PANEL_TASK_BYTE_0F \
 VU8(BATTLE_PANEL_TASK_PTR + 0xf)
#define BATTLE_PANEL_TASK_BYTE_0A \
 VU8(BATTLE_PANEL_TASK_PTR + 0x0au)
#define BATTLE_PANEL_TASK_BYTE_0B \
 VU8(BATTLE_PANEL_TASK_PTR + 0x0bu)
#define BATTLE_PANEL_TASK_BYTE_0D \
 VU8(BATTLE_PANEL_TASK_PTR + 0x0du)
#define BATTLE_PANEL_TASK_HALF_10 \
 VU16(BATTLE_PANEL_TASK_PTR + 0x10)
#define BATTLE_PANEL_TASK_HALF_12 \
 VU16(BATTLE_PANEL_TASK_PTR + 0x12)
#define BATTLE_UI_RING_INDEX VU8(0x801ebf04u)
#define BATTLE_UI_RING_TARGET VU8(0x801ec328u)
#define BATTLE_UI_RING_BYTE0(index) \
 VU8(0x801eb5b0u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_BYTE1(index) \
 VU8(0x801eb5b1u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_WORD2(index) \
 VU32(0x801eb5b4u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_BYTE(index) \
 VU8(0x801eb4fau + ((u32)(index) * 0x0cu))
#define BATTLE_UI_RING_WORD(index) \
 VU32(0x801eb4f4u + ((u32)(index) * 0x0cu))
#define BATTLE_UI_MODE_TABLE_AF27 CVPTR(u8, 0x801eaf27u)
#define BATTLE_QUAD_OFFSET_TABLE_AD30 CVPTR(s16, 0x801ead30u)
#define BATTLE_SPRITE_OFFSET_TABLE_AE50 CVPTR(s16, 0x801eae50u)
#define BATTLE_ICON_OFFSET_TABLE_AE94 CVPTR(u8, 0x801eae94u)
#define BATTLE_PANEL_FRAME_TABLE_AEE8 CVPTR(s16, 0x801eaee8u)
#define BATTLE_PANEL_ICON_TABLE_AEB0 CVPTR(u32, 0x801eaeb0u)
#define BATTLE_ICON_CLUT_TABLE_0C64 CVPTR(u8, 0x801d0c64u)
#define BATTLE_GLOBAL_PTR_BF08 VPPTR(u8, 0x801ebf08u)
#define BATTLE_LOCAL_ALT_WORK_ARRAY VPTR(u8, 0x801ebf20u)
#define BATTLE_LOCAL_STATUS_ARRAY VPTR(u8, 0x801ec048u)
#define BATTLE_PANEL_TASK_ICON_TABLE \
 CVPTR(Battle03Handler, 0x801d0ff8u)
#define BATTLE_RESULT_UI_AUX_HANDLER_0 ((Battle03Handler)0x801e8684u)
#define BATTLE_RESULT_UI_AUX_HANDLER_1 ((Battle03Handler)0x801e8d04u)
#define BATTLE_PREVIEW_SEQUENCE_TABLE \
 CVPTR(Battle03Handler, 0x801d0f44u)
#define BATTLE_SAVED_PREVIEW_RESULT_TABLE \
 CVPTR(Battle03Handler, 0x801d0f6cu)

void func_8014d290(void);
void func_8014d5f0(u8 arg0, u32 arg1, s32 arg2);
void func_8014f800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
u16 func_8017a620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
u8 func_8017e3d4(void);
void func_8017a904(u32 arg0, s32 arg1);
void func_8017a9a4(u32 arg0);
void func_8017a9b8(u32 arg0);
u16 func_8017a6f0(s32 arg0, s32 arg1);
void func_8017aa6c(u32 arg0);
void func_8017aa80(u32 arg0);
void func_8017aa1c(void);
void func_8017e3f4(void* arg0, const void* arg1, ...);
void func_8017e364(void* arg0, const void* arg1);
void func_8017c2d8(u32 arg0, s32 arg1, s32 arg2, u16 arg3, s32 arg4);
void func_80158db8(u8 arg0, u8 arg1);
u8 func_8014d978(void);
u8 func_8014daec(void);
void func_8014e5a0(u8 arg0, u8 arg1);
u32 func_8014d8d4(u8 arg0);
u8 func_801db524(u8 arg0);
u8 func_800a9304(u8 arg0);
u8 func_800a94a8(void);
u8 func_800a955c(void);
void func_800a36f0(u32 arg0, u32 arg1);
u16 func_800a2ae0(u8 arg0);
u8 func_800a3df8(u32 arg0, u32 arg1);
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
u8 func_801d64c4(u32 arg0);
u8 func_801ddcb4(u32 arg0);
void func_801644d8(u32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, u32 arg5);
void func_801deeb4(void);
void func_801de190(u32 arg0);
void func_801de560(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u32 arg4);
void func_801de9a8(u32 arg0);
void func_801dea18(u32 arg0);
void func_801dcef8(u32 arg0);
u32 func_801502d0(u32 arg0);
void func_801501e4(void* arg0, u32 arg1, u32 arg2);
void func_80150098(s16 arg0, s16 arg1, u32 arg2, void* arg3);
u32 func_801e590c(u32 arg0, u32 arg1);
u8 func_801e2e30(void);
s16 func_8015477c(s32 arg0, s32 arg1);

/* from old bof3/include/bof3/modules/battle/03.h */
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
