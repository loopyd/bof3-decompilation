#include "bof3/bof3.h"

/* First coordinate of the shared view-origin mirror pair of the main
 * executable. This body reads it as a 16.16 fixed-point position and splits it
 * into the scroll byte and the render-cell row coordinate.
 * @source 0x80149308 @kind data
 */
extern s32 D_80149308;

/* Second coordinate of the shared view-origin mirror pair.
 * @source 0x8014930C @kind data
 */
extern s32 D_8014930C;

/* Shared scroll word derived from D_80149308.
 * @source 0x80149310 @kind data
 */
extern u32 D_80149310;

/* Shared scroll word derived from D_8014930C.
 * @source 0x80149314 @kind data
 */
extern u32 D_80149314;

/* Shared signed view step of the main executable, read here only when the
 * map-header halfword at 0x8010401E is zero.
 * @source 0x80149318 @kind data
 */
extern s32 D_80149318;

/* Shared reference halfwords of the render-cell mapping: the cell coordinates
 * the mapped-position helper subtracts from the high half of a 16.16 position.
 * The exact same-target body at 0x801F12B8 declares the same pair with this
 * spelling.
 * @source 0x80149320 @kind data
 */
extern s16 D_80149320;

/* @source 0x80149322 @kind data */
extern s16 D_80149322;

/* Halfwords this body writes with the cell cursor of the finished map.
 * @source 0x80149324 @kind data
 */
extern s16 D_80149324;

/* @source 0x80149326 @kind data */
extern s16 D_80149326;

/* Halfwords of the shared render-cell state this body clears on entry.
 * @source 0x8014932E @kind data
 */
extern u16 D_8014932E;

/* @source 0x80149330 @kind data */
extern u16 D_80149330;

/* Shared mode byte the map handlers publish.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Map-ready flag this body raises once the cell grid is built.
 * @source 0x80149334 @kind data
 */
extern s16 D_80149334;

/* Halfword this body clears beside the pending scroll amount.
 * @source 0x80149336 @kind data
 */
extern s16 D_80149336;

/* Pending vertical scroll of the shared render-cell state, drained here in 0x200
 * steps through the two scroll helpers. It is compared as a signed halfword but
 * the steps are written from its unsigned load, so it stays halfword-typed.
 * @source 0x80149338 @kind data
 */
extern s16 D_80149338;

/* @source 0x8014933D @kind data */
extern u8 D_8014933D;

/* @source 0x80149340 @kind data */
extern u8 D_80149340;

/* Map-header halfword of the main executable: a nonzero value is taken as the
 * whole pending scroll amount, so the value itself is what this body reads.
 * @source 0x8010401E @kind data
 */
extern u16 D_8010401E;

/* Render-cell grid: 0x38 rows of 0x1C four-byte cells, each cell holding the two
 * coordinate bytes the mapped-position helper writes at offset 0 and a halfword
 * this body clears at offset 2.
 * @source 0x8012C000 @kind unknown
 */
typedef struct BmagicMagic06403RenderCell {
  u8 x;
  u8 y;
  s16 unk_02;
} BmagicMagic06403RenderCell;

extern BmagicMagic06403RenderCell D_8012C000[0x38][0x1C];

/* Halfword at offset 0 of both 0x28-byte entries of every 0x50-byte record of
 * the shared record block, cleared here. The original materialises this base and
 * its partner below separately, so each is declared at its own address.
 * @source 0x8012D89E @kind unknown
 */
extern u8 D_8012D89E[];

/* Halfword at offset 8 of those same entries, cleared here.
 * @source 0x8012D8A6 @kind unknown
 */
extern u8 D_8012D8A6[];

/* Tail halfword of the reverse cell index: this body fills 0x400 halfwords from
 * this address downwards with 0x3FF down to 0.
 * @source 0x80142CBE @kind unknown
 */
extern u8 D_80142CBE[];

/* Shared scroll helpers of the main executable, called once per 0x200 step of
 * the pending scroll amount. Only their addresses are established here.
 * @source 0x80152FB0
 */
extern void func_80152FB0(void);

/* @source 0x80152E94 */
extern void func_80152E94(void);

/* Shared helpers of the main executable this body calls once the cell grid is
 * built. Their roles are not established.
 * @source 0x80154438
 */
extern void func_80154438(void);

/* @source 0x80157F98 */
extern void func_80157F98(void);

extern void writeDiagonalOffsetMapCellCoordinatesOrZeroBmagicMagic06403_801F12B8(s32 diagonal, s32 offset, u8 *out);

