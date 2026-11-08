#include "bof3/bof3.h"

/* The main executable's field-record table: the exact battle lifts index the
 * base 0x80144968 with a record stride of 0xA4 bytes, so one record is forty-one
 * words. The byte at 0x80181B11 selects one of those records. */
typedef struct Boss01316FieldRecord {
  u32 words_00[41];
} Boss01316FieldRecord;

extern Boss01316FieldRecord D_80144968[]; /* @source 0x80144968 @kind unknown */

/* Record index of local battler slot 1. The original spells the address as a
 * byte of its own, one above the D_80181B10 index array the battle and shop
 * lifts use, so this reference keeps that address instead of indexing the
 * array. */
extern u8 D_80181B11; /* @source 0x80181B11 @kind data */

/* The shared name slot the bounded string copy at 0x801501E4 writes into; the
 * equipment-name lift copies record names here with its own bounded count. */
extern u8 D_801490D8[]; /* @source 0x801490D8 @kind data */

/* Overlay step countdown byte this payload's step handler decrements and fires
 * on; publishBattleHooksAndSeedStepCountdownBossBoss01316_800C20D8 seeds it and
 * func_800C1E90, entry 0 of the handler run at 0x800C26D4, decrements it. */
extern u8 D_800C26E0; /* @source 0x800C26E0 @kind data */

/* Interior word of the payload data row D_800C25D8 (its +0x24 word), the row
 * the overlay step setup publishes into the same work-record word this function
 * overwrites, so it is referenced as its own data symbol rather than as a row
 * start. */
extern u8 D_800C25FC; /* @source 0x800C25FC @kind data */

/* Shared battle flag byte four above the battle flag halfword 0x801462E8; the
 * sibling lifts of this overlay test its bit 0x40 and set its bits 0x2 and 0x8. */
extern u8 D_801462ED; /* @source 0x801462ED @kind unknown */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad work-record pointer cell. The original re-reads the cell before
 * each access because the stores through the loaded record may alias the cell,
 * so each access names the symbol instead of keeping a pointer in a local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* The work record's +0xEC word, the data-row pointer the overlay step setup at
 * 0x800C1D08 publishes and this function republishes one row further in. */
typedef struct Boss01316WorkTail {
  u8 unk_00[0xEC];
  void *data_row;
} Boss01316WorkTail;

/* Shared main-executable services, each bound by this target's generated
 * undefined_funcs_auto row: 0x801D9484 initializes the battle UI bundle slot 0,
 * 0x801501E4 is the bounded string copy, 0x801502D0 resolves a cue texture id,
 * 0x801DE560 queues an event row, 0x801C1254 is the shared battle transition
 * helper and 0x8014D6B8 requests a resource by id. */
void func_801D9484(void); /* @source 0x801D9484 */
void func_801501E4(void *destination, void *source, u32 count); /* @source 0x801501E4 */
u32 func_801502D0(u32 texture_id); /* @source 0x801502D0 */
void func_801DE560(u8 kind, u8 arg1, u8 arg2, u8 row, u32 texture); /* @source 0x801DE560 */
void func_801C1254(u32 arg0, u32 arg1); /* @source 0x801C1254 */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* @source 0x800C1F24
 * @behavior Entry 1 of this payload's handler run at 0x800C26D4, the run
 * dispatchIndexedHandlerBossBoss01316_800C1E58 scales a byte index into: it
 * initializes the battle UI bundle slot 0, copies eight bytes of the field
 * record selected by the local-battler byte 0x80181B11 into the shared name slot
 * 0x801490D8, resolves texture 0x25 through 0x801502D0 and queues it as event
 * row kind 2 with halfword 0x2D, clears the overlay step countdown byte
 * 0x800C26E0, runs the shared transition helper 0x801C1254 with 1 and 1, stores
 * 4 into byte 0x29 of the work record published at the scratchpad pointer cell
 * 0x1F800044, republishes the in-row pointer 0x800C25FC (0x800C25D8 + 0x24) into
 * the +0xEC data-row word of the current enemy work record from 0x801EB4E8,
 * requests resource (work byte +8) + 4 through 0x8014D6B8 and raises bit 0x40 of
 * the shared flag byte 0x801462ED. No jal in the payload names this function;
 * only the word at 0x800C26D8 holds its address, so the main program reaches it
 * through that run. The function shares its whole statement sequence with the
 * countdown-expiry branch at 0x800C2220 of the asm hook 0x800C2138, which adds
 * the enemy-record publication and reorders the 0x29 store behind the +0xEC
 * store, so that branch is shape evidence only. The 0x20 byte prologue frame
 * keeps the return address at 0x18 and the queued texture in the 0x10 argument
 * slot, the plain return path with the stack restore in its delay slot places
 * the boundary, and the 0xE0 byte row reaches the inert handler at 0x800C2004.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initBattleUiCopyBattlerNameQueueEvent2dWithTexture25ClearCountdownSetWorkByte29To4RaiseFlag40BossBoss01316_800C1F24(
    void) {
  Boss01316WorkTail *work;
  u32 texture;

  func_801D9484();
  func_801501E4(D_801490D8, &D_80144968[D_80181B11], 8u);
  texture = func_801502D0(0x25u);
  func_801DE560(2u, 0u, 0u, 0x2du, texture);
  D_800C26E0 = 0;
  func_801C1254(1u, 1u);
  D_1F800044[0x29] = 4;
  work = (Boss01316WorkTail *)D_801EB4E8;
  work->data_row = &D_800C25FC;
  func_8014D6B8((u8)(D_1F800044[8] + 4));
  {
    u8 *flags = &D_801462ED;
    *flags |= 0x40;
  }
}
