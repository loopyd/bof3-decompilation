#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell of the overlay run. Spelled as the named
 * fixed address rather than through the constant-address accessor, because the
 * original re-emits `lui $vN,0x1F80` plus the folded `lw $vN,68($vN)` at each
 * record read, including the two loop tests. The `D_<ADDRESS>` spelling binds
 * with no map row.
 * @source 0x1F800044 @kind unknown
 */
extern u8* D_1F800044;

/* Shape-rectangle table of this overlay: five records of four bytes each, and
 * the payload handler table at 0x801F47E8 begins where the fifth record ends.
 * The work record's signed halfword at +0x36 selects the record, scaled by
 * four, and record byte 0 is the first column, byte 1 the first row, byte 2 the
 * last column and byte 3 the last row. The body materialises each column with
 * its own `lui` of that address, so the four columns stay separate bindings
 * instead of one wider record view.
 * @source 0x801F47D4 @kind table
 */
extern u8 D_801F47D4[];
/* @source 0x801F47D5 @kind table */
extern u8 D_801F47D5[];
/* @source 0x801F47D6 @kind table */
extern u8 D_801F47D6[];
/* @source 0x801F47D7 @kind table */
extern u8 D_801F47D7[];

/* Resident map block of the main executable. The body reads the row stride from
 * the block base itself, which it materialises once as a pointer, and then the
 * two header cells below at their fixed addresses: `lbu` of the byte at
 * 0x80104001 and `lhu` of the halfword at 0x80104002.
 * @source 0x80104001 @kind data
 */
extern u8 D_80104001;
/* @source 0x80104002 @kind data */
extern u16 D_80104002;

/* Shared texture-record table of the main executable; the body offsets its
 * 0x50-byte record entries by the draw-buffer index byte 0x80143D44 scaled by
 * 0x28.
 * @source 0x8012D880 @kind table
 */
extern u8 D_8012D880[];
/* @source 0x80143D44 @kind data */
extern u8 D_80143D44;

/* Shared cell-record resolver and texture attacher of the main executable, both
 * outside this payload, so this target's generated undefined_funcs_auto.txt
 * binds their addresses and no map row is owed. */
/* @source 0x801558F0 */
extern s32 func_801558F0(s32 x, s32 y);
/* @source 0x80155560 */
extern void func_80155560(u32 command, void* state, s32 mode);
/* Shared work-area reset of the main executable. */
/* @source 0x80196070 */
extern void func_80196070(void);

/* @source 0x801F2FE4
 * @behavior State-2 handler of the work-record state-byte-2 dispatch run of this
 *           overlay: the payload handler table at 0x801F47E8 holds this address
 *           at word 2, so the body has no `jal` caller. It increments byte +9 of
 *           the work record published through the scratchpad pointer cell
 *           0x1F800044, re-reads that byte and clamps it to 16, then walks the
 *           shape table at 0x801F47D4 with the stride-four index of the record
 *           signed halfword at +0x36. A shape record is a map rectangle, so the
 *           outer loop covers its rows and the inner loop its columns. For each
 *           cell the body reads the resident map block at 0x80104000: the row
 *           stride byte at that base, the byte at 0x80104001 and the halfword at
 *           0x80104002. It takes the halfword grid entry at the block base plus
 *           0x80104002 * 4 plus row * stride * 2 plus column * 2 and addresses
 *           the cell command word at the block base plus
 *           ((stride * 0x80104001 + 1) >> 1) * 4 plus 0x80104002 * 4 plus four
 *           times that grid entry. Into bits 19 to 23 of that word it stores the
 *           clamped step count and keeps every other bit, then resolves the cell
 *           record with 0x801558F0 and, when that record is nonzero, hands the
 *           command word and the record's 0x50-byte entry of 0x8012D880 offset
 *           by 0x80143D44 * 0x28 to the texture helper 0x80155560 with mode 1.
 *           Both loop tests re-read the scratchpad pointer cell and the record
 *           halfword, because a store through the loaded cell pointer may alias
 *           the cell. After the walk, when record byte +9 reads 0x12 or more, the
 *           body calls the shared work-area reset 0x80196070. Takes no arguments
 *           and returns nothing; its 0x28-byte frame keeps ra and the five saved
 *           registers, and the first record-pointer load precedes the frame
 *           creation.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stampShapeAttachEighteen(
    void) {
  u8* mapWork;
  u8* work;
  u32 areaBase;
  u32 biasBase;
  u32 cellOffset;
  u32* entry;
  s32 record;
  s32 col;
  s32 row;
  s32 level;
  s32 size;
  s32 shapeOffset;

  D_1F800044[9]++;
  level = D_1F800044[9];
  size = 0x10;
  if ((u32)level < 0x11) {
    size = level;
  }

  work = D_1F800044;
  shapeOffset = FIELD_REF(s16, work, 0x36) * 4;
  for (row = D_801F47D5[shapeOffset]; row <= D_801F47D7[shapeOffset]; row++) {
    mapWork = PSX_PTR(u8, 0x80104000);
    shapeOffset = FIELD_REF(s16, work, 0x36) * 4;
    for (col = D_801F47D4[shapeOffset]; col <= D_801F47D6[shapeOffset]; col++) {
      areaBase = ((mapWork[0] * D_80104001 + 1) >> 1) * 4 + (u32)mapWork;
      biasBase = D_80104002 * 4;
      cellOffset = col * 2 + (biasBase + row * mapWork[0] * 2);
      biasBase += areaBase;
      entry = (u32*)(biasBase + PSX_REF(u16, cellOffset + (u32)mapWork) * 4);
      *entry = (*entry & 0xFF07FFFF) | (size << 19);
      record = func_801558F0(col, row);
      if (record != 0) {
        u8* textureBase;
        record *= 0x50;
        textureBase = D_8012D880 + D_80143D44 * 0x28;
        func_80155560(*entry, PSX_PTR(u8, record + (u32)textureBase), 1);
      }
      shapeOffset = FIELD_REF(s16, D_1F800044, 0x36) * 4;
    }
    work = D_1F800044;
    shapeOffset = FIELD_REF(s16, work, 0x36) * 4;
  }

  if (D_1F800044[9] >= 0x12) {
    func_80196070();
  }
}
