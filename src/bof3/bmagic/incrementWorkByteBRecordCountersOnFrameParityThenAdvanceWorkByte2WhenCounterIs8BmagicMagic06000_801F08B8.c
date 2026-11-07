#include "bof3/bof3.h"

/* Shared frame counter in main RAM, outside this overlay image. The exact
 * sibling advanceWorkByteAOnFrameParityInvokePointHelperCountdownWorkByte9
 * DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4 reads the same word
 * and treats its low bit as a per-frame phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays,
 * bound at 0x1F800044 by this target map. The original re-materializes the cell
 * (lui $v0,0x1F80 / lw $v0,0x44) once per access, so each of the four groups
 * below reads it again through this named cell instead of holding one address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Byte views of the three counter bytes this handler advances. The counters sit
 * at record offsets 0x2, 0x3 and 0x4 of the 12-byte records based at 0x80145BD4,
 * and the original reaches the second and third byte through their own addresses
 * (0x80145BD7 and the reviewed symbol 0x80145BD8). The increments stay byte
 * arrays with a computed byte offset, not record fields: the record-typed form
 * of the same three statements reserves a 16-byte frame that this leaf does not
 * have, while the byte arrays emit the original lui $at / addu $at,$at,$v1 pair
 * around each counter's load and store. The exact sibling
 * incrementWorkByteARecordCountersThenDecrementWorkByte9AndAdvanceWorkByte2On
 * ZeroBmagicMagic21303_801EF23C advances this table in the same form. */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* Record view of the same table, needed for the signed tail compare: the
 * original reloads the first counter with lb, so the compare keeps the sign of
 * the stored byte rather than the zero-extended lbu the increments use. Bytes
 * 0x2-0x4 of the 12-byte record are the counters and the payload gives no role
 * for the leading two bytes or the trailing seven. */
typedef struct BmagicMagic06000CounterRecord {
  u8 unk_00[2];
  s8 counter_02;
  s8 counter_03;
  s8 counter_04;
  u8 pad_05[7];
} BmagicMagic06000CounterRecord;

extern BmagicMagic06000CounterRecord D_80145BD4[]; /* @source 0x80145BD4 @kind table */

/* @source 0x801F08B8
 * @behavior Per-frame counter step, index 1 (the second pointer) of the
 *           three-entry handler run at 0x801F0BDC that the work-byte-2
 *           dispatcher resolves, so the work object byte 0x2 holds 1 while this
 *           body runs. When the low bit of the shared frame counter at
 *           0x80143E6C is set it advances the three counter bytes of the
 *           12-byte counter record selected by the work object byte 0xB: each
 *           group re-reads the scratchpad pointer cell at 0x1F800044 and that
 *           work byte into a byte offset (idx*12 as idx*3*4), then increments
 *           the selected byte through the address temporary the original spells
 *           as lui $at,%hi / addu $at,$at,$v1 around an lbu and an sb. The
 *           groups therefore count in pairs of frames rather than every frame.
 *           It then re-reads the cell and the work byte 0xB and, when the
 *           selected record byte at offset 0x2 equals 8, increments the work
 *           object byte 0x2 with a byte-width add and advances the work byte 2
 *           dispatcher to its next handler; that compare loads with lb, so it
 *           reads the counter through the signed record view while the
 *           increments above use the unsigned byte views. Leaf with no frame,
 *           no saved registers and no calls; takes no arguments, returns
 *           nothing and reads or writes no other state. No payload in the
 *           repository holds a byte-identical copy of these 296 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBRecordCountersOnFrameParityThenAdvanceWorkByte2WhenCounterIs8BmagicMagic06000_801F08B8(void) {
  u32 offset;

  if (D_80143E6C & 1u) {
    offset = g_battle_work[0xB] * 12;
    D_80145BD6[offset]++;
    offset = g_battle_work[0xB] * 12;
    D_80145BD7[offset]++;
    offset = g_battle_work[0xB] * 12;
    D_80145BD8[offset]++;
  }
  if (D_80145BD4[g_battle_work[0xB]].counter_02 == 8) {
    g_battle_work[2]++;
  }
}
