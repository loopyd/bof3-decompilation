#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); a bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared overlay state byte of the main executable, outside this payload. The
 * world02/area100/13 and world04/area171/13 bodies that seed a row of the same
 * record family raise it in front of the seed; this body raises it to 0x23, a
 * value no other lifted body stores.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* Front-end record table of the shared main executable: 20 rows of 0x74 bytes
 * based at 0x80143FC8, whose byte 0 is the occupancy flag and whose byte 5 is
 * the handler selector of the table dispatched at 0x801C7C70 (the shared
 * RecordSlot view in include/bof3/ui/game00_internal.h documents the same
 * offsets). The original re-materialises the `%hi` half of each field's own
 * symbol and adds the scaled row offset in place, so this body declares every
 * field base it writes separately and indexes each with one shared offset.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 1, the entry state byte of the record family; seeded with 0x17.
 * @source 0x80143FC9 @kind unknown
 */
extern u8 D_80143FC9[];
/* Row byte 2, seeded with 0x0A. No other lifted body writes this byte, so its
 * role stays described by offset and value.
 * @source 0x80143FCA @kind unknown
 */
extern u8 D_80143FCA[];
/* Row byte 5, the handler selector, seeded with 0x18.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];

/* Scenario progress byte of the main executable; the seed block runs only once
 * the story has reached value 0x0C. */
extern u8 g_ScenarioProgress;

/* Shared per-frame counter, incremented once for each displayed frame. */
extern u32 D_80143E6C;

/* Front-end record allocator of the main executable: returns the index of a
 * free row of the 0x74-byte table at 0x80143FC8, or 0xFF when no row is free.
 * This body does not test the full-table value and seeds the row regardless. */
extern u8 func_8019601C(void);

void func_801F4E70(u32 variant);
void func_801F49A0(u32 pass);
void func_801F482C(void);

/* @source 0x801F3C6C
 * @behavior Overlay step handler, entry 1 of the work-byte-2 handler table at
 *           0x801F5954. Once the shared scenario progress byte reads 0x0C it
 *           reads the work record published at the scratchpad pointer cell
 *           0x1F800044 and increments its byte-2 dispatch step, raises the
 *           shared overlay state byte 0x8014933B to 0x23, asks the front-end
 *           record allocator func_8019601C for a row index and seeds that
 *           0x74-byte row of the table at 0x80143FC8 with the occupancy flag 1
 *           at byte 0, handler selector 0x18 at byte 5, entry state 0x17 at
 *           byte 1 and 0x0A at byte 2. Every frame then reads the shared frame
 *           counter at 0x80143E6C and, when the counter is a multiple of five,
 *           calls the block drawer func_801F4E70 with the four-way variant
 *           (counter / 5) & 3, and always ends with func_801F49A0(0),
 *           func_801F49A0(1) and the four-block drawer func_801F482C. Takes no
 *           arguments, returns nothing and uses a 0x18-byte frame that holds
 *           only the saved return address at 0x10. The roles of record bytes 1
 *           and 2, of the 0x23 state value and of the three callees beyond their
 *           drawing effects stay described by offset and value.
 *
 * Entry: no jal in this payload names the body. The word 0x801F3C6C at
 * 0x801F5958 is entry 1 of the code-pointer run based at 0x801F5954, which the
 * dispatcher dispatchStateByte2Table5954World02Area10313_801F3BD0 indexes with
 * the work record's byte 2.
 *
 * Shape evidence: the tail from the shared frame counter through the epilogue is
 * word-for-word the same as the exact sibling
 * rampWorkYPositionDrawOnFifthTickAndAdvanceWorkByte2At19World02Area10313_801F3E10
 * once the absolute addresses are shifted by their 0x5A4 gap, so the statement
 * order and operand forms of that tail are that sibling's. The seed block's
 * declaration style and `(u32)index * 0x74u` offset spelling come from the exact
 * sibling
 * seedFrontEndRecordHandler37AtFixedPositionQueueCue202RaiseFlagBank20To22ByPendingWorldElseBumpCountdown7EWorld02Area09613_801F3188,
 * whose row-store sequence this body keeps with its own literals. A byte compare
 * of this 0xFC span against all 396 extracted payloads under out/binaries finds
 * it only at its own offset, so no byte-identical twin exists to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordHandler18AndRaiseSharedOverlayStateByte23OnScenarioProgress0cThenDrawFourBlocksWorld02Area10313_801F3C6C(
    void) {
  u8* work;
  u8 index;
  u32 offset;
  u32 frame;

  if (g_ScenarioProgress == 0xC) {
    work = D_1F800044;
    work[2]++;
    D_8014933B = 0x23;
    index = func_8019601C();
    offset = (u32)index * 0x74u;
    D_80143FC8[offset] = 1u;
    D_80143FCD[offset] = 0x18u;
    D_80143FC9[offset] = 0x17u;
    D_80143FCA[offset] = 0x0Au;
  }
  frame = D_80143E6C;
  if (frame % 5 == 0) {
    func_801F4E70((frame / 5) & 3);
  }
  func_801F49A0(0);
  func_801F49A0(1);
  func_801F482C();
}
