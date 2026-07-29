#ifndef EMI_BATTLE_03_INTERNAL_H
#define EMI_BATTLE_03_INTERNAL_H

#include "bof3/bof3.h"
#include "battle/ability.h"
#include "battle/ram.h"

typedef void (*Battle03Handler)(void);
typedef void (*Battle03EnemyModeHandler)(s32 arg0);
typedef void (*Battle03ForwardingHandler)(s32 arg0, s32 arg1, s32 arg2,
                                          s32 arg3, s32 arg4, s32 arg5,
                                          s32 arg6, u8* selector);
typedef struct Battle03DispatchTable {
  Battle03Handler handlers[3];
} Battle03DispatchTable;
typedef struct Battle03FiveDispatchTable {
  Battle03Handler handlers[5];
} Battle03FiveDispatchTable;

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
  u32 unk_40;
  u32 unk_44;
  u8  unk_48;
  u8  pad_49[0xda];
  u8  unk_123;
  u8  pad_124[0x10];
  u16 unk_134;
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

typedef struct Battle03SpritePrimitive {
  u8  pad_00[4];
  u8  r0;
  u8  g0;
  u8  b0;
  u8  pad_07;
  s16 x0;
  s16 y0;
  s16 unk_0c;
  s16 unk_0e;
} Battle03SpritePrimitive;

extern volatile AbilityObject ABILITY_OBJECTS[];
extern Battle03LocalWork* volatile D_80146250;
extern volatile u8* g_battle03_work;
extern u8*          D_8014598C;
extern u8           D_801462E1[];
extern volatile Battle03QueuedSlot* D_801EC2E0;
extern u32                           D_80148648;
extern u8                            D_801462E5;
extern u8                            D_8014630C;
extern u8                            D_8014864C;
extern u32                           D_80181B10;
extern u8                            D_801EB4E0;
extern Battle03FiveDispatchTable    D_801D0F58;
extern Battle03DispatchTable        D_801D0F20;
extern Battle03DispatchTable        D_801D0F2C;
extern Battle03DispatchTable        D_801D0FA0;
extern Battle03DispatchTable        D_801D0FAC;
extern Battle03DispatchTable        D_801D0FB8;
extern Battle03DispatchTable        D_801D0FC4;
extern Battle03Handler D_801EACD4[];
extern Battle03Handler D_801EACE8[];
extern Battle03Handler D_801EACF4[];
extern Battle03Handler D_801EAD00[];
extern Battle03Handler D_801EAD0C[];
extern Battle03Handler D_801EAD20[];
extern Battle03Handler D_801EB1B4[];
extern Battle03Handler D_801EB1BC[];
extern Battle03Handler D_801EB1D4[];
extern Battle03Handler D_801EB1F4[];
extern Battle03Handler D_801EB3E4[];
extern Battle03Handler D_801EB3EC[];
extern Battle03Handler D_801EB3F4[];
extern Battle03Handler D_801EB404[];
extern Battle03Handler D_801EB40C[];
extern Battle03Handler D_801EB424[];
extern Battle03Handler D_801EB444[];
extern Battle03Handler D_801EB460[];
extern Battle03Handler D_801EB478[];
extern void             D_801492B8;
extern u8               D_801EB000[];
extern s8               D_801EB4F2;
extern volatile u8  BATTLE_LOCAL_BYTE_62EC;
extern volatile u8  BATTLE_GLOBAL_BYTE_62E0;
extern volatile u8  BATTLE_GLOBAL_BYTE_62E1;
extern volatile u8  BATTLE_GLOBAL_BYTE_62E2;
extern volatile u16 BATTLE_GLOBAL_HALF_62E8;
extern volatile u8  BATTLE_GLOBAL_BYTE_62EA;
extern volatile u8  BATTLE_GLOBAL_BYTE_62EE;
extern volatile u8  BATTLE_GLOBAL_BYTE_62F4;
extern volatile u8  BATTLE_GLOBAL_BYTE_62F3;
extern volatile u8  BATTLE_GLOBAL_BYTE_6301;
extern volatile u8  BATTLE_GLOBAL_BYTE_6302;
extern volatile u8  BATTLE_GLOBAL_BYTE_6303;
extern volatile u32 BATTLE_GLOBAL_WORD_59F0;
extern volatile u16 BATTLE_SCRATCH_HALF_000;
extern volatile u8  BATTLE_SCRATCH_BYTE_000;
extern volatile u8  BATTLE_SCRATCH_BYTE_001;
extern volatile u8  BATTLE_SCRATCH_BYTE_002;
extern volatile u8  BATTLE_LOCAL_FLAG_63CE;
extern volatile u8  BATTLE_GLOBAL_BYTE_6325;
extern volatile u8  BATTLE_GLOBAL_BYTE_6327;
extern volatile u8  BATTLE_GLOBAL_BYTE_6328;
extern volatile u32 BATTLE_GLOBAL_WORD_632C;
extern volatile u32 BATTLE_GLOBAL_WORD_6330;
extern volatile u8  BATTLE_GLOBAL_BYTE_6374;
extern volatile u8  BATTLE_GLOBAL_BYTE_6375;
extern volatile u8  BATTLE_GLOBAL_BYTE_6384;
extern volatile u8  BATTLE_GLOBAL_BYTE_6324;
extern volatile u8  BATTLE_GLOBAL_BYTE_6304;
extern volatile u8  BATTLE_GLOBAL_BYTE_6308;
extern volatile u16 BATTLE_GLOBAL_HALF_63B8;
extern volatile u8  BATTLE_GLOBAL_BYTE_63BA;
extern volatile u16 BATTLE_GLOBAL_HALF_63C0;
extern volatile u16 BATTLE_GLOBAL_HALF_63C2;
extern volatile u8  BATTLE_GLOBAL_BYTE_6322;
extern volatile u8  BATTLE_GLOBAL_BYTE_6323;
extern volatile u8  BATTLE_GLOBAL_BYTE_63CE;
extern volatile u16 BATTLE_GLOBAL_HALF_63DA;
extern volatile u8  BATTLE_GLOBAL_BYTE_63CA;
extern volatile u16 BATTLE_GLOBAL_HALF_63D0;
extern volatile u8  BATTLE_GLOBAL_BYTE_EC324;
extern volatile u16 BATTLE_GLOBAL_HALF_EC30C;
extern volatile u8  BATTLE_UI_BYTE_8356;
extern volatile u8  BATTLE_UI_BYTE_8357;
extern volatile u16 BATTLE_UI_HALF_8358;
extern volatile u16 BATTLE_UI_HALF_835A;
extern volatile u8  BATTLE_UI_BYTE_835C;
extern volatile u8  BATTLE_UI_BYTE_835D;
extern volatile u8  BATTLE_UI_BYTE_835E;
extern volatile u8  BATTLE_UI_BYTE_837A;
extern volatile u8  BATTLE_UI_BYTE_837B;
extern volatile u16 BATTLE_UI_HALF_837C;
extern volatile u16 BATTLE_UI_HALF_837E;
extern volatile u8  BATTLE_UI_BYTE_839E;
extern volatile u8  BATTLE_UI_BYTE_839F;
extern volatile u8  BATTLE_UI_BYTE_83C2;
extern volatile u8  BATTLE_UI_BYTE_83C3;
extern volatile u16 BATTLE_UI_HALF_83C4;
extern volatile u16 BATTLE_UI_HALF_83C6;
extern volatile u8  BATTLE_UI_BYTE_8332;
extern volatile u8  BATTLE_UI_BYTE_8333;
extern volatile u16 BATTLE_UI_HALF_8334;
extern volatile u16 BATTLE_UI_HALF_8336;
extern volatile u32 BATTLE_CURRENT_QUEUED_WORD_4B20;
extern volatile u8  BATTLE_GLOBAL_BYTE_63C9;
extern volatile u8  BATTLE_GLOBAL_BYTE_44F58;
extern volatile s8  BATTLE_GLOBAL_BYTE_4952;
extern volatile u8  BATTLE_RANDOM_TABLE_AC58_DATA[];
extern volatile u8  BATTLE_RANDOM_TABLE_AC78_DATA[];
extern volatile u8  BATTLE_UI_RING_INDEX;
extern volatile u8  BATTLE_UI_RING_TARGET;
extern u8           func_8017E3D4(void);

