#ifndef BOF3_CONTEXT_GAME_00_PROTOTYPES_H
#define BOF3_CONTEXT_GAME_00_PROTOTYPES_H

/* Known function prototypes for the GAME_e00 overlay. */

/* Already-lifted functions (from internal.h) */
void func_801960c0(u8 record_index);
void func_8014ba04(void);
void func_80158e50(void);
void func_80158c80(void);
void func_80198cac(void);
void func_801c1400(u32 mode);
void func_8019fa28(u16 selection_seed, u32 context_a, u32 context_b, u8 context_kind);
void func_8014e284(void);
void emi_stream_init_slot(u32 slot_id);
void func_801a7804(void);
void func_801a782c(void);
void func_801992b8(void);
u8*  func_801af270(u8 sprite_id, u8 flags);
void func_801af2a0(s16 x, s16 y, u8 sprite_id, u8 flags);
void func_801af390(s16 base_x, s16 base_y, const u8* record_table, u8 flags);
s16  func_80154f28(s32 x, s32 y);
u8   func_8014d978(void);

/* GAME_e00 overlay functions (not yet lifted) called internally */
s32  func_801a1bc0(void);
s32  func_801bde14(s32 arg0, s32 arg1, u8 arg2);
s32  func_801be0c0(s32 arg0, s32 arg1, u8 arg2);
void func_8019601c(void);

/* Main exe functions called by overlay */
s16  func_80154f28(s32 x, s32 y);
u8   func_8014d978(void);
void func_8015477c(s32 a, s32 b);

#endif