/* @source 0x801F103C
 * @behavior Builds the overlay's render-cell map from the shared view-origin
 * mirror pair. Clears the halfwords at offsets 0 and 8 of both 0x28-byte entries
 * of every 0x50-byte record of the 0x400-record block anchored at 0x8012D89E and
 * 0x8012D8A6, then splits the 16.16 view coordinates 0x80149308 and 0x8014930C
 * into the shared scroll words 0x80149310 and 0x80149314 (0x7FFF minus and
 * 0x8000 minus their high bytes) and into the render-cell origin pair
 * 0x80149320 and 0x80149322 (their high halves minus 0x18 and plus 3). Seeds the
 * pending scroll halfword 0x80149338 with the map-header halfword at 0x8010401E
 * when that is nonzero, or with twice the negated view step 0x80149318 when it
 * is zero, and then drains it: it calls func_80152FB0 once per 0x200 taken off
 * the amount while the signed halfword stays at or above 0x200, and func_80152E94
 * once per 0x200 added back while it stays below -0x1FF. Clears the halfword at
 * offset 2 of every one of the 0x38 by 0x1C four-byte cells of the 0x8012C000
 * grid and rewrites each cell's coordinate bytes through the diagonal-offset
 * helper, fills the 0x400 halfwords from 0x80142CBE downwards with 0x3FF down to
 * 0, then sets the map-ready halfword 0x80149334 to 1, the cell cursor halfwords
 * 0x80149324 and 0x80149326 to 27 and 55 and the mode byte 0x80149333 to 3, and
 * calls the two shared helpers func_80154438 and func_80157F98. Takes no
 * arguments and returns nothing; the 0x30-byte frame keeps the return address at
 * 0x28 and six saved registers.
 *
 * Representation notes: the record-clearing walk keeps an explicit byte offset
 * and a byte offset inside the record instead of one multiplied index. The
 * multiplied spelling measured 118/161 instructions and left both record bases
 * materialised in a2 and a3, while the offset pair reproduces the original's
 * own lui/addend/store triple per halfword; taking the inner copy from the outer
 * offset in the inner loop start measured 135/159 and moving the inner step into
 * the loop tail measured 137/159 against 136/159 in the increment clause. The
 * cell walk indexes the 0x38 by 0x1C render-cell array rather than a byte base
 * plus two products, which measured 140/159 against 137/159, and it reproduces
 * the original's separate row-offset and column-offset addends. Seeding the
 * pending scroll through a local and an if statement measured 140/159; the
 * conditional expression measured 156/159. A volatile pending-scroll halfword
 * measured 129/159, a volatile store cast 149/159 and an array-element base for
 * that halfword 143/159 and 123/159, so all three were reverted.
 * @status partial
 * @match 98.11
 * @residual The first drain test of the pending scroll halfword. The original
 * stores the amount and then reloads it with lh before comparing it against
 * 0x200, while this body keeps the stored value in a register and sign-extends
 * it in place, so that one hunk measures three instructions against two and the
 * hunk is the only difference in the body. Everything else, including the whole
 * record walk, the view-origin split, the two drain loops, the cell grid and the
 * reverse index, matches instruction for instruction.
 */
void initializeRenderCellMapFromViewOriginThenBuildReverseCellIndexBmagicMagic06403_801F103C(void) {
  s32 i;
  s32 j;
  s32 offset;
  s32 sub;
  s32 x;
  s32 y;
  u8 *cell;
  s16 *order;

  for (i = 0, offset = 0; i < 0x400; i++) {
    for (j = 0, sub = offset; j < 2; j++) {
      *(s16 *)(D_8012D89E + sub) = 0;
      *(s16 *)(D_8012D8A6 + sub) = 0;
      sub += 0x28;
    }
    offset += 0x50;
  }
  x = D_80149308;
  y = D_8014930C;
  D_8014933D = 0;
  D_80149310 = 0x7FFF - (x >> 8);
  D_80149314 = 0x8000 - (y >> 8);
  D_80149320 = (x >> 16) - 0x18;
  D_80149322 = (y >> 16) + 3;
  D_80149338 = D_8010401E != 0 ? D_8010401E : -D_80149318 * 2;
  D_80149336 = 0;
  while (D_80149338 >= 0x200) {
    func_80152FB0();
    D_80149338 -= 0x200;
  }
  while (D_80149338 < -0x1FF) {
    func_80152E94();
    D_80149338 += 0x200;
  }
  D_8014932E = 0;
  D_80149330 = 0;
  D_80149340 = 0;
  D_80149333 = 3;
  for (i = 0; i < 0x38; i++) {
    for (j = 0; j < 0x1C; j++) {
      cell = (u8 *)&D_8012C000[i][j];
      *(s16 *)(cell + 2) = 0;
      writeDiagonalOffsetMapCellCoordinatesOrZeroBmagicMagic06403_801F12B8(i, j, cell);
    }
  }
  for (i = 0x3FF, order = (s16 *)D_80142CBE; i >= 0; i--) {
    *order = i;
    order--;
  }
  D_80149334 = 1;
  D_80149324 = 0x1B;
  D_80149326 = 0x37;
  func_80154438();
  func_80157F98();
}