void func_8014D290(void);
void func_801E72A8(void);
void func_801E74B8(void);
void func_801E72F4(void);
void func_8014D5F0(u8 arg0, u32 arg1, s32 arg2);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
u16  func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A904(u32 arg0, s32 arg1);
void func_8017A9A4(u32 arg0);
void func_8017A9B8(u32 arg0);
u16  func_8017A6F0(s32 arg0, s32 arg1);
void func_8017AA6C(u32 arg0);
void func_8017AA80(u32 arg0);
void func_8017AA1C(void);
void func_8017E3F4(void* arg0, const void* arg1, ...);
void func_8017E364(void* arg0, const void* arg1);
void func_8017C2D8(u32 arg0, s32 arg1, s32 arg2, u16 arg3, s32 arg4);
void func_80158DB8(u8 arg0, u8 arg1);
u8   func_8014D978(void);
u8   func_8014DAEC(void);
void func_8014E5A0(u8 arg0, u8 arg1);
u32  func_8014D8D4(u8 arg0);
u8   func_801DB524(u8 arg0);
u8   func_800A9304(u8 arg0);
u8   func_800A94A8(void);
u8   func_800A955C(void);
void func_800A36F0(u32 arg0, u32 arg1);
u16  func_800A2AE0(u8 arg0);
u8   func_800A3DF8(u32 arg0, u32 arg1);
void func_800A31E0(u32 arg0, u32 arg1);
void func_800AD074(u8 arg0);
void func_800AAA74(void);
void func_800A4458(void);
void func_800A9BD8(u8 arg0);
void func_8015DF18(u16 arg0);
void func_801636A0(u32 arg0, u32 arg1);
void func_80164A44(u32 arg0);
void func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_80196718(void* arg0);
void func_801D7EB0(s32 arg0, s32 arg1);
u8   func_801D64C4(u32 arg0);
u8   func_801DDCB4(u32 arg0);
void func_801644D8(u32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, u32 arg5);
void func_801DEEB4(void);
void func_801DE190(u32 arg0);
void func_801DE560(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u32 arg4);
void func_801DE9A8(u32 arg0);
void func_801DEA18(u32 arg0);
void func_801DCEF8(u32 arg0);
u32  func_801502D0(u32 arg0);
void func_801501E4(void* arg0, u32 arg1, u32 arg2);
void func_80150098(s16 arg0, s16 arg1, u32 arg2, void* arg3);
u32  func_801E590C(u32 arg0, u32 arg1);
u8   func_801E2E30(void);
s16  func_8015477C(s32 arg0, s32 arg1);

