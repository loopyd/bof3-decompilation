#include "bof3/bof3.h"

extern u8 D_801DB080[]; /* @source 0x801DB080 @kind data */
extern s8 D_801DB0B8;   /* @source 0x801DB0B8 @kind data */

extern void func_801D2CF8(s16 x, s16 y, u8 tile);

/* @source 0x801D4EA4
 * @behavior Draws the three card slots of one row through func_801D2CF8 at the
 *           same x, x + 0x20 and x + 0x40 left edges the sibling drawThreeCardRow
 *           uses, each slot 0x20 by 0x28. Unlike that sibling the tile id of
 *           slot i is byte i of the five-byte record that starts three bytes
 *           into the 0x801DB080 run selected by hand, so a whole run of three
 *           faces is drawn for the record (hand * 5 + 3) & 0xFF. The slot whose
 *           index equals the signed byte at 0x801DB0B8 is drawn eight pixels
 *           higher when raised is nonzero; every other slot, and every slot of a
 *           zero raised call, keeps row y. Nothing is written and no value is
 *           returned.
 * @status partial
 * @match 77.19
 * @residual The frame, the seven saved registers and the operation sequence
 *           match, but the size is four bytes short. The original narrows the
 *           raised row coordinate where it is defined and sign-extends it at the
 *           call, while this build folds both shifts onto the call. The unraised
 *           row copy, raised and the raised coordinate also hold each other's
 *           adjacent saved registers inside one allocator priority bucket, and
 *           the original's entry `addiu` on the incoming row argument, `move` of
 *           the unraised copy and loop counter store order differ from this
 *           build's.
 */
void drawThreeCardRowWithRaisedCard(s32 x, s32 y, u8 hand, u32 raised) {
    u8 base;
    s32 top;
    s16 yRaised;
    s32 i;

    yRaised = (s16)(y - 8);
    top = y;
    base = (u8)(hand * 5 + 3);
    for (i = 0; i < 3; i++) {
        if (D_801DB0B8 == i && (u8)raised != 0) {
            func_801D2CF8((s16)(x + i * 0x20), yRaised, D_801DB080[base + i]);
        } else {
            func_801D2CF8((s16)(x + i * 0x20), (s16)top, D_801DB080[base + i]);
        }
    }
}
