#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;
/* @source 0x8017F974 @kind table */
extern void* D_8017F974[];
/* One record of the shared 0x74-byte front-end entry table at 0x80143FC8, the
 * table whose head bytes 0x00-0x05 mark a claimed entry. Only the four bytes
 * this overlay writes are named. */
typedef struct Area12114FrontEndRecord {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05;
  u8 pad_06[4];    /* 0x06 - 0x09 */
  u8 unk_0A;
  u8 pad_0B[0x69]; /* 0x0B - 0x73 */
} Area12114FrontEndRecord;
/* @source 0x80143FC8 @kind table */
extern Area12114FrontEndRecord D_80143FC8[];

void func_8015AC8C(u8* arg0);
u32 func_80166140(u32 arg0);
s32 func_8019601C(void);
void publishSharedPositionWordsToWorkThenDrawMeterWorld03Area12114_801F5C7C(
    void);

/* @source 0x801F5CD0
 * @behavior Byte-1 state handler 0 of the byte-1 handler table 0x801F68A4 for
 * the work record published at the scratchpad pointer cell 0x1F800044, the
 * entry state of that four-handler group (1 is 0x801F5E2C, 2 is 0x801F5FF8,
 * 3 is 0x801F6068). It copies the shared mode byte 0x80145E98 into work byte
 * 0x08, so the record facing matches the mode before the group turns, and
 * releases the world record's bound sub-object: it indexes the world record
 * table 0x8017F974 with the shared world/state id 0x80143F00, takes the word
 * at that record offset 0x08, adds 8 and passes it to the resident helper
 * 0x8015AC8C. It then clears the work tint triplet at 0x5D/0x5E/0x5F to -0x80
 * through the released pointer, advances work byte 0x01 so the next frame
 * dispatches handler 1, and runs the shared position publish and meter draw at
 * 0x801F5C7C on the same frame. On the frames where the shared world/state id
 * reads 0x68 it first asks the shared selector 0x80166140 for id 0xA and
 * returns early when that answers zero; otherwise it lays out four front-end
 * entries of the 0x74-byte record table 0x80143FC8, taking one record index per
 * iteration from 0x8019601C, and stores byte 0x00 = 1, byte 0x05 = 0x5C,
 * byte 0x01 = 3 and byte 0x0A = the iteration index times 8. The scratchpad
 * pointer cell is re-read before every store group, so each store keeps its own
 * cell load. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armMeterStateFromModeByteClearTintAdvanceAndSeedFourEntriesWorld03Area12114_801F5CD0(
    void) {
  u8 index;
  s32 i;

  area12114WorkCursor_1F800044[8] = D_80145E98;
  func_8015AC8C((u8*)(*(u32*)((u8*)D_8017F974[D_80143F00] + 8) + 8));
  ((s8*)area12114WorkCursor_1F800044)[0x5D] = -0x80;
  ((s8*)area12114WorkCursor_1F800044)[0x5E] = -0x80;
  ((s8*)area12114WorkCursor_1F800044)[0x5F] = -0x80;
  area12114WorkCursor_1F800044[1] += 1;
  publishSharedPositionWordsToWorkThenDrawMeterWorld03Area12114_801F5C7C();
  if (D_80143F00 == 0x68) {
    if (func_80166140(0xA) == 0) {
      return;
    }
  }
  for (i = 0; i < 4; i++) {
    index = func_8019601C();
    D_80143FC8[index].unk_00 = 1;
    D_80143FC8[index].unk_05 = 0x5C;
    D_80143FC8[index].unk_01 = 3;
    D_80143FC8[index].unk_0A = i * 8;
  }
}