void func_801DECE0(void);
void func_801DED54(void);
u8   func_801DEDE4(void);
u8   func_801DEE4C(void);
void func_801DEF0C(void);
void func_801DEFE4(void);
void func_801DF34C(void);
void func_801DF8AC(void);
void func_801DF914(void);
void func_801DFC20(void);
void func_801E019C(void);
void func_801E046C(void);
void func_801E1298(void);
void func_801E1450(void);
void func_801E1670(void);
void func_801E1B64(void);
void func_801E1CD8(void);
void func_801E1E7C(void);
void func_801E2170(void);
void func_801E25E0(u8 arg0);
void func_801E2948(s8 arg0);
void func_801D9304(u8 arg0);
u8   func_801D3844(void);
void func_801D4850(void);
void func_801D9388(u8 arg0);
void func_801D93E4(u8 arg0);
void func_801D9428(u8 arg0);
void func_801D9484(void);
u8   func_801D54F8(void);
u8   func_801D5658(void);
u8   func_801D57AC(void);
u8   func_801D590C(void);
u8   func_801D5A60(void);
u8   func_801D5BC0(void);
u8   func_801D5DCC(void);
void func_801D4D44(void);
void func_801D750C(s32 arg0, s32 arg1);
void func_801D7A40(s16 arg0, s16 arg1);
void func_801D7D10(u8 arg0, s16 arg1, s16 arg2, u16 arg3, u8 arg4, u8 arg5);
void func_801D8270(s32 arg0, s32 arg1);
void func_801D8450(u32 arg0);
void func_801D8690(s32 arg0, s32 arg1, s32 arg2);
void func_801D8AE4(s32 arg0, s32 arg1, s32 arg2);
void func_801D8DF8(s32 arg0, s32 arg1, u32 arg2);
u8   func_801DB058(void);
void func_801D94D4(s16 arg0, u16 arg1, s32 arg2, s16 arg3);
void func_801D9684(s16 arg0, u16 arg1, s32 arg2, u16 arg3);
void func_801D9AB4(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801D9C80(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801D9DBC(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4);
void func_801D9E9C(s16 arg0, s16 arg1, u16 arg2, u16 arg3, s8 arg4);
void func_801DA078(s16 arg0, s16 arg1, s32 arg2);
void func_801DA408(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
u8   func_801DA69C(u32 arg0);
u32  func_801DB434(u8 arg0, u32 arg1);
u8   func_801DB844(u32 arg0);
u8   func_801DB9E4(u32 arg0);
u8   func_801DB2F8(u32 arg0);
u8   func_801DB3A0(u32 arg0, u32 arg1, u32 arg2);
u8   func_801DB3E4(u32 arg0, u32 arg1, u32 arg2);
void func_801DB494(void);
void func_801DA7D4(void);
void func_801DAAE4(void);
u32  func_801DC73C(s16 arg0, u32 arg1, u32 arg2);
u32  func_801DCCB0(void);
u32  func_801DCD50(u32 arg0, u8 arg1, s32 arg2);
u32  func_801DC044(u8 arg0, u8 arg1, u16 arg2);
u32  func_801DCAD8(u8 arg0, u8 arg1, s8 arg2);
u32  func_801DC894(s16 arg0, u8 arg1, u32 arg2);
u32  func_801DBB78(u8 arg0, u8 arg1);
void func_801DD08C(void);
void func_801DD14C(u8 arg0);
void func_801DD29C(void);
void func_801DD350(s32 arg0);
void func_801DD3CC(s32 arg0);
u8   func_801DD448(void);
void func_801DD858(u32 arg0);
void func_801DD8AC(u32 arg0);
void func_801DDAB4(u32 arg0);
void func_801DDAF0(void);
void func_801DD800(void);
u8   func_801DDE7C(u32 arg0, u32 arg1);
void func_801DDF28(void);
u8   func_801DDF50(u16 arg0, u32 arg1);
void func_801DDFEC(u32 arg0);
void func_801DE1B0(u32 arg0);
void func_801DE1D4(void);
void func_801DE60C(u32 arg0, u8 arg1, u8 arg2, u8 arg3, u8 arg4, u32 arg5);
void func_801DE690(void);
void func_801DE7FC(void);
void func_801DE804(void);
u8   func_801DE858(s8 arg0);
void func_801DE8C0(s8 arg0, s8 arg1, u32 arg2);
u8   func_801DE92C(void);
void func_801DEA64(s32 arg0);
void func_801DDB7C(void);
u8   func_801E0E0C(void);
void func_801E0B64(void);
void func_801E1B2C(void);
void func_801E1DD4(void);
u8   func_801E3160(void);
u8   func_801E4368(void);
void func_801E2314(u32 arg0);
void func_801E4F34(void);
void func_801E531C(void);
void func_801E52F0(s16 arg0);
void func_801E54EC(void);
void func_801E567C(void);
void func_801E5704(void);
void func_801E5988(void);
void func_801E5A38(void);
void func_801E62BC(u8 arg0);
u8   func_801E7B34(void);
void func_801E8DD8(void);
u8   func_801E8FA8(void);
void func_801DEAE0(void);
void func_801DEBC4(u32 arg0, u32 arg1);
void func_801D9804(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801D9900(void);
void func_801D99AC(s16 arg0, s16 arg1, s32 arg2);
void func_801DA4B4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
void func_801DA5A8(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
u8   func_801E29B4(u8 arg0);
s8   func_801E2A88(u8 arg0);
u8   func_801E2CA4(void);
u8   func_801E2D4C(s8 arg0);
u8   func_801E2D90(void);
u8   func_801E30F8(void);
u8   func_801E30B8(s8 arg0);
void func_801E31C8(void);
void func_801E4490(void);
void func_801E4928(void);
void func_801E5AF4(void);
void func_801E5824(void);
void func_801E6C84(void);
void func_801E7818(void);
void func_801E9074(void);
void func_801E915C(void);
void func_801E91CC(void);
void func_801E949C(void);
void func_801EA174(void);
void func_801EA1A4(void);
void func_801EA1E0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, u8* selector);
void func_801E862C(void);
void func_801EA650(void);
void func_801EA7DC(void);
void func_801EAAB8(void);
u8   func_801EAB38(void);
s8   func_801EAB6C(u8* arg0);

#define BATTLE_LOCAL_WORK_ARRAY PSX_PTR(volatile Battle03LocalWork, 0x80145e90u)
#define BATTLE_ENEMY_WORK_ARRAY PSX_PTR(volatile Battle03EnemyWork, 0x801eb630u)
#define BATTLE_QUEUED_SLOT_ARRAY                                               \
  PSX_PTR(volatile Battle03QueuedSlot, 0x801ec330u)
#define BATTLE_LOCAL_WORK_PTR                                                  \
  SPAD_PTR_SLOT(volatile Battle03LocalWork, 0x80146250u)
#define BATTLE_GLOBAL_BYTE_62FC(index)                                         \
  PSX_REF(volatile u8, 0x801462fcu + (u32)(index))
#define BATTLE_GLOBAL_BYTE_62F6(index)                                         \
  PSX_REF(volatile u8, 0x801462f6u + (u32)(index))
#define BATTLE_GLOBAL_BYTE_630C(index)                                         \
  PSX_REF(volatile u8, 0x8014630cu + (u32)(index))
#define BATTLE_UI_CHAR_BUFFER PSX_PTR(volatile u8, 0x80145ad4u)
#define BATTLE_GLOBAL_HALF_6334(index)                                         \
  PSX_REF(volatile u16, 0x80146334u + ((u32)(index) * 2u))
#define BATTLE_GLOBAL_BYTE_6354(index)                                         \
  PSX_REF(volatile u8, 0x80146354u + (u32)(index))
#define BATTLE_GLOBAL_PTR_6380 SPAD_PTR_SLOT(volatile u8, 0x80146380u)
#define BATTLE_UI_BYTE_8333_INDEX(index)                                       \
  PSX_REF(volatile u8, 0x80148333u + ((u32)(index) * 0x24u))
#define BATTLE_UI_BYTE_833A(index)                                             \
  PSX_REF(volatile u8, 0x8014833au + ((u32)(index) * 0x24u))
#define BATTLE_LOCAL_SCRATCH_PTR                                               \
  SPAD_PTR_SLOT(volatile Battle03LocalWork, 0x1f800044u)
#define BATTLE_ENEMY_SCRATCH_PTR                                               \
  SPAD_PTR_SLOT(volatile Battle03EnemyWork, 0x1f800044u)
#define BATTLE_CURRENT_ENEMY_PTR                                               \
  SPAD_PTR_SLOT(volatile Battle03EnemyWork, 0x801eb4e8u)
#define BATTLE_CURRENT_QUEUED_SLOT_PTR                                         \
  SPAD_PTR_SLOT(volatile Battle03QueuedSlot, 0x801ec2e0u)
#define BATTLE_CURRENT_QUEUED_PTR_4B20 SPAD_PTR_SLOT(volatile u8, 0x801eb4e0u)
#define BATTLE_SLOT_STORE_FLAG(index)                                          \
  PSX_REF(volatile u8, 0x801ec339u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_PTR(index)                                           \
  PSX_REF(volatile u32, 0x801ec3a4u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_34(index)                                       \
  PSX_REF(volatile u32, 0x801ec364u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_38(index)                                       \
  PSX_REF(volatile u32, 0x801ec368u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_3C(index)                                       \
  PSX_REF(volatile u32, 0x801ec36cu + ((u32)(index) * 0x78u))
#define BATTLE_EVENT_SLOT_FLAG(index)                                          \
  PSX_REF(volatile u8, 0x801eb4f0u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_A(index)                                             \
  PSX_REF(volatile u8, 0x801eb4f1u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_B(index)                                             \
  PSX_REF(volatile u8, 0x801eb4f2u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_C(index)                                             \
  PSX_REF(volatile u8, 0x801eb4f3u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_KIND(index)                                          \
  PSX_REF(volatile u8, 0x801eb4f5u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_MODE(index)                                          \
  PSX_REF(volatile u8, 0x801eb4f6u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_WORD(index)                                          \
  PSX_REF(volatile u32, 0x801eb4f4u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_HALF(index)                                          \
  PSX_REF(volatile u16, 0x801eb4f8u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_BYTE(index)                                          \
  PSX_REF(volatile u8, 0x801eb4fau + ((u32)(index) * 0x0cu))
#define BATTLE_SLOT_STORE_BYTE_01(index)                                       \
  PSX_REF(volatile u8, 0x801ec331u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_02(index)                                       \
  PSX_REF(volatile u8, 0x801ec332u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_05(index)                                       \
  PSX_REF(volatile u8, 0x801ec335u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_06(index)                                       \
  PSX_REF(volatile u8, 0x801ec336u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_29(index)                                       \
  PSX_REF(volatile u8, 0x801ec359u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5C(index)                                       \
  PSX_REF(volatile u8, 0x801ec38cu + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5D(index)                                       \
  PSX_REF(volatile u8, 0x801ec38du + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5E(index)                                       \
  PSX_REF(volatile u8, 0x801ec38eu + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5F(index)                                       \
  PSX_REF(volatile u8, 0x801ec38fu + ((u32)(index) * 0x78u))
#define BATTLE_LOCAL_STATE_TABLE                                               \
  PSX_PTR(const volatile Battle03Handler, 0x801eb120u)
#define BATTLE_LOCAL_FLAGS_80(work) PSX_REF(volatile u16, (u32)(work) + 0x80u)
#define BATTLE_LOCAL_BYTE_79(work)  PSX_REF(volatile u8, (u32)(work) + 0x79u)
#define BATTLE_LOCAL_BYTE_7A(work)  PSX_REF(volatile u8, (u32)(work) + 0x7au)
#define BATTLE_LOCAL_BYTE_82(work)  PSX_REF(volatile u8, (u32)(work) + 0x82u)
#define BATTLE_LOCAL_BYTE_85(work)  PSX_REF(volatile u8, (u32)(work) + 0x85u)
#define BATTLE_LOCAL_BYTE_86(work)  PSX_REF(volatile u8, (u32)(work) + 0x86u)
#define BATTLE_LOCAL_BYTE_87(work)  PSX_REF(volatile u8, (u32)(work) + 0x87u)
#define BATTLE_LOCAL_BYTE_4B(work)  PSX_REF(volatile u8, (u32)(work) + 0x4bu)
#define BATTLE_LOCAL_BYTE_09(work)  PSX_REF(volatile u8, (u32)(work) + 9u)
#define BATTLE_LOCAL_BYTE_0A(work)  PSX_REF(volatile u8, (u32)(work) + 10u)
#define BATTLE_LOCAL_BYTE_119(work) PSX_REF(volatile u8, (u32)(work) + 0x119u)
#define BATTLE_LOCAL_BYTE_118(work) PSX_REF(volatile u8, (u32)(work) + 0x118u)
#define BATTLE_LOCAL_HALF_11A(work) PSX_REF(volatile u16, (u32)(work) + 0x11au)
#define BATTLE_LOCAL_WORD_124(work) PSX_REF(volatile u32, (u32)(work) + 0x124u)
#define BATTLE_LOCAL_WORD_128(work) PSX_REF(volatile u32, (u32)(work) + 0x128u)
#define BATTLE_ABILITY_RECORD_TABLE ABILITY_OBJECTS
#define BATTLE_LOCAL_KIND_MASK(kind)                                           \
  (BATTLE_ABILITY_RECORD_TABLE[(kind)].tail_10.selection_mask)
#define BATTLE_PANEL_SLOT_MASK(kind)                                           \
  PSX_REF(volatile u8, 0x801d90ebu + ((u32)(kind) * 0x18u))
#define BATTLE_LOCAL_HALF_88(work)  PSX_REF(volatile u16, (u32)(work) + 0x88u)
#define BATTLE_LOCAL_HALF_8A(work)  PSX_REF(volatile u16, (u32)(work) + 0x8au)
#define BATTLE_LOCAL_HALF_90(work)  PSX_REF(volatile u16, (u32)(work) + 0x90u)
#define BATTLE_LOCAL_HALF_92(work)  PSX_REF(volatile u16, (u32)(work) + 0x92u)
#define BATTLE_LOCAL_HALF_96(work)  PSX_REF(volatile u16, (u32)(work) + 0x96u)
#define BATTLE_LOCAL_HALF_98(work)  PSX_REF(volatile u16, (u32)(work) + 0x98u)
#define BATTLE_LOCAL_BYTE_8C(work)  PSX_REF(volatile u8, (u32)(work) + 0x8cu)
#define BATTLE_LOCAL_BYTE_2A(work)  PSX_REF(volatile u8, (u32)(work) + 0x2au)
#define BATTLE_LOCAL_BYTE_9E(work)  PSX_REF(volatile u8, (u32)(work) + 0x9eu)
#define BATTLE_LOCAL_BYTE_A6(work)  PSX_REF(volatile u8, (u32)(work) + 0xa6u)
#define BATTLE_LOCAL_BYTE_A9(work)  PSX_REF(volatile u8, (u32)(work) + 0xa9u)
#define BATTLE_LOCAL_BYTE_21(work)  PSX_REF(volatile u8, (u32)(work) + 0x21u)
#define BATTLE_LOCAL_HALF_1C(work)  PSX_REF(volatile u16, (u32)(work) + 0x1cu)
#define BATTLE_LOCAL_HALF_1E(work)  PSX_REF(volatile u16, (u32)(work) + 0x1eu)
#define BATTLE_LOCAL_BYTE_136(work) PSX_REF(volatile u8, (u32)(work) + 0x136u)
#define BATTLE_LOCAL_BYTE_137(work) PSX_REF(volatile u8, (u32)(work) + 0x137u)
#define BATTLE_LOCAL_BYTE_138(work) PSX_REF(volatile u8, (u32)(work) + 0x138u)
#define BATTLE_LOCAL_BYTE_139(work) PSX_REF(volatile u8, (u32)(work) + 0x139u)
#define BATTLE_LOCAL_WORD_134(work) PSX_REF(volatile u32, (u32)(work) + 0x134u)
#define BATTLE_LOCAL_BYTE_134(work) PSX_REF(volatile u8, (u32)(work) + 0x134u)
#define BATTLE_LOCAL_BYTE_120(work) PSX_REF(volatile u8, (u32)(work) + 0x120u)
#define BATTLE_LOCAL_BYTE_121(work) PSX_REF(volatile u8, (u32)(work) + 0x121u)
#define BATTLE_LOCAL_BYTE_122(work) PSX_REF(volatile u8, (u32)(work) + 0x122u)
#define BATTLE_LOCAL_BYTE_13C(work) PSX_REF(volatile u8, (u32)(work) + 0x13cu)
#define BATTLE_LOCAL_HALF_2C(work)  PSX_REF(volatile u16, (u32)(work) + 0x2cu)
#define BATTLE_LOCAL_STATE_TABLE_015C                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb15cu)
#define BATTLE_LOCAL_STATE_TABLE_0188                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb188u)
#define BATTLE_LOCAL_BYTE_TABLE_018C PSX_PTR(const volatile u8, 0x801eb18cu)
#define BATTLE_LOCAL_BYTE_TABLE_0198 PSX_PTR(const volatile u8, 0x801eb198u)
#define BATTLE_LOCAL_SUBSTATE3_TABLE                                           \
  PSX_PTR(const volatile Battle03Handler, 0x801eb1e0u)
#define BATTLE_LOCAL_STATE4_TABLE                                              \
  PSX_PTR(const volatile Battle03Handler, 0x801eb210u)
#define BATTLE_LOCAL_ALT_STATE3_TABLE                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb218u)
#define BATTLE_LOCAL_STATE2_CLASS_TABLE                                        \
  PSX_PTR(const volatile Battle03Handler, 0x801eb224u)
#define BATTLE_LOCAL_STATE2_EVENT_TABLE                                        \
  PSX_PTR(const volatile Battle03Handler, 0x801eb26cu)
#define BATTLE_LOCAL_STATE2_FOLLOWUP_TABLE                                     \
  PSX_PTR(const volatile Battle03Handler, 0x801eb274u)
#define BATTLE_LOCAL_DEFAULT_CLASS_TABLE                                       \
  PSX_PTR(const volatile Battle03Handler, 0x801eb27cu)
#define BATTLE_ENEMY_DISPATCH_TABLE_A                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb294u)
#define BATTLE_ENEMY_DISPATCH_TABLE_B                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb298u)
#define BATTLE_ENEMY_FLAGS_82(work) PSX_REF(volatile u16, (u32)(work) + 0x82u)
#define BATTLE_ENEMY_FLAGS_80(work) PSX_REF(volatile u16, (u32)(work) + 0x80u)
#define BATTLE_ENEMY_BYTE_7E(work)  PSX_REF(volatile u8, (u32)(work) + 0x7eu)
#define BATTLE_ENEMY_BYTE_7D(work)  PSX_REF(volatile u8, (u32)(work) + 0x7du)
#define BATTLE_ENEMY_BYTE_7C(work)  PSX_REF(volatile u8, (u32)(work) + 0x7cu)
#define BATTLE_ENEMY_BYTE_7F(work)  PSX_REF(volatile u8, (u32)(work) + 0x7fu)
#define BATTLE_ENEMY_BYTE_02(work)  PSX_REF(volatile u8, (u32)(work) + 2u)
#define BATTLE_ENEMY_BYTE_03(work)  PSX_REF(volatile u8, (u32)(work) + 3u)
#define BATTLE_ENEMY_BYTE_04(work)  PSX_REF(volatile u8, (u32)(work) + 4u)
#define BATTLE_ENEMY_BYTE_05(work)  PSX_REF(volatile u8, (u32)(work) + 5u)
#define BATTLE_ENEMY_HALF_AA(work)  PSX_REF(volatile u16, (u32)(work) + 0xaau)
#define BATTLE_ENEMY_HALF_94(work)  PSX_REF(volatile u16, (u32)(work) + 0x94u)
#define BATTLE_ENEMY_BYTE_88(work)  PSX_REF(volatile u8, (u32)(work) + 0x88u)
#define BATTLE_ENEMY_HALF_A8(work)  PSX_REF(volatile u16, (u32)(work) + 0xa8u)
#define BATTLE_ENEMY_HALF_A0(work)  PSX_REF(volatile u16, (u32)(work) + 0xa0u)
#define BATTLE_ENEMY_HALF_F8(work)  PSX_REF(volatile u16, (u32)(work) + 0xf8u)
#define BATTLE_ENEMY_HALF_FA(work)  PSX_REF(volatile u16, (u32)(work) + 0xfau)
#define BATTLE_ENEMY_BYTE_F5(work)  PSX_REF(volatile u8, (u32)(work) + 0xf5u)
#define BATTLE_ENEMY_BYTE_FC(work)  PSX_REF(volatile u8, (u32)(work) + 0xfcu)
#define BATTLE_ENEMY_BYTE_FD(work)  PSX_REF(volatile u8, (u32)(work) + 0xfdu)
#define BATTLE_ENEMY_HALF_F6(work)  PSX_REF(volatile u16, (u32)(work) + 0xf6u)
#define BATTLE_ENEMY_WORD_100(work) PSX_REF(volatile u32, (u32)(work) + 0x100u)
#define BATTLE_ENEMY_BYTE_100(work) PSX_REF(volatile u8, (u32)(work) + 0x100u)
#define BATTLE_ENEMY_WORD_104(work) PSX_REF(volatile u32, (u32)(work) + 0x104u)
#define BATTLE_ENEMY_BYTE_112(work) PSX_REF(volatile u8, (u32)(work) + 0x112u)
#define BATTLE_ENEMY_BYTE_114(work) PSX_REF(volatile u8, (u32)(work) + 0x114u)
#define BATTLE_ENEMY_BYTE_115(work) PSX_REF(volatile u8, (u32)(work) + 0x115u)
#define BATTLE_ENEMY_BYTE_E6(work)  PSX_REF(volatile u8, (u32)(work) + 0xe6u)
#define BATTLE_ENEMY_PTR_EC(work)                                              \
  SPAD_PTR_SLOT(volatile u8, (volatile u8*)(work) + 0xecu)
#define BATTLE_WEIGHT_TABLE_0394       PSX_PTR(const volatile u8, 0x801eb394u)
#define BATTLE_WEIGHT_TABLE_039C       PSX_PTR(const volatile u8, 0x801eb39cu)
#define BATTLE_RANDOM_TABLE_AC58       PSX_PTR(const volatile u8, 0x801eac58u)
#define BATTLE_RANDOM_TABLE_AC78       PSX_PTR(const volatile u8, 0x801eac78u)
#define BATTLE_RETRY_TABLE_AFF4        PSX_PTR(const volatile u8, 0x801eaff4u)
#define BATTLE_COUNTER_TABLE_AFFC      PSX_PTR(const volatile u8, 0x801eaffcu)
#define BATTLE_PERCENT_TABLE_AF3C      PSX_PTR(const volatile u16, 0x801eaf3cu)
#define BATTLE_RANDOM_BONUS_TABLE_AF48 PSX_PTR(const volatile s8, 0x801eaf48u)
#define BATTLE_RANK_TABLE_AF88         PSX_PTR(const volatile u8, 0x801eaf88u)
#define BATTLE_VARIANCE_TABLE_AFA0     PSX_PTR(const volatile s32, 0x801eafa0u)
#define BATTLE_SCALE_TABLE_AFC0        PSX_PTR(const volatile s16, 0x801eafc0u)
#define BATTLE_DAMAGE_SCALE_TABLE_0C7C PSX_PTR(const volatile u8, 0x801d0c7cu)
#define BATTLE_EFFECT_TABLE_AFD0       PSX_PTR(const volatile u16, 0x801eafd0u)
#define BATTLE_EVENT_PICK_TABLE_0C98   PSX_PTR(const volatile u8, 0x801d0c98u)
#define BATTLE_EVENT_PICK_TABLE_0CB8   PSX_PTR(const volatile u8, 0x801d0cb8u)
#define BATTLE_EVENT_SCRIPT_TABLE_B09C PSX_PTR(const volatile u16, 0x801eb09cu)
#define BATTLE_COUNTER_PTR_TABLE_893C  ((volatile u32**)0x801c893cu)
#define BATTLE_COUNTER_BYTE_TABLE_8950 ((volatile u8**)0x801c8950u)
#define BATTLE_TRIGGER_TABLE_6178      ((volatile u32**)0x800b6178u)
#define BATTLE_VARIANCE_TABLE_AF94     PSX_PTR(const volatile u8, 0x801eaf94u)
#define BATTLE_TARGET_MODE_PACK(index)                                         \
  PSX_REF(volatile u8, 0x800b51f8u + (u32)(index))
#define BATTLE_ENEMY_SLOT_KIND(index)                                          \
  PSX_REF(volatile u8, 0x801eb6acu + ((u32)(index) * 0x118u))
#define BATTLE_KIND_BYTE_00(kind)                                              \
  PSX_REF(volatile u8, 0x801ca718u + ((u32)(kind) * 0x14u))
#define BATTLE_LOCAL_PRESENTATION_STATE1_TABLE                                 \
  PSX_PTR(const volatile Battle03Handler, 0x801eb3b0u)
#define BATTLE_LOCAL_PRESENTATION_BYTE3_TABLE                                  \
  PSX_PTR(const volatile Battle03Handler, 0x801eb430u)
#define BATTLE_QUEUED_RESULT_SUBSTATE_TABLE                                    \
  PSX_PTR(const volatile Battle03Handler, 0x801eb454u)
#define BATTLE_ACTIVE_SLOT_TABLE_0                                             \
  PSX_PTR(const volatile Battle03Handler, 0x801d0cd0u)
#define BATTLE_QUEUED_SLOT_TABLE                                               \
  PSX_PTR(const volatile Battle03Handler, 0x801d0cc0u)
#define BATTLE_PANEL_TASK_ROOT_TABLE                                           \
  PSX_PTR(const volatile Battle03Handler, 0x801d0f80u)
#define BATTLE_PANEL_TASK_ARG_DISPATCH_TABLE                                   \
  PSX_PTR(const volatile Battle03ForwardingHandler, 0x801d0fecu)
#define BATTLE_PANEL_TASK_PTR SPAD_PTR_SLOT(volatile u8, 0x80148648u)
#define BATTLE_PANEL_TASK_HALF_04                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 4)
#define BATTLE_PANEL_TASK_HALF_06                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 6)
#define BATTLE_PANEL_TASK_BYTE_03                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 3)
#define BATTLE_PANEL_TASK_BYTE_0F                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0xf)
#define BATTLE_PANEL_TASK_BYTE_0A                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0x0au)
#define BATTLE_PANEL_TASK_BYTE_0B                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0x0bu)
#define BATTLE_PANEL_TASK_BYTE_0D                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0x0du)
#define BATTLE_PANEL_TASK_HALF_10                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 0x10)
#define BATTLE_PANEL_TASK_HALF_12                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 0x12)
#define BATTLE_UI_RING_BYTE0(index)                                            \
  PSX_REF(volatile u8, 0x801eb5b0u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_BYTE1(index)                                            \
  PSX_REF(volatile u8, 0x801eb5b1u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_WORD2(index)                                            \
  PSX_REF(volatile u32, 0x801eb5b4u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_BYTE(index)                                             \
  PSX_REF(volatile u8, 0x801eb4fau + ((u32)(index) * 0x0cu))
#define BATTLE_UI_RING_WORD(index)                                             \
  PSX_REF(volatile u32, 0x801eb4f4u + ((u32)(index) * 0x0cu))
#define BATTLE_UI_MODE_TABLE_AF27       PSX_PTR(const volatile u8, 0x801eaf27u)
#define BATTLE_QUAD_OFFSET_TABLE_AD30   PSX_PTR(const volatile s16, 0x801ead30u)
#define BATTLE_SPRITE_OFFSET_TABLE_AE50 PSX_PTR(const volatile s16, 0x801eae50u)
#define BATTLE_ICON_OFFSET_TABLE_AE94   PSX_PTR(const volatile u8, 0x801eae94u)
#define BATTLE_PANEL_FRAME_TABLE_AEE8   PSX_PTR(const volatile s16, 0x801eaee8u)
#define BATTLE_PANEL_ICON_TABLE_AEB0    PSX_PTR(const volatile u32, 0x801eaeb0u)
#define BATTLE_ICON_CLUT_TABLE_0C64     PSX_PTR(const volatile u8, 0x801d0c64u)
#define BATTLE_GLOBAL_PTR_BF08          SPAD_PTR_SLOT(volatile u8, 0x801ebf08u)
#define BATTLE_LOCAL_ALT_WORK_ARRAY     PSX_PTR(volatile u8, 0x801ebf20u)
#define BATTLE_LOCAL_STATUS_ARRAY       PSX_PTR(volatile u8, 0x801ec048u)
#define BATTLE_PANEL_TASK_ICON_TABLE                                           \
  PSX_PTR(const volatile Battle03Handler, 0x801d0ff8u)
#define BATTLE_RESULT_UI_AUX_HANDLER_0 ((Battle03Handler)0x801e8684u)
#define BATTLE_RESULT_UI_AUX_HANDLER_1 ((Battle03Handler)0x801e8d04u)
#define BATTLE_PREVIEW_SEQUENCE_TABLE                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801d0f44u)
#define BATTLE_SAVED_PREVIEW_RESULT_TABLE                                      \
  PSX_PTR(const volatile Battle03Handler, 0x801d0f6cu)

/* Scratchpad flags at 0x1F800000 (u16). */
#define BATTLE_SCRATCH_FLAGS SPAD_REF(volatile u16, 0u)

/* Scratchpad pointer cell at 0x1F800044, read as several cell types. */
#define BATTLE_SCRATCH_CELL_U8PTR PSX_REF(volatile u8*, 0x1f800044u)
#define BATTLE_SCRATCH_CELL_WORKPTR                                            \
  PSX_REF(volatile Battle03LocalWork*, 0x1f800044u)
#define BATTLE_SCRATCH_CELL_WORD PSX_REF(volatile u32, 0x1f800044u)

/* Local-work-array (0x80145e90, stride 0x140) absolute-field accessors. */
#define BATTLE_LOCAL_ABS_BYTE_5E90(index)                                      \
  PSX_REF(volatile u8, 0x80145e90u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F10(index)                                      \
  PSX_REF(volatile u16, 0x80145f10u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F18(index)                                      \
  PSX_REF(volatile u16, 0x80145f18u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F1A(index)                                      \
  PSX_REF(volatile u16, 0x80145f1au + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_BYTE_5F1C(index)                                      \
  PSX_REF(volatile u8, 0x80145f1cu + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_BYTE_5FB0(index)                                      \
  PSX_REF(volatile u8, 0x80145fb0u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F26(index)                                      \
  PSX_REF(volatile u16, 0x80145f26u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F28(index)                                      \
  PSX_REF(volatile u16, 0x80145f28u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_BYTE_5FCC(index)                                      \
  PSX_REF(volatile u8, 0x80145fccu + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_WORD_5F04(index)                                      \
  PSX_PTR(volatile u32, 0x80145f04u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_WORD_5FB8(index)                                      \
  PSX_REF(volatile u32, 0x80145fb8u + ((u32)(index) * 0x140u))

/* Ability template records (0x80144968, stride 0xa4) absolute-field accessors. */
#define BATTLE_TEMPLATE_ABS_WORD_4968(index)                                   \
  PSX_PTR(const volatile u32, 0x80144968u + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_HALF_497C(index)                                   \
  PSX_REF(volatile u16, 0x8014497cu + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_HALF_497E(index)                                   \
  PSX_REF(volatile u16, 0x8014497eu + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_HALF_4974(index)                                   \
  PSX_REF(volatile u16, 0x80144974u + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_BYTE_4980(index)                                   \
  PSX_REF(volatile u8, 0x80144980u + ((u32)(index) * 0xa4u))

/* Enemy-work-array (0x801eb630, stride 0x118) absolute-field accessors. */
#define BATTLE_ENEMY_ABS_HALF_6B2(index)                                       \
  PSX_REF(volatile u16, 0x801eb6b2u + ((u32)(index) * 0x118u))
#define BATTLE_ENEMY_ABS_WORD_734(index)                                       \
  PSX_REF(volatile u32, 0x801eb734u + ((u32)(index) * 0x118u))

/* Queued-slot (0x801ec330, stride 0x78) missing byte-field accessors. */
#define BATTLE_SLOT_STORE_BYTE_00(index)                                       \
  PSX_REF(volatile u8, 0x801ec330u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_03(index)                                       \
  PSX_REF(volatile u8, 0x801ec333u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_04(index)                                       \
  PSX_REF(volatile u8, 0x801ec334u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_48(index)                                       \
  PSX_REF(volatile u8, 0x801ec378u + ((u32)(index) * 0x78u))

/* UI ring head byte at 0x801ec328. */
#define BATTLE_UI_RING_HEAD PSX_REF(volatile u8, 0x801ec328u)

/* Local-state dispatch tables reached as (0x801f0000 - offset) + index*4. */
#define BATTLE_DISPATCH_STATE4(index)                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb110u + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_STATE2_CLASS(index)                                    \
  PSX_PTR(const volatile Battle03Handler, 0x801eb124u + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_STATE2_EVENT(index)                                    \
  PSX_PTR(const volatile Battle03Handler, 0x801eb16cu + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_STATE2_FOLLOWUP(index)                                 \
  PSX_PTR(const volatile Battle03Handler, 0x801eb174u + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_DEFAULT_CLASS(index)                                   \
  PSX_PTR(const volatile Battle03Handler, 0x801eb17cu + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_PRESENTATION_BYTE3(index)                              \
  PSX_PTR(const volatile Battle03Handler, 0x801eb24cu + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_QUEUED_RESULT(index)                                   \
  PSX_PTR(const volatile Battle03Handler, 0x801eb270u + ((u32)(index) * 4u))

/* Offset/position tables for queued-slot rendering (func_801E8DD8). */
#define BATTLE_OFFSET_TABLE_0B10(a, b)                                         \
  PSX_REF(volatile s8, 0x801eb0b0u + ((u32)(a) * 2u) + ((u32)(b) * 8u))
#define BATTLE_ENEMY_OFFSET_TABLE_0B08(b)                                      \
  PSX_REF(volatile u8, 0x801eb108u + ((u32)(b) * 2u))
#define BATTLE_ENEMY_OFFSET_TABLE_0B09(b)                                      \
  PSX_REF(volatile u8, 0x801eb109u + ((u32)(b) * 2u))
#define BATTLE_ENEMY_OFFSET_U8_0B10(index)                                     \
  PSX_REF(volatile u8, 0x801eb710u + ((u32)(index) * 0x118u))
#define BATTLE_ENEMY_OFFSET_S8_0B12(index)                                     \
  PSX_REF(volatile s8, 0x801eb712u + ((u32)(index) * 0x118u))
#define BATTLE_CLASS_OFFSET_0C0CB(b)                                           \
  PSX_REF(volatile u8, 0x800e40cbu + ((u32)(b) * 0x88u))

/* Panel-task root pointer cell at 0x801485c8 (u8* volatile). */
#define BATTLE_PANEL_ROOT_CELL PSX_REF(u8* volatile, 0x801485c8u)
#define BATTLE_PANEL_ROOT_BASE PSX_PTR(u8* volatile, 0x80150000u)

/* Base-pointer accessors for fixed RAM regions (preserve lui+offset codegen). */
#define BATTLE_GLOBAL_RAM_U8  PSX_PTR(volatile u8, 0x80140000u)
#define BATTLE_GLOBAL_RAM_U16 PSX_PTR(volatile u16, 0x80140000u)
#define BATTLE_GLOBAL_RAM_U32 PSX_PTR(volatile u32, 0x80140000u)
#define BATTLE_HIGH_RAM_U8    PSX_PTR(volatile u8, 0x801f0000u)
#define BATTLE_HIGH_RAM_U16   PSX_PTR(volatile u16, 0x801f0000u)
#define BATTLE_HIGH_RAM_U32   PSX_PTR(volatile u32, 0x801f0000u)

/* Additional fixed-base tables and dispatch helpers. */
#define BATTLE_ENEMY_TABLE_6D8    PSX_PTR(volatile u16, 0x801eb6d8u)
#define BATTLE_MODE_TUPLE_62E0    PSX_PTR(volatile u8, 0x801462e0u)
#define BATTLE_TABLE_81B10        PSX_PTR(volatile u8, 0x80181b10u)
#define BATTLE_SCRIPT_TABLE_490D8 PSX_PTR(void, 0x801490d8u)
#define BATTLE_ROM_BASE_D0000     PSX_PTR(const u8, 0x801d0000u)
#define BATTLE_DISPATCH_SUBSTATE3(index)                                       \
  PSX_PTR(const volatile Battle03Handler, 0x801eb1e0u + ((u32)(index) * 4u))
#define BATTLE_SCRIPT_TABLE_492B8 PSX_PTR(void, 0x801492b8u)
#define BATTLE_SCRIPT_TABLE_0B00D PSX_PTR(const void, 0x801eb00du)

/* Single fixed-address globals (inline; no symbol binding required). */
#define BATTLE_GLOBAL_WORD_598C  PSX_REF(u32, 0x8014598cu)
#define BATTLE_GLOBAL_BYTE_62F0  PSX_REF(volatile u8, 0x801462f0u)
#define BATTLE_GLOBAL_BYTE_62F3  PSX_REF(u8, 0x801462f3u)
#define BATTLE_GLOBAL_BYTE_C303  PSX_REF(volatile u8, 0x801ec303u)
#define BATTLE_GLOBAL_HALF_EC2EE PSX_REF(volatile u16, 0x801ec2eeu)

#endif
