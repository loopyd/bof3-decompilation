#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The original re-materialises the cell with its own
 * lui/lw pair before the dispatch index, again after the indirect call and a
 * third time after the update pass, because each intervening call may republish
 * the pointer, so every read stays on this named cell as in the exact bmagic
 * siblings
 * dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenWorkBytes0And2SetBmagicMagic09203_801EF840
 * and dispatchWorkByte2HandlerThenShiftMotionPairAndDrawWhenActiveBmagicMagic15903_801EEE28,
 * which read the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Word 11 of the fifteen-word code-pointer run based at 0x801F07BC, which Splat
 * places as the single textbin row T_801F07BC: 0x801F0320, 0x801F038C,
 * 0x801F03DC and 0x801F0420 follow it. The work record's byte 2 scales by four
 * and is added to 0x801F07E8, so the extern is declared at the indexed word and
 * the run stays that one map row; the generated undefined_syms_auto.txt row for
 * 0x801F07E8 already names this word. */
extern void (*D_801F07E8[])(void); /* @source 0x801F07E8 @kind data */

/* Shared main-executable update pass at 0x800AF764. The payload calls it once
 * per active step with no arguments and no result; the generated
 * undefined_funcs_auto.txt row for that address binds it. */
void func_800AF764(void);

/* This overlay's own translucent Gouraud fan draw pass at 0x801F04BC, lifted
 * exact as
 * drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairWithRandomRadiusTintedByWorkByte9TimesTenBmagicMagic09203_801F04BC:
 * it seeds a random radius from rand, selects a draw mode from GetGraphType,
 * installs it with SetDrawMode on the primitive cursor and emits eight
 * semi-transparent triangles through SetSemiTrans, reading the work record
 * halfwords 0x2E and 0x30 as the position pair. This body is its only in-image
 * caller, and it takes no arguments and returns nothing. */
void
drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairWithRandomRadiusTintedByWorkByte9TimesTenBmagicMagic09203_801F04BC(void);

/* @source 0x801F0260
 * @behavior Entry 6 of the fifteen non-zero words of the in-image
 *   handler-pointer run based at 0x801F07BC, which
 *   dispatchStateByte1Table07bcBmagicMagic09203_801EF000 indexes with scratchpad
 *   work byte 1 scaled by four, so no jal in this payload names this body. It
 *   reads the work record from the pointer cell at 0x1F800044, scales that
 *   record byte 2 by four and calls the selected word of the handler table at
 *   0x801F07E8 with no arguments and no index bound, then re-reads the cell
 *   because the handler may republish it. Only when the reloaded record bytes 0
 *   and 2 are both non-zero does the arm run: it calls the shared
 *   main-executable update pass func_800AF764, re-reads the cell a third time,
 *   moves the unsigned halfword at offset 0x2E by minus 0x10 when the record
 *   byte at offset 0xB equals 1 and by plus 0x10 otherwise, subtracts 0xC from
 *   the unsigned halfword at offset 0x30 and then calls this overlay's draw pass
 *   drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairWithRandomRadiusTintedByWorkByte9TimesTenBmagicMagic09203_801F04BC with the 0x30 store filling that call delay
 *   slot. The 0x18-byte
 *   frame keeps only the return address at 0x10 and the cell load precedes frame
 *   creation; the 0xC0-byte span takes no arguments and returns nothing through
 *   the jr $ra at 0x801F0318. Work bytes 0x0B and 0x00..0x04 and halfwords 0x2E
 *   and 0x30 stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeUpdateShiftHalfwords2EAnd30AndDrawWhenWorkBytes0And2SetBmagicMagic09203_801F0260(
    void) {
  u8 *stepWork;
  u8 *gateWork;
  u8 *shiftWork;
  u16 x;
  u16 y;

  stepWork = g_battle_work;
  D_801F07E8[stepWork[2]]();
  gateWork = g_battle_work;
  if (gateWork[0] != 0 && gateWork[2] != 0) {
    func_800AF764();
    shiftWork = g_battle_work;
    /* The original duplicates both halfword loads into each arm of the byte
     * 0xB test instead of hoisting them above the branch, which is why the
     * loads appear twice in its bytes; writing them inside both arms
     * reproduces that placement. */
    if (shiftWork[0x0B] == 1) {
      x = *(u16 *)(shiftWork + 0x2E) - 0x10;
      y = *(u16 *)(shiftWork + 0x30);
    } else {
      x = *(u16 *)(shiftWork + 0x2E) + 0x10;
      y = *(u16 *)(shiftWork + 0x30);
    }
    y -= 0xC;
    *(u16 *)(shiftWork + 0x2E) = x;
    *(u16 *)(shiftWork + 0x30) = y;
    drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairWithRandomRadiusTintedByWorkByte9TimesTenBmagicMagic09203_801F04BC();
  }
}
