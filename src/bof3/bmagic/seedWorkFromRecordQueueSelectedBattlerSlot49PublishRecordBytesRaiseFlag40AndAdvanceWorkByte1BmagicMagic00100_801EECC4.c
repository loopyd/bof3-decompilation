#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 D_80146374; /* @source 0x80146374 @kind data */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC335[]; /* @source 0x801EC335 @kind data */
extern u8 D_801EC336[]; /* @source 0x801EC336 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C @kind function */
extern u32 func_800AFB48(u8 *record); /* @source 0x800AFB48 @kind function */
extern void func_800AF8F0(u8 *work); /* @source 0x800AF8F0 @kind function */

typedef struct Magic00100QueuedRecordHead {
  u32 words[29];
} Magic00100QueuedRecordHead;

/* @source 0x801EECC4
 * @behavior Work-byte-1 handler at entry 0 of the table at 0x801EEC04.
 *           Copies record byte 8 and words 0x34, 0x38 and 0x3C from the
 *           pointer at 0x801EB4E0 into the scratchpad work object, setting work
 *           byte B to 1 between the last two word stores. Claims one slot with
 *           0x801E590C(1, 0x49), truncates its index to a byte, and copies the
 *           selected battler record's 0x74-byte head to the 0x78-byte slot at
 *           0x801EC330. Selector 0x80146374 below 3 uses base 0x80145E90 and
 *           stride 0x140; otherwise it uses base 0x801EB630 and stride 0x118
 *           with selector minus 3. Stamps slot byte 6 to 1, bytes 1 and 2 to 0,
 *           byte 5 to 0x49 and byte 9 to 0, and publishes the work pointer in
 *           slot word 0x74. Stores the byte returned by 0x800AFB48(record) in
 *           work byte 0x27, copies record bytes 0x28 and 0x24, then calls
 *           0x800AF8F0(work), raises record byte 0 bit 0x40 and advances work
 *           byte 1. Does not check the allocator failure code. No arguments or
 *           return value; unestablished field and helper roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkFromRecordQueueSelectedBattlerSlot49PublishRecordBytesRaiseFlag40AndAdvanceWorkByte1BmagicMagic00100_801EECC4(void) {
  u8 *work;
  u8 *record;
  u32 slot;
  u32 offset;
  u32 word3C;
  u8 selector;

  D_1F800044[8] = D_801EB4E0[8];
  work = D_1F800044;
  *(u32 *)&work[0x34] = *(u32 *)&D_801EB4E0[0x34];
  *(u32 *)&work[0x38] = *(u32 *)&D_801EB4E0[0x38];
  word3C = *(u32 *)&D_801EB4E0[0x3C];
  work[0xB] = 1;
  *(u32 *)&work[0x3C] = word3C;
  slot = func_801E590C(1, 0x49);
  selector = D_80146374;
  if (selector < 3) {
    Magic00100QueuedRecordHead *src =
        (Magic00100QueuedRecordHead *)&D_80145E90[selector * 0x140];
    Magic00100QueuedRecordHead *dst =
        (Magic00100QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  } else {
    Magic00100QueuedRecordHead *src =
        (Magic00100QueuedRecordHead *)&D_801EB630[(selector - 3) * 0x118];
    Magic00100QueuedRecordHead *dst =
        (Magic00100QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78];
    *dst = *src;
  }
  offset = (u8)slot * 0x78;
  work = D_1F800044;
  D_801EC336[offset] = 1;
  D_801EC331[offset] = 0;
  D_801EC332[offset] = 0;
  D_801EC335[offset] = 0x49;
  D_801EC339[offset] = 0;
  record = D_801EB4E0;
  *(u32 *)&D_801EC3A4[offset] = (u32)work;
  D_1F800044[0x27] = (u8)func_800AFB48(record);
  D_1F800044[0x28] = D_801EB4E0[0x28];
  D_1F800044[0x24] = D_801EB4E0[0x24];
  func_800AF8F0(D_1F800044);
  D_801EB4E0[0] |= 0x40;
  D_1F800044[1]++;
}
