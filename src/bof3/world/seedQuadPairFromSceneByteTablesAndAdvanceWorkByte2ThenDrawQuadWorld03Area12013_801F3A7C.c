#include "bof3/bof3.h"

/* Typed view of the scratchpad work record this overlay publishes at the
 * pointer cell 0x1F800044, limited to the fields this handler writes: the row
 * byte the quad drawer clears, the counter byte it advances, the word that
 * holds the record pointer and the two words whose upper halfwords are the
 * quad corner pair the drawer reads at 0x36 and 0x3A. The fields carry the
 * offsets only; the pointer word and the pair words are plain words, which is
 * how the original stores them. */
typedef struct Area12013WorkRecord {
  u8 pad_00[9];
  u8 byte_09;
  u8 byte_0A;
  u8 pad_0B;
  u32 ptr_0C;
  u8 pad_10[0x24];
  u32 pair_34;
  u32 pair_38;
} Area12013WorkRecord;

/* Scratchpad work-record pointer cell this overlay publishes and every record
 * handler of the target reads. The original materialises the cell with `lui`
 * plus an offset load for every access, so the body reloads it where the
 * original does instead of keeping the pointer live.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* First of the two scene byte tables this target keeps packed in its own
 * payload: entry n of a pair occupies the two bytes at 0x801F4404 + 2n here and
 * 0x801F4405 + 2n in the declaration below. The body reads the two bytes of the
 * entry the scene index selects, so both stay byte views at their own
 * addresses.
 * @source 0x801F4404 @kind data
 */
extern u8 D_801F4404[];

/* Second byte of the same scene table pair, read at 0x801F4405 + 2n.
 * @source 0x801F4405 @kind data
 */
extern u8 D_801F4405[];

/* Base of the 0x118-byte-stride record table in the main executable, the same
 * battle work band the boss targets bind at this address. It sits outside this
 * payload, so the address stays this target's own binding; the body only takes
 * the entry the scene index selects and publishes its address through the work
 * record.
 * @source 0x801EB748 @kind unknown
 */
extern u8 D_801EB748[];

/* Quad drawer of this target at 0x801F3F5C, lifted alongside this handler, so
 * only its prototype is declared here. It advances work record byte 0x0A by its
 * third argument and builds a two-pass textured quad from the work halfwords
 * 0x36 and 0x3A, the pair this handler has just written. */
void drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
    u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* @source 0x801F3A7C
 * @behavior Step 0 of the work-byte-2 handler run this overlay dispatches
 *           through the local table at 0x801F43E8, whose first pointer word
 *           names this body. It reads the signed halfword 0x36 of the work
 *           record published through pointer cell 0x1F800044 as a scene index
 *           and seeds the record from it: the byte pair that index selects at
 *           0x801F4404 + 2n and 0x801F4405 + 2n becomes the quad corner pair in
 *           the record halfwords 0x36 and 0x3A, each written as a 32-bit store
 *           of the byte shifted left sixteen, which clears the halfword below
 *           it; work word 0x0C takes the address of entry n of the
 *           0x118-byte-stride record table at 0x801EB748, the battle work band
 *           of the main executable; and work bytes 0x09 and 0x0A, the row the
 *           drawer clears and the counter it advances, start at zero. The
 *           post-increment of work byte 0x02 then moves the run's own
 *           dispatcher to step 1, the handler at 0x801F3B28, so this body runs
 *           once per entry into the run. It closes by calling the quad drawer
 *           at 0x801F3F5C with the fixed arguments (0x80, 0x40, 1, 0). Takes no
 *           arguments and returns nothing; its 0x18-byte frame holds only the
 *           saved return address at 0x10, and bytes 0x0A and 0x02 are read back
 *           through fresh loads of the pointer cell, which the source keeps as
 *           separate uses. The roles of the scene index beyond selecting the
 *           table entry, of the table's byte values and of the record band at
 *           0x801EB748 stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedQuadPairFromSceneByteTablesAndAdvanceWorkByte2ThenDrawQuadWorld03Area12013_801F3A7C(
    void) {
  Area12013WorkRecord* record;
  s16 scene;

  record = (Area12013WorkRecord*)D_1F800044;
  scene = *(s16*)((u8*)record + 0x36);
  record->pair_34 = (u32)D_801F4404[scene * 2] << 16;
  record->pair_38 = (u32)D_801F4405[scene * 2] << 16;
  record->ptr_0C = (u32)(D_801EB748 + scene * 280);
  record->byte_09 = 0;
  D_1F800044[0x0A] = 0;
  D_1F800044[2]++;
  drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
      0x80, 0x40, 1, 0);
}
