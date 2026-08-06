#ifndef SLUS_004_22_SYMBOLS_FUNCTIONS_H
#define SLUS_004_22_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

typedef void (*GameCallbackEntry)(void);

/* Runtime equipment-record dispatch. */
void* getEquipRecordBase(s32 item_type, s32 item_index);

/* LOGO.EXE is loaded independently; this call targets its reviewed entry
 * address rather than linking LOGO.EXE implementation into SLUS_004.22. */
void func_801CE758(void);

/* SLUS startup, callback scheduler, and executable-file loading. */
void bootNoop(void);
void bootMainLoop(void);
void initBootRuntime(void);
void initBootDiscEvents(void);
void initBootDisplayEnvs(void);
void clearBootOtEntry(u8* work);
void captureBootVsync(void);
void runLogoExe(void);
void func_8014AFC0(void);
void dispatchCallbackSlots(void);
void func_8014B6B4(void);
void func_8014E22C(void);
s32  func_8014E0FC(const char* path);
void clearRenderRect(s16 x, s16 y, s16 width, s16 height);
void func_8014E6D0(void);
void func_8014EA80(void);
void func_8015CEBC(void);
void func_8015D044(void);

/* Newly discovered SLUS services — pending decompilation. */
void yieldCallbackSlotScheduler(u16 countdown);
void appendRenderPrim(u32 ot_index, u32 primitive_size);
void fadeLoop(s32 a0, s32 a1, s32 a2);
u8   drawFadeTile(s16* value, s32 arg, u8 arg2, u8 arg3, u8 arg4);
void func_80150098(s16 x, s16 y, u32 clut, const u8* text);
void dispatchSoundCue(u32 cue_id);
s32  func_801655F4(u8* counter, s32 delta);

/* EMI loader and CD callback path. */
void    initEmiLoader(void);
DiscLba emiLoaderSlotLba(EmiLoaderSlotId slot_id);
void    beginEmiLoaderTransfer(void);
void    emiCdSyncCallback(s32 status, u8* result);
void    emiCdReadyCallback(u8 status, u8* result);
void    validateEmiLoaderHeader(void);
void    copyEmiType0Payload(void);
void    recordEmiDispatchHandler(void);
void    recordEmiPackedDispatch(void);
void    selectNextEmiEntry(void);
void    startEmiEntryTransfer(void);
void    selectPrimaryEmiDestination(void);
void    selectAlternateEmiDestination(void);
s32     stageEmiTransferSlot(u8 slot);
void    copyEmiTransferChunk(void);
void    selectEmiLoaderMode6(void);
void    dispatchEmiModeCallback(void);
void    func_80163010(void);

/* Remaining reviewed SLUS services, kept address-traceable pending promotion. */
void func_8016728C(u8 index, u8 family);
void SpuSetTransferMode(s32 arg);
void func_8016AD2C(s32 owner);
void SsUtAllKeyOff(s32 arg);
void SsVabClose(s32 resource_id);
s16  SsVabOpenHeadSticky(u32 source, s32 resource_id, u32 destination);
/* Semantic aliases preserve address-based names for analyzer and matching
 * tools while making reviewed call sites readable. */

#endif
