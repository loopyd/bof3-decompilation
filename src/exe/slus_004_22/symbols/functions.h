#ifndef BOF3_SLUS_004_22_SYMBOLS_FUNCTIONS_H
#define BOF3_SLUS_004_22_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

typedef void (*Bof3CallbackEntry)(void);

void* func_800df548(s32 item_type, s32 item_index);

/* LOGO.EXE is loaded independently; this call targets its reviewed entry
 * address rather than linking LOGO.EXE implementation into SLUS_004.22. */
void func_801ce758(void);

void func_8014aa04(void);
void func_8014aac8(void);
void func_8014aca0(void);
void func_8014ad28(void);
void func_8014ae08(void);
void func_8014ae9c(u8* work);
void func_8014aee0(void);
void func_8014afc0(void);
void func_8014b020(void);
void func_8014b0f0(void);
void func_8014b33c(void);
void func_8014b6b4(void);
void func_8014b73c(void);
void func_8014e22c(void);
s32  func_8014e0fc(const char* path);
void func_8014e564(s16 x, s16 y, s16 width, s16 height);
void func_8014e6d0(void);
void func_8014ea80(void);
void func_8015cebc(void);
void func_8015d044(void);
void func_80161f58(void);
void func_80161fdc(u32 slot_id);
u32  func_80162160(u32 slot_id);
void func_80162178(void);
void func_801621e8(u8 status, u8* result);
void func_80162230(u8 status, u8* result);
void func_80162500(void);
void func_801625e4(void);
void func_80162618(void);
void func_80162698(void);
void func_80162790(void);
void func_80162898(void);
void func_801629f0(void);
void func_80162a6c(void);
s32  func_80162b08(u8 slot);
void func_80162c14(void);
void func_80162cd8(void);
s32  func_80162d00(void);
void func_80162d18(void);
void func_80163010(void);
void func_8016728c(u8 index, u8 family);
void func_801690b8(s32 arg);
void func_8016ad2c(s32 owner);
void func_8016debc(s32 arg);
void func_80173818(s32 resource_id);
s16  func_80173c50(u32 source, s32 resource_id, u32 destination);
void func_8017b2d4(void* arg0);
void func_8017b330(s32 arg0);
void func_8017b3cc(s32 arg0);
void func_8017b8d4(void* arg0, s32 arg1);
void func_8017b9cc(void* arg0);
void func_8017ba40(void* arg0);
void func_8017bc98(void* arg0);
void func_8017e3d4(void);
s32  func_8017ed9c(Bof3CallbackEntry callback, u32 open_arg, u32 open_arg_2);
s32  func_8017edac(s32 thread_id);

/* Semantic aliases preserve address-based names for analyzer and matching
 * tools while making reviewed call sites readable. */
#define emi_cd_sync_callback  func_801621e8
#define emi_cd_ready_callback func_80162230
#define emi_loader_initialize func_80161f58
#define emi_loader_slot_lba   func_80162160

#endif
