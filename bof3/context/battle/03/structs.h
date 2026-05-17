#ifndef BOF3_CONTEXT_03_STRUCTS_H
#define BOF3_CONTEXT_03_STRUCTS_H

/* struct, typedef, and type definitions */

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
#endif
