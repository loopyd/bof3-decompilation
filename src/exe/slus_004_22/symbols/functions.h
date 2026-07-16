#ifndef SLUS_004_22_SYMBOLS_FUNCTIONS_H
#define SLUS_004_22_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

typedef void (*GameCallbackEntry)(void);

/* Shared executable entry points. */
void* func_800DF548(s32 item_type, s32 item_index);

/* LOGO.EXE is loaded independently; this call targets its reviewed entry
 * address rather than linking LOGO.EXE implementation into SLUS_004.22. */
void func_801CE758(void);

/* SLUS startup, callback scheduler, and executable-file loading. */
void func_8014AA04(void);
void func_8014AAC8(void);
void func_8014ACA0(void);
void func_8014AD28(void);
void func_8014AE08(void);
void func_8014AE9C(u8* work);
void func_8014B17C(void);
void func_8014AEE0(void);
void func_8014AFC0(void);
void func_8014B33C(void);
void func_8014B6B4(void);
void func_8014E22C(void);
s32  func_8014E0FC(const char* path);
void func_8014E564(s16 x, s16 y, s16 width, s16 height);
void func_8014E6D0(void);
void func_8014EA80(void);
void func_8015CEBC(void);
void func_8015D044(void);

/* Newly discovered SLUS services — pending decompilation. */
void func_8014B87C(u16 countdown);
void func_8014E5A0(u32 ot_index, u32 primitive_size);
void func_8014F514(void);
void func_8014F704(void);
void func_80150098(s16 x, s16 y, u32 clut, const u8* text);
void func_8015DF18(void);
s32  func_801655F4(u8* counter, s32 delta);

/* EMI loader and CD callback path. */
void    func_80161F58(void);
DiscLba func_80162160(EmiLoaderSlotId slot_id);
void    func_80162178(void);
void    func_801621E8(s32 status, u8* result);
void    func_80162230(u8 status, u8* result);
void    func_80162500(void);
void    func_801625E4(void);
void    func_80162618(void);
void    func_80162698(void);
void    func_80162790(void);
void    func_80162898(void);
void    func_801629F0(void);
void    func_80162A6C(void);
s32     func_80162B08(u8 slot);
void    func_80162C14(void);
void    func_80162CD8(void);
void    func_80162D18(void);
void    func_80163010(void);

/* Remaining reviewed SLUS services, kept address-traceable pending promotion. */
void func_8016728C(u8 index, u8 family);
void SpuSetTransferMode(s32 arg);
void func_8016AD2C(s32 owner);
void SsUtAllKeyOff(s32 arg);
void SsVabClose(s32 resource_id);
s16  SsVabOpenHeadSticky(u32 source, s32 resource_id, u32 destination);
void func_8017B8D4(void* arg0, s32 arg1);
void func_8017BA40(void* arg0);
void func_8017BC98(void* arg0);
/* Semantic aliases preserve address-based names for analyzer and matching
 * tools while making reviewed call sites readable. */
#define emi_cd_sync_callback  func_801621E8
#define emi_cd_ready_callback func_80162230
#define emi_loader_initialize func_80161F58
#define emi_loader_slot_lba   func_80162160

#endif
